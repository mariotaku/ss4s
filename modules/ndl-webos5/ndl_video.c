#include "ndl_common.h"

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, SS4S_VideoInstance **instance,
                                      SS4S_PlayerContext *context) {
    switch (info->codec) {
        case SS4S_VIDEO_H264: {
            context->mediaInfo.video.type = NDL_VIDEO_TYPE_H264;
            break;
        }
        case SS4S_VIDEO_H265: {
            context->mediaInfo.video.type = NDL_VIDEO_TYPE_H265;
            break;
        }
        default:
            return SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
    }
    context->mediaInfo.video.width = info->width;
    context->mediaInfo.video.height = info->height;

    if (SS4S_NDL_webOS5_ReloadMedia(context) != 0) {
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
    context->mediaInfo.video.type = 0;
    SS4S_NDL_webOS5_ReloadMedia(context);
}

const SS4S_VideoDriver SS4S_NDL_webOS5_VideoDriver = {
        .Open = OpenVideo,
        .Feed = FeedVideo,
        .Close = CloseVideo,
};