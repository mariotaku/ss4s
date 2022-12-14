find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 sdl2)

if (SDL2_FOUND)
    ss4s_add_module(sdl FOR_AUDIO DISPLAY_NAME "SDL Audio" SOURCES sdl.c ringbuf.c)
    target_include_directories(${SS4S_MODULE_NAME} SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${SS4S_MODULE_NAME} PRIVATE ${SDL2_LIBRARIES})
endif ()