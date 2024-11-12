#include "ndl_common.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static SS4S_PlayerContext *CreatePlayerContext(SS4S_Player *player);

static void DestroyPlayerContext(SS4S_PlayerContext *context);

static void PlayerSetWaitAudioVideoReady(SS4S_PlayerContext *context, bool option);

const SS4S_PlayerDriver SS4S_NDL_Esplayer_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
        .SetWaitAudioVideoReady = PlayerSetWaitAudioVideoReady,
};

static int UnloadMedia(SS4S_PlayerContext *context);

static int LoadMedia(SS4S_PlayerContext *context);

static void LoadCallback(int type, long long numValue, const char *strValue);

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

static int UnloadMedia(SS4S_PlayerContext *context) {
    int ret = 0;
    if (context->mediaLoaded) {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "Unloading media");
        context->mediaLoaded = false;
        ret = NDL_EsplayerUnload(SS4S_NDL_Esplayer_Handle);
    }
    return ret;
}

static int LoadMedia(SS4S_PlayerContext *context) {
    int ret;
    assert(SS4S_NDL_Esplayer_Handle);
    assert(!context->mediaLoaded);
    NDL_ESP_META_DATA info = context->mediaInfo;
    if (info.video_codec == NDL_ESP_VIDEO_NONE && info.audio_codec == NDL_ESP_AUDIO_NONE) {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelWarn, "NDL", "LoadMedia but audio and video has no type");
        return -1;
    } else if (context->waitAudioVideoReady &&
               (info.video_codec == NDL_ESP_VIDEO_NONE || info.audio_codec == NDL_ESP_AUDIO_NONE)) {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "Defer LoadMedia because audio or video has no type");
        return 0;
    }
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "NDL_DirectMediaLoad(video=%u (%d*%d), atype=%u)", info.video_codec,
                          info.width, info.height, info.audio_codec);
    if ((ret = NDL_EsplayerLoad(SS4S_NDL_Esplayer_Handle, &info)) != 0) {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelWarn, "NDL", "NDL_DirectMediaLoad returned %d", ret);
        return ret;
    }
    if (context->mediaInfo.audio_codec == NDL_ESP_AUDIO_CODEC_PCM_48000_2CH) {
        unsigned short empty_buf[8] = {0};
        int numChannels = (int) context->mediaInfo.channels;
        size_t size = numChannels * sizeof(unsigned short);
        NDL_ESP_STREAM_BUFFER buff = {
                .data = (uint8_t *) empty_buf,
                .data_len = size,
                .offset = 0,
                .stream_type = NDL_ESP_AUDIO_ES,
                .timestamp = 0,
        };
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "Playing empty PCM audio frame (%u bytes)", size);
        NDL_EsplayerFeedData(SS4S_NDL_Esplayer_Handle, &buff);
    }

    context->mediaLoaded = true;
    return ret;
}

static void LoadCallback(int type, long long numValue, const char *strValue) {
    switch (type) {
        case 0x16: {
            SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "%s STATE_UPDATE_LOADCOMPLETED: %s", __FUNCTION__,
                                  strValue);
            break;
        }
        case 0x17: {
            SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "%s STATE_UPDATE_UNLOADCOMPLETED: %s", __FUNCTION__,
                                  strValue);
            break;
        }
        case 0x1a: {
            SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "%s STATE_UPDATE_PLAYING: %s", __FUNCTION__, strValue);
            break;
        }
        default: {
            SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "%s type=0x%02x, numValue=0x%llx, strValue=%p",
                                  __FUNCTION__,
                                  type, numValue, strValue);
            break;
        }
    }
}