#pragma once

#include "ss4s/modapi.h"
#include "stats.h"
#include "mutex.h"

struct SS4S_Player {
    struct {
        SS4S_PlayerContext *audio;
        SS4S_PlayerContext *video;
    } context;
    SS4S_AudioInstance *audio;
    SS4S_VideoInstance *video;
    void *userdata;
    int viewportWidth, viewportHeight;
    SS4S_Mutex *mutex;
    struct {
        SS4S_StatsCounter video;
    } stats;
};

const SS4S_AudioDriver *SS4S_GetAudioDriver();

const SS4S_VideoDriver *SS4S_GetVideoDriver();

const SS4S_PlayerDriver *SS4S_GetAudioPlayerDriver();

const SS4S_PlayerDriver *SS4S_GetVideoPlayerDriver();


uint32_t SS4S_VideoStatsBeginFrame(SS4S_Player *player);

void SS4S_VideoStatsEndFrame(SS4S_Player *player, uint32_t beginFrameResult);

void SS4S_VideoStatsReportFrame(SS4S_Player *player, uint32_t latencyUs);
