if (NOT SS4S_TARGET_WEBOS AND NOT SS4S_ENABLE_TESTS)
    return()
endif ()

add_subdirectory(utils)

add_subdirectory(ndl)
add_subdirectory(lgnc)
add_subdirectory(smp)
