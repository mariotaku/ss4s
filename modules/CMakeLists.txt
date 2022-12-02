add_subdirectory(interface)

set(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY ${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY} PARENT_SCOPE)

function(ss4s_add_module NAME)
    cmake_parse_arguments(MODULE "" "" "SOURCES" ${ARGN})
    set(SS4S_MODULE_NAME "ss4s-module-${NAME}")
    add_library(${SS4S_MODULE_NAME} SHARED ${MODULE_SOURCES})
    target_link_libraries(${SS4S_MODULE_NAME} PRIVATE ss4s-module-api)
    set_target_properties(${SS4S_MODULE_NAME} PROPERTIES
            LIBRARY_OUTPUT_NAME "ss4s-${NAME}"
            LIBRARY_OUTPUT_DIRECTORY "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")
    set(SS4S_MODULE_NAME ${SS4S_MODULE_NAME} PARENT_SCOPE)
    add_dependencies(ss4s ${SS4S_MODULE_NAME})
    get_target_property(SS4S_MODULE_NAMES ss4s SS4S_MODULE_NAMES)
    if (NOT SS4S_MODULE_NAMES)
        set_target_properties(ss4s PROPERTIES SS4S_MODULE_NAMES "${SS4S_MODULE_NAME}")
    else ()
        set_target_properties(ss4s PROPERTIES SS4S_MODULE_NAMES "${SS4S_MODULE_NAMES};${SS4S_MODULE_NAME}")
    endif ()
    if (MSVC)
        target_compile_options(${SS4S_MODULE_NAME} PRIVATE /WX)
    else ()
        target_compile_options(${SS4S_MODULE_NAME} PRIVATE -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter)
    endif ()
endfunction()


add_subdirectory(alsa)
add_subdirectory(pulse)
add_subdirectory(sdl)
add_subdirectory(mmal)

add_subdirectory(ndl-webos4)
add_subdirectory(ndl-webos5)
add_subdirectory(lgnc)
