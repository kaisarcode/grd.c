cmake_minimum_required(VERSION 3.14)
project(grd C)

set(CMAKE_C_STANDARD 11)

# Output binary to the root directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Performance optimizations
option(GRD_NATIVE "Enable native architecture optimizations" OFF)

if(NOT MSVC)
    add_compile_options(-O3 -fno-finite-math-only)
    if(GRD_NATIVE)
        add_compile_options(-march=native)
    endif()
    add_definitions(-D_GNU_SOURCE)
else()
    add_compile_options(/O2)
endif()

# Main executable
add_executable(grd
    grd.c
    libgrd.c
)

# Include directories
target_include_directories(grd PRIVATE .)

# System libraries
if(UNIX)
    target_link_libraries(grd m)
endif()
