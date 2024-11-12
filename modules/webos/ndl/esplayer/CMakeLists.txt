ss4s_add_module(ndl-esplayer FOR_AUDIO FOR_VIDEO IN_GROUP ndl DISPLAY_NAME "webOS NDL"
        SOURCES ndl_module.c ndl_player.c ndl_audio.c ndl_video.c
        WEIGHT 50 CONFLICTS lgnc OS_VERSION ">=2.2,<3.5")
if (NOT SS4S_MODULE_TARGET)
    return()
endif ()
target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${NDL_DIRECTMEDIA2_INCLUDE_DIRS})
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ${NDL_DIRECTMEDIA2_LIBRARIES} pthread)
target_compile_options(${SS4S_MODULE_TARGET} PRIVATE -Wno-error=attributes)