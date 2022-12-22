#include <string.h>
#include "ndl_common.h"

static SS4S_VideoCapabilities GetCapabilities() {
    return SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
}

static SS4S_VideoOpenResult OpenVideo(const SS4S_VideoInfo *info, SS4S_VideoInstance **instance,
                                      SS4S_PlayerContext *context) {
    memset(&context->mediaInfo.video, 0, sizeof(context->mediaInfo.video));
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
    context->mediaInfo.video.unknown1 = 0;

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
        SS4S_NDL_webOS5_Log(SS4S_LogLevelWarn, "NDL", "NDL_DirectVideoPlay returned %d: %s", rc,
                            NDL_DirectMediaGetError());
        return SS4S_VIDEO_FEED_ERROR;
    }
    return SS4S_VIDEO_FEED_OK;
}

static bool SetHDRInfo(SS4S_VideoInstance *instance, const SS4S_VideoHDRInfo *info) {
    (void) instance;
    return NDL_DirectVideoSetHDRInfo(info->displayPrimariesX[0], info->displayPrimariesY[0], info->displayPrimariesX[1],
                                     info->displayPrimariesY[1], info->displayPrimariesX[2], info->displayPrimariesY[2],
                                     info->whitePointX, info->whitePointY, info->maxDisplayMasteringLuminance,
                                     info->minDisplayMasteringLuminance, info->maxContentLightLevel,
                                     info->maxPicAverageLightLevel) == 0;
}

static void CloseVideo(SS4S_VideoInstance *instance) {
    SS4S_PlayerContext *context = (void *) instance;
    context->mediaInfo.video.type = 0;
    SS4S_NDL_webOS5_ReloadMedia(context);
}

const SS4S_VideoDriver SS4S_NDL_webOS5_VideoDriver = {
        .Base = {
                .PostInit = SS4S_NDL_webOS5_Driver_PostInit,
                .Quit = SS4S_NDL_webOS5_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenVideo,
        .Feed = FeedVideo,
        .SetHDRInfo = SetHDRInfo,
        .Close = CloseVideo,
};