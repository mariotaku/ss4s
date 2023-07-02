#pragma once

#include "ss4s/modapi.h"

#include <pthread.h>

#include <SLAudio.h>
#include <SLVideo.h>


extern SS4S_LoggingFunction *SS4S_SteamLink_Log;
extern pthread_mutex_t SS4S_SteamLink_Lock;

struct SS4S_PlayerContext {
    CSLAudioStream *audioStream;
    CSLAudioContext *audioContext;
    size_t audioFrameSize;
    CSLVideoStream *videoStream;
    CSLVideoContext *videoContext;
};

extern const SS4S_PlayerDriver SS4S_STEAMLINK_PlayerDriver;
extern const SS4S_AudioDriver SS4S_STEAMLINK_AudioDriver;
extern const SS4S_VideoDriver SS4S_STEAMLINK_VideoDriver;

