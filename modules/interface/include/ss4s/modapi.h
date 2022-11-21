#pragma once

#ifndef SS4S_MODAPI_H
#define SS4S_MODAPI_H

#include "ss4s/audio.h"
#include "ss4s/video.h"

#include <stdbool.h>

typedef struct SS4S_DriverBase {
    void (*Init)(int argc, char *argv[]);

    void (*PostInit)(int argc, char *argv[]);

    void (*Quit)();
} SS4S_DriverBase;

typedef struct SS4S_AudioInstance SS4S_AudioInstance;

typedef struct SS4S_AudioDriver {
    SS4S_DriverBase Base;

    SS4S_AudioInstance *(*Open)(const SS4S_AudioInfo *info);

    int (*Feed)(SS4S_AudioInstance *instance, const unsigned char *data, size_t size);

    void (*Close)(SS4S_AudioInstance *instance);
} SS4S_AudioDriver;

typedef struct SS4S_VideoInstance SS4S_VideoInstance;

typedef struct SS4S_VideoDriver {
    SS4S_DriverBase Base;

    SS4S_VideoInstance *(*Open)(const SS4S_VideoInfo *info);

    int (*Feed)(SS4S_VideoInstance *instance, const unsigned char *data, size_t size);

    void (*Close)(SS4S_VideoInstance *instance);
} SS4S_VideoDriver;

typedef struct SS4S_Module SS4S_Module;

struct SS4S_Module {
    const char *Name;
    const SS4S_AudioDriver *AudioDriver;
    const SS4S_VideoDriver *VideoDriver;
};

#define SS4S_MODULE_ENTRY __attribute__((unused))

/**
 * Multiple calls to this function should have same effect.
 */
typedef bool (SS4S_ModuleOpenFunction)(SS4S_Module *module);

#endif // SS4S_MODAPI_H
