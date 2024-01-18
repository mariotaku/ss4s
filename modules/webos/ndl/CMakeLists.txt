find_package(PkgConfig REQUIRED)

if (NOT NDL_DIRECTMEDIA_FOUND)
    pkg_check_modules(NDL_DIRECTMEDIA NDL_directmedia)
endif ()
if (NOT NDL_DIRECTMEDIA_FOUND)
    include(ExternalWebOSUserland)
endif ()

add_subdirectory(webos5)
add_subdirectory(webos4)