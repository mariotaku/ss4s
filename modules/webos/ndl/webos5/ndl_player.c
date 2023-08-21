#include "ndl_common.h"

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

static SS4S_PlayerContext *CreatePlayerContext();

static void DestroyPlayerContext(SS4S_PlayerContext *context);

static void PlayerSetWaitAudioVideoReady(SS4S_PlayerContext *context, bool option);

const SS4S_PlayerDriver SS4S_NDL_webOS5_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
        .SetWaitAudioVideoReady = PlayerSetWaitAudioVideoReady,
};

static void UnloadMedia(SS4S_PlayerContext *context);

static int LoadMedia(SS4S_PlayerContext *context);

static void LoadCallback(int type, long long numValue, const char *strValue);

static SS4S_PlayerContext *ActivatePlayerContext = NULL;

int SS4S_NDL_webOS5_ReloadMedia(SS4S_PlayerContext *context) {
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "Reloading media");
    UnloadMedia(context);
    return LoadMedia(context);
}

static SS4S_PlayerContext *CreatePlayerContext() {
    assert(ActivatePlayerContext == NULL);
    SS4S_PlayerContext *created = calloc(1, sizeof(SS4S_PlayerContext));
    ActivatePlayerContext = created;
    return created;
}

static void DestroyPlayerContext(SS4S_PlayerContext *context) {
    UnloadMedia(context);
    free(context);
    assert(context == ActivatePlayerContext);
    ActivatePlayerContext = NULL;
}

static void PlayerSetWaitAudioVideoReady(SS4S_PlayerContext *context, bool option) {
    context->waitAudioVideoReady = option;
}

static void UnloadMedia(SS4S_PlayerContext *context) {
    if (context->mediaLoaded) {
        context->mediaLoaded = false;
        NDL_DirectMediaUnload();
    }
//    if (SS4S_NDL_webOS5_Initialized) {
//        NDL_DirectMediaQuit();
//        SS4S_NDL_webOS5_Initialized = false;
//    }
}

static int LoadMedia(SS4S_PlayerContext *context) {
    int ret = 0;
//    if (!SS4S_NDL_webOS5_Initialized) {
//        if ((ret = NDL_DirectMediaInit(getenv("APPID"), NULL)) != 0) {
//            goto cleanup;
//        }
//        SS4S_NDL_webOS5_Initialized = true;
//    }
    assert(SS4S_NDL_webOS5_Initialized);
    assert(!context->mediaLoaded);
    NDL_DIRECTMEDIA_DATA_INFO_T info = context->mediaInfo;
    if (info.video.type == 0 && info.audio.type == 0) {
        SS4S_NDL_webOS5_Log(SS4S_LogLevelWarn, "NDL", "LoadMedia but audio and video has no type");
        return -1;
    } else if (context->waitAudioVideoReady && (info.video.type == 0 || info.audio.type == 0)) {
        SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "Defer LoadMedia because audio or video has no type");
        return 0;
    }
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "NDL_DirectMediaLoad(video=%u (%d*%d), atype=%u)", info.video.type,
                        info.video.width, info.video.height, info.audio.type);
    if ((ret = NDL_DirectMediaLoad(&info, LoadCallback)) != 0) {
        SS4S_NDL_webOS5_Log(SS4S_LogLevelWarn, "NDL", "NDL_DirectMediaLoad returned %d: %s", ret,
                            NDL_DirectMediaGetError());
        return ret;
    }
    context->mediaLoaded = true;
    return ret;
}

static void LoadCallback(int type, long long numValue, const char *strValue) {
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "MediaLoadCallback type=%d, numValue=%llx, strValue=%p\n",
                        type, numValue, strValue);
//    SS4S_PlayerContext *context = ActivatePlayerContext;
}