#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum SS4S_AudioCodec {
    SS4S_AUDIO_PCM,
    SS4S_AUDIO_OPUS,
} SS4S_AudioCodec;

typedef struct SS4S_AudioInfo {
    SS4S_AudioCodec codec;
    int sampleRate;
    int numOfChannels;
    int samplesPerFrame;
    const unsigned char *codecData;
    const size_t codecDataLen;
} SS4S_AudioInfo;

typedef struct SS4S_Player SS4S_Player;

#ifndef SS4S_MODAPI_H

bool SS4S_PlayerAudioOpen(SS4S_Player *player, const SS4S_AudioInfo *info);

bool SS4S_PlayerAudioFeed(SS4S_Player *player, const unsigned char *data, size_t size);

bool SS4S_PlayerAudioClose(SS4S_Player *player);

#endif // SS4S_MODAPI_H