import re
import sys

try:
    args = sys.argv[1:]
    zip_err_str_c = args.pop(0)
    zip_h = args.pop(0)
    zipint_h = args.pop(0)
except IndexError:
    print('expected three parameters: a path to write the zip_err_str.c file, a path to zip.h and a path to zipint.h', file=sys.stderr)
    sys.exit(1)

zip_h_re = re.compile(r'''#define ZIP_ER_[A-Z0-9_]+ [0-9]+[ \t]+/\*\s*([A-Z])\s+([A-Za-z0-9 ',*-]+?)\s*\*/''', re.ASCII)
zipint_h_re = re.compile(r'''#define ZIP_ER_DETAIL_[A-Z0-9_]+ [0-9]+[ \t]+/\*\s*([A-Z])\s+([A-Za-z0-9 ',*-]+?)\s*\*/''', re.ASCII)

part1 = '''/*
  This file was generated automatically by CMake
  from zip.h and zipint.h; make changes there.
*/

#include "zipint.h"

#define L ZIP_ET_LIBZIP
#define N ZIP_ET_NONE
#define S ZIP_ET_SYS
#define Z ZIP_ET_ZLIB

#define E ZIP_DETAIL_ET_ENTRY
#define G ZIP_DETAIL_ET_GLOBAL

const struct _zip_err_info _zip_err_str[] = {
'''

part2 = '''};

const int _zip_err_str_count = sizeof(_zip_err_str)/sizeof(_zip_err_str[0]);

const struct _zip_err_info _zip_err_details[] = {
'''

part3 = '''};

const int _zip_err_details_count = sizeof(_zip_err_details)/sizeof(_zip_err_details[0]);
'''

with open(zip_err_str_c, "w") as out_handle:
    out_handle.write(part1)
    with open(zip_h, "r") as handle:
        for line in handle:
            m = zip_h_re.fullmatch(line.rstrip())
            if m is not None:
                out_handle.write(f"""    {{ {m.group(1)}, "{m.group(2)}" }},\n""")
    out_handle.write(part2)
    with open(zipint_h, "r") as handle:
        for line in handle:
            m = zipint_h_re.fullmatch(line.rstrip())
            if m is not None:
                out_handle.write(f"""    {{ {m.group(1)}, "{m.group(2)}" }},\n""")
    out_handle.write(part3)
