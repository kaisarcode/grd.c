cmake_minimum_required(VERSION 3.14)
project(grd C)

set(CMAKE_C_STANDARD 11)

if(NOT DEFINED GRD_BUILD_VERSION)
    set(GRD_BUILD_VERSION 0)
endif()

option(GRD_NATIVE "Enable native CPU optimizations" OFF)
option(GRD_BUILD_TESTS "Build grd tests" OFF)

set(GRD_TEST_EXTERNAL_ARTIFACTS OFF)
if(GRD_BUILD_TESTS AND GRD_TEST_SHARED_LIBRARY)
    set(GRD_TEST_EXTERNAL_ARTIFACTS ON)
endif()

if(NOT MSVC)
    add_compile_options(-O3 -Wall -Wextra -Werror -fno-finite-math-only)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        add_compile_options(-Wno-unused-command-line-argument)
    endif()
    add_definitions(-D_GNU_SOURCE)
    if(GRD_NATIVE)
        add_compile_options(-march=native)
    endif()
else()
    add_compile_options(/O2 /W3 /WX)
endif()

set(GRD_SOURCES src/libgrd.c)
set(GRD_HEADERS src/grd.h)

set_source_files_properties(src/libgrd.c PROPERTIES
    COMPILE_DEFINITIONS KC_GRD_BUILD_VERSION=${GRD_BUILD_VERSION}
)

if(NOT GRD_TEST_EXTERNAL_ARTIFACTS)
    add_library(grd_shared SHARED ${GRD_SOURCES})
    set_target_properties(grd_shared PROPERTIES OUTPUT_NAME grd PREFIX "lib")
    target_include_directories(grd_shared PRIVATE src)

    add_library(grd_static STATIC ${GRD_SOURCES})
    set_target_properties(grd_static PROPERTIES OUTPUT_NAME grd PREFIX "lib")
    target_include_directories(grd_static PRIVATE src)

    add_executable(grd_exe src/grd.c ${GRD_SOURCES})
    set_target_properties(grd_exe PROPERTIES OUTPUT_NAME grd)
    target_include_directories(grd_exe PRIVATE src)

    if(UNIX AND NOT ANDROID AND NOT APPLE)
        target_link_libraries(grd_shared m)
        target_link_libraries(grd_static m)
        target_link_libraries(grd_exe m)
    elseif(ANDROID)
        target_link_libraries(grd_shared m)
        target_link_libraries(grd_static m)
        target_link_libraries(grd_exe m)
    endif()
endif()

if(GRD_BUILD_TESTS)
    enable_testing()

    if(GRD_TEST_SHARED_LIBRARY)
        add_library(grd_test_shared SHARED IMPORTED)
        set_target_properties(grd_test_shared PROPERTIES
            IMPORTED_LOCATION "${GRD_TEST_SHARED_LIBRARY}"
        )
        if(GRD_TEST_IMPORT_LIBRARY)
            set_target_properties(grd_test_shared PROPERTIES
                IMPORTED_IMPLIB "${GRD_TEST_IMPORT_LIBRARY}"
            )
        endif()
    else()
        set(GRD_TEST_SHARED_TARGET grd_shared)
    endif()

    if(TARGET grd_test_shared)
        set(GRD_TEST_SHARED_TARGET grd_test_shared)
    endif()

    add_executable(grd_contract_test src/test.c)
    target_include_directories(grd_contract_test PRIVATE src)
    target_link_libraries(grd_contract_test PRIVATE ${GRD_TEST_SHARED_TARGET})

    if(UNIX AND NOT ANDROID AND NOT APPLE)
        target_link_libraries(grd_contract_test PRIVATE m)
    endif()

    add_custom_command(TARGET grd_contract_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${GRD_TEST_SHARED_TARGET}>
            $<TARGET_FILE_DIR:grd_contract_test>
    )

    add_test(NAME library-version COMMAND grd_contract_test version)
    add_test(NAME library-options-default COMMAND grd_contract_test options-default)
    add_test(NAME library-options-load-env COMMAND grd_contract_test options-load-env)
    add_test(NAME library-options-free COMMAND grd_contract_test options-free)
    add_test(NAME library-box-new COMMAND grd_contract_test box-new)
    add_test(NAME library-box-free COMMAND grd_contract_test box-free)
    add_test(NAME library-split-set COMMAND grd_contract_test split-set)
    add_test(NAME library-split-add COMMAND grd_contract_test split-add)
    add_test(NAME library-split-weight COMMAND grd_contract_test split-weight)
    add_test(NAME library-box-bounds COMMAND grd_contract_test box-bounds)
    add_test(NAME library-box-layout COMMAND grd_contract_test box-layout)
    add_test(NAME library-split-gap COMMAND grd_contract_test split-gap)
    add_test(NAME library-gap-hit COMMAND grd_contract_test gap-hit)
    add_test(NAME library-drag COMMAND grd_contract_test drag)
    add_test(NAME library-box-close COMMAND grd_contract_test box-close)
    add_test(NAME library-split-at COMMAND grd_contract_test split-at)
    add_test(NAME library-stop COMMAND grd_contract_test stop)
    add_test(NAME library-on-signal COMMAND grd_contract_test on-signal)
    add_test(NAME library-raise-signal COMMAND grd_contract_test raise-signal)
    add_test(NAME library-listen-signals COMMAND grd_contract_test listen-signals)
    add_test(NAME library-listen-signal COMMAND grd_contract_test listen-signal)
    add_test(NAME library-signal-listener COMMAND grd_contract_test signal-listener)
    add_test(NAME library-multictx COMMAND grd_contract_test multictx)
endif()
