find_package(PkgConfig REQUIRED)

pkg_check_modules(GStreamer REQUIRED IMPORTED_TARGET gstreamer-1.0 gstreamer-app-1.0)

add_library(esplayer-datasrc STATIC esplayer-datasrc.c)
target_include_directories(esplayer-datasrc PUBLIC .)
target_link_libraries(esplayer-datasrc PRIVATE PkgConfig::GStreamer)