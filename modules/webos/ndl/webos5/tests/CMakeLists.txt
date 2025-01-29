if (NOT SS4S_ENABLE_TESTS)
    return()
endif ()

enable_testing()

add_executable(opus_empty_is_empty opus_empty_is_empty.c)
target_link_libraries(opus_empty_is_empty PRIVATE ss4s-opus-empty)
add_test(NAME opus_empty_is_empty COMMAND opus_empty_is_empty)