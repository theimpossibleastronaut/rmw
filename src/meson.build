dep_curses = dependency('curses')
dep_menu = dependency('menuw', required : false)
if not dep_menu.found()
  dep_menu = dependency('menu', required : true)
endif

src = [
  'globals.c',
  'restore_rmw.c',
  'config_rmw.c',
  'parse_cli_options.c',
  'strings_rmw.c',
  'purging_rmw.c',
  'messages_rmw.c',
  'time_rmw.c',
  'trashinfo_rmw.c',
  'utils_rmw.c',
  'bst.c'
  ]

lib_rmw = static_library(
  'rmw',
  src,
  include_directories : '..',
  dependencies: [gettext_deps, dep_canfigger_static, dep_menu, dep_curses]
  )

dep_rmw = declare_dependency(link_with: lib_rmw, include_directories : '..')
