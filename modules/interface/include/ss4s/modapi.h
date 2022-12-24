#pragma once

#ifndef SS4S_MODAPI_H
#define SS4S_MODAPI_H

#include "ss4s/audio.h"
#include "ss4s/video.h"
#include "ss4s/logging.h"

#include <stdbool.h>

typedef struct SS4S_DriverBase {
    int (*Init)(int argc, char *argv[]);

    int (*PostInit)(int argc, char *argv[]);

    void (*Quit)();
} SS4S_DriverBase;

typedef struct SS4S_PlayerContext SS4S_PlayerContext;

typedef struct SS4S_PlayerDriver {
    SS4S_PlayerContext *(*Create)();

    void (*Destroy)(SS4S_PlayerContext *context);
} SS4S_PlayerDriver;

typedef struct SS4S_AudioInstance SS4S_AudioInstance;

typedef struct SS4S_AudioDriver {
    SS4S_DriverBase Base;

    SS4S_AudioCapabilities (*GetCapabilities)();

    SS4S_AudioOpenResult (*Open)(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                 SS4S_PlayerContext *context);

    SS4S_AudioFeedResult (*Feed)(SS4S_AudioInstance *instance, const unsigned char *data, size_t size);

    void (*Close)(SS4S_AudioInstance *instance);
} SS4S_AudioDriver;

typedef struct SS4S_VideoInstance SS4S_VideoInstance;

typedef struct SS4S_VideoDriver {
    SS4S_DriverBase Base;

    SS4S_VideoCapabilities (*GetCapabilities)();

    /**
     * Required.
     * @param info Size and code of the video stream
     * @param instance Video instance field to assign
     * @param context Player context
     * @return
     */
    SS4S_VideoOpenResult (*Open)(const SS4S_VideoInfo *info, SS4S_VideoInstance **instance,
                                 SS4S_PlayerContext *context);

    /**
     * Required.
     * @param instance Video instance
     * @param data Video stream data
     * @param size Size of the data
     * @param flags
     * @return
     */
    SS4S_VideoFeedResult (*Feed)(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                 SS4S_VideoFeedFlags flags);

    bool (*SizeChanged)(SS4S_VideoInstance *instance, int width, int height);

    bool (*SetHDRInfo)(SS4S_VideoInstance *instance, const SS4S_VideoHDRInfo *info);

    bool (*SetDisplayArea)(SS4S_VideoInstance *instance, const SS4S_VideoRect *src, const SS4S_VideoRect *dst);

    void (*Close)(SS4S_VideoInstance *instance);
} SS4S_VideoDriver;

typedef struct SS4S_Module SS4S_Module;

struct SS4S_Module {
    const char *Name;
    const SS4S_PlayerDriver *PlayerDriver;
    const SS4S_AudioDriver *AudioDriver;
    const SS4S_VideoDriver *VideoDriver;
};

typedef struct SS4S_LibraryContext {
    SS4S_LoggingFunction *Log;
} SS4S_LibraryContext;

#define SS4S_MODULE_ENTRY __attribute__((unused,visibility("default")))

/**
 * Multiple calls to this function should have same effect.
 */
typedef bool (SS4S_ModuleOpenFunction)(SS4S_Module *module, const SS4S_LibraryContext *context);

#endif // SS4S_MODAPI_H
