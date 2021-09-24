
include(${TOP_DIR}/engine/CMakeLists.txt)

include_directories(
        ${GTEST_DIRS}/include
        ${MOCKCPP_DIR}/include
)


if (MINGW)
include_directories(
        ${TOP_DIR}/engine/demo/windows/ffmpeg/include
        ${TOP_DIR}/engine/demo/windows/SDL2.0/include
)
link_directories(
        ${TOP_DIR}/engine/demo/windows/ffmpeg/lib
        ${TOP_DIR}/engine/demo/windows/SDL2.0/lib/x64
)
else()
include_directories(
)
endif()

file(GLOB UT_TEST_SRCS ./*.cpp)

set(SRC
        ${HISTREAMER_SRCS}
        ${UT_TEST_SRCS}
        ../main.cpp
        )

add_executable(MediaPlayerUtTests ${SRC})

link_directories(
        ${GTEST_DIRS}/lib/
        ${MOCKCPP_DIR}/lib/
        /usr/local/lib
        ${TOP_DIR}/engine/demo/linux/ffmpeg/lib
        ${TOP_DIR}/engine/demo/linux/SDL2.0/lib
)

target_compile_definitions(MediaPlayerUtTests PRIVATE UNIT_TEST)

if (MSVC)
    target_link_libraries(MediaPlayerUtTests
            ${GTEST_DIRS}/lib/gtestd.lib
            ${MOCKCPP_DIR}/lib/mockcpp.lib
            pthreadVC2.lib
            )
elseif (MINGW)
    set(ffmpeg_path ${TOP_DIR}/engine/demo/windows/ffmpeg)
    set(sdl_path ${TOP_DIR}/engine/demo/windows/SDL2.0)
    target_link_libraries(MediaPlayerUtTests
            # dl
            ${sdl_path}/lib/x64/SDL2.lib
            ${ffmpeg_path}/lib/avcodec.lib
            ${ffmpeg_path}/lib/swresample.lib
            ${ffmpeg_path}/lib/avformat.lib
            ${ffmpeg_path}/lib/avutil.lib
            ${ffmpeg_path}/lib/avdevice.lib
            ${ffmpeg_path}/lib/avfilter.lib
            ${ffmpeg_path}/lib/swscale.lib
            ${GTEST_DIRS}/lib/libgtest.a
            ${MOCKCPP_DIR}/lib/libmockcpp_mingw8.a
            )
    message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
    file(GLOB ffmpeg_shared_libraries ${ffmpeg_path}/bin/*.dll)
    file(GLOB sdl_shared_libraries ${sdl_path}/lib/x64/*.dll)
    file(COPY ${ffmpeg_shared_libraries} DESTINATION ${CMAKE_BINARY_DIR}/tests/ut)
    file(COPY ${sdl_shared_libraries} DESTINATION ${CMAKE_BINARY_DIR}/tests/ut)
else ()
    set(ffmpeg_path ${TOP_DIR}/engine/demo/linux/ffmpeg)
    set(sdl_path ${TOP_DIR}/engine/demo/linux/SDL2.0)
    target_link_libraries(MediaPlayerUtTests
            dl
            ${ffmpeg_path}/lib/libavformat.a
            ${ffmpeg_path}/lib/libavcodec.a
            ${ffmpeg_path}/lib/libavdevice.a
            ${ffmpeg_path}/lib/libavfilter.a
            ${ffmpeg_path}/lib/libavutil.a
            ${ffmpeg_path}/lib/libswscale.a
            ${ffmpeg_path}/lib/libswresample.a
            ${ffmpeg_path}/lib/liblzma.a
            m
            z
            ${sdl_path}/lib/libSDL2.a
            ${GTEST_DIRS}/lib/libgtest.a
            pthread
            ${MOCKCPP_DIR}/lib/libmockcpp.a
            )
endif ()
add_test(Test MediaPlayerUtTests)
enable_testing()
