
CMAKE_MINIMUM_REQUIRED(VERSION 3.13...3.20)

project(plugin_audiocapture)

include_directories(
        ${TOP_DIR}/engine
        ${TOP_DIR}/engine/foundation
        ${TOP_DIR}/engine/plugin
)

set(BUILD_PLUGIN_AUDIO_CAPTURE_SHARED ON)

if (BUILD_PLUGIN_AUDIO_CAPTURE_SHARED)
    add_library(plugin_audiocapture SHARED ./audio_capture_plugin.cpp ../../../common/plugin_buffer.cpp ../../../../utils/utils.cpp)
else()
    add_library(plugin_audiocapture STATIC ./audio_capture_plugin.cpp ../../../common/plugin_buffer.cpp)
endif()

message("------------------ BUILD plugin_audiocapture ------------------")
install(TARGETS plugin_audiocapture DESTINATION "${CMAKE_BINARY_DIR}/bin/plugins/")
