add_executable(test-pacer-normal test_pacer_normal.c)
target_link_libraries(test-pacer-normal ss4s-pacer)
add_test(pacer-normal test-pacer-normal)

add_executable(test-pacer-overflow-clear test_pacer_overflow_clear.c)
target_link_libraries(test-pacer-overflow-clear ss4s-pacer)
add_test(pacer-overflow-clear test-pacer-overflow-clear)

add_executable(test-pacer-overflow-rewind test_pacer_overflow_rewind.c)
target_link_libraries(test-pacer-overflow-rewind ss4s-pacer)
add_test(pacer-overflow-rewind test-pacer-overflow-rewind)