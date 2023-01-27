subdir('bsdutils')

dep_menu = []
dep_curses = []
if not get_option('without-curses')
  dep_curses = dependency('curses')
  dep_menu = dependency('menuw', required : false)
  if not dep_menu.found()
    dep_menu = dependency('menu', required : false)
    if not dep_menu.found()
      dep_menu = cc.find_library('menuw')
      if not dep_menu.found()
        dep_menu = cc.find_library('menu')
        if not dep_menu.found()
          add_project_arguments('-DDISABLE_CURSES', language : 'c')
        endif
      endif
    endif
  endif
else
  add_project_arguments('-DDISABLE_CURSES', language : 'c')
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
  'utils_rmw.c'
  ]

lib_rmw = static_library(
  'rmw',
  src,
  include_directories : include_directories('..'),
  dependencies: [
    dep_intl,
    dep_canfigger,
    dep_menu,
    dep_curses
    ]
  )

dep_rmw = declare_dependency(
  link_with: [ lib_rmw, lib_bsdutilsrm ],
  include_directories : include_directories('..')
  )
