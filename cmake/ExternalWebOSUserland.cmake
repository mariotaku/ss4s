# To suppress warnings for ExternalProject DOWNLOAD_EXTRACT_TIMESTAMP
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif ()

message(STATUS "Using external webos-userland")

if (NOT TARGET ext_webos_userland)
    include(ExternalProject)

    set(EXT_WEBOS_USERLAND_TOOLCHAIN_ARGS)
    if (CMAKE_TOOLCHAIN_FILE)
        list(APPEND EXT_WEBOS_USERLAND_TOOLCHAIN_ARGS "-DCMAKE_TOOLCHAIN_FILE:string=${CMAKE_TOOLCHAIN_FILE}")
    endif ()
    if (CMAKE_TOOLCHAIN_ARGS)
        list(APPEND EXT_WEBOS_USERLAND_TOOLCHAIN_ARGS "-DCMAKE_TOOLCHAIN_ARGS:string=${CMAKE_TOOLCHAIN_ARGS}")
    endif ()

    ExternalProject_Add(ext_webos_userland
            URL https://github.com/sundermann/webos-userland/archive/refs/heads/main.zip
            CMAKE_ARGS ${EXT_WEBOS_USERLAND_TOOLCHAIN_ARGS}
            -DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            )
endif ()

ExternalProject_Get_Property(ext_webos_userland INSTALL_DIR)

macro(add_webos_userland_library)
    cmake_parse_arguments(_ADD "" "PREFIX;SONAME" "INCLUDES" ${ARGN})
    string(TOLOWER "${_ADD_PREFIX}" _ADD_PREFIX_LOWER)
    set(_ADD_TARGET "${_ADD_PREFIX_LOWER}_target")
    add_library(${_ADD_TARGET} SHARED IMPORTED)
    set_target_properties(${_ADD_TARGET} PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/${_ADD_SONAME}")

    add_dependencies(${_ADD_TARGET} ext_webos_userland)

    if (_ADD_INCLUDES)
        list(TRANSFORM _ADD_INCLUDES PREPEND "${INSTALL_DIR}/include/")
        set(${_ADD_PREFIX}_INCLUDE_DIRS ${_ADD_INCLUDES})
    else ()
        set(${_ADD_PREFIX}_INCLUDE_DIRS ${INSTALL_DIR}/include)
    endif ()
    set(${_ADD_PREFIX}_INCLUDEDIR ${INSTALL_DIR}/include)
    set(${_ADD_PREFIX}_LIBRARY ${_ADD_TARGET})
    set(${_ADD_PREFIX}_LIBRARIES ${_ADD_TARGET})
endmacro()

add_webos_userland_library(PREFIX LGNCOPENAPI SONAME liblgncopenapi.so INCLUDES lgncopenapi)
add_webos_userland_library(PREFIX NDL_DIRECTMEDIA SONAME libNDL_directmedia.so INCLUDES libndl-media)
add_webos_userland_library(PREFIX LIBPLAYERAPIS SONAME libplayerAPIs.so INCLUDES starfish-media-pipeline)
