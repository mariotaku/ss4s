#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct SS4S_Player SS4S_Player;

typedef enum SS4S_VideoCodec {
    SS4S_VIDEO_H264,
    SS4S_VIDEO_H265,
} SS4S_VideoCodec;

typedef enum SS4S_VideoOpenResult {
    SS4S_VIDEO_OPEN_OK = 0,
    SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC = 1,
    SS4S_VIDEO_OPEN_ERROR = -1,
} SS4S_VideoOpenResult;

typedef enum SS4S_VideoFeedResult {
    SS4S_VIDEO_FEED_OK = 0,
    SS4S_VIDEO_FEED_NOT_READY = 1,
    SS4S_VIDEO_FEED_REQUEST_KEYFRAME = 2,
    SS4S_VIDEO_FEED_ERROR = -1,
} SS4S_VideoFeedResult;

typedef enum SS4S_VideoFeedFlags {
    SS4S_VIDEO_FEED_DATA_NONE = 0,
    SS4S_VIDEO_FEED_DATA_FRAME_START = 1 << 0,
    SS4S_VIDEO_FEED_DATA_FRAME_END = 1 << 1,
    SS4S_VIDEO_FEED_DATA_KEYFRAME = 1 << 2,
} SS4S_VideoFeedFlags;

typedef struct SS4S_VideoInfo {
    SS4S_VideoCodec codec;
    int width, height;
} SS4S_VideoInfo;

typedef struct SS4S_VideoHDRInfo {
    int displayPrimariesX[3];
    int displayPrimariesY[3];
    int whitePointX;
    int whitePointY;
    int maxDisplayMasteringLuminance;
    int minDisplayMasteringLuminance;
    int maxContentLightLevel;
    int maxPicAverageLightLevel;
} SS4S_VideoHDRInfo;

#ifndef SS4S_MODAPI_H

SS4S_VideoOpenResult SS4S_PlayerVideoOpen(SS4S_Player *player, const SS4S_VideoInfo *info);

SS4S_VideoFeedResult SS4S_PlayerVideoFeed(SS4S_Player *player, const unsigned char *data, size_t size,
                                          SS4S_VideoFeedFlags flags);

bool SS4S_PlayerVideoSetHDRInfo(SS4S_Player *player, const SS4S_VideoHDRInfo *info);

bool SS4S_PlayerVideoClose(SS4S_Player *player);

#endif // SS4S_MODAPI_H