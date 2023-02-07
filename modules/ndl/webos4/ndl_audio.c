#include "ndl_common.h"

#include <string.h>

static SS4S_AudioOpenResult OpenAudio(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    pthread_mutex_lock(&SS4S_NDL_webOS4_Lock);
    SS4S_AudioOpenResult result;
    switch (info->codec) {
        case SS4S_AUDIO_PCM_S16LE: {
            NDL_DIRECTAUDIO_DATA_INFO pcmInfo = {
                    .numChannel = info->numOfChannels,
                    .bitPerSample = 16,
                    .nodelay = 1,
                    .upperThreshold = 48,
                    .lowerThreshold = 16,
                    .channel = NDL_DIRECTAUDIO_CH_MAIN,
                    .srcType = NDL_DIRECTAUDIO_SRC_TYPE_PCM,
                    .samplingFreq = NDL_DIRECTAUDIO_SAMPLING_FREQ_OF(info->sampleRate),
            };
            context->audioInfo = pcmInfo;
            break;
        }
        default:
            result = SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
            goto finish;
    }
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
    (void) instance;
    SS4S_PlayerContext *context = (SS4S_PlayerContext *) instance;
    if (!context->audioOpened) {
        return SS4S_AUDIO_FEED_NOT_READY;
    }
    int rc = NDL_DirectAudioPlay(data, size, 0);
    if (rc != 0) {
        return SS4S_AUDIO_FEED_ERROR;
    }
    return SS4S_AUDIO_FEED_OK;
}

static void CloseAudio(SS4S_AudioInstance *instance) {
    pthread_mutex_lock(&SS4S_NDL_webOS4_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    memset(&context->audioInfo, 0, sizeof(NDL_DIRECTAUDIO_DATA_INFO));
    context->audioOpened = false;
    NDL_DirectAudioClose();
    pthread_mutex_unlock(&SS4S_NDL_webOS4_Lock);
}

const SS4S_AudioDriver SS4S_NDL_webOS4_AudioDriver = {
        .Base = {
                .Init = SS4S_NDL_webOS4_Driver_Init,
                .Quit = SS4S_NDL_webOS4_Driver_Quit,
        },
        .Open = OpenAudio,
        .Feed = FeedAudio,
        .Close = CloseAudio,
};