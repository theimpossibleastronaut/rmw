subdir('bsdutils')
disable_curses = ''
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
          conf.set('DISABLE_CURSES', 1)
        endif
      endif
    endif
  endif
else
  conf.set('DISABLE_CURSES', 1)
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

config_h = configure_file(output : 'config.h', configuration : conf)

deps_librmw = [
    dep_intl,
    canfigger_dep,
    dep_menu,
    dep_curses
    ]

lib_rmw = static_library(
  'rmw',
  src,
  include_directories : include_directories('.'),
  dependencies: deps_librmw
  )

rmw_dep = declare_dependency(
  link_with: [ lib_rmw, lib_bsdutilsrm ],
  include_directories : include_directories('.'),
  dependencies: deps_librmw
  )
