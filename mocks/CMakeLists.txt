macro(add_mock NAME HEADER_DIRECTORY)
    string(TOUPPER ${NAME} NAME_UPPER)
    add_subdirectory(${NAME})
    set(${NAME_UPPER}_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/${HEADER_DIRECTORY}")
    target_include_directories(${NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/${HEADER_DIRECTORY}")
    set(${NAME_UPPER}_INCLUDE_DIRS "${${NAME_UPPER}_INCLUDE_DIRS}" PARENT_SCOPE)
    set(${NAME_UPPER}_FOUND TRUE PARENT_SCOPE)
    set(${NAME_UPPER}_LIBRARIES ${NAME} PARENT_SCOPE)
endmacro()

add_mock(lgncopenapi lgncopenapi)
add_mock(NDL_directmedia libndl-media)