#pragma once

#include "ss4s/modapi.h"

struct SS4S_Player {
    struct {
        SS4S_PlayerContext *audio;
        SS4S_PlayerContext *video;
    } context;
    SS4S_AudioInstance *audio;
    SS4S_VideoInstance *video;
    void *userdata;
    int viewportWidth, viewportHeight;
};

const SS4S_AudioDriver *SS4S_GetAudioDriver();

const SS4S_VideoDriver *SS4S_GetVideoDriver();

const SS4S_PlayerDriver *SS4S_GetAudioPlayerDriver();

const SS4S_PlayerDriver *SS4S_GetVideoPlayerDriver();