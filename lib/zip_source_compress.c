/*
  zip_source_compress.c -- (de)compression routines
  Copyright (C) 2017-2021 Dieter Baron and Thomas Klausner

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

struct stream {
    bool end_of_input;
    bool end_of_stream;
    bool can_store;
    bool is_stored; /* only valid if end_of_stream is true */

    zip_uint64_t size;
    zip_int64_t first_read;
    zip_uint8_t buffer[BUFSIZE];

    void *ud;
};


struct context {
    zip_error_t error;
    bool compress;
    zip_int32_t method;
    int compression_flags;
    zip_compression_algorithm_t *algorithm;

    struct stream stream;
};


struct implementation {
    zip_uint16_t method;
    zip_compression_algorithm_t *compress;
    zip_compression_algorithm_t *decompress;
};

static struct implementation implementations[] = {
    {ZIP_CM_DEFLATE, &zip_algorithm_deflate_compress, &zip_algorithm_deflate_decompress},
#if defined(HAVE_LIBBZ2)
    {ZIP_CM_BZIP2, &zip_algorithm_bzip2_compress, &zip_algorithm_bzip2_decompress},
#endif
#if defined(HAVE_LIBLZMA)
    {ZIP_CM_LZMA, &zip_algorithm_xz_compress, &zip_algorithm_xz_decompress},
    /*  Disabled - because 7z isn't able to unpack ZIP+LZMA2
        archives made this way - and vice versa.

        {ZIP_CM_LZMA2, &zip_algorithm_xz_compress, &zip_algorithm_xz_decompress},
    */
    {ZIP_CM_XZ, &zip_algorithm_xz_compress, &zip_algorithm_xz_decompress},
#endif
#if defined(HAVE_LIBZSTD)
    {ZIP_CM_ZSTD, &zip_algorithm_zstd_compress, &zip_algorithm_zstd_decompress},
#endif

};

static size_t implementations_size = sizeof(implementations) / sizeof(implementations[0]);

static zip_source_t *compression_source_new(zip_t *za, zip_source_t *src, zip_int32_t method, bool compress, int compression_flags);
static zip_int64_t compress_callback(zip_source_t *, zip_int64_t, void *, void *, zip_uint64_t, zip_source_cmd_t);
static void context_free(struct context *ctx);
static struct context *context_new(zip_int32_t method, bool compress, int compression_flags, zip_compression_algorithm_t *algorithm);
static zip_int64_t compress_open(zip_source_t *, struct context *, struct stream *);
static zip_int64_t compress_read(zip_source_t *, zip_int64_t, struct context *, struct stream *, void *, zip_uint64_t);
static zip_int64_t compress_close(struct context *, struct stream *);
static bool stream_init(struct context *, struct stream *);
static void stream_fini(struct context *, struct stream *);

zip_compression_algorithm_t *
_zip_get_compression_algorithm(zip_int32_t method, bool compress) {
    size_t i;
    zip_uint16_t real_method = ZIP_CM_ACTUAL(method);

    for (i = 0; i < implementations_size; i++) {
        if (implementations[i].method == real_method) {
            if (compress) {
                return implementations[i].compress;
            }
            else {
                return implementations[i].decompress;
            }
        }
    }

    return NULL;
}

ZIP_EXTERN int
zip_compression_method_supported(zip_int32_t method, int compress) {
    if (method == ZIP_CM_STORE) {
        return 1;
    }
    return _zip_get_compression_algorithm(method, compress) != NULL;
}

zip_source_t *
zip_source_compress(zip_t *za, zip_source_t *src, zip_int32_t method, int compression_flags) {
    return compression_source_new(za, src, method, true, compression_flags);
}

zip_source_t *
zip_source_decompress(zip_t *za, zip_source_t *src, zip_int32_t method) {
    return compression_source_new(za, src, method, false, 0);
}


static zip_source_t *
compression_source_new(zip_t *za, zip_source_t *src, zip_int32_t method, bool compress, int compression_flags) {
    struct context *ctx;
    zip_source_t *s2;
    zip_compression_algorithm_t *algorithm = NULL;

    if (src == NULL) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return NULL;
    }

    if ((algorithm = _zip_get_compression_algorithm(method, compress)) == NULL) {
        zip_error_set(&za->error, ZIP_ER_COMPNOTSUPP, 0);
        return NULL;
    }

    if ((ctx = context_new(method, compress, compression_flags, algorithm)) == NULL) {
        zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
        return NULL;
    }

    if ((s2 = zip_source_layered(za, src, compress_callback, ctx)) == NULL) {
        context_free(ctx);
        return NULL;
    }

    return s2;
}


