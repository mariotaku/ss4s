# API 1 and API 2 give different signatures to the same symbols, so they can not live in one
# archive. Build one library per version, and let each module link the one it targets.
foreach (VERSION 1 2)
    set(TARGET NDL_directmedia_v${VERSION})
    add_library(${TARGET} STATIC ndl_directmedia_mock.c ndl_directmedia_v${VERSION}.c)
    target_link_libraries(${TARGET} pthread)
    set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endforeach ()
