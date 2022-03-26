/*
  zip_source_open.c -- open zip_source (prepare for reading)
  Copyright (C) 2009-2021 Dieter Baron and Thomas Klausner

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

#include "zipint.h"

ZIP_EXTERN zip_int64_t
zip_source_open_stream(zip_source_t *src) {
    zip_int64_t parent_stream_id;
    zip_source_args_stream_t args;
    zip_uint64_t stream_id;
    zip_stream_t* stream;

    if (src->source_closed) {
        return -1;
    }
    if (src->write_state == ZIP_SOURCE_WRITE_REMOVED) {
        zip_error_set(&src->error, ZIP_ER_DELETED, 0);
        return -1;
    }
    parent_stream_id = -1;
    if (ZIP_SOURCE_IS_LAYERED(src)) {
        if ((parent_stream_id = zip_source_open_stream(src->src)) < 0) {
            _zip_error_set_from_source(&src->error, src->src);
            return -1;
        }
    }

    args.user_stream = NULL;
    args.data = NULL;
    args.len = 0;

    if (_zip_source_call(src, parent_stream_id, &args, sizeof(args), ZIP_SOURCE_OPEN_STREAM) < 0) {
        if (ZIP_SOURCE_IS_LAYERED(src)) {
            zip_source_close_stream(src->src, parent_stream_id);
        }
        return -1;
    }

    if (args.user_stream == NULL) {
        if (ZIP_SOURCE_IS_LAYERED(src))
            zip_source_close_stream(src->src, parent_stream_id);
        zip_error_set(&src->error, ZIP_ER_OPEN, 0);
        return -1;
    }

    if (src->nfree_stream_ids > 0) {
        src->nfree_stream_ids--;
        stream_id = src->free_stream_ids[src->nfree_stream_ids];
    }
    else {
        if (src->nstreams + 1 > src->nstreams_alloced) {
            int err;
            if ((err = _zip_realloc((void**)&src->streams, &src->nstreams_alloced, sizeof(*src->streams), src->nstreams_alloced / 2)) != ZIP_ER_OK) {
                _zip_source_call(src, parent_stream_id, &args, sizeof(args), ZIP_SOURCE_CLOSE_STREAM);
                if (ZIP_SOURCE_IS_LAYERED(src))
                    zip_source_close_stream(src->src, parent_stream_id);
                zip_error_set(&src->error, err, 0);
                return -1;
            }
        }
        stream_id = src->nstreams;
        ++src->nstreams;
    }

    stream = (zip_stream_t*)malloc(sizeof(*stream));
    if (stream == NULL) {
        --src->nstreams;
        _zip_source_call(src, parent_stream_id, &args, sizeof(args), ZIP_SOURCE_CLOSE_STREAM);
        if (ZIP_SOURCE_IS_LAYERED(src))
            zip_source_close_stream(src->src, parent_stream_id);
        zip_error_set(&src->error, ZIP_ER_MEMORY, 0);
        return -1;
    }
    stream->parent_stream_id = parent_stream_id;
    stream->eof = false;
    stream->had_read_error = false;
    stream->bytes_read = 0;
    stream->user_stream = args.user_stream;

    src->streams[stream_id] = stream;

    return stream_id;
}

ZIP_EXTERN int
zip_source_open(zip_source_t *src) {
    if (src->source_closed) {
        return -1;
    }
    if (src->write_state == ZIP_SOURCE_WRITE_REMOVED) {
        zip_error_set(&src->error, ZIP_ER_DELETED, 0);
        return -1;
    }

    if (ZIP_SOURCE_IS_OPEN_READING(src)) {
        if ((zip_source_supports(src) & ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_SEEK)) == 0) {
            zip_error_set(&src->error, ZIP_ER_INUSE, 0);
            return -1;
        }
    }
    else {
        if (ZIP_SOURCE_IS_LAYERED(src)) {
            if (zip_source_open(src->src) < 0) {
                _zip_error_set_from_source(&src->error, src->src);
                return -1;
            }
        }

        if (_zip_source_call(src, -1, NULL, 0, ZIP_SOURCE_OPEN) < 0) {
            if (ZIP_SOURCE_IS_LAYERED(src)) {
                zip_source_close(src->src);
            }
            return -1;
        }
    }

    src->eof = false;
    src->had_read_error = false;
    _zip_error_clear(&src->error);
    src->bytes_read = 0;
    src->open_count++;

    return 0;
}
