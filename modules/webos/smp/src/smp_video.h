#pragma once

#include "StarfishMediaAPIs_C.h"
#include "ss4s/modapi.h"

typedef struct SS4S_VideoInstance StarfishVideo;

StarfishVideo *StarfishVideoCreate(const SS4S_LibraryContext *lib, SS4S_Player*player);

void StarfishVideoDestroy(StarfishVideo *ctx);

bool StarfishVideoCapabilities(SS4S_VideoCapabilities *capabilities);

SS4S_VideoOpenResult StarfishVideoLoad(StarfishVideo *ctx, const SS4S_VideoInfo *info);

bool StarfishVideoUnload(StarfishVideo *ctx);

SS4S_VideoFeedResult StarfishVideoFeed(StarfishVideo *ctx, const unsigned char *data, size_t size,
                                       SS4S_VideoFeedFlags flags);

bool StarfishVideoSizeChanged(StarfishVideo *ctx, int width, int height);

bool StarfishVideoSetHDRInfo(StarfishVideo *ctx, const SS4S_VideoHDRInfo *info);

bool StarfishVideoSetDisplayArea(StarfishVideo *ctx, const SS4S_VideoRect *src, const SS4S_VideoRect *dst);
