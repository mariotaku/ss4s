#include "ndl_common.h"

#include <string.h>
#include <stdlib.h>

typedef struct PlayUserData {
    SS4S_PlayerContext *context;
    uint32_t beginResult;
} PlayUserData;

_Static_assert(sizeof(PlayUserData) == sizeof(unsigned long long), "PlayUserData too large");

static const SS4S_PlayerContext *CurrentContext = NULL;

static void FitVideo(const NDL_DIRECTVIDEO_DATA_INFO_T *info);

static void VideoCallback(unsigned long long userdata);

static bool GetCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264;
    capabilities->transform = SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
    capabilities->maxBitrate = 65000;
    capabilities->suggestedBitrate = 35000;
    return true;
}

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                      SS4S_VideoInstance **instance, SS4S_PlayerContext *context) {
    (void) extraInfo;
    pthread_mutex_lock(&SS4S_NDL_webOS4_Lock);
    SS4S_VideoOpenResult result;
    if (info->codec != SS4S_VIDEO_H264) {
        result = SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
        goto finish;
    }
    context->videoInfo.width = info->width;
    context->videoInfo.height = info->height;
    context->aspectRatio = info->width * 100 / info->height;

    if (NDL_DirectVideoOpen(&context->videoInfo) != 0) {
        result = SS4S_VIDEO_OPEN_ERROR;
        goto finish;
    }
    CurrentContext = context;
    NDL_DirectVideoSetCallback(VideoCallback);
    context->videoOpened = true;
    FitVideo(&context->videoInfo);
    *instance = (SS4S_VideoInstance *) context;
    result = SS4S_VIDEO_OPEN_OK;

    finish:
    pthread_mutex_unlock(&SS4S_NDL_webOS4_Lock);
    return result;
}

static SS4S_VideoFeedResult FeedVideo(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                      SS4S_VideoFeedFlags flags) {
    (void) flags;
    SS4S_PlayerContext *context = (SS4S_PlayerContext *) instance;
    if (!context->videoOpened) {
        return SS4S_VIDEO_FEED_NOT_READY;
    }
    PlayUserData *dataStruct = calloc(1, sizeof(PlayUserData));
    pthread_mutex_lock(&SS4S_NDL_webOS4_Lock);
    dataStruct->context = context;
    dataStruct->beginResult = SS4S_NDL_webOS4_LibContext->VideoStats.BeginFrame(context->player);
    pthread_mutex_unlock(&SS4S_NDL_webOS4_Lock);
    int rc = NDL_DirectVideoPlayWithCallback((void *) data, size, (unsigned int) dataStruct);
    if (rc != 0) {
        return SS4S_VIDEO_FEED_ERROR;
    }
    return SS4S_VIDEO_FEED_OK;
}

static bool SizeChanged(SS4S_VideoInstance *instance, int width, int height) {
    pthread_mutex_lock(&SS4S_NDL_webOS4_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    if (width <= 0 || height <= 0) {
        pthread_mutex_unlock(&SS4S_NDL_webOS4_Lock);
        return false;
    }
    int aspectRatio = width * 100 / height;
    if (context->aspectRatio != aspectRatio) {
        context->aspectRatio = aspectRatio;
        context->videoInfo.width = width;
        context->videoInfo.height = height;
        if (context->videoOpened) {
            NDL_DirectVideoClose();
        }
        SS4S_NDL_webOS4_Log(SS4S_LogLevelInfo, "NDL", "Reopen video with size %d * %d", width, height);
        if (NDL_DirectVideoOpen(&context->videoInfo) != 0) {
            CurrentContext = NULL;
            context->videoOpened = false;
            pthread_mutex_unlock(&SS4S_NDL_webOS4_Lock);
            return false;
        }
        FitVideo(&context->videoInfo);
    }
    pthread_mutex_unlock(&SS4S_NDL_webOS4_Lock);
    return true;
}

static void CloseVideo(SS4S_VideoInstance *instance) {
    pthread_mutex_lock(&SS4S_NDL_webOS4_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    memset(&context->videoInfo, 0, sizeof(NDL_DIRECTVIDEO_DATA_INFO_T));
    CurrentContext = NULL;
    if (context->videoOpened) {
        NDL_DirectVideoClose();
    }
    context->videoOpened = false;
    pthread_mutex_unlock(&SS4S_NDL_webOS4_Lock);
}

static void FitVideo(const NDL_DIRECTVIDEO_DATA_INFO_T *info) {
    int scaledHeight = 1920 * info->height / info->width;
    if (scaledHeight == 1080) {
        NDL_DirectVideoSetArea(0, 0, 1920, 1080);
    } else if (scaledHeight < 1080) {
        NDL_DirectVideoSetArea(0, (1080 - scaledHeight) / 2, 1920, scaledHeight);
    } else {
        int scaledWidth = 1920 * 1080 / scaledHeight;
        NDL_DirectVideoSetArea((1920 - scaledWidth) / 2, 0, scaledWidth, 1080);
    }
}

static void VideoCallback(unsigned long long userdata) {
    pthread_mutex_lock(&SS4S_NDL_webOS4_Lock);
    PlayUserData *dataStruct = (void *) (unsigned int) userdata;
    if (dataStruct->context == CurrentContext && dataStruct->context->videoOpened) {
        SS4S_NDL_webOS4_LibContext->VideoStats.EndFrame(dataStruct->context->player, dataStruct->beginResult);
    }
    pthread_mutex_unlock(&SS4S_NDL_webOS4_Lock);
    free(dataStruct);
}

const SS4S_VideoDriver SS4S_NDL_webOS4_VideoDriver = {
        .Base = {
                .Init = SS4S_NDL_webOS4_Driver_Init,
                .Quit = SS4S_NDL_webOS4_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenVideo,
        .Feed = FeedVideo,
        .SizeChanged = SizeChanged,
        .Close = CloseVideo,
};