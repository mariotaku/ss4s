#pragma once

#include <pthread.h>
#include <NDL_directmedia_v2.h>

#include "ss4s/modapi.h"
#include "opus_empty.h"
#include "opus_fix.h"

extern bool SS4S_NDL_webOS5_Initialized;
extern SS4S_LoggingFunction *SS4S_NDL_webOS5_Log;
extern pthread_mutex_t SS4S_NDL_webOS5_Lock;
extern const SS4S_LibraryContext *SS4S_NDL_webOS5_Lib;

struct SS4S_PlayerContext {
    SS4S_Player *player;
    uint64_t lastFrameTime;
    NDL_DIRECTMEDIA_DATA_INFO_T mediaInfo;
    void *streamHeader;
    SS4S_NDLOpusEmpty *opusEmpty;
    SS4S_NDLOpusFix *opusFix;
    bool mediaLoaded;
    bool waitAudioVideoReady;
    int aspectRatio;
    bool hasHdrInfo;
};

extern const SS4S_PlayerDriver SS4S_NDL_webOS5_PlayerDriver;
extern const SS4S_AudioDriver SS4S_NDL_webOS5_AudioDriver;
extern const SS4S_VideoDriver SS4S_NDL_webOS5_VideoDriver;

int SS4S_NDL_webOS5_ReloadMedia(SS4S_PlayerContext *context);

int SS4S_NDL_webOS5_UnloadMedia(SS4S_PlayerContext *context);

int SS4S_NDL_webOS5_Driver_PostInit(int argc, char *argv[]);

void SS4S_NDL_webOS5_Driver_Quit();
