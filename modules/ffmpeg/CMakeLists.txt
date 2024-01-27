find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG libavcodec libavformat libavutil)

if (NOT FFMPEG_FOUND)
    return()
endif ()

ss4s_add_module(ffmpeg FOR_VIDEO DISPLAY_NAME "FFmpeg" SOURCES ffmpeg_module.c WEIGHT 10)
target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ${FFMPEG_LIBRARIES})
target_include_directories(${SS4S_MODULE_TARGET} PRIVATE ${FFMPEG_INCLUDE_DIRS})