if (NOT TARGET playerAPIs)
    return()
endif ()

enable_language(CXX OPTIONAL)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(PBNJSON_CPP REQUIRED pbnjson_cpp)
pkg_check_modules(SDL2 REQUIRED sdl2)


set(CMAKE_CXX_STANDARD 11)
ss4s_add_module(smp-webos FOR_VIDEO IN_GROUP smp DISPLAY_NAME "webOS SMP" WEIGHT 99 SOURCES
        src/Module.cpp
        src/AVStreamPlayer.cpp
        src/Base64Encoder.cpp
        )
target_include_directories(${SS4S_MODULE_TARGET} PRIVATE ${SDL2_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${PBNJSON_CPP_INCLUDE_DIRS})
target_include_directories(${SS4S_MODULE_TARGET} PRIVATE src)
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE playerAPIs ${SDL2_LIBRARIES} ${PBNJSON_CPP_LIBRARIES})
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE USE_SDL_WEBOS SMP_DECODER_NS=SS4S_SMP
        SS4S_MODULE_NAME="smp-webos" SS4S_MODULE_ENTRY=SS4S_ModuleOpen_SMP_WEBOS)
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

#set(CMAKE_CXX_STANDARD 11)
#ss4s_add_module(smp-webos4 FOR_AUDIO FOR_VIDEO IN_GROUP smp DISPLAY_NAME "webOS SMP" SOURCES
#        src/Module.cpp
#        src/AVStreamPlayer.cpp
#        src/Base64Encoder.cpp
#        )
#target_include_directories(${SS4S_MODULE_TARGET} PRIVATE ${GLIB_INCLUDE_DIRS} ${PBNJSON_CPP_INCLUDE_DIRS})
#target_include_directories(${SS4S_MODULE_TARGET} PRIVATE src)
#target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE playerAPIs Acb ${PBNJSON_CPP_LIBRARIES})
#target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE USE_ACB SMP_DECODER_NS=SS4S_SMP_webOS4 SS4S_MODULE_NAME="smp-webos4")
#target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
#
#set(CMAKE_CXX_STANDARD 11)
#ss4s_add_module(smp-webos3 FOR_AUDIO FOR_VIDEO IN_GROUP smp DISPLAY_NAME "webOS SMP" SOURCES
#        src/Module.cpp
#        src/AVStreamPlayer.cpp
#        src/Base64Encoder.cpp
#        )
#
#target_include_directories(${SS4S_MODULE_TARGET} PRIVATE ${GLIB_INCLUDE_DIRS} ${PBNJSON_CPP_INCLUDE_DIRS})
#target_include_directories(${SS4S_MODULE_TARGET} PRIVATE src)
#target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE playerAPIs Acb ${PBNJSON_CPP_LIBRARIES})
#target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE USE_ACB SMP_DECODER_NS=SS4S_SMP_webOS3 SS4S_MODULE_NAME="smp-webos3")
#target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
## webOS 3 and before don't have GLIBCXX_3.4.21
#target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE _GLIBCXX_USE_CXX11_ABI=0)