#include "lgnc_common.h"

#include <string.h>

static SS4S_VideoCapabilities GetCapabilities() {
    return SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
}

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, SS4S_VideoInstance **instance,
                                      SS4S_PlayerContext *context) {
    if (info->codec != SS4S_VIDEO_H264) {
        return SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
    }
    context->videoInfo.vdecFmt = LGNC_VDEC_FMT_H264;
    context->videoInfo.trid_type = LGNC_VDEC_3D_TYPE_NONE;
    context->videoInfo.width = info->width;
    context->videoInfo.height = info->height;

    if (LGNC_DIRECTVIDEO_Open(&context->videoInfo) != 0) {
        return SS4S_VIDEO_OPEN_ERROR;
    }
    *instance = (SS4S_VideoInstance *) context;
    return SS4S_VIDEO_OPEN_OK;
}

static SS4S_VideoFeedResult FeedVideo(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                      SS4S_VideoFeedFlags flags) {
    (void) instance;
    (void) flags;
    int rc = LGNC_DIRECTVIDEO_Play(data, size);
    if (rc != 0) {
        return SS4S_VIDEO_FEED_ERROR;
    }
    return SS4S_VIDEO_FEED_OK;
}

static void CloseVideo(SS4S_VideoInstance *instance) {
    SS4S_PlayerContext *context = (void *) instance;
    memset(&context->videoInfo, 0, sizeof(LGNC_VDEC_DATA_INFO_T));
    LGNC_DIRECTVIDEO_Close();
}

const SS4S_VideoDriver SS4S_LGNC_VideoDriver = {
        .Base = {
                .Init = SS4S_LGNC_Driver_Init,
                .Quit = SS4S_LGNC_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenVideo,
        .Feed = FeedVideo,
        .Close = CloseVideo,
};