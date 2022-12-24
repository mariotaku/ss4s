#include "dummy_common.h"

static SS4S_VideoOpenResult ReloadWithSize(SS4S_PlayerContext *context, int width, int height);

static SS4S_VideoCapabilities GetCapabilities() {
    return SS4S_VIDEO_CAP_CODEC_H264 | SS4S_VIDEO_CAP_CODEC_H265 | SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
}

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, SS4S_VideoInstance **instance,
                                      SS4S_PlayerContext *context) {
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
    SS4S_PlayerContext *context = (void *) instance;
    if (!context->mediaLoaded) {
        return SS4S_VIDEO_FEED_NOT_READY;
    }
    return SS4S_VIDEO_FEED_OK;
}

static bool SizeChanged(SS4S_VideoInstance *instance, int width, int height) {
    SS4S_PlayerContext *context = (void *) instance;
    if (width <= 0 || height <= 0) {
        return false;
    }
    int aspectRatio = width * 100 / height;
    if (context->aspectRatio != aspectRatio) {
        context->aspectRatio = aspectRatio;
        ReloadWithSize(context, width, height);
    }
    return true;
}

static bool SetHDRInfo(SS4S_VideoInstance *instance, const SS4S_VideoHDRInfo *info) {
    (void) instance;
    (void) info;
    return true;
}

static void CloseVideo(SS4S_VideoInstance *instance) {
    SS4S_Dummy_Log(SS4S_LogLevelInfo, "Dummy", "%s()", __FUNCTION__);
    SS4S_PlayerContext *context = (void *) instance;
    SS4S_Dummy_ReloadMedia(context);
}

static SS4S_VideoOpenResult ReloadWithSize(SS4S_PlayerContext *context, int width, int height) {
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