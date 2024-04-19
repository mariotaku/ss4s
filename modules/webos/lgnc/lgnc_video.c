#include "lgnc_common.h"

#include <string.h>

static void FitVideo(const LGNC_VDEC_DATA_INFO_T *info);

static bool GetCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264;
    capabilities->transform = SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
    capabilities->maxBitrate = 40000;
    capabilities->suggestedBitrate = 30000;
    capabilities->maxFps = 60;
    capabilities->fullColorRange = false;
    capabilities->colorSpace = SS4S_VIDEO_CAP_COLORSPACE_BT709;
    return true;
}

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                      SS4S_VideoInstance **instance, SS4S_PlayerContext *context) {
    (void) extraInfo;
    if (info->codec != SS4S_VIDEO_H264) {
        return SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
    }
    context->videoInfo.vdecFmt = LGNC_VDEC_FMT_H264;
    context->videoInfo.trid_type = LGNC_VDEC_3D_TYPE_NONE;
    context->videoInfo.width = info->width;
    context->videoInfo.height = info->height;
    context->aspectRatio = info->width * 100 / info->height;

    if (LGNC_DIRECTVIDEO_Open(&context->videoInfo) != 0) {
        return SS4S_VIDEO_OPEN_ERROR;
    }
    context->videoOpened = true;
    FitVideo(&context->videoInfo);
    *instance = (SS4S_VideoInstance *) context;
    return SS4S_VIDEO_OPEN_OK;
}

static SS4S_VideoFeedResult FeedVideo(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                      SS4S_VideoFeedFlags flags) {
    (void) flags;
    SS4S_PlayerContext *context = (SS4S_PlayerContext *) instance;
    if (!context->videoOpened) {
        return SS4S_VIDEO_FEED_NOT_READY;
    }
    int rc = LGNC_DIRECTVIDEO_Play(data, size);
    if (rc != 0) {
        return SS4S_VIDEO_FEED_ERROR;
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
        context->videoInfo.width = width;
        context->videoInfo.height = height;
        if (context->videoOpened) {
            LGNC_DIRECTVIDEO_Close();
        }
        SS4S_LGNC_Log(SS4S_LogLevelInfo, "NDL", "Reopen video with size %d * %d", width, height);
        if (LGNC_DIRECTVIDEO_Open(&context->videoInfo) != 0) {
            context->videoOpened = false;
            return false;
        }
        FitVideo(&context->videoInfo);
    }
    return true;
}

static void CloseVideo(SS4S_VideoInstance *instance) {
    SS4S_PlayerContext *context = (void *) instance;
    memset(&context->videoInfo, 0, sizeof(LGNC_VDEC_DATA_INFO_T));
    if (context->videoOpened) {
        LGNC_DIRECTVIDEO_Close();
    }
    context->videoOpened = false;
}

static void FitVideo(const LGNC_VDEC_DATA_INFO_T *info) {
    int scaledHeight = 1920 * info->height / info->width;
    if (scaledHeight == 1080) {
        _LGNC_DIRECTVIDEO_SetDisplayWindow(0, 0, 1920, 1080);
    } else if (scaledHeight < 1080) {
        _LGNC_DIRECTVIDEO_SetDisplayWindow(0, (1080 - scaledHeight) / 2, 1920, scaledHeight);
    } else {
        int scaledWidth = 1920 * 1080 / scaledHeight;
        _LGNC_DIRECTVIDEO_SetDisplayWindow((1920 - scaledWidth) / 2, 0, scaledWidth, 1080);
    }
}

const SS4S_VideoDriver SS4S_LGNC_VideoDriver = {
        .Base = {
                .Init = SS4S_LGNC_Driver_Init,
                .Quit = SS4S_LGNC_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenVideo,
        .Feed = FeedVideo,
        .SizeChanged = SizeChanged,
        .Close = CloseVideo,
};