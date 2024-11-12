find_package(PkgConfig REQUIRED)

include(CMakePushCheckState)
include(CheckStructHasMember)

if (NOT NDL_DIRECTMEDIA_FOUND)
    pkg_check_modules(NDL_DIRECTMEDIA NDL_directmedia REQUIRED)
endif ()

if (NOT LIBNDL_DIRECTMEDIA2_FOUND)
    pkg_check_modules(LIBNDL_DIRECTMEDIA2 libndl-directmedia2 QUIET)
endif ()

# NDL_DirectVideoSetHDRInfo has updated to include VUI. If the toolchain does not have the updated header file,
# we need to use the headers in mocks directory.
list(APPEND CMAKE_MESSAGE_INDENT "  ")
cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES ${NDL_DIRECTMEDIA_INCLUDE_DIRS})
check_struct_has_member("struct NDL_DIRECTVIDEO_HDR_INFO" transferCharacteristics NDL_directmedia_v2.h
        NDL_DIRECTVIDEO_HDR_INFO_HAS_VUI LANGUAGE C)
cmake_pop_check_state()
list(POP_BACK CMAKE_MESSAGE_INDENT)

if (NOT NDL_DIRECTVIDEO_HDR_INFO_HAS_VUI)
    set(NDL_DIRECTMEDIA_INCLUDE_DIRS ${SS4S_SYSTEM_LIBRARY_MOCKS}/include/libndl-media)
endif ()

add_subdirectory(webos5)
add_subdirectory(webos4)

if (NOT LIBNDL_DIRECTMEDIA2_FOUND)
    add_library(ndl-directmedia2 SHARED fakelib/dummy.c)
    set_target_properties(ndl-directmedia2 PROPERTIES VERSION 1.0.0 SOVERSION 1)
    target_link_options(ndl-directmedia2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/fakelib/ndl-directmedia2.lds)

    set(NDL_DIRECTMEDIA2_LIBRARIES ndl-directmedia2)
    set(NDL_DIRECTMEDIA2_INCLUDE_DIRS ${SS4S_SYSTEM_LIBRARY_MOCKS}/include)
endif ()

add_subdirectory(esplayer)