add_library(ss4s-webos-utils STATIC knlp_check.c o22plus_check.c)
target_include_directories(ss4s-webos-utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(ss4s-webos-utils PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        C_VISIBILITY_PRESET hidden)