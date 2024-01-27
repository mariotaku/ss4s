if (NOT TARGET esplayer-datasrc)
    return()
endif ()
find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED sdl2)

add_executable(sample-bigbuckbunny sample.c)
target_link_libraries(sample-bigbuckbunny PRIVATE ss4s esplayer-datasrc ${SDL2_LIBRARIES})
target_include_directories(sample-bigbuckbunny PRIVATE ${SDL2_INCLUDE_DIRS})

set_target_properties(sample-bigbuckbunny PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")