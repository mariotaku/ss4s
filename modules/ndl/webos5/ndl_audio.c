#include "ndl_common.h"

static SS4S_AudioOpenResult OpenAudio(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    switch (info->codec) {
        case SS4S_AUDIO_PCM_S16LE: {
            NDL_DIRECTMEDIA_AUDIO_PCM_INFO pcmInfo = {
                    .type = NDL_AUDIO_TYPE_PCM,
                    .format = NDL_DIRECTMEDIA_AUDIO_PCM_FORMAT_S16LE,
                    .channelMode = NDL_DIRECTMEDIA_AUDIO_PCM_MODE_STEREO,
                    .sampleRate = NDL_DIRECTAUDIO_SAMPLING_FREQ_OF(info->sampleRate),
            };
            context->mediaInfo.audio.pcm = pcmInfo;
            break;
        }
        case SS4S_AUDIO_OPUS: {
            NDL_DIRECTMEDIA_AUDIO_OPUS_INFO opusInfo = {
                    .type = NDL_AUDIO_TYPE_OPUS,
                    .channels = info->numOfChannels,
                    .sampleRate = info->sampleRate / 1000.0,
                    .streamHeader = NULL /* Not supported yet*/,
            };
            context->mediaInfo.audio.opus = opusInfo;
            break;
        }
        default:
            return SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
    }
    if (SS4S_NDL_webOS5_ReloadMedia(context) != 0) {
        return SS4S_AUDIO_OPEN_ERROR;
    }
    *instance = (SS4S_AudioInstance *) context;
    return SS4S_AUDIO_OPEN_OK;
}

static SS4S_AudioFeedResult FeedAudio(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    (void) instance;
    int rc = NDL_DirectAudioPlay(data, size, 0);
    if (rc != 0) {
        return SS4S_AUDIO_FEED_ERROR;
    }
    return SS4S_AUDIO_FEED_OK;
}

static void CloseAudio(SS4S_AudioInstance *instance) {
    SS4S_PlayerContext *context = (void *) instance;
    context->mediaInfo.audio.type = 0;
    SS4S_NDL_webOS5_ReloadMedia(context);
}

const SS4S_AudioDriver SS4S_NDL_webOS5_AudioDriver = {
        .Base = {
                .PostInit = SS4S_NDL_webOS5_Driver_PostInit,
                .Quit = SS4S_NDL_webOS5_Driver_Quit,
        },
        .Open = OpenAudio,
        .Feed = FeedAudio,
        .Close = CloseAudio,
};