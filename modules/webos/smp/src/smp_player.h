#pragma once

#include "ss4s/modapi.h"
#include <pthread.h>

typedef enum PlayerState {
    SMP_STATE_UNLOADED,
    SMP_STATE_LOADED,
    SMP_STATE_PLAYING,
} PlayerState;

typedef enum FeedResult {
    SMP_FEED_OK,
    SMP_FEED_NOT_READY,
    SMP_FEED_BUFFER_FULL,
    SMP_FEED_ERROR = -1,
} FeedResult;

struct SS4S_PlayerContext {
    char *appId;
    pthread_mutex_t lock;
    SS4S_Player *player;

    SS4S_AudioInfo audioInfo;
    bool hasAudio;
    SS4S_VideoInfo videoInfo;
    bool hasVideo;

    PlayerState state;
    uint64_t openTime;
    int aspectRatio;
    bool hdr, shouldStop;

    bool waitAudioVideoReady;

    struct StarfishMediaAPIs_C *api;
    struct StarfishResource *res;

};
extern const SS4S_LibraryContext *StarfishLibContext;

typedef struct StarfishPlayer StarfishPlayer;

bool StarfishPlayerLoadInner(SS4S_PlayerContext *ctx);

bool StarfishPlayerUnloadInner(SS4S_PlayerContext *ctx);

FeedResult StarfishPlayerFeed(SS4S_PlayerContext *ctx, const unsigned char *data, size_t size, int esData);

uint64_t StarfishPlayerGetTime();

void StarfishPlayerLock(SS4S_PlayerContext *ctx);

void StarfishPlayerUnlock(SS4S_PlayerContext *ctx);
