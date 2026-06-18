project(
  'rmw',
  'c',
  version: '0.10.0-dev',
  meson_version: '>= 0.59.0',
  default_options: [
    'c_std=gnu99',
    'warning_level=3',
    #    'b_sanitize=address,undefined',
    #    'b_lundef=false',
    'pkg_config_path=/opt/homebrew/opt/gettext/lib/pkgconfig,/opt/homebrew/opt/ncurses/lib/pkgconfig,/usr/local/opt/ncurses/lib/pkgconfig',
  ],
)

cc = meson.get_compiler('c')
host_sys = host_machine.system()

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
conf.set_quoted(
  'PACKAGE_URL',
  'https://theimpossibleastronaut.github.io/rmw-website/',
)
conf.set_quoted(
  'PACKAGE_BUGREPORT',
  'https://github.com/theimpossibleastronaut/rmw/issues',
)

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
  version: '>=0.3.2',
  fallback: ['canfigger', 'canfigger_dep'],
  default_options: 'default_library=static',
)

glib_dep = dependency('glib-2.0', version: '>=2.52')
gio_dep = dependency('gio-2.0', version: '>=2.52')

subdir('src')

main_bin = executable('rmw', 'src/main.c', dependencies: rmw_dep, install: true)

install_man('rmw.1')

# The man page is generated from MANUAL.md with go-md2man. To edit it, edit
# MANUAL.md and run `ninja -C <builddir> regen-man` to refresh rmw.1, then
# commit both. Normal builds just install the committed rmw.1, so go-md2man is
# not a build requirement (distros without it are unaffected).
go_md2man = find_program('go-md2man', required: false)
if go_md2man.found()
  run_target(
    'regen-man',
    command: [
      go_md2man,
      '-in',
      meson.project_source_root() / 'MANUAL.md',
      '-out',
      meson.project_source_root() / 'rmw.1',
    ],
  )
endif

if get_option('build_tests')
  subdir('test')
endif

install_data(
  files(
    'AUTHORS.md',
    'COPYING',
    'ChangeLog',
    'README.md',
    'rmwrc.example',
  ),
  install_dir: get_option('docdir'),
)

# Third-party licenses
install_subdir('licenses', install_dir: get_option('docdir'))

# Shell completions
if get_option('install_shell_completions')
  install_data(
    'completions/fish/rmw.fish',
    install_dir: get_option('datadir') / 'fish' / 'vendor_completions.d',
  )
endif
