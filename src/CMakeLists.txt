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

# module needed for the check_include_function
include(CheckIncludeFile)
# if these are found, the VARIABLE will be set to true and defined in config.h
check_include_file("ncursesw/menu.h" HAVE_NCURSESW_MENU_H)
check_include_file("ncurses/menu.h" HAVE_NCURSES_MENU_H)
check_include_file("menu.h" HAVE_MENU_H)

check_include_file(ncursesw/curses.h HAVE_NCURSESW_CURSES_H)
check_include_file(ncursesw.h HAVE_NCURSESW_H)
check_include_file(ncurses/curses.h HAVE_NCURSES_CURSES_H)
check_include_file(ncurses.h HAVE_NCURSES_H)
check_include_file(curses.h HAVE_CURSES_H)

# module needed for the pkg_check_modules function
include(FindPkgConfig)
# uses pkg-config to find the correct linker flags.
# NCURSES_LDFLAGS (used for target_link... below) will be set to whatever would be output by `pkg-config --libs ncurses`
pkg_check_modules(NCURSES ncurses)
# MENU_LDFLAGS will be set to whatever would be output by `pkg-config --libs menu`
pkg_check_modules(MENU menu)

include(CheckFunctionExists)
# if the function works, the VARIABLE will be set to true in config.h
check_function_exists(gettext HAVE_GETTEXT)
check_function_exists(dcgettext HAVE_DCGETTEXT)

find_package(Intl)
if(Intl_FOUND)
  set(EXTERNAL_LIBS ${EXTERNAL_LIBS} ${Intl_LIBRARIES})
  set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${Intl_INCLUDE_DIR})
endif()

find_package(Gettext)

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src ${INCLUDE_DIRECTORIES})

target_link_libraries (${BIN_OUTPUT} ${EXTERNAL_LIBS} ${MENU_LDFLAGS} ${NCURSES_LDFLAGS})
