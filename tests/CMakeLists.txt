
CMAKE_MINIMUM_REQUIRED(VERSION 3.13...3.20)

#PROJECT(mediapipeline_test)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

if(NOT MINGW)
    # support findSymbol, cmake >= 3.13
    # ADD_LINK_OPTIONS( -rdynamic)
endif()

ADD_SUBDIRECTORY(ut)
