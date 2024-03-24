find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG IMPORTED_TARGET libavformat libavcodec libavutil)
if (NOT FFMPEG_FOUND)
    return()
endif ()

add_library(esplayer-datasrc STATIC esplayer-datasrc.c)
target_include_directories(esplayer-datasrc PUBLIC .)
target_link_libraries(esplayer-datasrc PRIVATE PkgConfig::FFMPEG)