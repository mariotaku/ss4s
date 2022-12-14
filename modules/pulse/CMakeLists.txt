find_package(PkgConfig REQUIRED)

pkg_check_modules(PULSE libpulse-simple)

if (PULSE_FOUND)
    ss4s_add_module(pulse FOR_AUDIO DISPLAY_NAME PulseAudio SOURCES pulse.c)
    target_include_directories(${SS4S_MODULE_NAME} SYSTEM PRIVATE ${PULSE_INCLUDE_DIRS})
    target_link_libraries(${SS4S_MODULE_NAME} PRIVATE ${PULSE_LIBRARIES})
endif ()