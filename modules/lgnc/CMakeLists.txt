if (TARGET lgncopenapi)
    ss4s_add_module(lgnc FOR_VIDEO FOR_AUDIO DISPLAY_NAME "NetCast Legacy"
            SOURCES lgnc_module.c lgnc_player.c lgnc_audio.c lgnc_video.c
            WEIGHT 10 CONFLICTS ndl OS_VERSION "<5")
    if (NOT SS4S_MODULE_TARGET)
        return()
    endif ()
    target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE lgncopenapi)
endif ()