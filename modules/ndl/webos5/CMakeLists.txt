ss4s_add_module(ndl-webos5 FOR_VIDEO FOR_AUDIO DISPLAY_NAME "webOS NDL"
        SOURCES ndl_module.c ndl_player.c ndl_audio.c ndl_video.c
        CONFLICT lgnc)
target_link_libraries(${SS4S_MODULE_NAME} PRIVATE NDL_directmedia)