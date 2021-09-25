
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
    -fPIC
)
ENDIF(UNIX AND NOT CYGWIN)

ADD_DEFINITIONS(
    -std=c++11
)

IF(CMAKE_CL_64)
    ADD_DEFINITIONS(-DWIN64)
ENDIF(CMAKE_CL_64)


######################################################
# include directories
include_directories(
        ${TOP_DIR}/histreamer/interface
        ${TOP_DIR}/histreamer/engine
        ${TOP_DIR}/histreamer/engine/external
        ${TOP_DIR}/histreamer/engine/player
        ${TOP_DIR}/histreamer/engine/pipeline
        ${TOP_DIR}/histreamer/engine/pipeline/core
        ${TOP_DIR}/histreamer/engine/foundation
        ${TOP_DIR}/histreamer/engine/foundation/osal
        ${TOP_DIR}/histreamer/engine/pipeline/filters
        ${TOP_DIR}/histreamer/engine/pipeline/filters/player
        ${TOP_DIR}/histreamer/engine/plugin
        ${TOP_DIR}/histreamer/engine/3rdparty/curl/include
)

######################################################
#file(GLOB_RECURSE MY_SRCS ./*.cpp)  # source files from all subdirectories recursely

file(GLOB_RECURSE HISTREAMER_SRCS
        ${TOP_DIR}/histreamer/engine/pipeline/*.cpp
        ${TOP_DIR}/histreamer/engine/plugin/common/*.cpp
        ${TOP_DIR}/histreamer/engine/plugin/core/*.cpp
        ${TOP_DIR}/histreamer/engine/plugin/plugins/ffmpeg_adapter/*.cpp
        ${TOP_DIR}/histreamer/engine/plugin/plugins/sink/*.cpp
        ${TOP_DIR}/histreamer/engine/plugin/plugins/source/file_source/*.cpp
        ${TOP_DIR}/histreamer/engine/plugin/plugins/source/stream_source/*.cpp
        ${TOP_DIR}/histreamer/engine/plugin/types/*.cpp
        ${TOP_DIR}/histreamer/engine/player/*.cpp
        ${TOP_DIR}/histreamer/engine/foundation/*.cpp
        ${TOP_DIR}/histreamer/engine/external/*.cpp
        )

file(GLOB_RECURSE PLUGINS_STATIC_BUILD_SRCS
        ${TOP_DIR}/histreamer/engine/plugin/plugins/source/file_source/*.cpp
        )

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
