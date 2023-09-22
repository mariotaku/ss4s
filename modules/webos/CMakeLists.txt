get_filename_component(CMAKE_C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME)
if (NOT CMAKE_C_COMPILER_NAME MATCHES "arm-webos-linux-gnueabi-.*" AND
        NOT CMAKE_C_COMPILER_TARGET MATCHES "(i[3-6]86)-webos-linux-gnu")
    return()
endif ()

add_subdirectory(ndl)
add_subdirectory(smp)
add_subdirectory(lgnc)