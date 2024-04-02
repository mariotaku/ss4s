find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 sdl2)

if (NOT SDL2_FOUND)
    return()
endif ()

add_library(ss4s-ringbuf STATIC ringbuf.c)
target_include_directories(ss4s-ringbuf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ss4s-ringbuf SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(ss4s-ringbuf PRIVATE ${SDL2_LIBRARIES})

ss4s_add_module(sdl FOR_AUDIO DISPLAY_NAME "SDL Audio" SOURCES sdl.c)
if (NOT SS4S_MODULE_TARGET)
    return()
endif ()
target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ${SDL2_LIBRARIES} ss4s-ringbuf)