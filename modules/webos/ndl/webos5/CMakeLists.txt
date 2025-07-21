ss4s_add_module(ndl-webos5 FOR_AUDIO FOR_VIDEO IN_GROUP ndl DISPLAY_NAME "webOS NDL"
        SOURCES ndl_module.c ndl_player.c ndl_audio.c ndl_video.c
        WEIGHT 50 CONFLICTS lgnc OS_VERSION ">=5")
if (NOT SS4S_MODULE_TARGET)
    return()
endif ()

if (NOT OPUS_FOUND)
    pkg_check_modules(OPUS opus)
endif ()
if (NOT OPUS_FOUND)
    include(ExternalOPUS)
endif ()

add_library(ss4s-opus-fix STATIC opus_fix.c)
target_include_directories(ss4s-opus-fix PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} SYSTEM PRIVATE ${OPUS_INCLUDE_DIRS})
target_link_libraries(ss4s-opus-fix PUBLIC ${OPUS_LIBRARIES} PRIVATE ss4s-module-api)
set_target_properties(ss4s-opus-fix PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        C_VISIBILITY_PRESET hidden)


target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${NDL_DIRECTMEDIA_INCLUDE_DIRS} ${OPUS_INCLUDE_DIRS})
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ${NDL_DIRECTMEDIA_LIBRARIES} ${OPUS_LIBRARIES}
        pthread dl ss4s-opus-fix ss4s-webos-utils)
target_compile_definitions(${SS4S_MODULE_TARGET} PRIVATE NDL_DIRECTMEDIA_API_VERSION=2 _GNU_SOURCE)
target_compile_options(${SS4S_MODULE_TARGET} PRIVATE -Wno-error=attributes)