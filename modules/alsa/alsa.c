#include "ss4s/modapi.h"

#include <stdlib.h>
#include <stdint.h>

#include <alsa/asoundlib.h>

#define CHECK_RETURN(f) if ((f) < 0) return SS4S_AUDIO_OPEN_ERROR

static void alsa_logger_noop(const char *file, int line, const char *function, int err, const char *fmt, ...);

struct SS4S_AudioInstance {
    snd_pcm_t *handle;
    size_t unitSize;
};

static SS4S_AudioOpenResult Open(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                 SS4S_PlayerContext *context) {
    (void) context;
    if (info->codec != SS4S_AUDIO_PCM_S16LE) {
        return SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
    }
    snd_pcm_t *handle = NULL;

    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
    snd_pcm_uframes_t period_size = info->samplesPerFrame;
    size_t unitSize = sizeof(uint16_t) * info->numOfChannels;
    snd_pcm_uframes_t buffer_size = unitSize * period_size;
    unsigned int sampleRate = info->sampleRate;

    CHECK_RETURN(snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK));

    /* Set hardware parameters */
    CHECK_RETURN(snd_pcm_hw_params_malloc(&hw_params));
    CHECK_RETURN(snd_pcm_hw_params_any(handle, hw_params));
    CHECK_RETURN(snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED));
    CHECK_RETURN(snd_pcm_hw_params_set_format(handle, hw_params, SND_PCM_FORMAT_S16_LE));
    CHECK_RETURN(snd_pcm_hw_params_set_rate_near(handle, hw_params, &sampleRate, NULL));
    CHECK_RETURN(snd_pcm_hw_params_set_channels(handle, hw_params, info->numOfChannels));
    CHECK_RETURN(snd_pcm_hw_params_set_period_size_near(handle, hw_params, &period_size, NULL));
    CHECK_RETURN(snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &buffer_size));
    CHECK_RETURN(snd_pcm_hw_params(handle, hw_params));
    snd_pcm_hw_params_free(hw_params);

    /* Set software parameters */
    CHECK_RETURN(snd_pcm_sw_params_malloc(&sw_params));
    CHECK_RETURN(snd_pcm_sw_params_current(handle, sw_params));
    CHECK_RETURN(snd_pcm_sw_params_set_avail_min(handle, sw_params, period_size));
    CHECK_RETURN(snd_pcm_sw_params_set_start_threshold(handle, sw_params, 1));
    CHECK_RETURN(snd_pcm_sw_params(handle, sw_params));
    snd_pcm_sw_params_free(sw_params);

    CHECK_RETURN(snd_pcm_prepare(handle));

    SS4S_AudioInstance *newInstance = calloc(1, sizeof(SS4S_AudioInstance));
    newInstance->handle = handle;
    newInstance->unitSize = unitSize;
    *instance = newInstance;

    return SS4S_AUDIO_OPEN_OK;
}

static SS4S_AudioFeedResult Feed(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    snd_pcm_sframes_t rc;
    if ((rc = snd_pcm_writei(instance->handle, data, size / instance->unitSize)) == -EPIPE) {
        rc = snd_pcm_prepare(instance->handle);
    }
    if (rc >= 0) {
        return SS4S_AUDIO_FEED_OK;
    }
    return SS4S_AUDIO_FEED_OK;
}

static void Close(SS4S_AudioInstance *instance) {
    assert(instance->handle != NULL);
    snd_pcm_drain(instance->handle);
    snd_pcm_close(instance->handle);
    free(instance);
}

static const SS4S_AudioDriver ALSADriver = {
        .Open = Open,
        .Feed = Feed,
        .Close = Close,
};

SS4S_EXPORTED bool SS4S_ModuleOpen_ALSA(SS4S_Module *module, const SS4S_LibraryContext *context) {
    (void) context;
    module->Name = "alsa";
    module->AudioDriver = &ALSADriver;
    return true;
}

SS4S_EXPORTED bool SS4S_ModuleCheck_ALSA(SS4S_ModuleCheckFlag flags) {
    if (flags & SS4S_MODULE_CHECK_VIDEO) {
        return false;
    }
    snd_lib_error_set_handler(alsa_logger_noop);
    snd_pcm_t *handle = NULL;
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0 || handle == NULL) {
        return false;
    }
    snd_pcm_close(handle);
    snd_lib_error_set_handler(NULL);
    return true;
}

static void alsa_logger_noop(const char *file, int line, const char *function, int err, const char *fmt, ...) {
    // No-op
}