vg = find_program('valgrind', required : false)

if vg.found()
  add_test_setup(
    'valgrind',
    exe_wrapper : [join_paths(meson.current_source_dir(), 'valgrind_wrap.sh')]
    )
endif

add_test_setup(
  'fake_media_root',
  env: 'RMW_FAKE_MEDIA_ROOT=true'
  )

test_cases = [
  'strings_rmw',
  'utils_rmw',
  'trashinfo_rmw',
  'purging_rmw',
  'restore_rmw'
  ]

scripts = [
  'test_basic.sh',
  'test_purging.sh',
  'test_restore.sh',
  ]

RMW_FAKE_HOME = join_paths(meson.current_build_dir(), 'rmw-tests-home')

foreach case : test_cases
  exe = executable(
    'test_' + case,
    '../src/' + case + '.c',
    c_args : ['-DTEST_LIB', '-DRMW_FAKE_HOME="@0@"'.format(RMW_FAKE_HOME)],
    dependencies: [dep_canfigger, dep_rmw]
    )
  test('test_' + case, exe)
endforeach

foreach s : scripts
  test(s,
    files(s),
    env : [
      'MESON_SOURCE_ROOT=' + meson.project_source_root(),
      'RMW_FAKE_HOME=' + RMW_FAKE_HOME,
      'MESON_BUILD_ROOT=' + meson.project_build_root()
      ])
endforeach
