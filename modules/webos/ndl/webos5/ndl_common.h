#pragma once

#include <pthread.h>
#include <NDL_directmedia_v2.h>

#include "ss4s/modapi.h"
#include "ndl_logging.h"

extern bool SS4S_NDL_webOS5_Initialized;
extern pthread_mutex_t SS4S_NDL_webOS5_Lock;
extern const SS4S_LibraryContext *SS4S_NDL_webOS5_Lib;

struct SS4S_PlayerContext {
    SS4S_Player *player;
    uint64_t lastFrameTime;
    NDL_DIRECTMEDIA_DATA_INFO_T mediaInfo;
    struct SS4S_NDLOpusFix *opusFix;
    bool mediaLoaded;
    struct timespec mediaLoadedTime;
    bool waitAudioVideoReady;
    int aspectRatio;
    bool hasHdrInfo;
};

extern const SS4S_PlayerDriver SS4S_NDL_webOS5_PlayerDriver;
extern const SS4S_AudioDriver SS4S_NDL_webOS5_AudioDriver;
extern const SS4S_VideoDriver SS4S_NDL_webOS5_VideoDriver;

int SS4S_NDL_webOS5_ReloadMedia(SS4S_PlayerContext *context);

int SS4S_NDL_webOS5_UnloadMedia(SS4S_PlayerContext *context);

uint64_t SS4S_NDL_webOS5_GetPts(const SS4S_PlayerContext *context);

int SS4S_NDL_webOS5_Driver_PostInit(int argc, char *argv[]);

void SS4S_NDL_webOS5_Driver_Quit();
