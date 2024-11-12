#include <string.h>
#include "ndl_common.h"

static SS4S_VideoOpenResult ReloadWithSize(SS4S_PlayerContext *context, int width, int height);

static uint64_t GetTimeUs();

static bool GetCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264 | SS4S_VIDEO_H265;
    capabilities->transform = SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
    capabilities->maxBitrate = 65000;
    capabilities->suggestedBitrate = 35000;
    capabilities->hdr = false;
    // If fullColorRange is set to true, the video will be displayed with over saturated colors.
    capabilities->fullColorRange = false;
    capabilities->colorSpace = SS4S_VIDEO_CAP_COLORSPACE_BT2020 | SS4S_VIDEO_CAP_COLORSPACE_BT709;
    return true;
}

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                      SS4S_VideoInstance **instance, SS4S_PlayerContext *context) {
    (void) extraInfo;
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "OpenVideo called");
    pthread_mutex_lock(&SS4S_NDL_Esplayer_Lock);
    context->mediaInfo.video_codec = NDL_ESP_VIDEO_NONE;
    SS4S_VideoOpenResult result;
    switch (info->codec) {
        case SS4S_VIDEO_H264: {
            context->mediaInfo.video_codec = NDL_ESP_VIDEO_CODEC_H264;
            break;
        }
        case SS4S_VIDEO_H265: {
            context->mediaInfo.video_codec = NDL_ESP_VIDEO_CODEC_H265;
            break;
        }
        case SS4S_VIDEO_VP9: {
            context->mediaInfo.video_codec = NDL_ESP_VIDEO_CODEC_VP9;
            break;
        }
        default:
            result = SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
            goto finish;
    }
    result = ReloadWithSize(context, info->width, info->height);
    if (result != SS4S_VIDEO_OPEN_OK) {
        goto finish;
    }
    *instance = (SS4S_VideoInstance *) context;
    result = SS4S_VIDEO_OPEN_OK;

    finish:
    pthread_mutex_unlock(&SS4S_NDL_Esplayer_Lock);
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "OpenVideo returned %d", result);
    return result;
}

static SS4S_VideoFeedResult FeedVideo(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                      SS4S_VideoFeedFlags flags) {
    (void) flags;
    SS4S_PlayerContext *context = (void *) instance;
    if (!context->mediaLoaded) {
        return SS4S_VIDEO_FEED_NOT_READY;
    }
    NDL_ESP_STREAM_BUFFER buff = {
            .data = (uint8_t *) data,
            .data_len = size,
            .offset = 0,
            .stream_type = NDL_ESP_VIDEO_ES,
            .timestamp = 0,
    };
    int rc = NDL_EsplayerFeedData(SS4S_NDL_Esplayer_Handle, &buff);
    if (rc != 0) {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelWarn, "NDL", "NDL_DirectVideoPlay returned %d", rc);
        return SS4S_VIDEO_FEED_ERROR;
    }
    return SS4S_VIDEO_FEED_OK;
}

static bool SizeChanged(SS4S_VideoInstance *instance, int width, int height) {
    pthread_mutex_lock(&SS4S_NDL_Esplayer_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    if (width <= 0 || height <= 0) {
        pthread_mutex_unlock(&SS4S_NDL_Esplayer_Lock);
        return false;
    }
    int aspectRatio = width * 100 / height;
    if (context->aspectRatio != aspectRatio) {
        ReloadWithSize(context, width, height);
    }
    pthread_mutex_unlock(&SS4S_NDL_Esplayer_Lock);
    return true;
}

static void CloseVideo(SS4S_VideoInstance *instance) {
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "CloseVideo called");
    pthread_mutex_lock(&SS4S_NDL_Esplayer_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    context->mediaInfo.video_codec = 0;
    SS4S_NDL_Esplayer_UnloadMedia(context);
    pthread_mutex_unlock(&SS4S_NDL_Esplayer_Lock);
}

static SS4S_VideoOpenResult ReloadWithSize(SS4S_PlayerContext *context, int width, int height) {
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "ReloadWithSize(%d, %d) called", width, height);
    context->mediaInfo.width = width;
    context->mediaInfo.height = height;
    context->aspectRatio = width * 100 / height;

    if (SS4S_NDL_Esplayer_ReloadMedia(context) != 0) {
        return SS4S_VIDEO_OPEN_ERROR;
    }
    return SS4S_VIDEO_OPEN_OK;
}

static uint64_t GetTimeUs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * 1000000 + (uint64_t) ts.tv_nsec / 1000;
}

const SS4S_VideoDriver SS4S_NDL_Esplayer_VideoDriver = {
        .Base = {
                .PostInit = SS4S_NDL_Esplayer_Driver_PostInit,
                .Quit = SS4S_NDL_Esplayer_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenVideo,
        .Feed = FeedVideo,
        .SizeChanged = SizeChanged,
        .Close = CloseVideo,
};
