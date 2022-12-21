ss4s_add_module(ndl-webos4 FOR_VIDEO FOR_AUDIO IN_GROUP ndl DISPLAY_NAME "webOS NDL"
        SOURCES ndl_module.c ndl_player.c ndl_audio.c ndl_video.c
        WEIGHT 50 CONFLICT lgnc)
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE NDL_directmedia)