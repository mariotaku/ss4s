#include "dummy_common.h"

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

static pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;

static SS4S_PlayerContext *CreatePlayerContext();

static void DestroyPlayerContext(SS4S_PlayerContext *context);

static void UnloadMedia(SS4S_PlayerContext *context);

static int LoadMedia(SS4S_PlayerContext *context);

const SS4S_PlayerDriver SS4S_Dummy_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
};

int SS4S_Dummy_ReloadMedia(SS4S_PlayerContext *context) {
    UnloadMedia(context);
    return LoadMedia(context);
}

static SS4S_PlayerContext *CreatePlayerContext() {
    return calloc(1, sizeof(SS4S_PlayerContext));
}

static void DestroyPlayerContext(SS4S_PlayerContext *context) {
    UnloadMedia(context);
    free(context);
}

static void UnloadMedia(SS4S_PlayerContext *context) {
    pthread_mutex_lock(&globalMutex);
    if (context->mediaLoaded) {
        context->mediaLoaded = false;
    }
    pthread_mutex_unlock(&globalMutex);
}

static int LoadMedia(SS4S_PlayerContext *context) {
    pthread_mutex_lock(&globalMutex);
    int ret = 0;
    assert(SS4S_Dummy_Initialized);
    assert(!context->mediaLoaded);
    usleep(500 * 1000);
    context->mediaLoaded = true;
    pthread_mutex_unlock(&globalMutex);
    return ret;
}