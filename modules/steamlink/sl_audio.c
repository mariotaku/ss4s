#include "sl_common.h"

#include <string.h>

static bool GetCapabilities(SS4S_AudioCapabilities *capabilities, SS4S_AudioCodec wantedCodecs) {
    (void) wantedCodecs;
    capabilities->codecs = SS4S_AUDIO_PCM_S16LE;
    capabilities->maxChannels = 2;
    return true;
}

static SS4S_AudioOpenResult OpenAudio(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    pthread_mutex_lock(&SS4S_SteamLink_Lock);
    SS4S_AudioOpenResult result;
    if (info->codec != SS4S_AUDIO_PCM_S16LE) {
        result = SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
        goto finish;
    }
    CSLAudioContext *audioContext = SLAudio_CreateContext();
    if (audioContext == NULL) {
        result = SS4S_AUDIO_OPEN_ERROR;
        goto finish;
    }
    context->audioContext = audioContext;
    context->audioFrameSize = info->samplesPerFrame * sizeof(int16_t) * info->numOfChannels;
    CSLAudioStream *audioStream = SLAudio_CreateStream(audioContext, info->sampleRate, info->numOfChannels,
                                                       (int) context->audioFrameSize, 1);
    if (audioStream == NULL) {
        result = SS4S_AUDIO_OPEN_ERROR;
        goto finish;
    }
    context->audioStream = audioStream;
    *instance = (SS4S_AudioInstance *) context;
    result = SS4S_AUDIO_OPEN_OK;

    finish:
    pthread_mutex_unlock(&SS4S_SteamLink_Lock);
    return result;
}

static SS4S_AudioFeedResult FeedAudio(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    SS4S_PlayerContext *context = (SS4S_PlayerContext *) instance;
    if (context->audioStream == NULL) {
        return SS4S_AUDIO_FEED_NOT_READY;
    }
    for (size_t i = 0; i < size; i += context->audioFrameSize) {
        if (size - i < context->audioFrameSize) {
            return SS4S_AUDIO_FEED_OVERFLOW;
        }
        void *frameBuffer = SLAudio_BeginFrame(context->audioStream);
        memcpy(frameBuffer, data + i, context->audioFrameSize);
        SLAudio_SubmitFrame(context->audioStream);
    }
    return SS4S_AUDIO_FEED_OK;
}

static void CloseAudio(SS4S_AudioInstance *instance) {
    pthread_mutex_lock(&SS4S_SteamLink_Lock);
    SS4S_PlayerContext *context = (void *) instance;
    SLAudio_FreeStream(context->audioStream);
    context->audioStream = NULL;
    SLAudio_FreeContext(context->audioContext);
    context->audioContext = NULL;
    pthread_mutex_unlock(&SS4S_SteamLink_Lock);
}

const SS4S_AudioDriver SS4S_STEAMLINK_AudioDriver = {
        .GetCapabilities = GetCapabilities,
        .Open = OpenAudio,
        .Feed = FeedAudio,
        .Close = CloseAudio,
};