/*
  zip_source_close.c -- close zip_source (stop reading)
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

zip_int64_t
_zip_source_close(zip_source_t *src, zip_int64_t stream_id) {
  if (stream_id < 0)
    return zip_source_close(src);

  return zip_source_close_stream(src, stream_id);
}

ZIP_EXTERN int
zip_source_close_stream(zip_source_t *src, zip_int64_t stream_id) {
    zip_stream_t *stream;
    void *user_stream;
    zip_int64_t parent_stream_id;
    zip_source_args_stream_t args;
    bool failed;

    if (!ZIP_SOURCE_IS_VALID_STREAM_ID(src, stream_id)) {
        zip_error_set(&src->error, ZIP_ER_INVAL, 0);
        return -1;
    }
    stream = src->streams[stream_id];
    user_stream = stream->user_stream;
    parent_stream_id = stream->parent_stream_id;
    free(stream);
    src->streams[stream_id] = NULL;
    --src->nstreams;

    args.user_stream = user_stream;
    args.data = NULL;
    args.len = 0;

    failed = false;
    if (_zip_source_call(src, parent_stream_id, &args, sizeof(args), ZIP_SOURCE_CLOSE_STREAM) < 0)
        failed = true;
    if (ZIP_SOURCE_IS_LAYERED(src)) {
        if (zip_source_close_stream(src->src, stream->parent_stream_id) < 0) {
            if (!failed) {
                zip_error_set(&src->error, ZIP_ER_INTERNAL, 0);
                failed = true;
            }
        }
    }

    if (failed)
        return -1;

    if (stream_id == src->nstreams)
        return 0;

    /* record free stream ID if we made a hole in streams array */
    if (src->nfree_stream_ids + 1 > src->nfree_stream_ids_alloced) {
        int err;
        if ((err = _zip_realloc((void**)&src->free_stream_ids, &src->nfree_stream_ids_alloced, sizeof(*src->free_stream_ids), src->nfree_stream_ids_alloced / 2)) != ZIP_ER_OK) {
            zip_error_set(&src->error, err, 0);
            return -1;
        }
    }
    src->free_stream_ids[src->nfree_stream_ids] = stream_id;
    ++src->nfree_stream_ids;
}

ZIP_EXTERN int
zip_source_close(zip_source_t *src) {
    if (!ZIP_SOURCE_IS_OPEN_READING(src)) {
        zip_error_set(&src->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    src->open_count--;
    if (src->open_count == 0) {
        _zip_source_call(src, -1, NULL, 0, ZIP_SOURCE_CLOSE);

        if (ZIP_SOURCE_IS_LAYERED(src)) {
            if (zip_source_close(src->src) < 0) {
                zip_error_set(&src->error, ZIP_ER_INTERNAL, 0);
                return -1;
            }
        }
    }

    return 0;
}
