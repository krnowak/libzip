src_dir = include_directories('.')
getopt_dep = dependency('', required: false)
if not c.has_function('getopt')
  getopt_sources = files('getopt.c', 'getopt.h')
  getopt_dep = declare_dependency(include_directories: src_dir,
                                  sources: getopt_sources)
endif


executable('zipcmp',
           'zipcmp.c', 'diff_output.c',
           dependencies: [libzip_dep, zlib_dep, getopt_dep])

executable('zipmerge',
           'zipmerge.c',
           dependencies: [libzip_dep, getopt_dep])

ziptool = executable('ziptool',
                     'ziptool.c',
                     dependencies: [libzip_dep, getopt_dep])
