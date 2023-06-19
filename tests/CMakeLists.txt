set(TEST_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)

function(ss4s_add_test NAME)
    cmake_parse_arguments(TEST "" "" "SOURCES;ARGS;LIBRARIES" ${ARGN})
    set(SS4S_TEST_CASE "ss4s_test_${NAME}")
    add_executable(${SS4S_TEST_CASE} ${TEST_SOURCES})
    add_test(${NAME} ${SS4S_TEST_CASE} ${TEST_ARGS})
    target_include_directories(${SS4S_TEST_CASE} PRIVATE ${TEST_INCLUDES})
    target_link_libraries(${SS4S_TEST_CASE} PRIVATE ss4s ${TEST_LIBRARIES})
    set_target_properties(${SS4S_TEST_CASE} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH_USE_LINK_PATH TRUE
            INSTALL_RPATH "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")
    set_tests_properties(${NAME} PROPERTIES SKIP_RETURN_CODE 127)
    set(SS4S_TEST_CASE ${SS4S_TEST_CASE} PARENT_SCOPE)
endfunction()

configure_file(sample.pcm ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(sample.h264 ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

ss4s_add_test(initialization SOURCES test_initialization.c)

ss4s_add_test(pcm_playback_sdl SOURCES test_pcm_playback.c ARGS sdl)
ss4s_add_test(pcm_playback_alsa SOURCES test_pcm_playback.c ARGS alsa)
ss4s_add_test(pcm_playback_pulse SOURCES test_pcm_playback.c ARGS pulse)

ss4s_add_test(pcm_playback_mmal SOURCES test_h264_playback.c ARGS mmal)