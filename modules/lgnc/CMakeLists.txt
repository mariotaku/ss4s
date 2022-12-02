if (TARGET lgncopenapi)
    ss4s_add_module(lgnc SOURCES lgnc_module.c lgnc_player.c lgnc_audio.c lgnc_video.c)
    target_link_libraries(${SS4S_MODULE_NAME} PRIVATE lgncopenapi)
endif()