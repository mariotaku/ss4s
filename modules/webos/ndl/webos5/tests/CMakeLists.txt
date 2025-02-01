if (NOT SS4S_ENABLE_TESTS)
    return()
endif ()

enable_testing()

add_executable(opus_empty_test opus_empty_test.c)
target_link_libraries(opus_empty_test PRIVATE ss4s-opus-empty)
add_test(NAME opus_empty_test COMMAND opus_empty_test)