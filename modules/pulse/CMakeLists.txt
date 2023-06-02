find_package(PkgConfig REQUIRED)

pkg_check_modules(PULSE libpulse-simple)

if (PULSE_FOUND)
    ss4s_add_module(pulse FOR_AUDIO DISPLAY_NAME PulseAudio WEIGHT 10 SOURCES pulse.c)
    if (NOT SS4S_MODULE_TARGET)
        return()
    endif ()
    target_include_directories(${SS4S_MODULE_TARGET} SYSTEM PRIVATE ${PULSE_INCLUDE_DIRS})
    target_link_libraries(${SS4S_MODULE_TARGET} PRIVATE ${PULSE_LIBRARIES})
endif ()