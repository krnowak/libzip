test_programs = [
  'add_from_filep',
  'add_from_filep',
  'can_clone_file',
  'fopen_unchanged',
  'fseek',
  'nonrandomopentest',
  'liboverride-test',
]

getopt_users = [
  'fread',
  'tryopen',
]

hole_users = [
  'hole',
  'ziptool_regress',
]

foreach t : test_programs
  executable(t, t + '.c', dependencies: [libzip_dep])
endforeach

foreach t : getopt_users
  executable(t,
             t + '.c',
             dependencies: [libzip_dep, getopt_dep],
             include_directories: [src_dir])
endforeach

foreach t : hole_users
  executable(t,
             t + '.c', 'source_hole.c',
             dependencies: [libzip_dep, getopt_dep],
             include_directories: [src_dir])
endforeach

dl_users = [
  'nonrandomopen',
  'liboverride',
]

foreach t : dl_users
  shared_module(t, t + '.c', include_directories: [top_dir, libzip_dir])
endforeach

testcases = [
  'add_dir.test',
  'add_from_buffer.test',
  'add_from_file_duplicate.test',
  'add_from_filep.test',
  'add_from_file.test',
  'add_from_file_twice_duplicate.test',
  'add_from_file_unchange.test',
  'add_from_stdin.test',
  'add_from_zip_closed.test',
  'add_from_zip_deflated2.test',
  'add_from_zip_deflated.test',
  'add_from_zip_partial_deflated.test',
  'add_from_zip_partial_stored.test',
  'add_from_zip_stored.test',
  'add_stored_in_memory.test',
  'add_stored.test',
  'buffer-fragment-read.test',
  'buffer-fragment-write.test',
  'cancel_45.test',
  'cancel_90.test',
  'changing-size-decreases-fixed.test',
  'changing-size-decreases.test',
  'changing-size-increases-fixed.test',
  'changing-size-increases.test',
  'changing-size-increases-unchecked.test',
  'check_torrentzip_fail.test',
  'check_torrentzip_modified.test',
  'check_torrentzip_success.test',
  'clone-buffer-add.test',
  'clone-buffer-delete.test',
  'clone-buffer-replace.test',
  'clone-fs-add.test',
  'clone-fs-delete.test',
  'clone-fs-replace.test',
  'cm-default.test',
  'convert_to_torrentzip_ef.test',
  'convert_to_torrentzip.test',
  'count_entries.test',
  'create_empty_keep.test',
  'decrypt-correct-password-aes128.test',
  'decrypt-correct-password-aes192.test',
  'decrypt-correct-password-aes256.test',
  'decrypt-correct-password-pkware-2.test',
  'decrypt-correct-password-pkware.test',
  'decrypt-empty-file-pkware.test',
  'decrypt-no-password-aes256.test',
  'decrypt-wrong-password-aes128.test',
  'decrypt-wrong-password-aes192.test',
  'decrypt-wrong-password-aes256.test',
  'decrypt-wrong-password-pkware-2.test',
  'decrypt-wrong-password-pkware.test',
  'delete_add_same.test',
  'delete_invalid.test',
  'delete_last_keep.test',
  'delete_last.test',
  'delete_multiple_last.test',
  'delete_multiple_partial.test',
  'delete_renamed_rename.test',
  'encryption-nonrandom-aes128.test',
  'encryption-nonrandom-aes192.test',
  'encryption-nonrandom-aes256.test',
  'encryption-nonrandom-pkware-2.test',
  'encryption-nonrandom-pkware.test',
  'encryption-remove.test',
  'encryption-stat.test',
  'encrypt.test',
  'extra_add_multiple.test',
  'extra_add.test',
  'extra_count_by_id.test',
  'extra_count_ignore_zip64.test',
  'extra_count.test',
  'extra_delete_by_id.test',
  'extra_delete.test',
  'extra_field_align.test',
  'extra_get_by_id.test',
  'extra_get.test',
  'extra_set_modify_c.test',
  'extra_set_modify_l.test',
  'extra_set.test',
  'fdopen_ok.test',
  'file_comment_encmismatch.test',
  'fopen_multiple_reopen.test',
  'fopen_multiple.test',
  'fopen_unchanged.test',
  'fread.test',
  'fseek_deflated.test',
  'fseek_fail.test',
  'fseek_ok.test',
  'get_comment_long.test',
  'get_comment.test',
  'junk_at_end.test',
  'junk_at_start.test',
  'mtime-dstpoint.test',
  'mtime-post-dstpoint.test',
  'mtime-pre-dstpoint.test',
  'name_locate-cp437.test',
  'name_locate.test',
  'name_locate-utf8.test',
  'open_cons_extrabytes.test',
  'open_empty_2.test',
  'open_empty.test',
  'open_extrabytes.test',
  'open_file_count.test',
  'open_filename_duplicate_consistency.test',
  'open_filename_duplicate_empty_consistency.test',
  'open_filename_duplicate_empty.test',
  'open_filename_duplicate.test',
  'open_filename_empty.test',
  'open_incons.test',
  'open_many_fail.test',
  'open_many_ok.test',
  'open_multidisk.test',
  'open_new_but_exists.test',
  'open_new_ok.test',
  'open_nonarchive.test',
  'open_nosuchfile.test',
  'open_ok.test',
  'open_too_short.test',
  'open_truncated.test',
  'open_truncate.test',
  'open_zip64_3mf.test',
  'open_zip64_ok.test',
  'preload.test',
  'progress.test',
  'read_seek_read.test',
  'rename_ascii.test',
  'rename_cp437.test',
  'rename_deleted.test',
  'rename_fail.test',
  'rename_ok.test',
  'rename_utf8_encmismatch.test',
  'rename_utf8.test',
  'reopen_partial_rest.test',
  'reopen_partial.test',
  'reopen.test',
  'set_comment_all.test',
  'set_comment_localonly.test',
  'set_comment_removeglobal.test',
  'set_comment_revert.test',
  'set_compression_bzip2_to_deflate.test',
  'set_compression_deflate_to_bzip2.test',
  'set_compression_deflate_to_deflate.test',
  'set_compression_deflate_to_store.test',
  'set_compression_lzma_no_eos_to_store.test',
  'set_compression_lzma_to_store.test',
  'set_compression_store_to_bzip2.test',
  'set_compression_store_to_deflate.test',
  'set_compression_store_to_lzma.test',
  'set_compression_store_to_store.test',
  'set_compression_store_to_xz.test',
  'set_compression_store_to_zstd.test',
  'set_compression_unknown.test',
  'set_compression_xz_to_store.test',
  'set_compression_zstd_to_store.test',
  'set_file_dostime.test',
  'set_file_mtime_pkware.test',
  'set_file_mtime.test',
  'stat_index_cp437_guess.test',
  'stat_index_cp437_raw.test',
  'stat_index_cp437_strict.test',
  'stat_index_fileorder.test',
  'stat_index_streamed.test',
  'stat_index_streamed_zip64.test',
  'stat_index_utf8_guess.test',
  'stat_index_utf8_raw.test',
  'stat_index_utf8_strict.test',
  'stat_index_utf8_unmarked_strict.test',
  'stat_index_zip64.test',
  'truncate_empty_keep.test',
  'unchange-delete-namelocate.test',
  'utf-8-standardization.test',
  'want_torrentzip_stat.test',
  'zip64_creation.test',
  'zip64_stored_creation.test',
  'zipcmp_zip_dir_slash.test',
  'zipcmp_zip_dir.test',
  'zip-in-archive-comment.test',
]

