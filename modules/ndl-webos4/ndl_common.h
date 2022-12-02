#pragma once

#include <NDL_directmedia.h>

#include "ss4s/modapi.h"

extern bool SS4S_LGNC_Initialized;

struct SS4S_PlayerContext {
    NDL_DIRECTAUDIO_DATA_INFO audioInfo;
    NDL_DIRECTVIDEO_DATA_INFO videoInfo;
};

extern const SS4S_PlayerDriver SS4S_LGNC_PlayerDriver;
extern const SS4S_AudioDriver SS4S_LGNC_AudioDriver;
extern const SS4S_VideoDriver SS4S_LGNC_VideoDriver;

void SS4S_LGNC_Driver_Init(int argc, char *argv[]);

void SS4S_LGNC_Driver_Quit();

