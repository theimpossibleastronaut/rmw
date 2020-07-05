cmake_minimum_required(VERSION 3.2)

SET(ProjectName rmw)
project(${ProjectName} VERSION 0.7.06.1 HOMEPAGE_URL "https://remove-to-waste.info/")

SET(ProjectName rmw)

SET(BIN_OUTPUT ${ProjectName})

set(CMAKE_C_FLAGS "-Wall -O0")

# For gettext, where translation files get installed to
add_compile_definitions(LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

# process CMakeLists.txt in the src subdirectory
ADD_SUBDIRECTORY( ${CMAKE_SOURCE_DIR}/src )

# writes the config.h file
configure_file(config.h.in ${CMAKE_BINARY_DIR}/src/config.h)
