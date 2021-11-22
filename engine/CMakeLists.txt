
IF(MSVC)
    ADD_DEFINITIONS(-DMSVC_VMG_ENABLED /std:c++11 )  #  /Z7)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /vmg")
ENDIF(MSVC)

#IF(UNIX)
#MESSAGE("UNIX")
#ADD_DEFINITIONS(
#    -g -ggdb
#)
#ENDIF(UNIX)

IF(UNIX AND NOT CYGWIN)
ADD_DEFINITIONS(
    -fPIC -DPROFILE
)
#set(CMAKE_CXX_FLAGS "-g -ggdb3 -O0 -std=c++11 -Wall -Wl,--disable-new-dtags")
ENDIF(UNIX AND NOT CYGWIN)

ADD_DEFINITIONS(
    -std=c++11 -D__STDC_FORMAT_MACROS
)

IF(CMAKE_CL_64)
    ADD_DEFINITIONS(-DWIN64)
ENDIF(CMAKE_CL_64)


######################################################
# include directories
include_directories(
        ${TOP_DIR}/interface
        ${TOP_DIR}/engine
        ${TOP_DIR}/engine/external
        ${TOP_DIR}/engine/player
        ${TOP_DIR}/engine/pipeline
        ${TOP_DIR}/engine/pipeline/core
        ${TOP_DIR}/engine/foundation
        ${TOP_DIR}/engine/foundation/osal
        ${TOP_DIR}/engine/utils
        ${TOP_DIR}/engine/pipeline/filters
        ${TOP_DIR}/engine/pipeline/filters/player
        ${TOP_DIR}/engine/plugin
)

######################################################
#file(GLOB_RECURSE MY_SRCS ./*.cpp)  # source files from all subdirectories recursely

file(GLOB_RECURSE HISTREAMER_SRCS
        ${TOP_DIR}/engine/pipeline/*.cpp
        ${TOP_DIR}/engine/plugin/common/*.cpp
        ${TOP_DIR}/engine/plugin/core/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/ffmpeg_adapter/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/sink/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/source/file_source/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/source/stream_source/*.cpp
        ${TOP_DIR}/engine/plugin/types/*.cpp
        ${TOP_DIR}/engine/player/*.cpp
        ${TOP_DIR}/engine/foundation/*.cpp
        ${TOP_DIR}/engine/utils/*.cpp
        ${TOP_DIR}/engine/external/*.cpp
        )

file(GLOB_RECURSE PLUGINS_STATIC_BUILD_SRCS
        ${TOP_DIR}/engine/plugin/plugins/source/file_source/*.cpp
        )

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

link_directories(
        ${ffmpeg_lib_path}
        ${sdl_lib_path}
)

if (WIN32)
    link_libraries(
            pthread
            m
            avcodec
            avformat
            avutil
            avfilter
            swresample
            SDL2
    )
elseif (LINUX_DEMO)
else ()
    link_libraries(
            log
            FFmpeg
            SDL2
    )
endif ()

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_library(${PROJECT_NAME} SHARED ${HISTREAMER_SRCS} ${PLUGINS_STATIC_BUILD_SRCS}
       ${3RDPARTY_SRCS})
