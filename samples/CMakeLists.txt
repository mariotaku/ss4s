include(FetchContent)

set(INIH_STATIC ON)

FetchContent_Declare(
        commons
        GIT_REPOSITORY https://github.com/mariotaku/commons-c.git
        GIT_TAG c8404ce85742ea125bbfd3878b6997b8c27ca958
)

FetchContent_MakeAvailable(commons)

add_subdirectory(esplayer-datasrc)

add_subdirectory(bigbuckbunny)

add_subdirectory(av-threaded)