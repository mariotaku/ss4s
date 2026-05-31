#include "dummy_common.h"

static SS4S_VideoOpenResult ReloadWithSize(SS4S_PlayerContext *context, int width, int height);

static bool GetCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264 | SS4S_VIDEO_H265;
    capabilities->transform = SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING | SS4S_VIDEO_CAP_TRANSFORM_AREA_SRC |
                              SS4S_VIDEO_CAP_TRANSFORM_AREA_DEST;
    return true;
}

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                      SS4S_VideoInstance **instance, SS4S_PlayerContext *context) {
    SS4S_Dummy_Log(SS4S_LogLevelInfo, "Dummy", "%s(codec=%s, width=%d, height=%d)", __FUNCTION__,
                   SS4S_VideoCodecName(info->codec), info->width, info->height);
    SS4S_VideoOpenResult result = ReloadWithSize(context, info->width, info->height);
    if (result != SS4S_VIDEO_OPEN_OK) {
        return result;
    }
    *instance = (SS4S_VideoInstance *) context;
    return SS4S_VIDEO_OPEN_OK;
}

static SS4S_VideoFeedResult FeedVideo(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                      SS4S_VideoFeedFlags flags) {
    (void) flags;
    (void) data;
    (void) size;
    SS4S_PlayerContext *context = (void *) instance;
    if (!SS4S_Dummy_CheckFeedSafe(context)) {
        return SS4S_VIDEO_FEED_NOT_READY;
    }
    return SS4S_VIDEO_FEED_OK;
}

static bool SizeChanged(SS4S_VideoInstance *instance, int width, int height) {
    SS4S_Dummy_Log(SS4S_LogLevelInfo, "Dummy", "%s(width=%d, height=%d)", __FUNCTION__, width, height);
    SS4S_PlayerContext *context = (void *) instance;
    if (width <= 0 || height <= 0) {
        return false;
    }
    int aspectRatio = width * 100 / height;
    if (context->aspectRatio != aspectRatio) {
        SS4S_Dummy_EnterDestructive(context, 5);
        context->aspectRatio = aspectRatio;
        ReloadWithSize(context, width, height);
        SS4S_Dummy_ExitDestructive(context);
    }
    return true;
}

static bool SetHDRInfo(SS4S_VideoInstance *instance, const SS4S_VideoHDRInfo *info) {
    (void) info;
    SS4S_PlayerContext *context = (void *) instance;
    /* Simulate the Unload+Load race window that ndl-webos5 has on
     * SetHDRInfo(NULL). The sleep is the window in which an in-flight
     * Feed would crash the real decoder. */
    SS4S_Dummy_EnterDestructive(context, 5);
    SS4S_Dummy_ExitDestructive(context);
    return true;
}

static void CloseVideo(SS4S_VideoInstance *instance) {
    SS4S_Dummy_Log(SS4S_LogLevelInfo, "Dummy", "%s()", __FUNCTION__);
    SS4S_PlayerContext *context = (void *) instance;
    SS4S_Dummy_EnterDestructive(context, 0);
    SS4S_Dummy_ReloadMedia(context);
    SS4S_Dummy_ExitDestructive(context);
}

static SS4S_VideoOpenResult ReloadWithSize(SS4S_PlayerContext *context, int width, int height) {
    (void) width;
    (void) height;
    if (SS4S_Dummy_ReloadMedia(context) != 0) {
        return SS4S_VIDEO_OPEN_ERROR;
    }
    return SS4S_VIDEO_OPEN_OK;
}

const SS4S_VideoDriver SS4S_Dummy_VideoDriver = {
        .Base = {
                .PostInit = SS4S_Dummy_Driver_PostInit,
                .Quit = SS4S_Dummy_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenVideo,
        .Feed = FeedVideo,
        .SizeChanged = SizeChanged,
        .SetHDRInfo = SetHDRInfo,
        .Close = CloseVideo,
};
