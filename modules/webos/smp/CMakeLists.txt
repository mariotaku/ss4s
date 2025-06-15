find_package(PkgConfig REQUIRED)

if (NOT LIBPLAYERAPIS_FOUND OR NOT ACBAPI_FOUND)
    pkg_check_modules(LIBPLAYERAPIS libplayerAPIs)
    pkg_check_modules(ACBAPI AcbAPI)
    if (NOT LIBPLAYERAPIS_FOUND OR NOT ACBAPI_FOUND)
        message(WARNING "Skipping webOS SMP module because libplayerAPIs or AcbAPI not found")
        return()
    endif ()
endif ()

include(CheckSymbolExists)

cmake_push_check_state()
list(APPEND CMAKE_REQUIRED_INCLUDES "${ACBAPI_INCLUDE_DIRS}")
list(APPEND CMAKE_REQUIRED_LIBRARIES "${ACBAPI_LIBRARIES}")
check_symbol_exists(AcbAPI_setMediaAudioData "AcbAPI.h" HAVE_SETMEDIAAUDIODATA)
cmake_pop_check_state()


pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(PBNJSON_C REQUIRED pbnjson_c)
pkg_check_modules(SDL2 REQUIRED sdl2)

add_subdirectory(wrapper)

ss4s_add_module(smp-webos FOR_VIDEO IN_GROUP smp DISPLAY_NAME "webOS SMP" WEIGHT 10 OS_VERSION ">=5" SOURCES
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
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE playerAPIs_C dl ${SDL2_LIBRARIES} ${GLIB_LIBRARIES}
        ${PBNJSON_C_LIBRARIES})
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE USE_SDL_WEBOS _GNU_SOURCE SS4S_MODULE_NAME="smp-webos"
        SS4S_MODULE_ENTRY=SS4S_ModuleOpen_SMP_WEBOS SS4S_MODULE_CHECK=SS4S_ModuleCheck_SMP_WEBOS SS4S_SMP_AUDIO=1)
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")


ss4s_add_module(smp-webos4 FOR_VIDEO IN_GROUP smp DISPLAY_NAME "webOS SMP" WEIGHT 10 OS_VERSION "=4" SOURCES
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
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE playerAPIs_C ss4s-webos-utils dl ${GLIB_LIBRARIES}
        ${ACBAPI_LIBRARIES} ${PBNJSON_C_LIBRARIES})
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE USE_SDL_WEBOS _GNU_SOURCE SS4S_MODULE_NAME="smp-webos4"
        SS4S_MODULE_ENTRY=SS4S_ModuleOpen_SMP_WEBOS4 SS4S_MODULE_CHECK=SS4S_ModuleCheck_SMP_WEBOS4 SS4S_JAIL_CHECK=1)
if (HAVE_SETMEDIAAUDIODATA)
    target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE HAVE_SETMEDIAAUDIODATA)
endif ()
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

ss4s_add_module(smp-webos3 FOR_VIDEO IN_GROUP smp DISPLAY_NAME "webOS SMP" WEIGHT 10 OS_VERSION "<4" SOURCES
        src/smp_module.c
        src/smp_player.c
        src/smp_video.c
        src/smp_audio.c
        src/pbnjson_compat.c
        src/smp_resource_acb.c
)
target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${GLIB_INCLUDE_DIRS} ${ACBAPI_INCLUDE_DIRS}
        ${PBNJSON_C_INCLUDE_DIRS})
target_include_directories(${SS4S_MODULE_TARGET} PRIVATE src)
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE playerAPIs_C_Legacy ss4s-webos-utils dl ${GLIB_LIBRARIES}
        ${ACBAPI_LIBRARIES} ${PBNJSON_C_LIBRARIES})
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE USE_SDL_WEBOS _GNU_SOURCE SS4S_MODULE_NAME="smp-webos3"
        SS4S_MODULE_ENTRY=SS4S_ModuleOpen_SMP_WEBOS3 SS4S_MODULE_CHECK=SS4S_ModuleCheck_SMP_WEBOS3 SS4S_JAIL_CHECK=1)
if (HAVE_SETMEDIAAUDIODATA)
    target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE HAVE_SETMEDIAAUDIODATA)
endif ()
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")