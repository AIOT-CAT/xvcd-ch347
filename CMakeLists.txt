cmake_minimum_required(VERSION 3.10)
project(xvcd_ch347 VERSION 1.2)

set(CMAKE_C_STANDARD 99)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/ch347_xvcd_api.c
    src/io_ch347.c
    src/xvcd.c
)

if(UNIX)
    set(TARGET_ARCH "Linux")
    find_library(NEED_LIBRARY libusb-1.0.so REQUIRED)
    link_directories(${NEED_LIBRARY})
    set(C_FLAG "")
elseif(WIN32)
    set(CH347_DIR ${CMAKE_SOURCE_DIR}/lib/windows)
    
    if(NOT CMAKE_GENERATOR_PLATFORM)
        set(CMAKE_GENERATOR_PLATFORM "x64")
    endif()
    
    if(CMAKE_GENERATOR_PLATFORM STREQUAL "arm64")
        set(TARGET_ARCH "arm64")
        set(NEED_LIBRARY ${CH347_DIR}/arm64/CH347DLLA64.lib)
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
        set(TARGET_ARCH "amd64")
        set(NEED_LIBRARY ${CH347_DIR}/amd64/CH347DLLA64.lib)
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "win32")
        set(TARGET_ARCH "i386")
        set(NEED_LIBRARY ${CH347_DIR}/i386/CH347DLL.lib)
    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_GENERATOR_PLATFORM}")
    endif()
    set(C_FLAG "ws2_32")
endif()

set(TARGET_NAME "xvcd_ch347_${TARGET_ARCH}")
add_executable(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME} ${C_FLAG} ${NEED_LIBRARY})


