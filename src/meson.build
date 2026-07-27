subdir('bsdutils')
disable_curses = ''
dep_menu = []
dep_curses = []
if not get_option('without-curses')
  # Meson's curses dependency has a SYSTEM method of its own, so this resolves
  # even where no pkg-config files exist for it.
  dep_curses = dependency('curses')

  # The menu library ships with ncurses but is separate to the linker, and meson
  # has no menu dependency, so it has to be looked for by hand. The order is
  # deliberate:
  #
  #   menuw via pkg-config    matches the wide curses meson prefers, and is the
  #                           only route that finds a keg-only Homebrew ncurses
  #   the plain library names  need no pkg-config data at all; OpenBSD shipped
  #                           none for these before 2025, and its single libmenu
  #                           is already wide
  #   dependency('menu') last  menu.pc names the narrow library in
  #                           Requires.private, so it can pull a second curses
  #                           into the link (mesonbuild/meson#8867)
  dep_menu = dependency('menuw', required: false)
  if not dep_menu.found()
    dep_menu = cc.find_library('menuw', required: false)
  endif
  if not dep_menu.found()
    dep_menu = cc.find_library('menu', required: false)
  endif
  if not dep_menu.found()
    dep_menu = dependency('menu', required: false)
  endif
  if not dep_menu.found()
    conf.set('DISABLE_CURSES', 1)
  endif
else
  conf.set('DISABLE_CURSES', 1)
endif

# Probe each header against the curses that was actually found, rather than just
# for the file being present. The HAVE_* names below are borrowed from
# AX_WITH_CURSES, an autoconf-archive m4 macro that this project does not use and
# does not ship -- only its define names were taken:
#   https://gitweb.git.savannah.gnu.org/gitweb/?p=autoconf-archive.git;a=blob_plain;f=m4/ax_with_curses.m4
# There each one is set by a link test against the library the macro selected, so
# it means "this header works with our curses". Passing dep_curses keeps that
# meaning here. It is empty when curses is disabled, so that case is unchanged.
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
  if cc.has_header(h[0], dependencies: dep_curses)
    conf.set(h[1], 1)
  endif
endforeach

src = [
  'globals.c',
  'ficlone.c',
  'restore.c',
  'config_rmw.c',
  'parse_cli_options.c',
  'strings_rmw.c',
  'purging.c',
  'messages.c',
  'time_rmw.c',
  'topdir_trash.c',
  'trashinfo.c',
  'utils.c',
]

# Used for FICLONE support
has_statfs = false
has_linux_fs_header = false
if host_sys == 'linux'
  if get_option('with-reflink')
    has_statfs = cc.has_function(
      'statfs',
      prefix: '#include <sys/statfs.h>',
    )
    has_linux_fs_header = cc.has_header('linux/fs.h')
    if has_statfs and has_linux_fs_header
      conf.set('HAVE_FICLONE', 1)
    else
      error(
        '''

  : Requirements not met for reflink clone support.
  : If missing linux/fs.h, you probably need to install the linux-headers package.
  : To build without reflink clone support and skip this check, add
  : "-Dwith-reflink=false" to the meson setup options.''',
      )
    endif
  endif
endif

config_h = configure_file(output: 'config.h', configuration: conf)

gio_unix_dep = dependency('gio-unix-2.0')

deps_librmw = [
  dep_intl,
  canfigger_dep,
  dep_menu,
  dep_curses,
  glib_dep,
  gio_dep,
  gio_unix_dep,
]

lib_rmw = static_library(
  'rmw',
  src,
  include_directories: include_directories(['.', inc_intl_arg]),
  dependencies: deps_librmw,
)

rmw_dep = declare_dependency(
  link_with: [lib_rmw, lib_bsdutilsrm],
  include_directories: include_directories(['.', inc_intl_arg]),
  dependencies: deps_librmw,
)
