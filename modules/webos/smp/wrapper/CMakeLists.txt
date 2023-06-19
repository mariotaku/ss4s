enable_language(CXX OPTIONAL)

set(CMAKE_CXX_STANDARD 11)
add_library(playerAPIs_C STATIC StarfishMediaAPIs_C.cpp)
target_link_libraries(playerAPIs_C PUBLIC ${LIBPLAYERAPIS_LIBRARIES} stdc++)
target_include_directories(playerAPIs_C PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        SYSTEM PRIVATE ${LIBPLAYERAPIS_INCLUDE_DIRS})

# webOS 3 and before don't have GLIBCXX_3.4.21
add_library(playerAPIs_C_Legacy STATIC StarfishMediaAPIs_C.cpp)
target_link_libraries(playerAPIs_C_Legacy PUBLIC ${LIBPLAYERAPIS_LIBRARIES} stdc++)
target_compile_definitions(playerAPIs_C_Legacy PRIVATE _GLIBCXX_USE_CXX11_ABI=0)
target_include_directories(playerAPIs_C_Legacy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        SYSTEM PRIVATE ${LIBPLAYERAPIS_INCLUDE_DIRS})