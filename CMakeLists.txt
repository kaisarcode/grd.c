cmake_minimum_required(VERSION 3.14)
project(grd C)

set(CMAKE_C_STANDARD 11)

option(GRD_NATIVE "Enable native CPU optimizations" OFF)

if(NOT MSVC)
    add_compile_options(-O3 -Wall -Wextra -Werror -fno-finite-math-only)
    add_definitions(-D_GNU_SOURCE)
    if(GRD_NATIVE)
        add_compile_options(-march=native)
    endif()
else()
    add_compile_options(/O2 /W3 /WX)
endif()

set(GRD_SOURCES src/libgrd.c)
set(GRD_HEADERS src/grd.h)

add_library(grd_shared SHARED ${GRD_SOURCES})
set_target_properties(grd_shared PROPERTIES OUTPUT_NAME grd PREFIX "lib")
target_include_directories(grd_shared PRIVATE src)

add_library(grd_static STATIC ${GRD_SOURCES})
set_target_properties(grd_static PROPERTIES OUTPUT_NAME grd PREFIX "lib")
target_include_directories(grd_static PRIVATE src)

add_executable(grd_exe src/grd.c ${GRD_SOURCES})
set_target_properties(grd_exe PROPERTIES OUTPUT_NAME grd)
target_include_directories(grd_exe PRIVATE src)

if(UNIX AND NOT ANDROID)
    target_link_libraries(grd_shared m)
    target_link_libraries(grd_static m)
    target_link_libraries(grd_exe m)
elseif(ANDROID)
    target_link_libraries(grd_shared m)
    target_link_libraries(grd_static m)
    target_link_libraries(grd_exe m)
endif()
