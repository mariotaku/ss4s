set(TEST_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)

function(ss4s_add_test NAME SOURCES)
    set(SS4S_TEST_CASE "ss4s_test_${NAME}")
    add_executable(${SS4S_TEST_CASE} ${SOURCES})
    add_test(${NAME} ${SS4S_TEST_CASE})
    target_include_directories(${SS4S_TEST_CASE} PRIVATE ${TEST_INCLUDES})
    target_link_libraries(${SS4S_TEST_CASE} PRIVATE ss4s)
    set_target_properties(${SS4S_TEST_CASE} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH_USE_LINK_PATH TRUE
            INSTALL_RPATH "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")
    set(SS4S_TEST_CASE ${SS4S_TEST_CASE} PARENT_SCOPE)
endfunction()

function(ss4s_test_link_libraries NAME LIBRARIES)
    set(SS4S_TEST_CASE "ss4s_test_${NAME}")
    target_link_libraries(${SS4S_TEST_CASE} PRIVATE ${LIBRARIES})
endfunction()

configure_file(sample.pcm ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

ss4s_add_test(initialization test_initialization.c)

ss4s_add_test(pcm_playback test_pcm_playback.c)

find_package(SDL2 REQUIRED)
target_link_libraries(${SS4S_TEST_CASE} PRIVATE ${SDL2_LIBRARIES})