/*
  zip_source_tell.c -- report current offset
  Copyright (C) 2014-2021 Dieter Baron and Thomas Klausner

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


#include "zipint.h"


zip_int64_t
_zip_source_tell(zip_source_t *src, zip_int64_t stream_id) {
  if (stream_id < 0)
    return zip_source_tell(src);

  return zip_source_tell_stream(src, stream_id);
}

ZIP_EXTERN zip_int64_t
zip_source_tell_stream(zip_source_t *src, zip_int64_t stream_id) {
    zip_stream_t* stream;
    zip_source_args_stream_t args;

    if (src->source_closed) {
        return -1;
    }
    if (!ZIP_SOURCE_IS_VALID_STREAM_ID(src, stream_id)) {
        zip_error_set(&src->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    stream = src->streams[stream_id];
    if (!zip_source_supports_multi_open_seekable(src)) {
        if (stream->bytes_read > ZIP_INT64_MAX) {
            zip_error_set(&src->error, ZIP_ER_TELL, EOVERFLOW);
            return -1;
        }
        return (zip_int64_t)stream->bytes_read;
    }

    args.user_stream = stream->user_stream;
    args.data = NULL;
    args.len = 0;

    return _zip_source_call(src, stream->parent_stream_id, &args, sizeof(args), ZIP_SOURCE_TELL_STREAM);
}


ZIP_EXTERN zip_int64_t
zip_source_tell(zip_source_t *src) {
    if (src->source_closed) {
        return -1;
    }
    if (!ZIP_SOURCE_IS_OPEN_READING(src)) {
        zip_error_set(&src->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    if ((src->supports & (ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_TELL) | ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_SEEK))) == 0) {
        if (src->bytes_read > ZIP_INT64_MAX) {
            zip_error_set(&src->error, ZIP_ER_TELL, EOVERFLOW);
            return -1;
        }
        return (zip_int64_t)src->bytes_read;
    }

    return _zip_source_call(src, -1, NULL, 0, ZIP_SOURCE_TELL);
}
