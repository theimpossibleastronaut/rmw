project(
  'rmw',
  'c',
  version: '0.9.3-dev',
  meson_version : '>= 0.59.0',
  default_options: [
    'warning_level=3',
    'b_sanitize=address,undefined',
    'b_lundef=false',
    ]
  )

cc = meson.get_compiler('c')
host_sys = host_machine.system()

if host_sys == 'darwin'
  pkg_config_path = get_option('pkg_config_path')
  foreach p : [
    '/usr/local/opt/ncurses/lib/pkgconfig',
    '/opt/homebrew/opt/gettext/lib/pkgconfig',
    '/opt/homebrew/opt/ncurses/lib/pkgconfig'
    ]
    pkg_config_path += p
  endforeach
  message(pkg_config_path)
endif

extra_flags = [
  '-fno-common',
  '-fstack-protector-strong',
  '-Wformat-security',
  '-Wformat-overflow=2',
  '-Wshadow',
  '-Wstrict-overflow=5',
  '-Werror=odr',
  '-Werror=lto-type-mismatch',
  '-Werror=strict-aliasing',
# '-Wconversion'
  ]

add_project_arguments(cc.get_supported_arguments(extra_flags), language: 'c')

meson.add_devenv({'RMW_LOCALEDIR': join_paths(meson.project_build_root(), 'po')})

conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('PACKAGE_STRING', meson.project_name())
conf.set_quoted('PACKAGE_URL', 'https://theimpossibleastronaut.github.io/rmw-website/')
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
  if cc.has_header(h[0])
    conf.set(h[1], 1)
  endif
endforeach

localedir = join_paths(get_option('prefix'), get_option('localedir'))
subdir('po')

# https://mesonbuild.com/Subprojects.html
canfigger_dep = dependency(
  'canfigger',
  version : '>=0.3.0',
  fallback : ['canfigger', 'canfigger_dep'], default_options: 'default_library=static',
  )

subdir('src')

main_bin = executable(
  'rmw',
  'src/main.c',
  dependencies: rmw_dep,
  install : true
  )

subdir ('man')

if get_option('build_tests')
  subdir ('test')
endif

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

# Copy completions to docdir
install_subdir(
  'completions',
  install_dir : join_paths(get_option('docdir')),
  strip_directory : false
  )
