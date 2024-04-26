#include <stdlib.h>
#include <stddef.h>
#include <dlfcn.h>
#include <string.h>

#include "ndl_common.h"
#include "opus_empty.h"
#include <opus_multistream.h>

static int SupportsPCM6Channel = 0;

static void Base64Enc(char *dst, const unsigned char *src, size_t srcLen);

static bool IsOpusPassthroughSupported(const OpusConfig *config);

static bool ParseOpusConfig(const unsigned char *codecData, size_t codecDataLen, OpusConfig *config);

static int DriverInit(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    /*
     * webOS 7.0 introduced PCM 6-channel support as well as NDL_DirectAudioRegisterCallback.
     * We can use this to determine if the platform supports 6-channel PCM. 
     */
    SupportsPCM6Channel = dlsym(RTLD_DEFAULT, "NDL_DirectAudioRegisterCallback") != 0;
    return 0;
}

static bool GetCapabilities(SS4S_AudioCapabilities *capabilities, SS4S_AudioCodec wantedCodecs) {
    SS4S_AudioCodec matchedCodecs = wantedCodecs & (SS4S_AUDIO_PCM_S16LE | SS4S_AUDIO_OPUS);
    if (matchedCodecs == 0) {
        return false;
    }
    capabilities->codecs = SS4S_AUDIO_PCM_S16LE | SS4S_AUDIO_OPUS;
    /*
     * Don't check for system settings to determine the number of channels, just provide 6 channels.
     * webOS should be able to down-mix that anyway.
     */
    capabilities->maxChannels = 6;
    return true;
}

static SS4S_AudioCodec GetPreferredCodecs(const SS4S_AudioInfo *info) {
    if (info->numOfChannels == 6) {
        return SS4S_AUDIO_OPUS;
    }
    return SS4S_AUDIO_PCM_S16LE;
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
                if (SupportsPCM6Channel) {
                    mode = "6-channel";
                } else {
                    SS4S_NDL_webOS5_Log(SS4S_LogLevelWarn, "NDL", "6-channel PCM is not supported, "
                                                                  "falling back to stereo");
                }
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
            NDL_DIRECTMEDIA_AUDIO_OPUS_INFO_T opusInfo = {
                    .type = NDL_AUDIO_TYPE_OPUS,
                    .channels = info->numOfChannels,
                    .sampleRate = info->sampleRate / 1000.0,
            };
            if (info->codecData && info->codecDataLen) {
                OpusConfig opusConfig;
                if (!ParseOpusConfig(info->codecData, info->codecDataLen, &opusConfig)) {
                    result = SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
                    goto finish;
                }
                context->streamHeader = malloc(((info->codecDataLen + 3 - 1) / 3) * 4 + 1);
                if (!context->streamHeader) {
                    result = SS4S_AUDIO_OPEN_ERROR;
                    goto finish;
                }
                Base64Enc(context->streamHeader, info->codecData, info->codecDataLen);
                opusInfo.streamHeader = context->streamHeader;
                context->opusEmptyFrameLen = opus_empty_frames_copy(context->opusEmptyFrame, opusConfig.channels,
                                                                    opusConfig.streamCount, opusConfig.coupledCount);
                if (opusConfig.channels == 6 && !IsOpusPassthroughSupported(&opusConfig)) {
                    SS4S_NDL_webOS5_Log(SS4S_LogLevelWarn, "NDL",
                                        "Channel config is not supported, enabling re-encoding. "
                                        "This will introduce audio latency");
                    context->opusFix = SS4S_NDLOpusFixCreate(&opusConfig);
                    if (!context->opusFix) {
                        result = SS4S_AUDIO_OPEN_ERROR;
                        goto finish;
                    }
                }
            }
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
    int rc;
    if (context->opusFix) {
        int fixed_size = SS4S_NDLOpusFixProcess(context->opusFix, data, size);
        rc = NDL_DirectAudioPlay((void *) SS4S_NDLOpusFixGetBuffer(context->opusFix), fixed_size, 0);
    } else {
        rc = NDL_DirectAudioPlay((void *) data, size, 0);
    }
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
    if (context->opusFix) {
        SS4S_NDLOpusFixDestroy(context->opusFix);
        context->opusFix = NULL;
    }
    context->opusEmptyFrameLen = 0;
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

bool IsOpusPassthroughSupported(const OpusConfig *config) {
    static const uint8_t wantedMapping[6] = {0, 1, 4, 5, 2, 3};
    return config->streamCount == 4 && config->coupledCount == 2 && memcmp(config->mapping, wantedMapping, 6) == 0;
}

bool ParseOpusConfig(const unsigned char *codecData, size_t codecDataLen, OpusConfig *config) {
    if (codecDataLen < 20 || memcmp(codecData, "OpusHead", 8) != 0) {
        return false;
    }
    memcpy(&config->sampleRate, codecData + 12, 4);
    config->channels = codecData[9];
    config->streamCount = codecData[19];
    config->coupledCount = codecData[20];
    if ((int) codecDataLen >= 21 + config->channels) {
        memcpy(config->mapping, codecData + 21, config->channels);
    } else {
        config->mapping[0] = 0;
        config->mapping[1] = 0;
    }
    return true;
}

const SS4S_AudioDriver SS4S_NDL_webOS5_AudioDriver = {
        .Base = {
                .Init = DriverInit,
                .PostInit = SS4S_NDL_webOS5_Driver_PostInit,
                .Quit = SS4S_NDL_webOS5_Driver_Quit,
        },
        .GetCapabilities = GetCapabilities,
        .GetPreferredCodecs = GetPreferredCodecs,
        .Open = OpenAudio,
        .Feed = FeedAudio,
        .Close = CloseAudio,
};