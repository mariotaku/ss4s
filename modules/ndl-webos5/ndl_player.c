#include "ndl_common.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

static SS4S_PlayerContext *CreatePlayerContext();

static void DestroyPlayerContext(SS4S_PlayerContext *context);

static void UnloadMedia(const SS4S_PlayerContext *context);

static int LoadMedia(const SS4S_PlayerContext *context);

static void LoadCallback(int type, long long numValue, const char *strValue);

const SS4S_PlayerDriver NDL_webOS5_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
};

int NDL_webOS5_ReloadMedia(SS4S_PlayerContext *context) {
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

static void UnloadMedia(const SS4S_PlayerContext *context) {
    if (context->mediaLoaded) {
        NDL_DirectMediaUnload();
    }
    if (NDL_webOS5_Initialized) {
        NDL_DirectMediaQuit();
        NDL_webOS5_Initialized = false;
    }
}

static int LoadMedia(const SS4S_PlayerContext *context) {
    int ret = 0;
    if (!NDL_webOS5_Initialized) {
        if ((ret = NDL_DirectMediaInit(getenv("APPID"), NULL)) != 0) {
            return ret;
        }
        NDL_webOS5_Initialized = true;
    }
    assert(!context->mediaLoaded);
    NDL_DIRECTMEDIA_DATA_INFO info = context->mediaInfo;
    if ((ret = NDL_DirectMediaLoad(&info, LoadCallback)) != 0) {
        return ret;
    }
    return 0;
}

static void LoadCallback(int type, long long numValue, const char *strValue) {
    fprintf(stderr, "MediaLoadCallback type=%d, numValue=%llx, strValue=%p\n", type, numValue, strValue);
}