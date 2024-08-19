if (HAS_DLFCN)
    set(SS4S_INI_LIB_TYPE SHARED)
else ()
    set(SS4S_INI_LIB_TYPE STATIC)
endif ()
add_library(ss4s-modules-list-ini ${SS4S_INI_LIB_TYPE} modules_ini.c)
set_target_properties(ss4s-modules-list-ini PROPERTIES
        PREFIX ""
        LIBRARY_OUTPUT_NAME "ss4s_modules"
        LIBRARY_OUTPUT_DIRECTORY "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}"
        )
target_include_directories(ss4s-modules-list-ini PUBLIC include PRIVATE include/ss4s)

if (HAS_DLFCN)
    target_compile_definitions(ss4s-modules-list-ini PRIVATE _GNU_SOURCE HAS_DLADDR)
    target_link_libraries(ss4s-modules-list-ini PRIVATE dl)
endif ()

install(TARGETS ss4s-modules-list-ini LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})