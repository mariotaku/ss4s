#pragma once

#include <stdbool.h>
#include <pbnjson.h>

#include "ss4s/modapi.h"

typedef struct StarfishResource StarfishResource;

StarfishResource *StarfishResourceCreate();

void StarfishResourceDestroy(StarfishResource *res);

bool StarfishResourceUpdateLoadPayload(StarfishResource *resource, jvalue_ref payload, const SS4S_VideoInfo *info);

bool StarfishResourcePostLoad(StarfishResource *resource, const SS4S_VideoInfo *info);