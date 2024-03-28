find_package(PkgConfig REQUIRED)

if (NOT LIBPLAYERAPIS_FOUND OR NOT ACBAPI_FOUND)
    pkg_check_modules(LIBPLAYERAPIS libplayerAPIs)
    pkg_check_modules(ACBAPI AcbAPI)
    if (NOT LIBPLAYERAPIS_FOUND OR NOT ACBAPI_FOUND)
        message(WARNING "Skipping webOS SMP module because libplayerAPIs or AcbAPI not found")
        return()
    endif ()
endif ()


pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(PBNJSON_C REQUIRED pbnjson_c)
pkg_check_modules(SDL2 REQUIRED sdl2)

add_subdirectory(wrapper)

ss4s_add_module(smp-webos FOR_AUDIO FOR_VIDEO IN_GROUP smp DISPLAY_NAME "webOS SMP" WEIGHT 20 CONFLICTS ndl OS_VERSION ">=5" SOURCES
        src/smp_module.c
        src/smp_player.c
        src/smp_video.c
        src/smp_audio.c
        src/pbnjson_compat.c
        src/smp_resource_webos5.c
)
target_include_directories(${SS4S_MODULE_TARGET} PRIVATE src)
target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS}
        ${PBNJSON_C_INCLUDE_DIRS})
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE playerAPIs_C dl ${SDL2_LIBRARIES} ${PBNJSON_C_LIBRARIES})
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE USE_SDL_WEBOS _GNU_SOURCE SS4S_MODULE_NAME="smp-webos"
        SS4S_MODULE_ENTRY=SS4S_ModuleOpen_SMP_WEBOS)
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")


ss4s_add_module(smp-webos4 FOR_AUDIO FOR_VIDEO IN_GROUP smp DISPLAY_NAME "webOS SMP" WEIGHT 40 OS_VERSION "=4" SOURCES
        src/smp_module.c
        src/smp_player.c
        src/smp_video.c
        src/smp_audio.c
        src/pbnjson_compat.c
        src/smp_resource_acb.c
)
target_include_directories(${SS4S_MODULE_TARGET} PRIVATE src)
target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${GLIB_INCLUDE_DIRS} ${ACBAPI_INCLUDE_DIRS}
        ${PBNJSON_C_INCLUDE_DIRS})
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE playerAPIs_C dl ${ACBAPI_LIBRARIES} ${PBNJSON_C_LIBRARIES})
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE USE_SDL_WEBOS _GNU_SOURCE SS4S_MODULE_NAME="smp-webos"
        SS4S_MODULE_ENTRY=SS4S_ModuleOpen_SMP_WEBOS4)
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

ss4s_add_module(smp-webos3 FOR_AUDIO FOR_VIDEO IN_GROUP smp DISPLAY_NAME "webOS SMP" WEIGHT 40 OS_VERSION "<4" SOURCES
        src/smp_module.c
        src/smp_player.c
        src/smp_video.c
        src/smp_audio.c
        src/pbnjson_compat.c
        src/smp_resource_acb.c
)
target_include_directories(${SS4S_MODULE_TARGET} PRIVATE ${GLIB_INCLUDE_DIRS} ${ACBAPI_INCLUDE_DIRS}
        ${PBNJSON_C_INCLUDE_DIRS})
target_include_directories(${SS4S_MODULE_TARGET} PRIVATE src)
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE playerAPIs_C_Legacy dl ${ACBAPI_LIBRARIES}
        ${PBNJSON_C_LIBRARIES})
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE USE_SDL_WEBOS _GNU_SOURCE SS4S_MODULE_NAME="smp-webos"
        SS4S_MODULE_ENTRY=SS4S_ModuleOpen_SMP_WEBOS3)
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")