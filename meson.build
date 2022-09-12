project(
  'rmw',
  'c',
  version: '0.8.1.555',
  meson_version : '>= 0.59.0',
  default_options: ['warning_level=3']
  )

cc = meson.get_compiler('c')
host_sys = host_machine.system()

extra_flags = [
  '-fno-common',
  '-fstack-protector-strong',
  '-fanalyzer',
  '-Wformat-security',
  '-Wformat-overflow=2',
  '-Wshadow',
  '-Wstrict-overflow=5',
# '-Wconversion'
  ]

foreach cflag : extra_flags
  if cc.has_argument(cflag)
    add_project_arguments(cflag, language: 'c')
  endif
endforeach

# gcc-12 gives false positives.
disable_flag = [
  '-Wno-analyzer-malloc-leak',
  '-Wno-analyzer-null-argument'
  ]
foreach cflag : disable_flag
  if cc.get_id() == 'gcc' and cc.version().startswith('12.')
    add_project_arguments(disable_flag, language: 'c')
  endif
endforeach

conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('PACKAGE_STRING', meson.project_name())
conf.set_quoted('PACKAGE_URL', 'https://remove-to-waste.info/')
conf.set_quoted('PACKAGE_BUGREPORT', 'https://github.com/theimpossibleastronaut/rmw/issues')

foo_file = 'foo_rmw.txt'
configure_file(output: foo_file, configuration: configuration_data())
r = run_command(find_program('rm'), '--one-file-system', join_paths(meson.project_build_root(), foo_file), check: false)
if r.returncode() == 0
  conf.set('RM_HAS_ONE_FILE_SYSTEM_ARG', 1)
endif

check_headers = [
  ['ncursesw/menu.h', 'HAVE_NCURSESW_MENU_H'],
  ['ncurses/menu.h', 'HAVE_NCURSES_MENU_H'],
  ['menu.h', 'HAVE_MENU_H'],
  ['ncursesw/curses.h', 'HAVE_NCURSESW_CURSES_H'],
  ['ncursesw.h', 'HAVE_NCURSESW_H'],
  ['ncurses/curses.h', 'HAVE_NCURSES_CURSES_H'],
  ['ncurses.h', 'HAVE_NCURSES_H'],
  ['curses.h', 'HAVE_CURSES_H'],
  ]

foreach h : check_headers
  if cc.has_header(h[0])
    conf.set(h[1], 1)
  endif
endforeach

localedir = join_paths(get_option('prefix'), get_option('localedir'))
subdir('po')

config_h = configure_file(output : 'config.h', configuration : conf)

# https://mesonbuild.com/Subprojects.html
dep_canfigger = dependency(
  'canfigger',
  version : '>=0.2.0',
  fallback : ['canfigger', 'dep_canfigger'], default_options: 'default_library=static',
  )

subdir('src')

executable(
  'rmw',
  'src/main.c',
  dependencies: [dep_canfigger, dep_rmw, dep_intl],
  install : true
  )

subdir ('man')
subdir ('test')

install_data(
  files(
    'README.md',
    'COPYING',
    'AUTHORS.md',
    'ChangeLog',
    'ReleaseNotes',
    'rmwrc.example'
    ),
  install_dir : get_option('docdir')
  )
