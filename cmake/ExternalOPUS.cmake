# To suppress warnings for ExternalProject DOWNLOAD_EXTRACT_TIMESTAMP
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif ()

include(ExternalProject)

set(OPUS_DISABLE_INTRINSICS OFF)

get_filename_component(CMAKE_C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME)
if (CMAKE_C_COMPILER_NAME MATCHES "arm-webos-linux-gnueabi-.*")
    execute_process(COMMAND ${CMAKE_C_COMPILER} -v ERROR_VARIABLE GCC_WEBOS_INFO OUTPUT_QUIET)
    if (NOT GCC_WEBOS_INFO MATCHES "--with-fpu=neon-fp16")
        message(STATUS "Disable intrinsics as this toolchain doesn't support NEON")
        set(OPUS_DISABLE_INTRINSICS ON)
    endif()
endif ()

set(EXT_OPUS_TOOLCHAIN_ARGS)
if (CMAKE_TOOLCHAIN_FILE)
    list(APPEND EXT_OPUS_TOOLCHAIN_ARGS "-DCMAKE_TOOLCHAIN_FILE:string=${CMAKE_TOOLCHAIN_FILE}")
endif ()
if (CMAKE_TOOLCHAIN_ARGS)
    list(APPEND EXT_OPUS_TOOLCHAIN_ARGS "-DCMAKE_TOOLCHAIN_ARGS:string=${CMAKE_TOOLCHAIN_ARGS}")
endif ()

set(LIB_FILENAME "${CMAKE_SHARED_LIBRARY_PREFIX}opus${CMAKE_SHARED_LIBRARY_SUFFIX}")

ExternalProject_Add(ext_opus
        URL https://downloads.xiph.org/releases/opus/opus-1.4.tar.gz
        URL_HASH SHA256=c9b32b4253be5ae63d1ff16eea06b94b5f0f2951b7a02aceef58e3a3ce49c51f
        CMAKE_ARGS ${EXT_OPUS_TOOLCHAIN_ARGS}
        -DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        -DOPUS_BUILD_SHARED_LIBRARY=ON
        -DOPUS_DISABLE_INTRINSICS=${OPUS_DISABLE_INTRINSICS}
        -DOPUS_INSTALL_PKG_CONFIG_MODULE=OFF
        -DOPUS_INSTALL_CMAKE_CONFIG_MODULE=OFF
        BUILD_BYPRODUCTS <INSTALL_DIR>/lib/${LIB_FILENAME}
        )
ExternalProject_Get_Property(ext_opus INSTALL_DIR)

add_library(ext_opus_target SHARED IMPORTED)
set_target_properties(ext_opus_target PROPERTIES IMPORTED_LOCATION ${INSTALL_DIR}/lib/${LIB_FILENAME})

add_dependencies(ext_opus_target ext_opus)

set(OPUS_INCLUDE_DIRS ${INSTALL_DIR}/include/opus)
set(OPUS_LIBRARIES ext_opus_target)
set(OPUS_FOUND TRUE)

if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib)
endif ()

install(DIRECTORY ${INSTALL_DIR}/lib/ DESTINATION ${CMAKE_INSTALL_LIBDIR})
