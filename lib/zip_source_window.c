/*
  zip_source_window.c -- return part of lower source
  Copyright (C) 2012-2021 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <libzip@nih.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <stdlib.h>
#include <string.h>

#include "zipint.h"

struct window_stream {
    zip_uint64_t offset; /* offset in src for next read */
};

typedef struct window_stream window_stream_t;

struct window {
    zip_uint64_t start; /* where in file we start reading */
    zip_uint64_t end;   /* where in file we stop reading */
    bool end_valid;     /* whether end is set, otherwise read until EOF */

    window_stream_t stream;

    zip_stat_t stat;
    zip_file_attributes_t attributes;
    zip_error_t error;
    zip_int64_t supports;
    bool needs_seek;
    bool needs_stream_seek;
};

static zip_int64_t window_read(zip_source_t *, zip_int64_t, void *, void *, zip_uint64_t, zip_source_cmd_t);
static bool _needs_seek(struct window *ctx, zip_int64_t stream_id);
static zip_int64_t window_read_open(zip_source_t *src, zip_int64_t stream_id, struct window *ctx, window_stream_t *stream);
static zip_int64_t window_read_read(zip_source_t *src, zip_int64_t stream_id, struct window *ctx, window_stream_t *stream, void *data, zip_uint64_t len);
static zip_int64_t window_read_seek(zip_source_t *src, zip_int64_t stream_id, struct window *ctx, window_stream_t *stream, void *data, zip_uint64_t len);
static zip_int64_t window_read_tell(struct window *ctx, window_stream_t *stream);


ZIP_EXTERN zip_source_t *
zip_source_window_create(zip_source_t *src, zip_uint64_t start, zip_int64_t len, zip_error_t *error) {
    return _zip_source_window_new(src, start, len, NULL, 0, NULL, 0, error);
}

zip_source_t *
_zip_source_window_new(zip_source_t *src, zip_uint64_t start, zip_int64_t length, zip_stat_t *st, zip_file_attributes_t *attributes, zip_t *source_archive, zip_uint64_t source_index, zip_error_t *error) {
    struct window *ctx;
    zip_uint64_t offset;

    if (src == NULL || length < -1 || (source_archive == NULL && source_index != 0)) {
        zip_error_set(error, ZIP_ER_INVAL, 0);
        return NULL;
    }

    if (length >= 0) {
        if (start + (zip_uint64_t)length < start) {
            zip_error_set(error, ZIP_ER_INVAL, 0);
            return NULL;
        }
    }

    offset = 0;
    if (source_archive) {
        if ((offset = _zip_file_get_offset(source_archive, source_index, error)) == 0) {
            return NULL;
        }
        if (length >= 0 && start + length + offset < start + length) {
            /* zip archive data claims end of data past zip64 limits */
            zip_error_set(error, ZIP_ER_INCONS, MAKE_DETAIL_WITH_INDEX(ZIP_ER_DETAIL_CDIR_ENTRY_INVALID, source_index));
            return NULL;
        }
    }

    if ((ctx = (struct window *)malloc(sizeof(*ctx))) == NULL) {
        zip_error_set(error, ZIP_ER_MEMORY, 0);
        return NULL;
    }

    ctx->start = start + offset;
    if (length == -1) {
        ctx->end_valid = false;
    }
    else {
        ctx->end = start + offset + (zip_uint64_t)length;
        ctx->end_valid = true;
    }
    zip_stat_init(&ctx->stat);
    if (attributes != NULL) {
        memcpy(&ctx->attributes, attributes, sizeof(ctx->attributes));
    }
    else {
        zip_file_attributes_init(&ctx->attributes);
    }
    zip_error_init(&ctx->error);
    ctx->supports = (zip_source_supports(src) & (ZIP_SOURCE_SUPPORTS_SEEKABLE | ZIP_SOURCE_SUPPORTS_REOPEN)) | (zip_source_make_command_bitmap(ZIP_SOURCE_GET_FILE_ATTRIBUTES, ZIP_SOURCE_SUPPORTS, ZIP_SOURCE_TELL, -1));
    ctx->needs_seek = (ctx->supports & ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_SEEK)) ? true : false;
    if (zip_source_supports_multi_open_seekable(src)) {
        ctx->supports |= ZIP_SOURCE_SUPPORTS_SEEKABLE_STREAMS;
        ctx->needs_stream_seek = true;
    } else {
        if (zip_source_supports_multi_open_readable(src))
            ctx->supports |= ZIP_SOURCE_SUPPORTS_READABLE_STREAMS;
        ctx->needs_stream_seek = false;
    }

    if (st) {
        if (_zip_stat_merge(&ctx->stat, st, error) < 0) {
            free(ctx);
            return NULL;
        }
    }
    
    return zip_source_layered_create(src, window_read, ctx, error);
}


