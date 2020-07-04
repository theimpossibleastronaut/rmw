add_executable (${BIN_OUTPUT} globals.c
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

SET (CURSES_NEED_NCURSES TRUE)
SET (CURSES_NEED_WIDE TRUE)
find_package(Curses REQUIRED)
IF (CURSES_FOUND)
  SET(EXTERNAL_LIBS ${EXTERNAL_LIBS} ${CURSES_LIBRARIES})
  SET(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${CURSES_INCLUDE_PATH})
ELSE()
  message (FATAL_ERROR "curses library not found")
ENDIF()

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src ${INCLUDE_DIRECTORIES})

target_link_libraries (${BIN_OUTPUT} ${EXTERNAL_LIBS})
