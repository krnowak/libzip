import re
import sys

try:
    args = sys.argv[1:]
    in_file = args.pop(0)
    out_file = args.pop(0)
except IndexError:
    print('expected two parameters: a path to cmake-config.h.in and a path to fixed cmake-config.h.in', file=sys.stderr)
    sys.exit(1)

with open(in_file, "r") as in_handle:
    with open(out_file, "w") as out_handle:
        for line in in_handle:
            out_handle.write(re.sub(r'@([^@]*)@', r'${\1}', line))
