project('libzip', ['c'],
        version: '1.10.1')

c = meson.get_compiler('c')

commoncrypto_found = c.check_header('CommonCrypto/CommonCrypto.h', required: get_option('commoncrypto'))
gnutls_found = false
gnutls_dep = dependency('gnutls', required: get_option('gnutls'))
nettle_dep = dependency('nettle', required: get_option('gnutls'))
mbedtls_dep = c.find_library('mbedcrypto',
                             has_headers: ['mbedtls/aes.h'],
                             required: get_option('mbedtls'))
openssl_dep = dependency('openssl', required: get_option('openssl'))
windows_crypto_found = false
if host_machine.system() == 'windows'
  if get_option('windows_crypto').enabled() or get_option('windows_crypto').auto()
    windows_crypto_found = true
  endif
else
  if get_option('windows_crypto').enabled()
    warning('The windows_crypto option is usable only on windows platform')
  endif
endif

cfg = configuration_data()

cfg.set('ENABLE_FDOPEN', get_option('fdopen'))

libext_args = ['-D__STDC_WANT_LIB_EXT1__=1']

if c.has_function('_close')
  cfg.set('HAVE__CLOSE', true)
endif
if c.has_function('_dup')
  cfg.set('HAVE__DUP', true)
endif
if c.has_function('_fdopen')
  cfg.set('HAVE__FDOPEN', true)
endif
if c.has_function('_fileno')
  cfg.set('HAVE__FILENO', true)
endif
if c.has_function('_setmode')
  cfg.set('HAVE__SETMODE', true)
endif
if c.has_header_symbol('stdio.h', '_snprintf')
  cfg.set('HAVE__SNPRINTF', true)
endif
if c.has_header_symbol('stdio.h', '_snprintf_s', args: libext_args)
  cfg.set('HAVE__SNPRINTF_S', true)
endif
if c.has_header_symbol('stdio.h', '_snwprintf_s', args: libext_args)
  cfg.set('HAVE__SNWPRINTF_S', true)
endif
if c.has_function('_strdup')
  cfg.set('HAVE__STRDUP', true)
endif
if c.has_header_symbol('string.h', '_stricmp')
  cfg.set('HAVE__STRICMP', true)
endif
if c.has_function('_strtoi64')
  cfg.set('HAVE__STRTOI64', true)
endif
if c.has_function('_strtoui64')
  cfg.set('HAVE__STRTOUI64', true)
endif
if c.has_function('_unlink')
  cfg.set('HAVE__UNLINK', true)
endif
if c.has_function('arc4random')
  cfg.set('HAVE_ARC4RANDOM', true)
endif
if c.has_function('clonefile')
  cfg.set('HAVE_CLONEFILE', true)
endif
if c.has_function('explicit_bzero')
  cfg.set('HAVE_EXPLICIT_BZERO', true)
endif
if c.has_function('explicit_memset')
  cfg.set('HAVE_EXPLICIT_MEMSET', true)
endif
if c.has_function('fchmod')
  cfg.set('HAVE_FCHMOD', true)
endif
if c.has_function('fileno')
  cfg.set('HAVE_FILENO', true)
endif
if c.has_function('fseeko')
  cfg.set('HAVE_FSEEKO', true)
endif
if c.has_function('ftello')
  cfg.set('HAVE_FTELLO', true)
endif
if c.has_function('getprogname')
  cfg.set('HAVE_GETPROGNAME', true)
endif
if c.has_header_symbol('time.h', 'localtime_r')
  cfg.set('HAVE_LOCALTIME_R', true)
endif
if c.has_header_symbol('time.h', 'localtime_s', args: libext_args)
  cfg.set('HAVE_LOCALTIME_S', true)
endif
if c.has_function('memcpy_s', args: libext_args)
  cfg.set('HAVE_MEMCPY_S', true)
endif
if c.has_function('random')
  cfg.set('HAVE_RANDOM', true)
endif
if c.has_function('setmode')
  cfg.set('HAVE_SETMODE', true)
endif
if c.has_header_symbol('stdio.h', 'snprintf')
  cfg.set('HAVE_SNPRINTF', true)
