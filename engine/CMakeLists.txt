
IF (MSVC)
    ADD_DEFINITIONS(-DMSVC_VMG_ENABLED /std:c++11)  #  /Z7)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /vmg")
ENDIF (MSVC)

#IF(UNIX)
#MESSAGE("UNIX")
#ADD_DEFINITIONS(
#    -g -ggdb
#)
#ENDIF(UNIX)

IF (UNIX AND NOT CYGWIN)
    ADD_DEFINITIONS(
            -fPIC -DPROFILE
    )
    set(CMAKE_CXX_FLAGS "-g -ggdb3 -O0 -std=c++14 -Wall -Wl,--disable-new-dtags")
ENDIF (UNIX AND NOT CYGWIN)

ADD_DEFINITIONS(
        -D__STDC_FORMAT_MACROS
)

IF (CMAKE_CL_64)
    ADD_DEFINITIONS(-DWIN64)
ENDIF (CMAKE_CL_64)

######################################################
# include directories
include_directories(
        ${TOP_DIR}/interface
        ${TOP_DIR}/engine
        ${TOP_DIR}/engine/external
        ${TOP_DIR}/engine/player
        ${TOP_DIR}/engine/recorder
        ${TOP_DIR}/engine/pipeline
        ${TOP_DIR}/engine/pipeline/core
        ${TOP_DIR}/engine/foundation
        ${TOP_DIR}/engine/foundation/osal
        ${TOP_DIR}/engine/utils
        ${TOP_DIR}/engine/pipeline/filters
        ${TOP_DIR}/engine/pipeline/filters/player
        ${TOP_DIR}/engine/plugin
)

if (NOT ${PLAYER_LITE})
    message(STATUS "engine factory header included.")
    include_directories(
            ${TOP_DIR}/engine/factory
    )
endif ()

######################################################

file(GLOB_RECURSE HISTREAMER_BASE_SRCS
        ${TOP_DIR}/engine/pipeline/*.cpp
        ${TOP_DIR}/engine/plugin/common/*.cpp
        ${TOP_DIR}/engine/plugin/core/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/ffmpeg_adapter/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/sink/sdl/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/sink/file_sink/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/source/file_source/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/source/stream_source/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/source/audio_capture/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/source/http_lite_source/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/minimp3_adapter/*.cpp
        ${TOP_DIR}/engine/plugin/plugins/minimp3_adapter/*.c
        ${TOP_DIR}/engine/plugin/types/*.cpp
        ${TOP_DIR}/engine/player/internal/*.cpp
        ${TOP_DIR}/engine/recorder/internal/*.cpp
        ${TOP_DIR}/engine/foundation/*.cpp
        ${TOP_DIR}/engine/utils/*.cpp
        ${TOP_DIR}/engine/external/*.cpp
        )

if (PLAYER_LITE)
    message(STATUS "#1 player lite source")
    file(GLOB_RECURSE PLAYER_SRCS
            ${TOP_DIR}/engine/player/lite/*.cpp
            )
else ()
    message(STATUS "player standard source")
    file(GLOB_RECURSE PLAYER_SRCS
            ${TOP_DIR}/engine/player/standard/*.cpp
            ${TOP_DIR}/engine/factory/*.cpp
            )
endif ()

if (RECORDER_LITE)
    message(STATUS "#1 recorder lite source")
    file(GLOB_RECURSE RECORDER_SRCS
            ${TOP_DIR}/engine/recorder/lite/*.cpp
            )
else ()
    message(STATUS "recorder standard source")
    file(GLOB_RECURSE RECORDER_SRCS
            ${TOP_DIR}/engine/recorder/standard/*.cpp
            ${TOP_DIR}/engine/factory/*.cpp
            )
endif ()

set(HISTREAMER_SRCS ${HISTREAMER_BASE_SRCS} ${PLAYER_SRCS} ${RECORDER_SRCS})

file(GLOB_RECURSE PLUGINS_STATIC_BUILD_SRCS
        ${TOP_DIR}/engine/plugin/plugins/source/file_source/*.cpp
        )

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

link_directories(
        ${ffmpeg_lib_path}
        ${sdl_lib_path}
        ${TOP_DIR}/../3rdparty/curl/lib/windows
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
            curl
    )
elseif (LINUX_DEMO)
else ()
    link_libraries(
            log
            FFmpeg
            SDL2
    )
endif ()

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_SHARED_LIBRARY_PREFIX "libmedia_engine_")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".z.so")

add_library(${PROJECT_NAME} SHARED ${HISTREAMER_SRCS} ${PLUGINS_STATIC_BUILD_SRCS}
        ${3RDPARTY_SRCS})
