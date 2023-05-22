ss4s_add_module(ndl-webos4 FOR_AUDIO FOR_VIDEO IN_GROUP ndl DISPLAY_NAME "webOS NDL"
        SOURCES ndl_module.c ndl_player.c ndl_audio.c ndl_video.c
        WEIGHT 50 CONFLICTS lgnc OS_VERSION ">=3.5.0,<5")
if (NOT SS4S_MODULE_TARGET)
    return()
endif ()
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE NDL_directmedia pthread dl)
target_compile_options(${SS4S_MODULE_TARGET} PRIVATE -Wno-ignored-attributes)