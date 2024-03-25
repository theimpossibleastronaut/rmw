add_test_setup(
  'fake_media_root',
  env: 'RMW_FAKE_MEDIA_ROOT=true'
  )

#
# This does not work
#
#faketime_lib = cc.find_library(
#  'faketime',
#  dirs: [
#    '/usr/lib/faketime',
#    '/lib/x86_64-linux-gnu/faketime',
#    '/lib/aarch64-linux-gnu/faketime'
#    ],
#  required: false
#  )

# TODO: Detect whether ubsan is required or not, determine the path
add_test_setup(
    'epochalypse',
    env: [
        #'LD_PRELOAD=' + faketime_lib.full_path(),
        'LD_PRELOAD=/usr/lib/faketime/libfaketime.so.1',
        'FAKETIME=+14y'
    ]
)

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

RMW_FAKE_HOME = join_paths(meson.current_build_dir(), 'rmw-tests-home')

foreach case : test_cases
  exe = executable(
    'test_' + case,
    '../src/' + case + '.c',
    c_args : ['-DTEST_LIB', '-DRMW_FAKE_HOME="@0@"'.format(RMW_FAKE_HOME)],
    dependencies: rmw_dep
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
