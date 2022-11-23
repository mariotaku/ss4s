#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum SS4S_AudioCodec {
    SS4S_AUDIO_PCM_S16LE = 0x1001,
    SS4S_AUDIO_OPUS = 0x2000,
} SS4S_AudioCodec;

typedef enum SS4S_AudioOpenResult {
    SS4S_AUDIO_OPEN_OK = 0,
    SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC = 1,
    SS4S_AUDIO_OPEN_ERROR = -1,
} SS4S_AudioOpenResult;

typedef enum SS4S_AudioFeedResult {
    SS4S_AUDIO_FEED_OK = 0,
    SS4S_AUDIO_FEED_NOT_READY = 1,
    SS4S_AUDIO_FEED_OVERFLOW = 2,
    SS4S_AUDIO_FEED_ERROR = -1,
} SS4S_AudioFeedResult;

typedef struct SS4S_AudioInfo {
    SS4S_AudioCodec codec;
    int sampleRate;
    int numOfChannels;
    int samplesPerFrame;
    const unsigned char *codecData;
    const size_t codecDataLen;
    const char *appName;
    const char *streamName;
} SS4S_AudioInfo;

typedef struct SS4S_Player SS4S_Player;

#ifndef SS4S_MODAPI_H

SS4S_AudioOpenResult SS4S_PlayerAudioOpen(SS4S_Player *player, const SS4S_AudioInfo *info);

SS4S_AudioFeedResult SS4S_PlayerAudioFeed(SS4S_Player *player, const unsigned char *data, size_t size);

bool SS4S_PlayerAudioClose(SS4S_Player *player);

#endif // SS4S_MODAPI_H