#pragma once

#include <NDL_directmedia_v2.h>

#include "ss4s/modapi.h"

extern bool SS4S_NDL_webOS5_Initialized;

struct SS4S_PlayerContext {
    NDL_DIRECTMEDIA_DATA_INFO mediaInfo;
    bool mediaLoaded;
};

extern const SS4S_PlayerDriver SS4S_NDL_webOS5_PlayerDriver;
extern const SS4S_AudioDriver SS4S_NDL_webOS5_AudioDriver;
extern const SS4S_VideoDriver SS4S_NDL_webOS5_VideoDriver;

int SS4S_NDL_webOS5_ReloadMedia(SS4S_PlayerContext *context);