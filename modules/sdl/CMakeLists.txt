find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 sdl2)

if (SDL2_FOUND)
    ss4s_add_module(sdl FOR_AUDIO DISPLAY_NAME "SDL Audio" SOURCES sdl.c ringbuf.c)
    if (NOT SS4S_MODULE_TARGET)
        return()
    endif ()
    target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ${SDL2_LIBRARIES})
endif ()