#include <stdlib.h>
#include <stddef.h>

#include "ndl_common.h"

static void Base64Enc(char *dst, const unsigned char *src, size_t srcLen);

static bool GetCapabilities(SS4S_AudioCapabilities *capabilities, SS4S_AudioCodec wantedCodecs) {
    if (!(wantedCodecs & SS4S_AUDIO_PCM_S16LE)) {
        return false;
    }
    capabilities->codecs = SS4S_AUDIO_PCM_S16LE;
    int supportsMultiChannel = 0;
    if (NDL_DirectAudioSupportMultiChannel(&supportsMultiChannel) == 0 && supportsMultiChannel) {
        capabilities->maxChannels = 6;
    } else {
        capabilities->maxChannels = 2;
    }
    return true;
}

static SS4S_AudioOpenResult OpenAudio(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "OpenAudio called");
    pthread_mutex_lock(&SS4S_NDL_webOS5_Lock);
    SS4S_AudioOpenResult result;
    switch (info->codec) {
        case SS4S_AUDIO_PCM_S16LE: {
            const char *mode = "stereo";
            if (info->numOfChannels == 1) {
                mode = "mono";
            } else if (info->numOfChannels == 6) {
                mode = "6-channel";
            }
            NDL_DIRECTAUDIO_PCM_INFO_T pcmInfo = {
                    .type = NDL_AUDIO_TYPE_PCM,
                    .format = NDL_DIRECTMEDIA_AUDIO_PCM_FORMAT_S16LE,
                    .channelMode = mode,
                    .sampleRate = NDL_DIRECTMEDIA_AUDIO_PCM_SAMPLE_RATE_OF(info->sampleRate),
            };
            context->mediaInfo.audio.pcm = pcmInfo;
            break;
        }
        case SS4S_AUDIO_OPUS: {
            if (info->codecData && info->codecDataLen) {
                context->streamHeader = malloc(((info->codecDataLen + 3 - 1) / 3) * 4 + 1);
                if (!context->streamHeader) {
                    result = SS4S_AUDIO_OPEN_ERROR;
                    goto finish;
                }
                Base64Enc(context->streamHeader, info->codecData, info->codecDataLen);
            }
            NDL_DIRECTMEDIA_AUDIO_OPUS_INFO_T opusInfo = {
                    .type = NDL_AUDIO_TYPE_OPUS,
                    .channels = info->numOfChannels,
                    .sampleRate = info->sampleRate / 1000.0,
                    .streamHeader = context->streamHeader,
            };
            context->mediaInfo.audio.opus = opusInfo;
            break;
        }
        default:
            result = SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
            goto finish;
    }
    if (SS4S_NDL_webOS5_ReloadMedia(context) != 0) {
        result = SS4S_AUDIO_OPEN_ERROR;
        goto finish;
    }
    *instance = (SS4S_AudioInstance *) context;
    result = SS4S_AUDIO_OPEN_OK;

    finish:
    pthread_mutex_unlock(&SS4S_NDL_webOS5_Lock);
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "OpenAudio returned %d", result);
    return result;
}

static SS4S_AudioFeedResult FeedAudio(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    const SS4S_PlayerContext *context = (void *) instance;
    if (!context->mediaLoaded) {
        return SS4S_AUDIO_FEED_NOT_READY;
    }
    int rc = NDL_DirectAudioPlay((void *) data, size, 0);
    if (rc != 0) {
        SS4S_NDL_webOS5_Log(SS4S_LogLevelWarn, "NDL", "NDL_DirectAudioPlay returned %d: %s", rc,
                            NDL_DirectMediaGetError());
        return SS4S_AUDIO_FEED_ERROR;
    }
    return SS4S_AUDIO_FEED_OK;
}

static void CloseAudio(SS4S_AudioInstance *instance) {
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "CloseAudio called");
    pthread_mutex_lock(&SS4S_NDL_webOS5_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    context->mediaInfo.audio.type = 0;
    if (context->streamHeader) {
        free(context->streamHeader);
        context->streamHeader = NULL;
    }
    SS4S_NDL_webOS5_UnloadMedia(context);
    pthread_mutex_unlock(&SS4S_NDL_webOS5_Lock);
}

void Base64Enc(char *dst, const unsigned char *src, size_t srcLen) {
    static const char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (size_t i = 0; i < srcLen; i += 3) {
        unsigned int val = src[i] << 16;
        if (i + 1 < srcLen) {
            val |= src[i + 1] << 8;
        }
        if (i + 2 < srcLen) {
            val |= src[i + 2];
        }
        for (int j = 0; j < 4; j++) {
            if (i + j <= srcLen) {
                dst[i / 3 * 4 + j] = base64[(val >> (18 - j * 6)) & 0x3F];
            } else {
                dst[i / 3 * 4 + j] = '=';
            }
        }
    }
    dst[(srcLen + 2) / 3 * 4] = '\0';
}

const SS4S_AudioDriver SS4S_NDL_webOS5_AudioDriver = {
        .Base = {
                .PostInit = SS4S_NDL_webOS5_Driver_PostInit,
                .Quit = SS4S_NDL_webOS5_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = OpenAudio,
        .Feed = FeedAudio,
        .Close = CloseAudio,
};