int
_zip_source_set_source_archive(zip_source_t *src, zip_t *za) {
    src->source_archive = za;
    return _zip_register_source(za, src);
}


/* called by zip_discard to avoid operating on file from closed archive */
void
_zip_source_invalidate(zip_source_t *src) {
    src->source_closed = 1;

    if (zip_error_code_zip(&src->error) == ZIP_ER_OK) {
        zip_error_set(&src->error, ZIP_ER_ZIPCLOSED, 0);
    }
}

static zip_int64_t
window_read(zip_source_t *src, zip_int64_t stream_id, void *_ctx, void *data, zip_uint64_t len, zip_source_cmd_t cmd) {
    struct window *ctx;
    zip_int64_t ret;
    zip_uint64_t n, i;

    ctx = (struct window *)_ctx;

    switch (cmd) {
    case ZIP_SOURCE_CLOSE:
        return 0;

    case ZIP_SOURCE_ERROR:
        return zip_error_to_data(&ctx->error, data, len);

    case ZIP_SOURCE_FREE:
        free(ctx);
        return 0;

    case ZIP_SOURCE_OPEN:
        return window_read_open(src, -1, ctx, &ctx->stream);

    case ZIP_SOURCE_READ:
        return window_read_read(src, -1, ctx, &ctx->stream, data, len);

    case ZIP_SOURCE_SEEK:
        return window_read_seek(src, -1, ctx, &ctx->stream, data, len);

    case ZIP_SOURCE_STAT: {
        zip_stat_t *st;

        st = (zip_stat_t *)data;

        if (_zip_stat_merge(st, &ctx->stat, &ctx->error) < 0) {
            return -1;
        }
        return 0;
    }

    case ZIP_SOURCE_GET_FILE_ATTRIBUTES:
        if (len < sizeof(ctx->attributes)) {
            zip_error_set(&ctx->error, ZIP_ER_INVAL, 0);
            return -1;
        }

        memcpy(data, &ctx->attributes, sizeof(ctx->attributes));
        return sizeof(ctx->attributes);

    case ZIP_SOURCE_SUPPORTS:
        return ctx->supports;

    case ZIP_SOURCE_TELL:
        return window_read_tell(ctx, &ctx->stream);

    case ZIP_SOURCE_OPEN_STREAM: {
        zip_source_args_stream_t *args;
        window_stream_t *stream;

        args = ZIP_SOURCE_GET_ARGS(zip_source_args_stream_t, data, len, &ctx->error);
        if (args == NULL)
            return -1;

        stream = (window_stream_t *)malloc(sizeof(*stream));
        if (!stream) {
            zip_error_set(&ctx->error, ZIP_ER_MEMORY, 0);
            return -1;
        }
        if (window_read_open(src, stream_id, ctx, stream) < 0) {
            free(stream);
            return -1;
        }
        args->user_stream = stream;
        return 0;
    }

    case ZIP_SOURCE_CLOSE_STREAM: {
        zip_source_args_stream_t *args;

        args = ZIP_SOURCE_GET_ARGS(zip_source_args_stream_t, data, len, &ctx->error);
        if (args == NULL)
            return -1;

        free(args->user_stream);
        return 0;
    }

    case ZIP_SOURCE_READ_STREAM: {
        zip_source_args_stream_t *args;
        window_stream_t *stream;

        args = ZIP_SOURCE_GET_ARGS(zip_source_args_stream_t, data, len, &ctx->error);
        if (args == NULL)
            return -1;

        stream = (window_stream_t *)args->user_stream;

        return window_read_read(src, stream_id, ctx, stream, data, len);
    }

    case ZIP_SOURCE_SEEK_STREAM: {
        zip_source_args_stream_t *args;
        window_stream_t *stream;

        args = ZIP_SOURCE_GET_ARGS(zip_source_args_stream_t, data, len, &ctx->error);
        if (args == NULL)
            return -1;

        stream = (window_stream_t *)args->user_stream;

        return window_read_seek(src, stream_id, ctx, stream, data, len);
    }

    case ZIP_SOURCE_TELL_STREAM: {
        zip_source_args_stream_t *args;
        window_stream_t *stream;

        args = ZIP_SOURCE_GET_ARGS(zip_source_args_stream_t, data, len, &ctx->error);
        if (args == NULL)
            return -1;

        stream = (window_stream_t *)args->user_stream;
        return window_read_tell(ctx, stream);
    }

    default:
        zip_error_set(&ctx->error, ZIP_ER_OPNOTSUPP, 0);
        return -1;
    }
}


void
_zip_deregister_source(zip_t *za, zip_source_t *src) {
    unsigned int i;

    for (i = 0; i < za->nopen_source; i++) {
        if (za->open_source[i] == src) {
            za->open_source[i] = za->open_source[za->nopen_source - 1];
            za->nopen_source--;
            break;
        }
    }
}


