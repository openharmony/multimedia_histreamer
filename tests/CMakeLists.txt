CMAKE_MINIMUM_REQUIRED(VERSION 3.13...3.20)

PROJECT(histreamer_tests)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Must comment out these two lines to compile standard player
# But must uncomment them to compile histreamer_tests
#unset(PLAYER_LITE CACHE)
#option(PLAYER_LITE "tests should use player lite interface" ON)

if (NOT MINGW)
    # support findSymbol, cmake >= 3.13
    # ADD_LINK_OPTIONS( -rdynamic)
endif ()

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin) 

ADD_SUBDIRECTORY(ut)
