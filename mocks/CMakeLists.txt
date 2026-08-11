include(WebOSUserland)

# The mocks build against the same headers the real libraries use. A signature change upstream
# then breaks the mock at compile time, instead of letting it drift.
macro(add_mock NAME HEADER_DIRECTORY)
    string(TOUPPER ${NAME} NAME_UPPER)
    add_subdirectory(${NAME})
    set(${NAME_UPPER}_INCLUDE_DIRS "${WEBOS_USERLAND_INCLUDE_DIR}/${HEADER_DIRECTORY}")
    foreach (TARGET ${ARGN})
        target_include_directories(${TARGET} PUBLIC "${WEBOS_USERLAND_INCLUDE_DIR}/${HEADER_DIRECTORY}")
    endforeach ()
    set(${NAME_UPPER}_INCLUDE_DIRS "${${NAME_UPPER}_INCLUDE_DIRS}" PARENT_SCOPE)
    set(${NAME_UPPER}_FOUND TRUE PARENT_SCOPE)
endmacro()

add_mock(lgncopenapi lgncopenapi lgncopenapi)
set(LGNCOPENAPI_LIBRARIES lgncopenapi PARENT_SCOPE)

# NDL_directmedia has one mock library per API version, see its CMakeLists.txt
add_mock(NDL_directmedia libndl-media NDL_directmedia_v1 NDL_directmedia_v2)
set(NDL_DIRECTMEDIA_V1_LIBRARIES NDL_directmedia_v1 PARENT_SCOPE)
set(NDL_DIRECTMEDIA_V2_LIBRARIES NDL_directmedia_v2 PARENT_SCOPE)