endif
if c.has_header_symbol('stdio.h', 'snprintf_s', args: libext_args)
  cfg.set('HAVE_SNPRINTF_S', true)
endif
if c.has_header_symbol('strings.h', 'strcasecmp')
  cfg.set('HAVE_STRCASECMP', true)
endif
if c.has_function('strdup')
  cfg.set('HAVE_STRDUP', true)
endif
if c.has_function('strerror_s', args: libext_args)
  cfg.set('HAVE_STRERROR_S', true)
endif
if c.has_function('strerrorlen_s', args: libext_args)
  cfg.set('HAVE_STRERRORLEN_S', true)
endif
if c.has_function('stricmp')
  cfg.set('HAVE_STRICMP', true)
endif
if c.has_function('strncpy_s', args: libext_args)
  cfg.set('HAVE_STRNCPY_S', true)
endif
if c.has_function('strtoll')
  cfg.set('HAVE_STRTOLL', true)
endif
if c.has_function('strtoull')
  cfg.set('HAVE_STRTOULL', true)
endif

fts_lib = dependency('', required: false)
if c.has_header('fts.h', prefix: '#include <sys/types.h>\n#include <sys/stat.h>')
  cfg.set('HAVE_FTS_H', true)
  if not c.has_function('fts_open')
    fts_lib_tmp = c.find_library('fts')
    if fts_lib_tmp.found()
      if c.has_function('fts_open', dependencies: fts_lib_tmp)
        fts_lib = fts_lib_tmp
      endif
    endif
  endif
endif

if c.has_header('stdbool.h')
  cfg.set('HAVE_STDBOOL_H', true)
endif
if c.has_header('strings.h')
  cfg.set('HAVE_STRINGS_H', true)
endif
if c.has_header('unistd.h')
  cfg.set('HAVE_UNISTD_H', true)
endif

have_inttypes_h = c.has_header('inttypes.h')
have_stdint_h = c.has_header('stdint.h')
have_sys_types_h = c.has_header('sys/types.h')
have_stddef_h = c.has_header('stddef.h')

sizeof_includes = '''
#if @0@
#include <inttypes.h>
#endif
#if @1@
#include <stdint.h>
#endif
#if @2@
#include <sys/types.h>
#endif
#if @3@
#include <stddef.h>
#endif
'''.format(have_inttypes_h.to_int(),
           have_stdint_h.to_int(),
           have_sys_types_h.to_int(),
           have_stddef_h.to_int())

__int8_size = c.sizeof('__int8', prefix: sizeof_includes)
int8_t_size = c.sizeof('int8_t', prefix: sizeof_includes)
uint8_t_size = c.sizeof('uint8_t', prefix: sizeof_includes)
__int16_size = c.sizeof('__int16', prefix: sizeof_includes)
int16_t_size = c.sizeof('int16_t', prefix: sizeof_includes)
uint16_t_size = c.sizeof('uint16_t', prefix: sizeof_includes)
__int32_size = c.sizeof('__int32', prefix: sizeof_includes)
int32_t_size = c.sizeof('int32_t', prefix: sizeof_includes)
uint32_t_size = c.sizeof('uint32_t', prefix: sizeof_includes)
__int64_size = c.sizeof('__int64', prefix: sizeof_includes)
int64_t_size = c.sizeof('int64_t', prefix: sizeof_includes)
uint64_t_size = c.sizeof('uint64_t', prefix: sizeof_includes)
short_size = c.sizeof('short', prefix: sizeof_includes)
int_size = c.sizeof('int', prefix: sizeof_includes)
long_size = c.sizeof('long', prefix: sizeof_includes)
long_long_size = c.sizeof('long long', prefix: sizeof_includes)

off_t_size = c.sizeof('off_t', prefix: sizeof_includes)
cfg.set('SIZEOF_OFF_T', off_t_size)
size_t_size = c.sizeof('size_t', prefix: sizeof_includes)
cfg.set('SIZEOF_SIZE_T', size_t_size)

ficlonerange_code = '''
#include <sys/ioctl.h>
#include <linux/fs.h>
int main(int argc, char *argv[]) { unsigned long x = FICLONERANGE; }
'''
if c.compiles(ficlonerange_code, name: 'FICLONERANGE')
  cfg.set('HAVE_FICLONERANGE', true)
