add_library(ss4s-webos-utils STATIC knlp_check.c highend_check.c)
target_include_directories(ss4s-webos-utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(ss4s-webos-utils PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        C_VISIBILITY_PRESET hidden)

if (NOT SS4S_ENABLE_TESTS)
    return()
endif ()

add_executable(ss4s-webos-utils-test test_highend_check.c)
add_test(ss4s-webos-utils-test ss4s-webos-utils-test)