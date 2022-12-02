#pragma once

#include <lgnc_directaudio.h>
#include <lgnc_directvideo.h>

#include "ss4s/modapi.h"

extern bool SS4S_LGNC_Initialized;

struct SS4S_PlayerContext {
    LGNC_ADEC_DATA_INFO_T audioInfo;
    LGNC_VDEC_DATA_INFO_T videoInfo;
};

extern const SS4S_PlayerDriver SS4S_LGNC_PlayerDriver;
extern const SS4S_AudioDriver SS4S_LGNC_AudioDriver;
extern const SS4S_VideoDriver SS4S_LGNC_VideoDriver;

void SS4S_LGNC_Driver_Init();

void SS4S_LGNC_Driver_Quit();

