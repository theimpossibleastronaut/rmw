# The tests must be configured with 'b_sanitize=none' for this to work
# unless faketime has been built with sanitizer
faketime = find_program('faketime', required: false)
if faketime.found()
    add_test_setup(
        'epochalypse',
        exe_wrapper: [faketime, '-f', '+14y'],
        env: ['RMW_TEST_EPOCHALYPSE=true'],
    )
endif

test_cases = ['strings_rmw', 'utils', 'trashinfo', 'restore']

scripts = [
    'test_basic.sh',
    'test_media_root.sh',
    'test_purging.sh',
    'test_restore.sh',
]

if has_statfs and has_btrfs_header
    scripts += ['test_btrfs_clone.sh']
endif

RMW_FAKE_HOME = join_paths(meson.current_build_dir(), 'rmw-tests-home')

foreach case : test_cases
    exe = executable(
        'test_' + case,
        '../src/' + case + '.c',
        c_args: ['-DTEST_LIB', '-DRMW_FAKE_HOME="@0@"'.format(RMW_FAKE_HOME)],
        dependencies: rmw_dep,
        override_options: ['b_lto=false'],
    )
    test('test_' + case, exe)
endforeach

foreach s : scripts
    test(
        s,
        files(s),
        env: [
            'MESON_SOURCE_ROOT=' + meson.project_source_root(),
            'RMW_FAKE_HOME=' + RMW_FAKE_HOME,
            'MESON_BUILD_ROOT=' + meson.project_build_root(),
        ],
        depends: main_bin,
    )
endforeach
