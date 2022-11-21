find_package(ALSA)

if (ALSA_FOUND)
    ss4s_add_module(alsa alsa.c)
    target_include_directories(${SS4S_MODULE_NAME} SYSTEM PRIVATE ${ALSA_INCLUDE_DIRS})
    target_link_libraries(${SS4S_MODULE_NAME} PRIVATE ${ALSA_LIBRARIES})
endif ()