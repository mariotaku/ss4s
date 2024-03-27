#include "ndl_common.h"

#include <string.h>

static bool GetCapabilities(SS4S_AudioCapabilities *capabilities) {
    capabilities->codecs = SS4S_AUDIO_PCM_S16LE | SS4S_AUDIO_AAC | SS4S_AUDIO_AC3;
    capabilities->maxChannels = 2;
    return true;
}

static SS4S_AudioOpenResult OpenAudio(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    pthread_mutex_lock(&SS4S_NDL_webOS4_Lock);
    SS4S_AudioOpenResult result;
    NDL_DIRECTAUDIO_SRC_TYPE srcType;
    switch (info->codec) {
        case SS4S_AUDIO_PCM_S16LE: {
            srcType = NDL_DIRECTAUDIO_SRC_TYPE_PCM;
            break;
        }
        case SS4S_AUDIO_AAC: {
            srcType = NDL_DIRECTAUDIO_SRC_TYPE_AAC;
            break;
        }
        case SS4S_AUDIO_AC3: {
            srcType = NDL_DIRECTAUDIO_SRC_TYPE_AC3;
            break;
        }
        default:
            result = SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
            goto finish;
    }
    NDL_DIRECTAUDIO_DATA_INFO_T audInfo = {
            .number_of_channel = info->numOfChannels,
            .bit_per_sample = 16,
            .no_delay_mode = 1,
            .no_delay_upper_time = 48,
            .no_delay_lower_time = 16,
            .channel = NDL_DIRECTAUDIO_CH_MAIN,
            .source = srcType,
            .frequency = NDL_DIRECTAUDIO_SAMPLING_FREQ_OF(info->sampleRate),
    };
    context->audioInfo = audInfo;
    if (NDL_DirectAudioOpen(&context->audioInfo) != 0) {
        result = SS4S_AUDIO_OPEN_ERROR;
        goto finish;
    }
    context->audioOpened = true;
    *instance = (SS4S_AudioInstance *) context;
    result = SS4S_AUDIO_OPEN_OK;

    finish:
    pthread_mutex_unlock(&SS4S_NDL_webOS4_Lock);
    return result;
}

static SS4S_AudioFeedResult FeedAudio(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    SS4S_PlayerContext *context = (SS4S_PlayerContext *) instance;
    if (!context->audioOpened) {
        return SS4S_AUDIO_FEED_NOT_READY;
    }
    int rc = NDL_DirectAudioPlay((void *) data, size);
    if (rc != 0) {
        return SS4S_AUDIO_FEED_ERROR;
    }
    return SS4S_AUDIO_FEED_OK;
}

static void CloseAudio(SS4S_AudioInstance *instance) {
    pthread_mutex_lock(&SS4S_NDL_webOS4_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    memset(&context->audioInfo, 0, sizeof(NDL_DIRECTAUDIO_DATA_INFO_T));
    if (context->audioOpened) {
        NDL_DirectAudioClose();
    }
    context->audioOpened = false;
    pthread_mutex_unlock(&SS4S_NDL_webOS4_Lock);
}

const SS4S_AudioDriver SS4S_NDL_webOS4_AudioDriver = {
        .Base = {
                .Init = SS4S_NDL_webOS4_Driver_Init,
                .Quit = SS4S_NDL_webOS4_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenAudio,
        .Feed = FeedAudio,
        .Close = CloseAudio,
};