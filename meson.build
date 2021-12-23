project(
  'rmw',
  'c',
  version: '0.8.0.555',
  meson_version : '>= 0.54.0',
  default_options: ['warning_level=3']
  )

cc = meson.get_compiler('c')

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

conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('PACKAGE_STRING', meson.project_name())
conf.set_quoted('PACKAGE_URL', 'https://remove-to-waste.info/')
conf.set_quoted('PACKAGE_BUGREPORT', 'https://github.com/theimpossibleastronaut/rmw/issues')

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
  if cc.has_header(h.get(0))
    conf.set(h.get(1), 1)
  endif
endforeach

localedir = join_paths(get_option('prefix'), get_option('localedir'))
subdir('po')

config_h = configure_file(output : 'config.h', configuration : conf)

canfigger_proj = subproject('canfigger')
canfigger_dep = canfigger_proj.get_variable('canfigger_dep')
#subdir('external/canfigger')

subdir('src')

executable(
  'rmw',
  'src/main.c',
  dependencies: [canfigger_dep, dep_rmw],
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

if get_option('nls')
  ul = configuration_data()
  ul.set('LOCALEDIR', localedir)
  ul.set('MESON_SOURCE_ROOT', meson.source_root())
  ul.set('MESON_PROJECT_NAME', meson.project_name())
  configure_file(input : 'uninstall_langs.sh.in',
                 output : 'uninstall_langs.sh',
                 configuration : ul)
endif
