#pragma once

#include "ss4s/modapi.h"

struct SS4S_Player {
    SS4S_AudioInstance *audio;
    SS4S_VideoInstance *video;
};

const SS4S_AudioDriver *SS4S_GetAudioDriver();

const SS4S_VideoDriver *SS4S_GetVideoDriver();

const char *SS4S_GetAudioModuleName();

const char *SS4S_GetVideoModuleName();

const char *SS4S_GetAppName();