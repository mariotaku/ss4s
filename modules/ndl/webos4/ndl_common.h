#pragma once

#include <pthread.h>
#include <NDL_directmedia.h>

#include "ss4s/modapi.h"

extern bool SS4S_NDL_webOS4_Initialized;
extern SS4S_LoggingFunction *SS4S_NDL_webOS4_Log;
extern pthread_mutex_t SS4S_NDL_webOS4_Lock;

struct SS4S_PlayerContext {
    bool audioOpened, videoOpened;
    NDL_DIRECTAUDIO_DATA_INFO audioInfo;
    NDL_DIRECTVIDEO_DATA_INFO videoInfo;
    int aspectRatio;
};

extern const SS4S_PlayerDriver SS4S_NDL_webOS4_PlayerDriver;
extern const SS4S_AudioDriver SS4S_NDL_webOS4_AudioDriver;
extern const SS4S_VideoDriver SS4S_NDL_webOS4_VideoDriver;

int SS4S_NDL_webOS4_Driver_Init(int argc, char *argv[]);

void SS4S_NDL_webOS4_Driver_Quit();

