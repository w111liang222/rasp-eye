cmake_minimum_required (VERSION 2.6)
project (SERVER)
# The version number.
set (SERVER_VERSION_MAJOR 2)
set (SERVER_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/ServerConfig.h.in"
    "${PROJECT_BINARY_DIR}/ServerConfig.h"
 ) 

# add the binary tree to the search path for include files
# so that we will find Config.h
include_directories ("${PROJECT_BINARY_DIR}")

option (USE_WRAP
        "Use libWRAP" ON)

# add the WRAP library?
#
if (USE_WRAP)
    include_directories ("${PROJECT_SOURCE_DIR}/WRAP")
    add_subdirectory (WRAP)
    set (EXTRA_LIBS ${EXTRA_LIBS} WRAP)
endif (USE_WRAP)

# add the executable
add_executable (SERVER Server.c)
target_link_libraries (SERVER ${EXTRA_LIBS})

# add the install targets
install (TARGETS SERVER DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/ServerConfig.h" DESTINATION include)
