add_subdirectory(interface)

set(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY ${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY} PARENT_SCOPE)

target_compile_definitions(ss4s PRIVATE SS4S_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")

function(ss4s_add_module NAME)
    unset(SS4S_MODULE_TARGET PARENT_SCOPE)
    string(REGEX REPLACE "[^0-9a-z]" "_" ENV_NAME "${NAME}")
    string(TOUPPER "${ENV_NAME}" ENV_NAME)
    if (SS4S_MODULE_DISABLE_${ENV_NAME})
        return()
    endif ()
    cmake_parse_arguments(MODULE "FOR_AUDIO;FOR_VIDEO" "IN_GROUP;DISPLAY_NAME;WEIGHT;OS_VERSION" "SOURCES;CONFLICTS" ${ARGN})

    if (NOT MODULE_IN_GROUP)
        set(MODULE_IN_GROUP "${NAME}")
    endif ()
    if (NOT MODULE_WEIGHT OR MODULE_WEIGHT LESS 0)
        set(MODULE_WEIGHT 0)
    elseif (MODULE_WEIGHT GREATER 100)
        set(MODULE_WEIGHT 100)
    endif ()

    set(SS4S_MODULE_TARGET "ss4s-module-${NAME}")
    add_library(${SS4S_MODULE_TARGET} SHARED ${MODULE_SOURCES})
    target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ss4s-module-api)
    set_target_properties(${SS4S_MODULE_TARGET} PROPERTIES
            C_VISIBILITY_PRESET hidden
            PREFIX ""
            LIBRARY_OUTPUT_NAME "ss4s-${NAME}"
            LIBRARY_OUTPUT_DIRECTORY "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}"
            SS4S_MODULE_NAME "${NAME}"
            SS4S_MODULE_GROUP "${MODULE_IN_GROUP}"
            SS4S_MODULE_DISPLAY_NAME "${MODULE_DISPLAY_NAME}"
            SS4S_MODULE_FOR_AUDIO ${MODULE_FOR_AUDIO}
            SS4S_MODULE_FOR_VIDEO ${MODULE_FOR_VIDEO}
            SS4S_MODULE_WEIGHT ${MODULE_WEIGHT}
            SS4S_MODULE_CONFLICTS "${MODULE_CONFLICTS}"
            SS4S_MODULE_OS_VERSION "${MODULE_OS_VERSION}"
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
    install(TARGETS ${SS4S_MODULE_TARGET} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
    message(STATUS "Enable SS4S module `${NAME}`")
endfunction()


add_subdirectory(alsa)
add_subdirectory(pulse)
add_subdirectory(sdl)
add_subdirectory(mmal)
add_subdirectory(webos)

if (SS4S_MODULE_BUILD_DUMMY)
    add_subdirectory(dummy)
endif ()

if (SS4S_GENERATE_MODULES_LIST)
    add_subdirectory(list-ini)
endif ()