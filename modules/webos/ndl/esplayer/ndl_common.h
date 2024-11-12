#pragma once

#include <pthread.h>
#include <ndl-directmedia2/esplayer-api.h>

#include "ss4s/modapi.h"

extern SS4S_LoggingFunction *SS4S_NDL_Esplayer_Log;
extern pthread_mutex_t SS4S_NDL_Esplayer_Lock;

struct SS4S_PlayerContext {
    SS4S_Player *player;
    NDL_EsplayerHandle handle;
    NDL_ESP_META_DATA mediaInfo;
    void *streamHeader;
    bool mediaLoaded;
    bool waitAudioVideoReady;
    int aspectRatio;
    bool hasHdrInfo;
};

extern const SS4S_PlayerDriver SS4S_NDL_Esplayer_PlayerDriver;
extern const SS4S_AudioDriver SS4S_NDL_Esplayer_AudioDriver;
extern const SS4S_VideoDriver SS4S_NDL_Esplayer_VideoDriver;

int SS4S_NDL_Esplayer_ReloadMedia(SS4S_PlayerContext *context);

int SS4S_NDL_Esplayer_UnloadMedia(SS4S_PlayerContext *context);

int SS4S_NDL_Esplayer_Driver_PostInit(int argc, char *argv[]);

void SS4S_NDL_Esplayer_Driver_Quit();
