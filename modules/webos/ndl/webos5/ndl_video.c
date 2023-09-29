#include <string.h>
#include "ndl_common.h"

static SS4S_VideoOpenResult ReloadWithSize(SS4S_PlayerContext *context, int width, int height);

static bool GetCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264 | SS4S_VIDEO_H265 | SS4S_VIDEO_VP9 | SS4S_VIDEO_AV1;
    capabilities->transform = SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
    capabilities->maxBitrate = 65000;
    capabilities->suggestedBitrate = 35000;
    capabilities->hdr = true;
    // If fullColorRange is set to true, the video will be displayed with over saturated colors.
    capabilities->fullColorRange = false;
    return true;
}

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                      SS4S_VideoInstance **instance, SS4S_PlayerContext *context) {
    (void) extraInfo;
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "OpenVideo called");
    pthread_mutex_lock(&SS4S_NDL_webOS5_Lock);
    memset(&context->mediaInfo.video, 0, sizeof(context->mediaInfo.video));
    SS4S_VideoOpenResult result;
    switch (info->codec) {
        case SS4S_VIDEO_H264: {
            context->mediaInfo.video.type = NDL_VIDEO_TYPE_H264;
            break;
        }
        case SS4S_VIDEO_H265: {
            context->mediaInfo.video.type = NDL_VIDEO_TYPE_H265;
            break;
        }
        case SS4S_VIDEO_VP9: {
            context->mediaInfo.video.type = NDL_VIDEO_TYPE_VP9;
            break;
        }
        case SS4S_VIDEO_AV1: {
            context->mediaInfo.video.type = NDL_VIDEO_TYPE_AV1;
            break;
        }
        default:
            result = SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
            goto finish;
    }
    context->mediaInfo.video.unknown1 = 0;
    result = ReloadWithSize(context, info->width, info->height);
    if (result != SS4S_VIDEO_OPEN_OK) {
        goto finish;
    }
    *instance = (SS4S_VideoInstance *) context;
    result = SS4S_VIDEO_OPEN_OK;

    finish:
    pthread_mutex_unlock(&SS4S_NDL_webOS5_Lock);
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "OpenVideo returned %d", result);
    return result;
}

static SS4S_VideoFeedResult FeedVideo(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                      SS4S_VideoFeedFlags flags) {
    (void) flags;
    const SS4S_PlayerContext *context = (void *) instance;
    if (!context->mediaLoaded) {
        return SS4S_VIDEO_FEED_NOT_READY;
    }
    int rc = NDL_DirectVideoPlay((void *) data, size, 0);
    if (rc != 0) {
        SS4S_NDL_webOS5_Log(SS4S_LogLevelWarn, "NDL", "NDL_DirectVideoPlay returned %d: %s", rc,
                            NDL_DirectMediaGetError());
        return SS4S_VIDEO_FEED_ERROR;
    }
    return SS4S_VIDEO_FEED_OK;
}

static bool SizeChanged(SS4S_VideoInstance *instance, int width, int height) {
    pthread_mutex_lock(&SS4S_NDL_webOS5_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    if (width <= 0 || height <= 0) {
        pthread_mutex_unlock(&SS4S_NDL_webOS5_Lock);
        return false;
    }
    int aspectRatio = width * 100 / height;
    if (context->aspectRatio != aspectRatio) {
        ReloadWithSize(context, width, height);
    }
    pthread_mutex_unlock(&SS4S_NDL_webOS5_Lock);
    return true;
}

static bool SetHDRInfo(SS4S_VideoInstance *instance, const SS4S_VideoHDRInfo *info) {
    SS4S_PlayerContext *context = (void *) instance;
    pthread_mutex_lock(&SS4S_NDL_webOS5_Lock);
    bool hasHdrInfo = info != NULL, needsReload = context->hasHdrInfo && !hasHdrInfo;
    context->hasHdrInfo = hasHdrInfo;
    if (needsReload) {
        int reloadResult = SS4S_NDL_webOS5_ReloadMedia(context);
        pthread_mutex_unlock(&SS4S_NDL_webOS5_Lock);
        return reloadResult == 0;
    }
    pthread_mutex_unlock(&SS4S_NDL_webOS5_Lock);
    if (info == NULL) {
        return true;
    }
    return NDL_DirectVideoSetHDRInfo(info->displayPrimariesX[0], info->displayPrimariesY[0], info->displayPrimariesX[1],
                                     info->displayPrimariesY[1], info->displayPrimariesX[2], info->displayPrimariesY[2],
                                     info->whitePointX, info->whitePointY, info->maxDisplayMasteringLuminance,
                                     info->minDisplayMasteringLuminance, info->maxContentLightLevel,
                                     info->maxPicAverageLightLevel) == 0;
}

static void CloseVideo(SS4S_VideoInstance *instance) {
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "CloseVideo called");
    pthread_mutex_lock(&SS4S_NDL_webOS5_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    context->mediaInfo.video.type = 0;
    SS4S_NDL_webOS5_ReloadMedia(context);
    pthread_mutex_unlock(&SS4S_NDL_webOS5_Lock);
}

static SS4S_VideoOpenResult ReloadWithSize(SS4S_PlayerContext *context, int width, int height) {
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "ReloadWithSize(%d, %d) called", width, height);
    context->mediaInfo.video.width = width;
    context->mediaInfo.video.height = height;
    context->aspectRatio = width * 100 / height;

    if (SS4S_NDL_webOS5_ReloadMedia(context) != 0) {
        return SS4S_VIDEO_OPEN_ERROR;
    }
    return SS4S_VIDEO_OPEN_OK;
}

const SS4S_VideoDriver SS4S_NDL_webOS5_VideoDriver = {
        .Base = {
                .PostInit = SS4S_NDL_webOS5_Driver_PostInit,
                .Quit = SS4S_NDL_webOS5_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenVideo,
        .Feed = FeedVideo,
        .SizeChanged = SizeChanged,
        .SetHDRInfo = SetHDRInfo,
        .Close = CloseVideo,
};