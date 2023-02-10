#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SS4S_ModuleCheckFlag {
    SS4S_MODULE_CHECK_AUDIO = 0x01,
    SS4S_MODULE_CHECK_VIDEO = 0x02,
    SS4S_MODULE_CHECK_ALL = 0xFF,
} SS4S_ModuleCheckFlag;

bool SS4S_ModuleAvailable(const char *name, SS4S_ModuleCheckFlag flags);

#ifdef __cplusplus
}
#endif