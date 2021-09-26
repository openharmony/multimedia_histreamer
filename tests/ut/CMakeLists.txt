
include(${TOP_DIR}/histreamer/engine/CMakeLists.txt)

set(GTEST_DIRS ${TOP_DIR}/3rdparty/gtest)
set(MOCKCPP_DIR ${TOP_DIR}/3rdparty/mockcpp)

include_directories(
        ${GTEST_DIRS}/include
        ${MOCKCPP_DIR}/include
)

set(ffmpeg_inc_path ${TOP_DIR}/3rdparty/ffmpeg/windows/include)
set(sdl_inc_path ${TOP_DIR}/3rdparty/SDL2.0/include)

if (MINGW)
set(ffmpeg_lib_path ${TOP_DIR}/3rdparty/ffmpeg/windows/lib)
set(sdl_lib_path ${TOP_DIR}/3rdparty/SDL2.0/windows/lib/x64)
set(gtest_lib_path ${GTEST_DIRS}/mingw64/lib/)
else()
set(ffmpeg_lib_path ${TOP_DIR}/3rdparty/ffmpeg/linux/lib)
set(sdl_lib_path ${TOP_DIR}/3rdparty/SDL2.0/linux/lib)
set(gtest_lib_path ${GTEST_DIRS}/linux/lib/)
endif()

include_directories(
        ${ffmpeg_inc_path}
        ${sdl_inc_path}
)
link_directories(
        ${ffmpeg_lib_path}
        ${sdl_lib_path}
        ${gtest_lib_path}
)

file(GLOB UT_TEST_SRCS ./*.cpp)

set(SRC
        ${HISTREAMER_SRCS}
        ${UT_TEST_SRCS}
        ${3RDPARTY_SRCS}
        ../main.cpp
        )

add_executable(HiStreamerUtTests ${SRC})

link_directories(
        ${MOCKCPP_DIR}/lib/
        /usr/local/lib
)

target_compile_definitions(HiStreamerUtTests PRIVATE UNIT_TEST)

if (MSVC)
    target_link_libraries(HiStreamerUtTests
            ${GTEST_DIRS}/lib/gtestd.lib
            ${MOCKCPP_DIR}/lib/mockcpp.lib
            pthreadVC2.lib
            )
elseif (MINGW)
    set(ffmpeg_path ${TOP_DIR}/3rdparty/ffmpeg/windows)
    set(sdl_path ${TOP_DIR}/3rdparty/SDL2.0/windows)
    target_link_libraries(HiStreamerUtTests
            # dl
            ${sdl_path}/lib/x64/SDL2.lib
            ${ffmpeg_path}/lib/avcodec.lib
            ${ffmpeg_path}/lib/swresample.lib
            ${ffmpeg_path}/lib/avformat.lib
            ${ffmpeg_path}/lib/avutil.lib
            ${ffmpeg_path}/lib/avdevice.lib
            ${ffmpeg_path}/lib/avfilter.lib
            ${ffmpeg_path}/lib/swscale.lib
            gtest
            ${MOCKCPP_DIR}/lib/libmockcpp_mingw8.a
            )
    message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
    file(GLOB ffmpeg_shared_libraries ${ffmpeg_path}/bin/*.dll)
    file(GLOB sdl_shared_libraries ${sdl_path}/lib/x64/*.dll)
    file(COPY ${ffmpeg_shared_libraries} DESTINATION ${CMAKE_BINARY_DIR}/tests/ut)
    file(COPY ${sdl_shared_libraries} DESTINATION ${CMAKE_BINARY_DIR}/tests/ut)
else ()
    set(ffmpeg_path ${TOP_DIR}/3rdparty/ffmpeg/linux)
    set(sdl_path ${TOP_DIR}/3rdparty/SDL2.0/linux)
    target_link_libraries(HiStreamerUtTests
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
            #z
            /lib/x86_64-linux-gnu/libz.so.1
            ${sdl_path}/lib/libSDL2.a
            gtest
            pthread
            ${MOCKCPP_DIR}/lib/libmockcpp.a
            )
endif ()
add_test(Test HiStreamerUtTests)
enable_testing()
