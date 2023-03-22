enable_language(CXX OPTIONAL)

set(CMAKE_CXX_STANDARD 11)
add_library(playerAPIs_C STATIC StarfishMediaAPIs_C.cpp)
target_link_libraries(playerAPIs_C PUBLIC playerAPIs stdc++)
target_include_directories(playerAPIs_C PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(playerAPIs_C_Legacy STATIC StarfishMediaAPIs_C.cpp)
target_link_libraries(playerAPIs_C_Legacy PUBLIC playerAPIs stdc++)
target_compile_definitions(playerAPIs_C_Legacy PRIVATE _GLIBCXX_USE_CXX11_ABI=0)
target_include_directories(playerAPIs_C_Legacy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})