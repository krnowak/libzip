fuzz_programs = [
  'zip_read_file_fuzzer',
  'zip_read_fuzzer',
  'zip_write_encrypt_aes256_file_fuzzer',
  'zip_write_encrypt_pkware_file_fuzzer',
]

foreach t : fuzz_programs
  executable(t, t + '.c', 'fuzz_main.c',
             include_directories: [top_dir, libzip_dir],
             dependencies: [libzip_dep])
endforeach
