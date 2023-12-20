
#include(${TOP_DIR}/src/CMakeLists.txt)

set(GTEST_DIRS ${THIRD_PARTY_DIR}/gtest)
set(MOCKCPP_DIR ${THIRD_PARTY_DIR}/mockcpp)

include_directories(
        ${GTEST_ROOT_DIR}/include
        ${MOCKCPP_DIR}/include
)

set(sdl_inc_path ${THIRD_PARTY_DIR}/SDL2.0/include)

if (MINGW)
set(sdl_lib_path ${THIRD_PARTY_DIR}/SDL2.0/windows/lib/x64)
set(gtest_lib_path ${GTEST_ROOT_DIR}/mingw64/lib/)
else()
    set(sdl_lib_path ${THIRD_PARTY_DIR}/SDL2.0/linux/lib)
    set(sdl_inc_path ${THIRD_PARTY_DIR}/SDL2.0/linux/include)
    set(gtest_lib_path ${THIRD_PARTY_DIR}/gtest/linux/lib)
    set(curl_lib_path ${THIRD_PARTY_DIR}/curl/lib/linux)
endif()

include_directories(
        ${sdl_inc_path}
)
link_directories(
        ${sdl_lib_path}
        ${gtest_lib_path}
        ${curl_lib_path}
)

file(GLOB_RECURSE FOUNDATION_SRCS
        ${TOP_DIR}/src/meta/*.cpp
        ${TOP_DIR}/../3rdparty/gtest/mingw64/src/gtest/src/hwext/*.cpp
        ${TOP_DIR}/../3rdparty/gtest/mingw64/src/gtest/src/hwext/*.cc
        )
ADD_DEFINITIONS(
        -DHST_ANY_WITH_NO_RTTI
)
#message(STATUS "ut ----------------")
#message(${FOUNDATION_SRCS})
#message(STATUS "ut ut ----------------")

file(GLOB NEW_UT_TEST_SRCS ./meta/*.cpp)

set(METATEST_SRCS
        ${FOUNDATION_SRCS}
        ${NEW_UT_TEST_SRCS}
        ${3RDPARTY_SRCS}
        ../main.cpp
        )

add_executable(histreamer_new_unittest ${METATEST_SRCS})

link_directories(
        ${MOCKCPP_DIR}/lib/
        /usr/local/lib
)

target_compile_definitions(histreamer_new_unittest PRIVATE NEW_UNIT_TEST)

if (MSVC)
    target_link_libraries(histreamer_new_unittest
            ${GTEST_ROOT_DIR}/lib/gtestd.lib
            ${MOCKCPP_DIR}/lib/mockcpp.lib
            pthreadVC2.lib
            )
elseif (MINGW)
    set(sdl_path ${THIRD_PARTY_DIR}/SDL2.0/windows)
    target_link_libraries(histreamer_new_unittest
            ${sdl_path}/lib/x64/SDL2.lib
            gtest
            ${MOCKCPP_DIR}/lib/libmockcpp_mingw8.a
            )
    message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
    file(GLOB sdl_shared_libraries ${sdl_path}/lib/x64/*.dll)
    file(COPY ${sdl_shared_libraries} DESTINATION ${CMAKE_BINARY_DIR}/_deps/histreamer-build/tests/unittest)
else ()
    target_link_libraries(media_foundation_unit_test
            dl
            m
            /usr/lib/x86_64-linux-gnu/libz.so.1
            SDL2-2.0
            gtest
            gtest_main
            pthread
            ${MOCKCPP_DIR}/lib/libmockcpp.a
            curl
            )
endif ()
add_test(Test histreamer_new_ut)
enable_testing()
