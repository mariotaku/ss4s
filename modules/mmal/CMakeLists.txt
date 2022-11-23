list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(BroadcomMMAL)

if (BROADCOMMMAL_FOUND)
    ss4s_add_module(mmal SOURCES mmalvid.c)
    target_include_directories(${SS4S_MODULE_NAME} SYSTEM PRIVATE ${BROADCOM_INCLUDE_DIR})
    target_link_libraries(${SS4S_MODULE_NAME} PRIVATE ${BROADCOM_MMAL_LIBRARIES})
endif ()