#include "ndl_common.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static SS4S_PlayerContext *CreatePlayerContext(SS4S_Player *player);

static void DestroyPlayerContext(SS4S_PlayerContext *context);

static void PlayerSetWaitAudioVideoReady(SS4S_PlayerContext *context, bool option);

static void EsplayerCallback(NDL_ESP_EVENT event, void *playerdata, void *userdata);

const SS4S_PlayerDriver SS4S_NDL_Esplayer_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
        .SetWaitAudioVideoReady = PlayerSetWaitAudioVideoReady,
};

static int UnloadMedia(SS4S_PlayerContext *context);

static int LoadMedia(SS4S_PlayerContext *context);

static SS4S_PlayerContext *ActivatePlayerContext = NULL;

int SS4S_NDL_Esplayer_ReloadMedia(SS4S_PlayerContext *context) {
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "Reloading media");
    if (UnloadMedia(context) != 0) {
        return -1;
    }
    return LoadMedia(context);
}

int SS4S_NDL_Esplayer_UnloadMedia(SS4S_PlayerContext *context) {
    return UnloadMedia(context);
}

static SS4S_PlayerContext *CreatePlayerContext(SS4S_Player *player) {
    assert(ActivatePlayerContext == NULL);
    SS4S_PlayerContext *created = calloc(1, sizeof(SS4S_PlayerContext));
    created->player = player;
    created->handle = NDL_EsplayerCreate(getenv("APPID"), EsplayerCallback, created);
    ActivatePlayerContext = created;
    return created;
}

static void DestroyPlayerContext(SS4S_PlayerContext *context) {
    UnloadMedia(context);
    NDL_EsplayerDestroy(context->handle);
    free(context);
    assert(context == ActivatePlayerContext);
    ActivatePlayerContext = NULL;
}

static void PlayerSetWaitAudioVideoReady(SS4S_PlayerContext *context, bool option) {
    context->waitAudioVideoReady = option;
}

static int UnloadMedia(SS4S_PlayerContext *context) {
    int ret = 0;
    if (context->mediaLoaded) {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "Unloading media");
        context->mediaLoaded = false;
        ret = NDL_EsplayerUnload(context->handle);
    }
    return ret;
}

static int LoadMedia(SS4S_PlayerContext *context) {
    int ret;
    assert(!context->mediaLoaded);
    assert(context->handle);
    NDL_ESP_META_DATA info = context->mediaInfo;
    if (info.video_codec == NDL_ESP_VIDEO_NONE && info.audio_codec == NDL_ESP_AUDIO_NONE) {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelWarn, "NDL", "LoadMedia but audio and video has no type");
        return -1;
    } else if (context->waitAudioVideoReady &&
               (info.video_codec == NDL_ESP_VIDEO_NONE || info.audio_codec == NDL_ESP_AUDIO_NONE)) {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "Defer LoadMedia because audio or video has no type");
        return 0;
    }
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "NDL_EsplayerLoad(video=%u (%d*%d), atype=%u)", info.video_codec,
                          info.width, info.height, info.audio_codec);
    NDL_EsplayerSetAppForegroundState(context->handle, NDL_ESP_APP_STATE_FOREGROUND);
    if ((ret = NDL_EsplayerLoad(context->handle, &info)) != 0) {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelWarn, "NDL", "NDL_EsplayerLoad returned %d", ret);
        return ret;
    }
    context->mediaLoaded = true;
    NDL_EsplayerPlay(context->handle);
    return ret;
}

void EsplayerCallback(NDL_ESP_EVENT event, void *playerdata, void *userdata) {
    (void) playerdata;
    (void) userdata;
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "Esplayer Callback %d", event);
}