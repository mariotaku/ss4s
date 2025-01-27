add_library(ss4s-ringbuf STATIC ringbuf.c)
target_include_directories(ss4s-ringbuf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(ss4s-ringbuf PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        C_VISIBILITY_PRESET hidden)