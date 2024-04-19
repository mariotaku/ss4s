#include "sl_common.h"

#include <string.h>

static bool GetCapabilities(SS4S_VideoCapabilities *capabilities, SS4S_VideoCodec wantedCodecs) {
    (void) wantedCodecs;
    capabilities->codecs = SS4S_VIDEO_H264;
    capabilities->maxBitrate = 30000;
    capabilities->suggestedBitrate = 25000;
    capabilities->maxWidth = 1920;
    capabilities->maxHeight = 1080;
    capabilities->maxFps = 60;
    capabilities->transform = SS4S_VIDEO_CAP_TRANSFORM_UI_EXCLUSIVE;
    capabilities->colorSpace = SS4S_VIDEO_CAP_COLORSPACE_BT709;
    capabilities->fullColorRange = false;
    return true;
}

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                      SS4S_VideoInstance **instance, SS4S_PlayerContext *context) {
    pthread_mutex_lock(&SS4S_SteamLink_Lock);
    SS4S_VideoOpenResult result;
    if (info->codec != SS4S_VIDEO_H264) {
        result = SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
        goto finish;
    }

    CSLVideoContext *videoContext = SLVideo_CreateContext();
    if (videoContext == NULL) {
        result = SS4S_VIDEO_OPEN_ERROR;
        goto finish;
    }
    context->videoContext = videoContext;

    CSLVideoStream *videoStream = SLVideo_CreateStream(videoContext, k_ESLVideoFormatH264, 1);
    if (videoStream == NULL) {
        result = SS4S_VIDEO_OPEN_ERROR;
        goto finish;
    }
    context->videoStream = videoStream;


    SLVideo_SetStreamVideoTransferMatrix(videoStream, k_ESLVideoTransferMatrix_BT709);
    if (info->frameRateNumerator > 0 && info->frameRateDenominator > 0) {
        SLVideo_SetStreamTargetFramerate(videoStream, info->frameRateNumerator, info->frameRateDenominator);
    }

    // Hack to force showing the video
    int width = 1920, height = 1080;
    SLVideo_GetDisplayResolution(videoContext, &width, &height);
    CSLVideoOverlay *overlay = SLVideo_CreateOverlay(videoContext, width, height);
    SLVideo_ShowOverlay(overlay);
    SLVideo_HideOverlay(overlay);
    SLVideo_FreeOverlay(overlay);

    context->viewportWidth = extraInfo->viewportWidth;
    context->viewportHeight = extraInfo->viewportHeight;

    *instance = (SS4S_VideoInstance *) context;
    result = SS4S_VIDEO_OPEN_OK;

    finish:
    pthread_mutex_unlock(&SS4S_SteamLink_Lock);
    return result;
}

static SS4S_VideoFeedResult FeedVideo(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                      SS4S_VideoFeedFlags flags) {
    (void) flags;
    SS4S_PlayerContext *context = (SS4S_PlayerContext *) instance;
    if (context->videoStream == NULL) {
        return SS4S_VIDEO_FEED_NOT_READY;
    }
    if (SLVideo_BeginFrame(context->videoStream, (int) size) != 0) {
        return SS4S_VIDEO_FEED_REQUEST_KEYFRAME;
    }
    if (SLVideo_WriteFrameData(context->videoStream, (void *) data, (int) size) != 0) {
        return SS4S_VIDEO_FEED_REQUEST_KEYFRAME;
    }
    if (SLVideo_SubmitFrame(context->videoStream) != 0) {
        return SS4S_VIDEO_FEED_REQUEST_KEYFRAME;
    }
    return SS4S_VIDEO_FEED_OK;
}

static void CloseVideo(SS4S_VideoInstance *instance) {
    pthread_mutex_lock(&SS4S_SteamLink_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    SLVideo_FreeStream(context->videoStream);
    context->videoStream = NULL;

    if (context->viewportWidth > 0 && context->viewportHeight > 0) {
        // Hack to force update the framebuffer
        CSLVideoOverlay *overlay = SLVideo_CreateOverlay(context->videoContext, context->viewportWidth,
                                                         context->viewportHeight);
        SLVideo_ShowOverlay(overlay);
        SLVideo_HideOverlay(overlay);
        SLVideo_FreeOverlay(overlay);
    }

    SLVideo_FreeContext(context->videoContext);
    context->videoContext = NULL;
    pthread_mutex_unlock(&SS4S_SteamLink_Lock);
}

const SS4S_VideoDriver SS4S_STEAMLINK_VideoDriver = {
        .GetCapabilities = GetCapabilities,
        .Open = OpenVideo,
        .Feed = FeedVideo,
        .Close = CloseVideo,
};