# The Simpsons Arcade - ReXGlue Static Recompilation Project
cmake_minimum_required(VERSION 3.25)
project(simpsons LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find ReXGlue SDK
if(DEFINED ENV{REXSDK})
    set(REXSDK_PATH "$ENV{REXSDK}")
else()
    set(REXSDK_PATH "${CMAKE_SOURCE_DIR}/../tools/rexglue-sdk/out/install/win-amd64")
endif()
list(APPEND CMAKE_PREFIX_PATH "${REXSDK_PATH}")

find_package(rexglue REQUIRED)

# Include generated source list
include(${CMAKE_SOURCE_DIR}/../generated/sources.cmake)

# Platform entry point from SDK
if(WIN32)
    set(ENTRY_POINT_SRC "${REXSDK_PATH}/share/rexglue/windowed_app_main_win.cpp")
else()
    set(ENTRY_POINT_SRC "${REXSDK_PATH}/share/rexglue/windowed_app_main_posix.cpp")
endif()

# Main executable
if(WIN32)
    add_executable(simpsons WIN32
        src/main.cpp
        src/stubs.cpp
        src/simpsons_settings.cpp
        src/simpsons_menu.cpp
        src/keyboard_driver.cpp
        ${ENTRY_POINT_SRC}
        ${GENERATED_SOURCES}
    )
else()
    add_executable(simpsons
        src/main.cpp
        src/stubs.cpp
        src/simpsons_settings.cpp
        src/simpsons_menu.cpp
        src/keyboard_driver.cpp
        ${ENTRY_POINT_SRC}
        ${GENERATED_SOURCES}
    )
endif()

target_include_directories(simpsons PRIVATE
    ${CMAKE_SOURCE_DIR}/../generated
    ${CMAKE_SOURCE_DIR}/../ppc
    ${REXSDK_PATH}/include/simde
)

target_link_libraries(simpsons PRIVATE
    rex::core
    rex::runtime
    rex::kernel
    rex::graphics
    rex::ui
)

target_compile_definitions(simpsons PRIVATE PPC_INCLUDE_DETAIL)

target_compile_options(simpsons PRIVATE
    -fno-strict-aliasing
    -ffp-model=strict
    -fno-char8_t
    $<$<CONFIG:DEBUG>:-g>
    $<$<CONFIG:DEBUG>:-O0>
    $<$<CONFIG:RELEASE>:-O3>
    $<$<CONFIG:RELEASE>:-DNDEBUG>
)

if(NOT MSVC)
    target_compile_options(simpsons PRIVATE -msse4.1)
endif()

# Console test executable (no GUI)
add_executable(simpsons_test
    src/test_boot.cpp
    src/stubs.cpp
    ${GENERATED_SOURCES}
)
target_include_directories(simpsons_test PRIVATE
    ${CMAKE_SOURCE_DIR}/../generated
    ${CMAKE_SOURCE_DIR}/../ppc
    ${REXSDK_PATH}/include/simde
)
target_link_libraries(simpsons_test PRIVATE
    rex::core
    rex::runtime
    rex::kernel
    rex::graphics
    rex::ui
)
target_compile_definitions(simpsons_test PRIVATE PPC_INCLUDE_DETAIL)

target_compile_options(simpsons_test PRIVATE
    -fno-strict-aliasing
    -ffp-model=strict
    -fno-char8_t
    $<$<CONFIG:RELEASE>:-O3>
    $<$<CONFIG:RELEASE>:-DNDEBUG>
)
if(NOT MSVC)
    target_compile_options(simpsons_test PRIVATE -msse4.1)
endif()

# Whole-archive link for kernel hooks (stubs defined in static lib need forced inclusion)
if(WIN32)
    target_link_options(simpsons PRIVATE "LINKER:/WHOLEARCHIVE:$<TARGET_FILE:rex::kernel>")
    target_link_options(simpsons_test PRIVATE "LINKER:/WHOLEARCHIVE:$<TARGET_FILE:rex::kernel>")
else()
    target_link_options(simpsons PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:rex::kernel> -Wl,--no-whole-archive
    )
    target_compile_options(simpsons PRIVATE -mcmodel=large)
endif()
