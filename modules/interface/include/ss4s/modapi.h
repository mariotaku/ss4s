#pragma once

#ifndef SS4S_MODAPI_H
#define SS4S_MODAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ss4s/audio.h"
#include "ss4s/video.h"
#include "ss4s/module.h"
#include "ss4s/logging.h"
#include "ss4s/stats.h"

#include <stdbool.h>

typedef struct SS4S_DriverBase {
    int (*Init)(int argc, char *argv[]);

    int (*PostInit)(int argc, char *argv[]);

    void (*Quit)();
} SS4S_DriverBase;

typedef struct SS4S_PlayerContext SS4S_PlayerContext;

typedef struct SS4S_PlayerDriver {
    /**
     * Required.
     * @param player Opaque player pointer
     * @return
     */
    SS4S_PlayerContext *(*Create)(SS4S_Player *player);

    void (*Destroy)(SS4S_PlayerContext *context);

    void (*SetWaitAudioVideoReady)(SS4S_PlayerContext *context, bool option);
} SS4S_PlayerDriver;

typedef struct SS4S_AudioInstance SS4S_AudioInstance;

typedef struct SS4S_AudioDriver {
    SS4S_DriverBase Base;

    /**
     * Get audio capabilities by specified codecs. The capabilities should be an union of all codecs.
     * This function is optional.
     * @param capabilities Audio capabilities field to assign
     * @param wantedCodecs Codecs to get capabilities for
     * @return True if the capabilities are available, false otherwise
     */
    bool (*GetCapabilities)(SS4S_AudioCapabilities *capabilities, SS4S_AudioCodec wantedCodecs);

    /**
     * Get preferred codecs for the specified audio configuration.
     * This function is optional.
     * @param info Audio info. It doesn't have to be complete.
     * @return Preferred codecs for the specified info
     */
    SS4S_AudioCodec (*GetPreferredCodecs)(const SS4S_AudioInfo *info);

    SS4S_AudioOpenResult (*Open)(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                 SS4S_PlayerContext *context);

    SS4S_AudioFeedResult (*Feed)(SS4S_AudioInstance *instance, const unsigned char *data, size_t size);

    void (*Close)(SS4S_AudioInstance *instance);
} SS4S_AudioDriver;

typedef struct SS4S_VideoInstance SS4S_VideoInstance;

typedef struct SS4S_VideoDriver {
    SS4S_DriverBase Base;

    /**
     * Get video capabilities. The capabilities should be an union of all codecs.
     * This function is optional.
     * @param capabilities Video capabilities field to assign
     * @return True if the capabilities are available, false otherwise
     */
    bool (*GetCapabilities)(SS4S_VideoCapabilities *capabilities);

    /**
     * Get preferred codecs for the specified video configuration.
     * This function is optional.
     * @param info Video info. It doesn't have to be complete.
     * @return Preferred codecs for the specified info
     */
    SS4S_VideoCodec (*GetPreferredCodecs)(const SS4S_VideoInfo *info);

    /**
     * Required.
     * @param info Size and code of the video stream
     * @param instance Video instance field to assign
     * @param context Player context
     * @return
     */
    SS4S_VideoOpenResult (*Open)(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                 SS4S_VideoInstance **instance, SS4S_PlayerContext *context);

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
    struct {
        SS4S_VideoStatsBeginFrameFunction *BeginFrame;
        SS4S_VideoStatsEndFrameFunction *EndFrame;
        SS4S_VideoStatsReportFrameFunction *ReportFrame;
    } VideoStats;
} SS4S_LibraryContext;

#define SS4S_EXPORTED __attribute__((unused, visibility("default")))

/**
 * Multiple calls to this function should have same effect.
 */
typedef bool (SS4S_ModuleOpenFunction)(SS4S_Module *module, const SS4S_LibraryContext *context);

typedef SS4S_ModuleCheckFlag (SS4S_ModuleCheckFunction)(SS4S_ModuleCheckFlag flags);

#ifdef __cplusplus
}
#endif

#endif // SS4S_MODAPI_H
