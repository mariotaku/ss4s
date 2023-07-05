find_library(SLAUDIO_LIBRARY SLAudio)
find_library(SLVIDEO_LIBRARY SLVideo)

if (NOT SLAUDIO_LIBRARY OR NOT SLVIDEO_LIBRARY)
    return()
endif ()

ss4s_add_module(steamlink FOR_VIDEO FOR_AUDIO DISPLAY_NAME "Steam Link"
        SOURCES sl_module.c sl_player.c sl_video.c sl_audio.c WEIGHT 90)
if (NOT SS4S_MODULE_TARGET)
    return()
endif ()
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ${SLAUDIO_LIBRARY} ${SLVIDEO_LIBRARY} pthread)