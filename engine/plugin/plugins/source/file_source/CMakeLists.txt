
CMAKE_MINIMUM_REQUIRED(VERSION 3.13...3.20)

project(plugin_filesource)

include_directories(
        ${TOP_DIR}/histreamer/engine
        ${TOP_DIR}/histreamer/engine/plugin
)

set(BUILD_PLUGIN_FILE_SOURCE_SHARED ON)

if (BUILD_PLUGIN_FILE_SOURCE_SHARED)
    add_library(plugin_filesource SHARED ./file_source_plugin.cpp ../../../common/plugin_buffer.cpp)
else()
    add_library(plugin_filesource STATIC ./file_source_plugin.cpp ../../../common/plugin_buffer.cpp)
endif()

message("------------------ BUILD plugin_filesource ------------------")
install(TARGETS plugin_filesource DESTINATION "${CMAKE_BINARY_DIR}/bin/plugins/")
