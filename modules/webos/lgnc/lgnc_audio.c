#include "lgnc_common.h"

#include <string.h>

static bool GetCapabilities(SS4S_AudioCapabilities *capabilities, SS4S_AudioCodec wantedCodecs) {
    (void) wantedCodecs;
    capabilities->codecs = SS4S_AUDIO_PCM_S16LE | SS4S_AUDIO_AC3 | SS4S_AUDIO_AAC;
    if (wantedCodecs & (SS4S_AUDIO_AC3 | SS4S_AUDIO_AAC)) {
        capabilities->maxChannels = 6;
    } else {
        capabilities->maxChannels = 2;
    }
    return true;
}

static SS4S_AudioOpenResult OpenAudio(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    LGNC_ADEC_FMT_T codec;
    switch (info->codec) {
        case SS4S_AUDIO_PCM_S16LE: {
            codec = LGNC_ADEC_FMT_PCM;
            break;
        }
        case SS4S_AUDIO_AC3: {
            codec = LGNC_ADEC_FMT_AC3;
            break;
        }
        case SS4S_AUDIO_AAC: {
            codec = LGNC_ADEC_FMT_AAC;
            break;
        }
        default:
            return SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
    }
    LGNC_ADEC_DATA_INFO_T audioInfo = {
            .codec = codec,
            .AChannel = LGNC_ADEC_CH_INDEX_MAIN,
            .samplingFreq = LGNC_ADEC_SAMPLING_FREQ_OF(info->sampleRate),
            .numberOfChannel = info->numOfChannels,
            .bitPerSample = 16,
    };
    context->audioInfo = audioInfo;
    if (LGNC_DIRECTAUDIO_Open(&context->audioInfo) != 0) {
        return SS4S_AUDIO_OPEN_ERROR;
    }
    context->audioOpened = true;
    *instance = (SS4S_AudioInstance *) context;
    return SS4S_AUDIO_OPEN_OK;
}

static SS4S_AudioFeedResult FeedAudio(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    SS4S_PlayerContext *context = (SS4S_PlayerContext *) instance;
    if (!context->audioOpened) {
        return SS4S_AUDIO_FEED_NOT_READY;
    }
    int rc = LGNC_DIRECTAUDIO_Play(data, size);
    if (rc != 0) {
        return SS4S_AUDIO_FEED_ERROR;
    }
    return SS4S_AUDIO_FEED_OK;
}

static void CloseAudio(SS4S_AudioInstance *instance) {
    SS4S_PlayerContext *context = (void *) instance;
    memset(&context->audioInfo, 0, sizeof(LGNC_ADEC_DATA_INFO_T));
    if (context->audioOpened) {
        LGNC_DIRECTAUDIO_Close();
    }
    context->audioOpened = false;
}

const SS4S_AudioDriver SS4S_LGNC_AudioDriver = {
        .Base = {
                .Init = SS4S_LGNC_Driver_Init,
                .Quit = SS4S_LGNC_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenAudio,
        .Feed = FeedAudio,
        .Close = CloseAudio,
};