#pragma once

#include <stdbool.h>
#include <pbnjson.h>

#include "ss4s/modapi.h"

typedef struct StarfishResource StarfishResource;

StarfishResource *StarfishResourceCreate(const char *appId);

void StarfishResourceDestroy(StarfishResource *res);

jvalue_ref StarfishResourceMakeLoadPayload(StarfishResource *resource, const SS4S_AudioInfo *audioInfo,
                                           const SS4S_VideoInfo *videoInfo);

bool StarfishResourceSetMediaVideoData(StarfishResource *resource, const char *data, bool hdr);

bool StarfishResourceLoadCompleted(StarfishResource *resource, const char *mediaId);

bool StarfishResourcePostLoad(StarfishResource *resource, const SS4S_VideoInfo *info);

bool StarfishResourceStartPlaying(StarfishResource *resource);

bool StarfishResourcePostUnload(StarfishResource *resource);


const char *StarfishAudioCodecName(SS4S_AudioCodec codec);

const char *StarfishVideoCodecName(SS4S_VideoCodec codec);