regress_cfg = configuration_data()
regress_cfg.set('CMAKE_CURRENT_SOURCE_DIR', meson.current_source_dir())
regress_cfg.set('PROJECT_BINARY_DIR', meson.project_build_root())

nihconf = configure_file(configuration: regress_cfg,
                         input: 'nihtest.conf.in',
                         output: 'nihtest.conf',
                         format: 'cmake@')

# zipzip -> list[zip, idx_in_zipzip]
test_archives = {
  'manyfiles-zip.zip': [
    ['manyfiles-133000.zip', 1],
    ['manyfiles-fewer.zip', 5],
    ['manyfiles.zip', 0],
    ['manyfiles-zip64.zip', 4],
    ['manyfiles-65536.zip', 2],
    ['manyfiles-zip64-modulo.zip', 3],
    ['manyfiles-more.zip', 6],
  ],
  'bigzero-zip.zip': [
    ['bigzero.zip', 0],
  ]
}

archive_targets = []
foreach zipzip, zipzip_entries : test_archives
  foreach zipzip_entry : zipzip_entries
    zip = zipzip_entry[0]
    idx = zipzip_entry[1]
    archive_targets += custom_target('@0@-@1@'.format(zipzip, zip),
                                     command: [
                                       ziptool,
                                       '@INPUT@',
                                       'cat',
                                       idx.to_string(),
                                     ],
                                     input: [
                                       zipzip,
                                     ],
                                     output: [
                                       zip
                                     ],
                                     capture: true)
  endforeach
endforeach

foreach tc : testcases
  test(tc, nihtest,
       args: ['-v', files(tc)],
       workdir: meson.current_build_dir(),
       depends: archive_targets,
       timeout: 100)
endforeach