int
_zip_register_source(zip_t *za, zip_source_t *src) {
    zip_source_t **open_source;

    if (za->nopen_source + 1 >= za->nopen_source_alloc) {
        unsigned int n;
        n = za->nopen_source_alloc + 10;
        open_source = (zip_source_t **)realloc(za->open_source, n * sizeof(zip_source_t *));
        if (open_source == NULL) {
            zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
            return -1;
        }
        za->nopen_source_alloc = n;
        za->open_source = open_source;
    }

    za->open_source[za->nopen_source++] = src;

    return 0;
}

static bool
_needs_seek(struct window *ctx, zip_int64_t stream_id) {
  if (stream_id < 0)
      return ctx->needs_seek;

  return ctx->needs_stream_seek;
}

static zip_int64_t
window_read_open(zip_source_t *src, zip_int64_t stream_id, struct window *ctx, window_stream_t *stream) {
    if (!_needs_seek(ctx, stream_id)) {
        DEFINE_BYTE_ARRAY(b, BUFSIZE);
        zip_uint64_t n, i;
        zip_int64_t ret;

        if (!byte_array_init(b, BUFSIZE)) {
            zip_error_set(&ctx->error, ZIP_ER_MEMORY, 0);
            return -1;
        }

        for (n = 0; n < ctx->start; n += (zip_uint64_t)ret) {
            i = (ctx->start - n > BUFSIZE ? BUFSIZE : ctx->start - n);
            ret = _zip_source_read(src, stream_id, b, i);

            if (ret < 0) {
                _zip_error_set_from_source(&ctx->error, src);
                byte_array_fini(b);
                return -1;
            }
            if (ret == 0) {
                zip_error_set(&ctx->error, ZIP_ER_EOF, 0);
                byte_array_fini(b);
                return -1;
            }
        }

        byte_array_fini(b);
    }

    stream->offset = ctx->start;
    return 0;
}

static zip_int64_t
window_read_read(zip_source_t *src, zip_int64_t stream_id, struct window *ctx, window_stream_t *stream, void *data, zip_uint64_t len) {
    zip_int64_t ret;

    if (ctx->end_valid && len > ctx->end - stream->offset) {
        len = ctx->end - stream->offset;
    }

    if (len == 0) {
        return 0;
    }

    if (_needs_seek(ctx, stream_id)) {
        if (_zip_source_seek(src, stream_id, (zip_int64_t)stream->offset, SEEK_SET) < 0) {
            _zip_error_set_from_source(&ctx->error, src);
            return -1;
        }
    }

    if ((ret = _zip_source_read(src, stream_id, data, len)) < 0) {
        zip_error_set(&ctx->error, ZIP_ER_EOF, 0);
        return -1;
    }

    stream->offset += (zip_uint64_t)ret;

    if (ret == 0) {
        if (ctx->end_valid && stream->offset < ctx->end) {
            zip_error_set(&ctx->error, ZIP_ER_EOF, 0);
            return -1;
        }
    }
    return ret;
}

static zip_int64_t
window_read_seek(zip_source_t *src, zip_int64_t stream_id, struct window *ctx, window_stream_t *stream, void *data, zip_uint64_t len) {
    zip_int64_t new_offset;

    if (!ctx->end_valid) {
        zip_source_args_seek_t *args = ZIP_SOURCE_GET_ARGS(zip_source_args_seek_t, data, len, &ctx->error);

        if (args == NULL) {
            return -1;
        }
        if (args->whence == SEEK_END) {
            if (_zip_source_seek(src, stream_id, args->offset, args->whence) < 0) {
                _zip_error_set_from_source(&ctx->error, src);
                return -1;
            }
            new_offset = _zip_source_tell(src, stream_id);
            if (new_offset < 0) {
                _zip_error_set_from_source(&ctx->error, src);
                return -1;
            }
            if ((zip_uint64_t)new_offset < ctx->start) {
                zip_error_set(&ctx->error, ZIP_ER_INVAL, 0);
                (void)_zip_source_seek(src, stream_id, (zip_int64_t)stream->offset, SEEK_SET);
                return -1;
            }
            stream->offset = (zip_uint64_t)new_offset;
            return 0;
        }
    }

    new_offset = zip_source_seek_compute_offset(stream->offset - ctx->start, ctx->end - ctx->start, data, len, &ctx->error);

    if (new_offset < 0) {
        return -1;
    }

    stream->offset = (zip_uint64_t)new_offset + ctx->start;
    return 0;
}

static zip_int64_t
window_read_tell(struct window *ctx, window_stream_t *stream) {
    return (zip_int64_t)(stream->offset - ctx->start);
}
