#pragma once

#include <pthread.h>
#include <NDL_directmedia_v2.h>

#include "ss4s/modapi.h"

extern bool SS4S_NDL_webOS5_Initialized;
extern SS4S_LoggingFunction *SS4S_NDL_webOS5_Log;
extern pthread_mutex_t SS4S_NDL_webOS5_Lock;

struct SS4S_PlayerContext {
    NDL_DIRECTMEDIA_DATA_INFO mediaInfo;
    bool mediaLoaded;
    int aspectRatio;
};

extern const SS4S_PlayerDriver SS4S_NDL_webOS5_PlayerDriver;
extern const SS4S_AudioDriver SS4S_NDL_webOS5_AudioDriver;
extern const SS4S_VideoDriver SS4S_NDL_webOS5_VideoDriver;

int SS4S_NDL_webOS5_ReloadMedia(SS4S_PlayerContext *context);

int SS4S_NDL_webOS5_Driver_PostInit(int argc, char *argv[]);

void SS4S_NDL_webOS5_Driver_Quit();