endif

nullable_code = '''
int foo(char * _Nullable bar);
int main(int argc, char *argv[]) { }
'''
have_nullable = c.compiles(nullable_code)
if have_nullable
  cfg.set('HAVE_NULLABLE', true)
endif

if host_machine.endian() == 'big'
  cfg.set('WORDS_BIGENDIAN', true)
endif

zlib_dep = dependency('zlib', version: '>=1.1.2')
bzip2_dep = dependency('bzip2', required: get_option('bzip2'))
if bzip2_dep.found()
  cfg.set('HAVE_LIBBZ2', true)
endif
lzma_dep = dependency('liblzma', version: '>= 5.2', required: get_option('lzma'))
if lzma_dep.found()
  cfg.set('HAVE_LIBLZMA', true)
endif
zstd_dep = dependency('libzstd', version: '>= 1.3.6', required: get_option('zstd'))
if zstd_dep.found()
  cfg.set('HAVE_LIBZSTD', true)
endif

have_crypto = false
if commoncrypto_found
  cfg.set('HAVE_CRYPTO', true)
  cfg.set('HAVE_COMMONCRYPTO', true)
  have_crypto = true
elif windows_crypto_found
  cfg.set('HAVE_CRYPTO', true)
  cfg.set('HAVE_WINDOWS_CRYPTO', true)
  have_crypto = true
elif openssl_dep.found()
  cfg.set('HAVE_CRYPTO', true)
  cfg.set('HAVE_OPENSSL', true)
  have_crypto = true
elif gnutls_dep.found() and nettle_dep.found()
  cfg.set('HAVE_CRYPTO', true)
  cfg.set('HAVE_GNUTLS', true)
  gnutls_found = true
  have_crypto = true
elif mbedtls_dep.found()
  cfg.set('HAVE_CRYPTO', true)
  cfg.set('HAVE_MBEDTLS', true)
  have_crypto = true
endif

if not have_crypto
  warning('-- neither Common Crypto, GnuTLS, mbed TLS, OpenSSL, nor Windows Cryptography found; AES support disabled')
endif

if c.get_id() == 'msvc'
  add_project_arguments('/D_CRT_SECURE_NO_WARNINGS')
  add_project_arguments('/D_CRT_NONSTDC_NO_DEPRECATE')
endif

if host_machine.system() == 'windows'
  if get_option('windows_uwp')
    add_project_arguments('/DMS_UWP')
  endif
else
  if get_option('windows_uwp')
    warning('The windows_uwp option is usable only on windows platform')
  endif
endif

# ADD_SUBDIRECTORY(lib)

zipconf = configuration_data()

inttypes_h_include = '''
#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS 1
#endif
#include <inttypes.h>
'''

if have_inttypes_h
  zipconf.set('LIBZIP_TYPES_INCLUDE', inttypes_h_include)
elif have_stdint_h
  zipconf.set('LIBZIP_TYPES_INCLUDE', '#include <stdint.h>')
elif have_sys_types_h
  zipconf.set('LIBZIP_TYPES_INCLUDE', '#include <sys/types.h>')
endif

if int8_t_size >= 0
  zipconf.set('ZIP_INT8_T', 'int8_t')
elif __int8_size >= 0
  zipconf.set('ZIP_INT8_T', '__int8')
else
  zipconf.set('ZIP_INT8_T', 'signed char')
endif

if uint8_t_size >= 0
  zipconf.set('ZIP_UINT8_T', 'uint8_t')
elif __int8_size >= 0
  zipconf.set('ZIP_UINT8_T', 'unsigned __int8')
else
  zipconf.set('ZIP_UINT8_T', 'unsigned char')
endif

if int16_t_size >= 0
  zipconf.set('ZIP_INT16_T', 'int16_t')
elif __int16_size >= 0
  zipconf.set('ZIP_INT16_T', '__int16')
elif short_size == 2
  zipconf.set('ZIP_INT16_T', 'short')
else
  error('no known signed 2 byte integral type')
endif

if uint16_t_size >= 0
  zipconf.set('ZIP_UINT16_T', 'uint16_t')
elif __int16_size >= 0
  zipconf.set('ZIP_UINT16_T', 'unsigned __int16')
