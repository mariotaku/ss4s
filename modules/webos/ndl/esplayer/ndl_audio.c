#include <stdlib.h>
#include <stddef.h>

#include "ndl_common.h"

static int DriverInit(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    return 0;
}

static bool GetCapabilities(SS4S_AudioCapabilities *capabilities, SS4S_AudioCodec wantedCodecs) {
    SS4S_AudioCodec matchedCodecs = wantedCodecs & (SS4S_AUDIO_PCM_S16LE);
    if (matchedCodecs == 0) {
        return false;
    }
    capabilities->codecs = SS4S_AUDIO_PCM_S16LE;
    /*
     * Don't check for system settings to determine the number of channels, just provide 6 channels.
     * webOS should be able to down-mix that anyway.
     */
    capabilities->maxChannels = 2;
    return true;
}

static SS4S_AudioCodec GetPreferredCodecs(const SS4S_AudioInfo *info) {
    (void) info;
    return SS4S_AUDIO_PCM_S16LE;
}

static SS4S_AudioOpenResult OpenAudio(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "OpenAudio called");
    pthread_mutex_lock(&SS4S_NDL_Esplayer_Lock);
    SS4S_AudioOpenResult result;
    switch (info->codec) {
        case SS4S_AUDIO_PCM_S16LE: {
            context->mediaInfo.audio_codec = NDL_ESP_AUDIO_CODEC_PCM_48000_2CH;
            break;
        }
        default:
            result = SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
            goto finish;
    }
    if (SS4S_NDL_Esplayer_ReloadMedia(context) != 0) {
        result = SS4S_AUDIO_OPEN_ERROR;
        goto finish;
    }
    *instance = (SS4S_AudioInstance *) context;
    result = SS4S_AUDIO_OPEN_OK;

    finish:
    pthread_mutex_unlock(&SS4S_NDL_Esplayer_Lock);
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "OpenAudio returned %d", result);
    return result;
}

static SS4S_AudioFeedResult FeedAudio(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    const SS4S_PlayerContext *context = (void *) instance;
    if (!context->mediaLoaded) {
        return SS4S_AUDIO_FEED_NOT_READY;
    }
    NDL_ESP_STREAM_BUFFER buff = {
            .data = (uint8_t *) data,
            .data_len = size,
            .offset = 0,
            .stream_type = NDL_ESP_AUDIO_ES,
            .timestamp = 0,
    };
    int rc = NDL_EsplayerFeedData(context->handle, &buff);
    if (rc < 0) {
        if (rc == NDL_ESP_RESULT_FEED_FULL) {
            return SS4S_AUDIO_FEED_OK;
        }
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelWarn, "NDL", "NDL_EsplayerFeedData returned %d", rc);
        return SS4S_AUDIO_FEED_ERROR;
    }
    return SS4S_AUDIO_FEED_OK;
}

static void CloseAudio(SS4S_AudioInstance *instance) {
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "CloseAudio called");
    pthread_mutex_lock(&SS4S_NDL_Esplayer_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    context->mediaInfo.audio_codec = NDL_ESP_AUDIO_NONE;
    if (context->streamHeader) {
        free(context->streamHeader);
        context->streamHeader = NULL;
    }
    SS4S_NDL_Esplayer_UnloadMedia(context);
    pthread_mutex_unlock(&SS4S_NDL_Esplayer_Lock);
}

const SS4S_AudioDriver SS4S_NDL_Esplayer_AudioDriver = {
        .Base = {
                .Init = DriverInit,
                .PostInit = SS4S_NDL_Esplayer_Driver_PostInit,
                .Quit = SS4S_NDL_Esplayer_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .GetPreferredCodecs = GetPreferredCodecs,
        .Open = OpenAudio,
        .Feed = FeedAudio,
        .Close = CloseAudio,
};