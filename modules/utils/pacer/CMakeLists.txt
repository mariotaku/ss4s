add_library(ss4s-pacer STATIC pacer.c)
target_include_directories(ss4s-pacer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ss4s-pacer PUBLIC ss4s-ringbuf pthread m)
set_target_properties(ss4s-pacer PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        C_VISIBILITY_PRESET hidden)

enable_testing()
add_subdirectory(tests)