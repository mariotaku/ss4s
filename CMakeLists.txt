cmake_minimum_required(VERSION 3.6)
project(ss4s C)

set(CMAKE_C_STANDARD 11)

option(SS4S_SANITIZE_ADDRESS "Link Address Sanitizer" OFF)

if (SS4S_SANITIZE_ADDRESS)
    add_compile_definitions(SS4S_SANITIZE_ADDRESS)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address -static-libasan)
    link_libraries(asan)
endif ()

include(CheckIncludeFile)

check_include_file("dlfcn.h" HAS_DLFCN)

add_library(ss4s src/library.c src/driver.c src/player.c src/audio.c src/video.c)
target_include_directories(ss4s PUBLIC include PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ss4s PRIVATE ss4s-module-api)

if (HAS_DLFCN)
    target_sources(ss4s PRIVATE src/module_libdl.c)
    target_link_libraries(ss4s PRIVATE dl)
endif ()

add_subdirectory(modules)

set_target_properties(ss4s PROPERTIES SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY ${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY})

# Don't include tests by default if used as library
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    enable_testing()
    add_subdirectory(tests)
endif ()