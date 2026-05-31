#include "dummy_common.h"

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

static pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;

/* Separate mutex for the in_destructive flag — the global mutex above
 * is held during the 500ms LoadMedia sleep, which would let Feed
 * block on the global mutex and miss the race we want to catch.
 * Keeping the in_destructive check on its own short-lived mutex
 * preserves the race window. */
static pthread_mutex_t destructiveMutex = PTHREAD_MUTEX_INITIALIZER;

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

void SS4S_Dummy_EnterDestructive(SS4S_PlayerContext *context, int sleep_ms) {
    pthread_mutex_lock(&destructiveMutex);
    assert(!context->in_destructive);
    context->in_destructive = true;
    pthread_mutex_unlock(&destructiveMutex);
    if (sleep_ms > 0) {
        usleep(sleep_ms * 1000);
    }
}

void SS4S_Dummy_ExitDestructive(SS4S_PlayerContext *context) {
    pthread_mutex_lock(&destructiveMutex);
    assert(context->in_destructive);
    context->in_destructive = false;
    pthread_mutex_unlock(&destructiveMutex);
}

bool SS4S_Dummy_CheckFeedSafe(SS4S_PlayerContext *context) {
    pthread_mutex_lock(&destructiveMutex);
    if (context->in_destructive) {
        pthread_mutex_unlock(&destructiveMutex);
        SS4S_Dummy_Log(SS4S_LogLevelError, "Dummy",
                       "FATAL: Feed reached driver during destructive op — "
                       "FeedGuard exclusive contract was violated");
        abort();
    }
    pthread_mutex_unlock(&destructiveMutex);
    /* mediaLoaded is a separate concern, racy by the dummy's design. */
    return context->mediaLoaded;
}