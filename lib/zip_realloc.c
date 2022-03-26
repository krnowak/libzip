/*
  zip_realloc.c -- reallocate with additional elements
  Copyright (C) 2009-2022 Dieter Baron and Thomas Klausner

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

int
_zip_realloc(void **inout_memory, zip_uint64_t *inout_alloced, zip_uint64_t element_size, zip_uint64_t additional_elements) {
    void *old_memory = *inout_memory;
    zip_uint64_t old_alloced = *inout_alloced;
    /* old size should not overflow - it's something we allocated before */
    zip_uint64_t old_size = old_alloced * element_size;
    zip_uint64_t new_alloced;
    zip_uint64_t new_size;
    void *new_memory;

    if (additional_elements == 0)
        additional_elements = 1;

    if (old_alloced > ZIP_UINT64_MAX - additional_elements)
        return ZIP_ER_MEMORY;

    new_alloced = old_alloced + additional_elements;

    if (new_alloced > ZIP_UINT64_MAX / element_size)
        return ZIP_ER_MEMORY;

    new_size = new_alloced * element_size;

    if ((new_memory = realloc(old_memory, new_size)) == NULL)
        return ZIP_ER_MEMORY;

    *inout_memory = new_memory;
    *inout_alloced = new_alloced;
    return ZIP_ER_OK;
}
