include(FetchContent)

FetchContent_Declare(
        commons
        GIT_REPOSITORY https://github.com/mariotaku/commons-c.git
        GIT_TAG 4ca605985e5c6cf9a637daf595efa2b55347c25b
)

FetchContent_MakeAvailable(commons)