#pragma once

#include <lgnc_directaudio.h>
#include <lgnc_directvideo.h>
#include <pthread.h>

#include "ss4s/modapi.h"

extern bool SS4S_LGNC_Initialized;
extern pthread_mutex_t SS4S_LGNC_Lock;
extern SS4S_LoggingFunction *SS4S_LGNC_Log;

struct SS4S_PlayerContext {
    bool audioOpened, videoOpened;
    LGNC_ADEC_DATA_INFO_T audioInfo;
    LGNC_VDEC_DATA_INFO_T videoInfo;
    int aspectRatio;
};

extern const SS4S_PlayerDriver SS4S_LGNC_PlayerDriver;
extern const SS4S_AudioDriver SS4S_LGNC_AudioDriver;
extern const SS4S_VideoDriver SS4S_LGNC_VideoDriver;

int SS4S_LGNC_Driver_Init();

void SS4S_LGNC_Driver_Quit();

