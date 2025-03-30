cmake_minimum_required(VERSION 3.10)
project(xvcd_ch347)

set(CMAKE_C_STANDARD 99)

set(SOURCES
    io_ch347.c
    xvcd_win.c
)

set(CH347_DIR ${CMAKE_SOURCE_DIR}/CH347)

include_directories(${CH347_DIR})

if(CMAKE_GENERATOR_PLATFORM STREQUAL "arm64")
    set(TARGET_ARCH "arm64")
    set(CH347_LIB_PATH ${CH347_DIR}/arm64/CH347DLLA64.lib)
elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
    set(TARGET_ARCH "amd64")
    set(CH347_LIB_PATH ${CH347_DIR}/amd64/CH347DLLA64.lib)
elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "win32")
    set(TARGET_ARCH "386")
    set(CH347_LIB_PATH ${CH347_DIR}/i386/CH347DLL.lib)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_GENERATOR_PLATFORM}")
endif()

set(TARGET_NAME "xvcd_ch347_${TARGET_ARCH}")

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} ws2_32 ${CH347_LIB_PATH})