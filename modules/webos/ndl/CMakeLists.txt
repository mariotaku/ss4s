find_package(PkgConfig REQUIRED)

include(CMakePushCheckState)
include(CheckStructHasMember)

if (NOT NDL_DIRECTMEDIA_FOUND)
    pkg_check_modules(NDL_DIRECTMEDIA NDL_directmedia REQUIRED)
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