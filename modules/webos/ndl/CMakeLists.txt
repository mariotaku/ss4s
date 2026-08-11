find_package(PkgConfig REQUIRED)

include(CMakePushCheckState)
include(CheckCSourceCompiles)

if (NOT NDL_DIRECTMEDIA_FOUND)
    pkg_check_modules(NDL_DIRECTMEDIA NDL_directmedia REQUIRED)
endif ()

if (NOT LIBNDL_DIRECTMEDIA2_FOUND)
    pkg_check_modules(LIBNDL_DIRECTMEDIA2 libndl-directmedia2 QUIET)
endif ()

# The API has changed a few times: NDL_DirectVideoSetHDRInfo gained the VUI fields, NDL_DirectMediaInit
# lost its second argument, and the PCM sample rate enum was reordered. If the toolchain header predates
# any of that, we need to fetch the headers from webos-userland.
list(APPEND CMAKE_MESSAGE_INDENT "  ")
cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES ${NDL_DIRECTMEDIA_INCLUDE_DIRS})
set(CMAKE_REQUIRED_DEFINITIONS -DNDL_DIRECTMEDIA_API_VERSION=2)
set(CMAKE_REQUIRED_FLAGS "-Wno-attributes")
# compile only, the probe must not need libNDL_directmedia to link.
# cmake_push_check_state does not cover this one, so save it here.
set(NDL_SAVED_TRY_COMPILE_TARGET_TYPE ${CMAKE_TRY_COMPILE_TARGET_TYPE})
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
check_c_source_compiles("
#include <NDL_directmedia_v2.h>

_Static_assert(NDL_DIRECTMEDIA_AUDIO_PCM_SAMPLE_RATE_16KHZ == 5, \"PCM sample rate enum out of order\");

int main(void) {
    NDL_DIRECTVIDEO_HDR_INFO_T hdr = {0};
    hdr.transferCharacteristics = 16;
    return NDL_DirectMediaInit(\"com.example.app\") + NDL_DirectVideoSetHDRInfo(hdr);
}
" NDL_DIRECTMEDIA_HEADER_UP_TO_DATE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE ${NDL_SAVED_TRY_COMPILE_TARGET_TYPE})
unset(NDL_SAVED_TRY_COMPILE_TARGET_TYPE)
cmake_pop_check_state()
list(POP_BACK CMAKE_MESSAGE_INDENT)

if (NOT NDL_DIRECTMEDIA_HEADER_UP_TO_DATE)
    include(WebOSUserland)
    set(NDL_DIRECTMEDIA_INCLUDE_DIRS ${WEBOS_USERLAND_INCLUDE_DIR}/libndl-media)
endif ()

# Without mocks both versions come from the same system library
if (NOT NDL_DIRECTMEDIA_V1_LIBRARIES)
    set(NDL_DIRECTMEDIA_V1_LIBRARIES ${NDL_DIRECTMEDIA_LIBRARIES})
endif ()
if (NOT NDL_DIRECTMEDIA_V2_LIBRARIES)
    set(NDL_DIRECTMEDIA_V2_LIBRARIES ${NDL_DIRECTMEDIA_LIBRARIES})
endif ()

add_subdirectory(webos5)
add_subdirectory(webos4)

if (NOT LIBNDL_DIRECTMEDIA2_FOUND)
    add_library(ndl-directmedia2 SHARED fakelib/dummy.c)
    set_target_properties(ndl-directmedia2 PROPERTIES VERSION 1.0.0 SOVERSION 1)
    target_link_options(ndl-directmedia2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/fakelib/ndl-directmedia2.lds)

    include(WebOSUserland)
    set(NDL_DIRECTMEDIA2_LIBRARIES ndl-directmedia2)
    set(NDL_DIRECTMEDIA2_INCLUDE_DIRS ${WEBOS_USERLAND_INCLUDE_DIR})
endif ()

add_subdirectory(esplayer)