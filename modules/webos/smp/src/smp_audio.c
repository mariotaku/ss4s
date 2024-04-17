#include <assert.h>
#include "StarfishMediaAPIs_C.h"
#include "ss4s/modapi.h"
#include "smp_player.h"

const char *StarfishAudioCodecName(SS4S_AudioCodec codec) {
    switch (codec) {
        case SS4S_AUDIO_PCM_S16LE:
            return "PCM";
        case SS4S_AUDIO_OPUS:
            return "OPUS";
        case SS4S_AUDIO_AAC:
            return "AAC";
        case SS4S_AUDIO_AC3:
            return "AC3";
        default:
            return NULL;
    }
}

static bool GetAudioCapabilities(SS4S_AudioCapabilities *capabilities, SS4S_AudioCodec wantedCodecs) {
    (void) wantedCodecs;
    capabilities->codecs = SS4S_AUDIO_PCM_S16LE | SS4S_AUDIO_OPUS | SS4S_AUDIO_AAC | SS4S_AUDIO_AC3;
    capabilities->maxChannels = 6;
    return true;
}

static SS4S_AudioOpenResult AudioOpen(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    if (context == NULL) {
        return SS4S_AUDIO_OPEN_ERROR;
    }
    StarfishPlayerLock(context);
    context->audioInfo = *info;
    context->hasAudio = true;
    *instance = (void *) context;
    if (!context->hasVideo && context->waitAudioVideoReady) {
        StarfishLibContext->Log(SS4S_LogLevelInfo, "SMP", "AudioOpen: defer loading until video is ready");
        StarfishPlayerUnlock(context);
        return SS4S_AUDIO_OPEN_OK;
    }
    if (context->state != SMP_STATE_UNLOADED) {
        StarfishPlayerUnloadInner(context);
    }
    if (StarfishPlayerLoadInner(context)) {
        StarfishPlayerUnlock(context);
        return SS4S_AUDIO_OPEN_OK;
    }
    StarfishPlayerUnlock(context);
    return SS4S_AUDIO_OPEN_ERROR;
}

static void AudioClose(SS4S_AudioInstance *instance) {
    assert(instance != NULL);
    SS4S_PlayerContext *context = (SS4S_PlayerContext *) instance;
    StarfishPlayerLock(context);
    StarfishPlayerUnloadInner(context);
    context->hasAudio = false;
    StarfishPlayerUnlock(context);
}

static SS4S_AudioFeedResult AudioFeed(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    switch (StarfishPlayerFeed((SS4S_PlayerContext *) instance, data, size, 2)) {
        case SMP_FEED_OK:
            return SS4S_AUDIO_FEED_OK;
        case SMP_FEED_NOT_READY:
            return SS4S_AUDIO_FEED_NOT_READY;
        case SMP_FEED_BUFFER_FULL:
            return SS4S_AUDIO_FEED_OVERFLOW;
        default:
            return SS4S_AUDIO_FEED_ERROR;
    }
}

const SS4S_AudioDriver StarfishAudioDriver = {
        .GetCapabilities = GetAudioCapabilities,
        .Open = AudioOpen,
        .Feed = AudioFeed,
        .Close = AudioClose,
};