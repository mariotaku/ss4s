get_filename_component(CMAKE_C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME)
if (NOT CMAKE_C_COMPILER_NAME MATCHES "arm-webos-linux-gnueabi-.*")
    return()
endif ()

add_subdirectory(ndl)
add_subdirectory(smp)
add_subdirectory(lgnc)