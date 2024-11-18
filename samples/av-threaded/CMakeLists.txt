find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)


add_executable(sample-av-threaded sample.c)

target_link_libraries(sample-av-threaded PRIVATE ss4s PkgConfig::SDL2)

set_target_properties(sample-av-threaded PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")

get_filename_component(CMAKE_C_COMPILER_NAME ${CMAKE_C_COMPILER} NAME)

if (CMAKE_C_COMPILER_NAME STREQUAL "arm-webos-linux-gnueabi-gcc")
    message(STATUS "Adding webOS IPK target")
    add_custom_target(sample-av-threaded-ipk
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/staging/
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/modules/lib ${CMAKE_CURRENT_BINARY_DIR}/staging/lib
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sample-av-threaded> ${CMAKE_CURRENT_BINARY_DIR}/staging/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icon.png ${CMAKE_CURRENT_BINARY_DIR}/staging/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/appinfo.json ${CMAKE_CURRENT_BINARY_DIR}/staging/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sample.h264 ${CMAKE_CURRENT_BINARY_DIR}/staging/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sample.pcm ${CMAKE_CURRENT_BINARY_DIR}/staging/
            COMMAND ares-package ${CMAKE_CURRENT_BINARY_DIR}/staging/ -o ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS sample-bigbuckbunny)
    add_custom_target(sample-av-threaded-install
            COMMAND ares-install ${CMAKE_CURRENT_BINARY_DIR}/org.mariotaku.ss4s.sample-av-threaded_1.0.0_arm.ipk
            DEPENDS sample-av-threaded-ipk)
endif ()