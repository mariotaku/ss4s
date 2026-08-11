include(FetchContent)

# webos-userland holds the webOS system library headers. Fetch them when the toolchain has no
# usable copy, and when building the mocks on a host that has no webOS SDK at all.
# The archive has no CMakeLists.txt at SOURCE_SUBDIR, so nothing of it gets built here.
FetchContent_Declare(
        webos_userland
        GIT_REPOSITORY https://github.com/webosbrew/webos-userland.git
        GIT_TAG 00a5f8721e15d5c10030b99b2f3e73e0c527bcce
        SOURCE_SUBDIR headers-only
)

FetchContent_MakeAvailable(webos_userland)

set(WEBOS_USERLAND_INCLUDE_DIR "${webos_userland_SOURCE_DIR}/include")
