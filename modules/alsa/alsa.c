#include <stdlib.h>
#include "ss4s/modapi.h"

#include <alsa/asoundlib.h>

#define CHECK_RETURN(f) if ((f) < 0) { return NULL; }

#define FRAME_SIZE 240
#define FRAME_BUFFER 2

struct SS4S_AudioInstance {
    snd_pcm_t *handle;
};

static SS4S_AudioInstance *Open(const SS4S_AudioInfo *info);

static int Feed(SS4S_AudioInstance *instance, const unsigned char *data, size_t size);

static void Close(SS4S_AudioInstance *instance);

const static SS4S_AudioDriver ALSADriver = {
        .Open = Open,
        .Feed = Feed,
        .Close = Close,
};

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_ALSA(SS4S_Module *module) {
    module->Name = "alsa";
    module->AudioDriver = &ALSADriver;
    return true;
}

static SS4S_AudioInstance *Open(const SS4S_AudioInfo *info) {
    snd_pcm_t *handle = NULL;


    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
    // Magic number, needs change later
    snd_pcm_uframes_t period_size = FRAME_SIZE * FRAME_BUFFER;
    snd_pcm_uframes_t buffer_size = 2 * period_size;
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

    SS4S_AudioInstance *instance = calloc(1, sizeof(SS4S_AudioInstance));
    instance->handle = handle;
    return instance;
}

static int Feed(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    int rc = 0;
//    if ((rc = snd_pcm_writei(instance->handle, data, size)) == -EPIPE) {
//        snd_pcm_prepare(instance->handle);
//    }
    return rc == 0;
}

static void Close(SS4S_AudioInstance *instance) {
    assert(instance->handle != NULL);
    snd_pcm_drain(instance->handle);
    snd_pcm_close(instance->handle);
    free(instance);
}