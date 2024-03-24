find_package(PkgConfig REQUIRED)

if (NOT LGNCOPENAPI_FOUND)
    pkg_check_modules(LGNCOPENAPI lgncopenapi)
endif ()
if (NOT LGNCOPENAPI_FOUND)
    include(ExternalWebOSUserland)
endif ()

ss4s_add_module(lgnc FOR_VIDEO FOR_AUDIO DISPLAY_NAME "NetCast Legacy"
        SOURCES lgnc_module.c lgnc_player.c lgnc_audio.c lgnc_video.c
        WEIGHT 40 CONFLICTS ndl OS_VERSION "<5")
if (NOT SS4S_MODULE_TARGET)
    return()
endif ()
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ${LGNCOPENAPI_LIBRARIES} pthread dl)
target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${LGNCOPENAPI_INCLUDE_DIRS})
