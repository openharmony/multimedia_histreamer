
CMAKE_MINIMUM_REQUIRED(VERSION 3.13...3.20)

project(histreamer)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else ()
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif ()

set(CMAKE_CXX_STANDARD 11)

set(TOP_DIR ${CMAKE_SOURCE_DIR})

add_definitions(-DOSAL_OHOS) # add -DVIDEO_SUPPORT to enable video play

set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(engine/plugin/plugins/source/file_source)
add_subdirectory(tests)
