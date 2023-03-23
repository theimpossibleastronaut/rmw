add_test_setup(
  'fake_media_root',
  env: 'RMW_FAKE_MEDIA_ROOT=true'
  )

newscripts = [
  'fail_restore_files_not_in_waste',
  '01_arg_u',
  'mrl_not_exist',
  'restore_with_wildcard'
  ]

build_test_root = meson.current_build_dir()
RMW_FAKE_HOME = join_paths(build_test_root, 'rmw-tests-home')

cat_prog = find_program('cat')
sh_prog = find_program('sh', required: true)

foreach scriptfoo : newscripts
#  run_command(
#    'concatenate.sh',
#    scriptfoo,
#    env: { 'BUILD_TEST_ROOT': build_test_root },
#    check: true
#    )

  concat_fragments = custom_target(
    scriptfoo,
    command: [ cat_prog, '@INPUT@' ],
    capture: true,
    input: ['script-head', 'fragments/@0@.sh'.format(scriptfoo)],
    output: scriptfoo + '.sh',
    install: false,
    build_by_default: true
  )

  test(
    scriptfoo,
    sh_prog,
    args: join_paths(meson.current_build_dir(), scriptfoo + '.sh'),
    env : {
      'MESON_SOURCE_ROOT': meson.project_source_root(),
      'RMW_FAKE_HOME': RMW_FAKE_HOME,
      'MESON_BUILD_ROOT': meson.project_build_root()
      },
    depends: main_bin
  )
endforeach

test_cases = [
  'strings_rmw',
  'utils_rmw',
  'trashinfo_rmw',
  'restore_rmw'
  ]

scripts = [
  'test_basic.sh',
  'test_purging.sh',
  'test_restore.sh',
  ]

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
      ],
    depends: main_bin
    )
endforeach