static struct context *
context_new(zip_int32_t method, bool compress, int compression_flags, zip_compression_algorithm_t *algorithm) {
    struct context *ctx;

    if ((ctx = (struct context *)malloc(sizeof(*ctx))) == NULL) {
        return NULL;
    }
    zip_error_init(&ctx->error);
    ctx->algorithm = algorithm;
    ctx->method = method;
    ctx->compress = compress;
    ctx->compression_flags = compression_flags;

    if (!stream_init(ctx, &ctx->stream)) {
        zip_error_fini(&ctx->error);
        free(ctx);
        return NULL;
    }

    return ctx;
}


static void
context_free(struct context *ctx) {
    if (ctx == NULL) {
        return;
    }

    stream_fini(ctx, &ctx->stream);
    zip_error_fini(&ctx->error);

    free(ctx);
}


static zip_int64_t
compress_open(zip_source_t *src, struct context *ctx, struct stream *stream) {
    zip_stat_t st;
    zip_file_attributes_t attributes;

    stream->end_of_input = false;
    stream->end_of_stream = false;
    stream->is_stored = false;
    stream->size = 0;
    stream->first_read = -1;
    if (zip_source_stat(src, &st) < 0 || zip_source_get_file_attributes(src, &attributes) < 0) {
        _zip_error_set_from_source(&ctx->error, src);
        return -1;
    }

    if (!ctx->algorithm->start(stream->ud, &st, &attributes)) {
        return -1;
    }

    return 0;
}


static zip_int64_t
compress_read(zip_source_t *src, zip_int64_t parent_stream_id, struct context *ctx, struct stream *stream, void *data, zip_uint64_t len) {
    zip_compression_status_t ret;
    bool end;
    zip_int64_t n;
    zip_uint64_t out_offset;
    zip_uint64_t out_len;

    if (zip_error_code_zip(&ctx->error) != ZIP_ER_OK) {
        return -1;
    }

    if (len == 0 || stream->end_of_stream) {
        return 0;
    }

    out_offset = 0;

    end = false;
    while (!end && out_offset < len) {
        out_len = len - out_offset;
        ret = ctx->algorithm->process(stream->ud, (zip_uint8_t *)data + out_offset, &out_len);

        if (ret != ZIP_COMPRESSION_ERROR) {
            out_offset += out_len;
        }

        switch (ret) {
        case ZIP_COMPRESSION_END:
            stream->end_of_stream = true;

            if (!stream->end_of_input) {
                /* TODO: garbage after stream, or compression ended before all data read */
            }

            if (stream->first_read < 0) {
                /* we got end of processed stream before reading any input data */
                zip_error_set(&ctx->error, ZIP_ER_INTERNAL, 0);
                end = true;
                break;
            }
            if (stream->can_store && (zip_uint64_t)stream->first_read <= out_offset) {
                stream->is_stored = true;
                stream->size = (zip_uint64_t)stream->first_read;
                memcpy(data, stream->buffer, stream->size);
                return (zip_int64_t)stream->size;
            }
            end = true;
            break;

        case ZIP_COMPRESSION_OK:
            break;

        case ZIP_COMPRESSION_NEED_DATA:
            if (stream->end_of_input) {
                /* TODO: error: stream not ended, but no more input */
                end = true;
                break;
            }

            if ((n = _zip_source_read(src, parent_stream_id, stream->buffer, sizeof(stream->buffer))) < 0) {
                _zip_error_set_from_source(&ctx->error, src);
                end = true;
                break;
            }
            else if (n == 0) {
                stream->end_of_input = true;
                ctx->algorithm->end_of_input(stream->ud);
                if (stream->first_read < 0) {
                    stream->first_read = 0;
                }
            }
            else {
                if (stream->first_read >= 0) {
                    /* we overwrote a previously filled stream->buffer */
                    stream->can_store = false;
                }
                else {
                    stream->first_read = n;
                }

                ctx->algorithm->input(stream->ud, stream->buffer, (zip_uint64_t)n);
            }
            break;

        case ZIP_COMPRESSION_ERROR:
            /* error set by algorithm */
            if (zip_error_code_zip(&ctx->error) == ZIP_ER_OK) {
                zip_error_set(&ctx->error, ZIP_ER_INTERNAL, 0);
            }
            end = true;
            break;
        }
    }

    if (out_offset > 0) {
        stream->can_store = false;
        stream->size += out_offset;
        return (zip_int64_t)out_offset;
    }

    return (zip_error_code_zip(&ctx->error) == ZIP_ER_OK) ? 0 : -1;
}


static zip_int64_t
compress_close(struct context *ctx, struct stream *stream) {
    if (!ctx->algorithm->end(stream->ud)) {
        return -1;
    }
    return 0;
}


