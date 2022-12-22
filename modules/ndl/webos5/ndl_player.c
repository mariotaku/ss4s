#include "ndl_common.h"

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

static pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;

static SS4S_PlayerContext *CreatePlayerContext();

static void DestroyPlayerContext(SS4S_PlayerContext *context);

static void UnloadMedia(SS4S_PlayerContext *context);

static int LoadMedia(SS4S_PlayerContext *context);

static void LoadCallback(int type, long long numValue, const char *strValue);

const SS4S_PlayerDriver SS4S_NDL_webOS5_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
};

int SS4S_NDL_webOS5_ReloadMedia(SS4S_PlayerContext *context) {
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
        NDL_DirectMediaUnload();
        context->mediaLoaded = false;
    }
//    if (SS4S_NDL_webOS5_Initialized) {
//        NDL_DirectMediaQuit();
//        SS4S_NDL_webOS5_Initialized = false;
//    }
    pthread_mutex_unlock(&globalMutex);
}

static int LoadMedia(SS4S_PlayerContext *context) {
    pthread_mutex_lock(&globalMutex);
    int ret = 0;
//    if (!SS4S_NDL_webOS5_Initialized) {
//        if ((ret = NDL_DirectMediaInit(getenv("APPID"), NULL)) != 0) {
//            goto cleanup;
//        }
//        SS4S_NDL_webOS5_Initialized = true;
//    }
    assert(SS4S_NDL_webOS5_Initialized);
    assert(!context->mediaLoaded);
    NDL_DIRECTMEDIA_DATA_INFO info = context->mediaInfo;
    if ((ret = NDL_DirectMediaLoad(&info, LoadCallback)) != 0) {
        goto cleanup;
    }
    context->mediaLoaded = true;
    cleanup:
    pthread_mutex_unlock(&globalMutex);
    return ret;
}

static void LoadCallback(int type, long long numValue, const char *strValue) {
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "MediaLoadCallback type=%d, numValue=%llx, strValue=%p\n",
                        type, numValue, strValue);
}