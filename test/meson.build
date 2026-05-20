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

test_cases = ['strings_rmw', 'utils', 'restore']

scripts = [
  'test_basic.sh',
  'test_media_root.sh',
  'test_purging.sh',
  'test_restore.sh',
]

if host_sys == 'linux'
  if has_statfs and has_linux_fs_header
    scripts += ['test_btrfs_clone.sh']
  endif
  scripts += ['test_bcachefs.sh']
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

# Set HOME to the fake home so rmw falls back safely if RMW_FAKE_HOME is ever
# unset mid-script, preventing accidental writes to the real home directory.
test_env = [
  'MESON_SOURCE_ROOT=' + meson.project_source_root(),
  'RMW_FAKE_HOME=' + RMW_FAKE_HOME,
  'HOME=' + RMW_FAKE_HOME,
  'XDG_DATA_HOME=' + RMW_FAKE_HOME + '/.local/share',
  'XDG_CONFIG_HOME=' + RMW_FAKE_HOME + '/.config',
  'MESON_BUILD_ROOT=' + meson.project_build_root(),
]

strace = find_program('strace', required: false)
if strace.found()
  test_env += ['STRACE=' + strace.full_path()]
endif

foreach s : scripts
  test(
    s,
    files(s),
    env: test_env,
    depends: main_bin,
  )
endforeach
