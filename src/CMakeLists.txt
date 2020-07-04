add_executable (${BIN_OUTPUT}
  globals.c
  main.c
  restore_rmw.c
  config_rmw.c
  parse_cli_options.c
  strings_rmw.c
  purging_rmw.c
  messages_rmw.c
  time_rmw.c
  trashinfo_rmw.c
  utils_rmw.c
  bst.c)

message(STATUS "Searching for curses")

include(CheckIncludeFile)
check_include_file("ncursesw/menu.h" HAVE_NCURSESW_MENU_H)
check_include_file("ncurses/menu.h" HAVE_NCURSES_MENU_H)
check_include_file("menu.h" HAVE_MENU_H)

check_include_file(ncursesw/curses.h HAVE_NCURSESW_CURSES_H)
check_include_file(ncursesw.h HAVE_NCURSESW_H)
check_include_file(ncurses/curses.h HAVE_NCURSES_CURSES_H)
check_include_file(ncurses.h HAVE_NCURSES_H)
check_include_file(curses.h HAVE_CURSES_H)

include(FindPkgConfig)
pkg_check_modules(NCURSES ncurses)
pkg_check_modules(MENU menu)

include(CheckFunctionExists)
check_function_exists(gettext HAVE_GETTEXT)

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src ${INCLUDE_DIRECTORIES})

target_link_libraries (${BIN_OUTPUT} ${EXTERNAL_LIBS} ${MENU_LDFLAGS} ${NCURSES_LDFLAGS})
