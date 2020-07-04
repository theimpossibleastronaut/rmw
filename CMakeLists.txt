cmake_minimum_required(VERSION 3.2)

SET(ProjectName rmw)
project(${ProjectName} VERSION 0.7.06.1)

SET(ProjectName rmw)

SET(BIN_OUTPUT ${ProjectName})

set(CMAKE_C_FLAGS "-Wall -O0 -DLOCALEDIR=${CMAKE_INSTALL_LOCALEDIR}")

ADD_SUBDIRECTORY( ${CMAKE_SOURCE_DIR}/src )
configure_file(config.h.in ${CMAKE_BINARY_DIR}/src/config.h)
