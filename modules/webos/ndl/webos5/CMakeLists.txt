ss4s_add_module(ndl-webos5 FOR_AUDIO FOR_VIDEO IN_GROUP ndl DISPLAY_NAME "webOS NDL"
        SOURCES ndl_module.c ndl_player.c ndl_audio.c ndl_video.c opus_empty.c opus_fix.c
        WEIGHT 50 CONFLICTS lgnc OS_VERSION ">=5")
if (NOT SS4S_MODULE_TARGET)
    return()
endif ()
target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${NDL_DIRECTMEDIA_INCLUDE_DIRS})
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ${NDL_DIRECTMEDIA_LIBRARIES} pthread dl)
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE NDL_DIRECTMEDIA_API_VERSION=2 _GNU_SOURCE)
target_compile_options(${SS4S_MODULE_TARGET} PRIVATE -Wno-error=attributes)

if (NOT OPUS_FOUND)
    pkg_check_modules(OPUS REQUIRED opus)
endif ()
target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${OPUS_INCLUDE_DIRS})
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ${OPUS_LIBRARIES})