else
  zipconf.set('ZIP_UINT16_T', 'unsigned short')
endif

if int32_t_size >= 0
  zipconf.set('ZIP_INT32_T', 'int32_t')
elif __int32_size >= 0
  zipconf.set('ZIP_INT32_T', '__int32')
elif int_size == 4
  zipconf.set('ZIP_INT32_T', 'int')
elif long_size == 4
  zipconf.set('ZIP_INT32_T', 'long')
else
  error('no known signed 4 byte integral type')
endif

if uint32_t_size >= 0
  zipconf.set('ZIP_UINT32_T', 'uint32_t')
elif __int32_size >= 0
  zipconf.set('ZIP_UINT32_T', 'unsigned __int32')
elif int_size == 4
  zipconf.set('ZIP_UINT32_T', 'unsigned int')
else
  zipconf.set('ZIP_UINT32_T', 'unsigned long')
endif

if int64_t_size >= 0
  zipconf.set('ZIP_INT64_T', 'int64_t')
elif __int64_size >= 0
  zipconf.set('ZIP_INT64_T', '__int64')
elif long_size == 8
  zipconf.set('ZIP_INT64_T', 'long')
elif long_long_size == 8
  zipconf.set('ZIP_INT64_T', 'long long')
else
  error('no known signed 8 byte integral type')
endif

if uint64_t_size >= 0
  zipconf.set('ZIP_UINT64_T', 'uint64_t')
elif __int64_size >= 0
  zipconf.set('ZIP_UINT64_T', 'unsigned __int64')
elif long_size == 8
  zipconf.set('ZIP_UINT64_T', 'unsigned long')
else
  zipconf.set('ZIP_UINT64_T', 'unsigned long long')
endif

nullable_defines = '''
#define _Nullable
#define _Nonnull
'''
if have_nullable
  zipconf.set('ZIP_NULLABLE_DEFINES', '')
else
  zipconf.set('ZIP_NULLABLE_DEFINES', nullable_defines)
endif

zipconf.set('libzip_VERSION', meson.project_version())
split_version = meson.project_version().split('.')
if split_version.length() > 0
  zipconf.set('libzip_VERSION_MAJOR', split_version[0])
endif
if split_version.length() > 1
  zipconf.set('libzip_VERSION_MINOR', split_version[1])
endif
if split_version.length() > 2
  zipconf.set('libzip_VERSION_PATCH', split_version[2])
endif

if get_option('default_library') == 'static'
  zipconf.set('ZIP_STATIC', true)
elif get_option('default_library') == 'shared'
  cfg.set('HAVE_SHARED', true)
else
  error('default_library option must only be either "shared" or "static" in libzip')
endif

pymod = import('python')
python = pymod.find_installation()

fix_cmake_config_h_in = files('fix_cmake_config_h_in.py')

# These are not used, but we add them to cfg to silence meson
# warnings.
cfg.set('CMAKE_PROJECT_NAME', 'libzip')
cfg.set('CMAKE_PROJECT_VERSION', meson.project_version())
buggy_cmake_config_h_in = files('cmake-config.h.in')
fixed_cmake_config_h_in = configure_file(input: buggy_cmake_config_h_in,
                                         output: 'fixed-cmake-config.h.in',
                                         command: [python,
                                                   fix_cmake_config_h_in,
                                                   buggy_cmake_config_h_in,
                                                   'fixed-cmake-config.h.in'])
configure_file(input: fixed_cmake_config_h_in,
               output: 'config.h',
               configuration: cfg,
               format: 'cmake')
configure_file(input: 'cmake-zipconf.h.in',
               output: 'zipconf.h',
               configuration: zipconf,
               format: 'cmake')

top_dir = include_directories('.')

subdir('lib')
if get_option('tools').allowed()
  subdir('src')
  if get_option('regress').allowed()
    nihtest = find_program('nihtest', required: get_option('regress').enabled())
    if nihtest.found()
      subdir('regress')
    endif
  endif
else
  if get_option('regress').enabled()
    error('tools build has been disabled, but they are needed for regression tests')
  endif
endif

if get_option('ossfuzz').allowed()
  subdir('ossfuzz')
endif
