add_subdirectory(interface)

set(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY ${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY} PARENT_SCOPE)

function(ss4s_add_module NAME)
    cmake_parse_arguments(MODULE "FOR_AUDIO;FOR_VIDEO" "IN_GROUP;DISPLAY_NAME" "SOURCES;CONFLICT" ${ARGN})
    set(SS4S_MODULE_TARGET "ss4s-module-${NAME}")
    add_library(${SS4S_MODULE_TARGET} SHARED ${MODULE_SOURCES})
    target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ss4s-module-api)
    if (NOT MODULE_IN_GROUP)
        set(MODULE_IN_GROUP "${NAME}")
    endif ()
    set_target_properties(${SS4S_MODULE_TARGET} PROPERTIES
            C_VISIBILITY_PRESET hidden
            LIBRARY_OUTPUT_NAME "ss4s-${NAME}"
            LIBRARY_OUTPUT_DIRECTORY "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}"
            SS4S_MODULE_NAME "${NAME}"
            SS4S_MODULE_GROUP "${MODULE_IN_GROUP}"
            SS4S_MODULE_DISPLAY_NAME "${MODULE_DISPLAY_NAME}"
            SS4S_MODULE_FOR_AUDIO ${MODULE_FOR_AUDIO}
            SS4S_MODULE_FOR_VIDEO ${MODULE_FOR_VIDEO}
            SS4S_MODULE_CONFLICT "${MODULE_CONFLICT}"
            )
    set(SS4S_MODULE_TARGET ${SS4S_MODULE_TARGET} PARENT_SCOPE)
    add_dependencies(ss4s ${SS4S_MODULE_TARGET})
    get_target_property(SS4S_MODULE_TARGETS ss4s SS4S_MODULE_TARGETS)
    if (SS4S_MODULE_TARGETS)
        set_target_properties(ss4s PROPERTIES SS4S_MODULE_TARGETS "${SS4S_MODULE_TARGETS};${SS4S_MODULE_TARGET}")
    else ()
        set_target_properties(ss4s PROPERTIES SS4S_MODULE_TARGETS "${SS4S_MODULE_TARGET}")
    endif ()
    if (MSVC)
        target_compile_options(${SS4S_MODULE_TARGET} PRIVATE /WX)
    else ()
        target_compile_options(${SS4S_MODULE_TARGET} PRIVATE -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter)
    endif ()
endfunction()


add_subdirectory(alsa)
add_subdirectory(pulse)
add_subdirectory(sdl)
add_subdirectory(mmal)

add_subdirectory(ndl)
add_subdirectory(lgnc)