static zip_int64_t
compress_callback(zip_source_t *src, zip_int64_t stream_id, void *ud, void *data, zip_uint64_t len, zip_source_cmd_t cmd) {
    struct context *ctx;

    ctx = (struct context *)ud;

    switch (cmd) {
    case ZIP_SOURCE_OPEN:
        return compress_open(src, ctx, &ctx->stream);

    case ZIP_SOURCE_READ:
        return compress_read(src, -1, ctx, &ctx->stream, data, len);

    case ZIP_SOURCE_CLOSE:
        return compress_close(ctx, &ctx->stream);

    case ZIP_SOURCE_STAT: {
        zip_stat_t *st;

        st = (zip_stat_t *)data;

        if (ctx->compress) {
            if (ctx->stream.end_of_stream) {
                st->comp_method = ctx->stream.is_stored ? ZIP_CM_STORE : ZIP_CM_ACTUAL(ctx->method);
                st->comp_size = ctx->stream.size;
                st->valid |= ZIP_STAT_COMP_SIZE | ZIP_STAT_COMP_METHOD;
            }
            else {
                st->valid &= ~(ZIP_STAT_COMP_SIZE | ZIP_STAT_COMP_METHOD);
            }
        }
        else {
            st->comp_method = ZIP_CM_STORE;
            st->valid |= ZIP_STAT_COMP_METHOD;
            if (ctx->stream.end_of_stream) {
                st->size = ctx->stream.size;
                st->valid |= ZIP_STAT_SIZE;
            }
        }
        return 0;
    }

    case ZIP_SOURCE_ERROR:
        return zip_error_to_data(&ctx->error, data, len);

    case ZIP_SOURCE_FREE:
        context_free(ctx);
        return 0;

    case ZIP_SOURCE_GET_FILE_ATTRIBUTES: {
        zip_file_attributes_t *attributes = (zip_file_attributes_t *)data;

        if (len < sizeof(*attributes)) {
            zip_error_set(&ctx->error, ZIP_ER_INVAL, 0);
            return -1;
        }

        attributes->valid |= ZIP_FILE_ATTRIBUTES_VERSION_NEEDED | ZIP_FILE_ATTRIBUTES_GENERAL_PURPOSE_BIT_FLAGS;
        attributes->version_needed = ctx->algorithm->version_needed;
        attributes->general_purpose_bit_mask = ZIP_FILE_ATTRIBUTES_GENERAL_PURPOSE_BIT_FLAGS_ALLOWED_MASK;
        attributes->general_purpose_bit_flags = (ctx->stream.is_stored ? 0 : ctx->algorithm->general_purpose_bit_flags(ctx->stream.ud));

        return sizeof(*attributes);
    }

    case ZIP_SOURCE_SUPPORTS:
        return ZIP_SOURCE_SUPPORTS_READABLE | zip_source_make_command_bitmap(ZIP_SOURCE_GET_FILE_ATTRIBUTES, ZIP_SOURCE_SUPPORTS_REOPEN, -1) | ZIP_SOURCE_SUPPORTS_READABLE_STREAMS;

    case ZIP_SOURCE_OPEN_STREAM: {
        zip_source_args_stream_t *args;
        struct stream *stream;

        args = ZIP_SOURCE_GET_ARGS(zip_source_args_stream_t, data, len, &ctx->error);
        if (args == NULL)
            return -1;

        stream = (struct stream *)malloc(sizeof(*stream));
        if (!stream_init(ctx, stream)) {
            free(stream);
            return -1;
        }
        if (compress_open(src, ctx, stream) < 0) {
            stream_fini(ctx, stream);
            free(stream);
            return -1;
        }
        args->user_stream = stream;
        return 0;
    }

    case ZIP_SOURCE_READ_STREAM: {
        zip_source_args_stream_t *args;
        struct stream *stream;

        args = ZIP_SOURCE_GET_ARGS(zip_source_args_stream_t, data, len, &ctx->error);
        if (args == NULL)
            return -1;

        stream = (struct stream *)args->user_stream;
        return compress_read(src, -1, ctx, stream, data, len);
    }

    case ZIP_SOURCE_CLOSE_STREAM: {
        zip_source_args_stream_t *args;
        struct stream *stream;
        zip_int64_t result;

        args = ZIP_SOURCE_GET_ARGS(zip_source_args_stream_t, data, len, &ctx->error);
        if (args == NULL)
            return -1;

        stream = (struct stream *)args->user_stream;
        result = compress_close(ctx, stream);
        free(stream);
        return result;
    }

    default:
        zip_error_set(&ctx->error, ZIP_ER_INTERNAL, 0);
        return -1;
    }
}


static bool
stream_init(struct context *ctx, struct stream *stream) {
    stream->can_store = ctx->compress ? ZIP_CM_IS_DEFAULT(ctx->method) : false;

    if ((stream->ud = ctx->algorithm->allocate(ZIP_CM_ACTUAL(ctx->method), ctx->compression_flags, &ctx->error)) == NULL) {
        return false;
    }

    return true;
}


static void
stream_fini(struct context *ctx, struct stream *stream) {
    ctx->algorithm->deallocate(stream->ud);
}
