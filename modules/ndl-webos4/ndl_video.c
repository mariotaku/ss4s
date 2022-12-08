#include "ndl_common.h"

#include <string.h>

static SS4S_VideoCapabilities GetCapabilities() {
    return SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
}

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, SS4S_VideoInstance **instance,
                                      SS4S_PlayerContext *context) {
    if (info->codec != SS4S_VIDEO_H264) {
        return SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
    }
    context->videoInfo.width = info->width;
    context->videoInfo.height = info->height;
    context->videoOpened = true;

    if (NDL_DirectVideoOpen(&context->videoInfo) != 0) {
        return SS4S_VIDEO_OPEN_ERROR;
    }
    *instance = (SS4S_VideoInstance *) context;
    return SS4S_VIDEO_OPEN_OK;
}

static SS4S_VideoFeedResult FeedVideo(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                      SS4S_VideoFeedFlags flags) {
    (void) instance;
    (void) flags;
    int rc = NDL_DirectVideoPlay(data, size, 0);
    if (rc != 0) {
        return SS4S_VIDEO_FEED_ERROR;
    }
    return SS4S_VIDEO_FEED_OK;
}

static void CloseVideo(SS4S_VideoInstance *instance) {
    SS4S_PlayerContext *context = (void *) instance;
    memset(&context->videoInfo, 0, sizeof(NDL_DIRECTVIDEO_DATA_INFO));
    context->videoOpened = false;
    NDL_DirectVideoClose();
}

const SS4S_VideoDriver SS4S_NDL_webOS4_VideoDriver = {
        .Base = {
                .Init = SS4S_NDL_webOS4_Driver_Init,
                .Quit = SS4S_NDL_webOS4_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenVideo,
        .Feed = FeedVideo,
        .Close = CloseVideo,
};