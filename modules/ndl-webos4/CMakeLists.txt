if (TARGET NDL_directmedia)
    ss4s_add_module(ndl-webos4 SOURCES ndl_module.c ndl_player.c ndl_audio.c ndl_video.c)
    target_link_libraries(${SS4S_MODULE_NAME} PRIVATE NDL_directmedia)
endif()