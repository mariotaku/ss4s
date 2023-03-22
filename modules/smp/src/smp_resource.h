#pragma once

#include <stdbool.h>
#include <pbnjson.h>

#include "ss4s/modapi.h"

typedef struct StarfishResource StarfishResource;

StarfishResource *StarfishResourceCreate(const char *appId);

void StarfishResourceDestroy(StarfishResource *res);

bool StarfishResourceUpdateLoadPayload(StarfishResource *resource, jvalue_ref payload, const SS4S_VideoInfo *info);

bool StarfishResourceSetMediaVideoData(StarfishResource *resource, const char *data);

bool StarfishResourceLoadCompleted(StarfishResource *resource, const char *mediaId);

bool StarfishResourcePostLoad(StarfishResource *resource, const SS4S_VideoInfo *info);

bool StarfishResourceStartPlaying(StarfishResource *resource);

bool StarfishResourcePostUnload(StarfishResource *resource);