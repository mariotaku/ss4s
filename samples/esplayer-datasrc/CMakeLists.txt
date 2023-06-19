find_package(PkgConfig REQUIRED)

pkg_check_modules(GStreamer IMPORTED_TARGET gstreamer-1.0 gstreamer-app-1.0)
if (NOT GStreamer_FOUND)
    return()
endif ()

add_library(esplayer-datasrc STATIC esplayer-datasrc.c)
target_include_directories(esplayer-datasrc PUBLIC .)
target_link_libraries(esplayer-datasrc PRIVATE PkgConfig::GStreamer)