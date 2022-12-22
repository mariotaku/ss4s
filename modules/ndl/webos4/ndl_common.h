#pragma once

#include "NDL_directmedia.h"

#include "ss4s/modapi.h"

extern bool SS4S_NDL_webOS4_Initialized;

struct SS4S_PlayerContext {
    bool audioOpened, videoOpened;
    NDL_DIRECTAUDIO_DATA_INFO audioInfo;
    NDL_DIRECTVIDEO_DATA_INFO videoInfo;
};

extern const SS4S_PlayerDriver SS4S_NDL_webOS4_PlayerDriver;
extern const SS4S_AudioDriver SS4S_NDL_webOS4_AudioDriver;
extern const SS4S_VideoDriver SS4S_NDL_webOS4_VideoDriver;

int SS4S_NDL_webOS4_Driver_Init(int argc, char *argv[]);

void SS4S_NDL_webOS4_Driver_Quit();

