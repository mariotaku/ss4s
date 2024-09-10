if (NOT TARGET esplayer-datasrc)
    return()
endif ()

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

add_executable(sample-bigbuckbunny sample.c)

target_link_libraries(sample-bigbuckbunny PRIVATE ss4s esplayer-datasrc PkgConfig::SDL2)

set_target_properties(sample-bigbuckbunny PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")

get_filename_component(CMAKE_C_COMPILER_NAME ${CMAKE_C_COMPILER} NAME)

if (CMAKE_C_COMPILER_NAME STREQUAL "arm-webos-linux-gnueabi-gcc")
    message(STATUS "Adding webOS IPK target")
    add_custom_target(sample-bigbuckbunny-ipk
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/staging/
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/modules/lib ${CMAKE_CURRENT_BINARY_DIR}/staging/lib
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sample-bigbuckbunny> ${CMAKE_CURRENT_BINARY_DIR}/staging/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icon.png ${CMAKE_CURRENT_BINARY_DIR}/staging/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/appinfo.json ${CMAKE_CURRENT_BINARY_DIR}/staging/
            COMMAND ares-package ${CMAKE_CURRENT_BINARY_DIR}/staging/ -o ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS sample-bigbuckbunny)
endif ()