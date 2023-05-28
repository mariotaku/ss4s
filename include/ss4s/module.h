#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SS4S_ModuleCheckFlag {
    SS4S_MODULE_CHECK_AUDIO = 0x01,
    SS4S_MODULE_CHECK_VIDEO = 0x02,
    SS4S_MODULE_CHECK_ALL = 0xFF,
} SS4S_ModuleCheckFlag;

/**
 *
 * Run check function defined in the module
 *
 * @param id[in] Module ID
 * @param flags[in] Flags to check for the module
 * @deprecated Please use \p SS4S_ModuleCheck instead
 * @return \p true if the module returns the same result as \p flags
 */
bool SS4S_ModuleAvailable(const char *name, SS4S_ModuleCheckFlag flags);

/**
 * Run check function defined in the module
 *
 * @param id[in] Module ID
 * @param flags[in] Flags to check for the module
 * @return Support flags passed in \p flags.
 */
SS4S_ModuleCheckFlag SS4S_ModuleCheck(const char *id, SS4S_ModuleCheckFlag flags);

#ifdef __cplusplus
}
#endif