add_subdirectory(interface)

set(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY ${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY} PARENT_SCOPE)

function(ss4s_add_module NAME SOURCES)
    set(SS4S_MODULE_NAME "ss4s-module-${NAME}")
    set(_SOURCES ${SOURCES} ${ARGN})
    add_library(${SS4S_MODULE_NAME} SHARED ${_SOURCES})
    target_link_libraries(${SS4S_MODULE_NAME} PRIVATE ss4s-module-api)
    set_target_properties(${SS4S_MODULE_NAME} PROPERTIES
            LIBRARY_OUTPUT_NAME "ss4s-${NAME}"
            LIBRARY_OUTPUT_DIRECTORY "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")
    set(SS4S_MODULE_NAME ${SS4S_MODULE_NAME} PARENT_SCOPE)
endfunction()

add_subdirectory(alsa)
add_subdirectory(pulse)
add_subdirectory(sdl)
add_subdirectory(mmal)
