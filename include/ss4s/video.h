#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct SS4S_Player SS4S_Player;

typedef enum SS4S_VideoCodec {
    SS4S_VIDEO_H264,
    SS4S_VIDEO_H265,
} SS4S_VideoCodec;

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

bool SS4S_PlayerVideoOpen(SS4S_Player *player, const SS4S_VideoInfo *info);

bool SS4S_PlayerVideoFeed(SS4S_Player *player, const unsigned char *data, size_t size);

bool SS4S_PlayerVideoSetHDRInfo(SS4S_Player *player, const SS4S_VideoHDRInfo *info);

bool SS4S_PlayerVideoClose(SS4S_Player *player);

#endif // SS4S_MODAPI_H