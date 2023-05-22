#include "ss4s/modapi.h"

#include <pulse/simple.h>
#include <stdlib.h>
#include <pulse/error.h>
#include <assert.h>

static const SS4S_LibraryContext *LibContext;

struct SS4S_AudioInstance {
    pa_simple *dev;
};

static SS4S_AudioOpenResult Open(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                 SS4S_PlayerContext *context) {
    (void) context;
    if (info->codec != SS4S_AUDIO_PCM_S16LE) {
        return SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
    }
    pa_sample_spec spec = {
            .format = PA_SAMPLE_S16LE,
            .rate = info->sampleRate,
            .channels = info->numOfChannels,
    };
    size_t frame_size = info->samplesPerFrame * 2 * sizeof(uint16_t);
    pa_buffer_attr buffer_attr = {
            .maxlength = frame_size * 8 /*40ms*/,
            .tlength = -1,
            .prebuf = -1,
            .minreq =  -1,
    };
    pa_channel_map channel_map;
    pa_channel_map *pchannel_map = pa_channel_map_init(&channel_map);
    pchannel_map->channels = info->numOfChannels;
    switch (info->numOfChannels) {
        case 8:
            pchannel_map->map[6] = PA_CHANNEL_POSITION_SIDE_LEFT;
            pchannel_map->map[7] = PA_CHANNEL_POSITION_SIDE_RIGHT;
            // Intentionally fallthrough
        case 6:
            pchannel_map->map[2] = PA_CHANNEL_POSITION_FRONT_CENTER;
            pchannel_map->map[3] = PA_CHANNEL_POSITION_LFE;
            pchannel_map->map[4] = PA_CHANNEL_POSITION_REAR_LEFT;
            pchannel_map->map[5] = PA_CHANNEL_POSITION_REAR_RIGHT;
            // Intentionally fallthrough
        default:
            pchannel_map->map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
            pchannel_map->map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
    }

    assert(info->appName != NULL);
    assert(info->streamName != NULL);
    int error = 0;
    pa_simple *dev = pa_simple_new(NULL, info->appName, PA_STREAM_PLAYBACK, NULL,
                                   info->streamName, &spec, pchannel_map, &buffer_attr, &error);
    if (error != 0) {
        LibContext->Log(SS4S_LogLevelError, "Pulse", "Can't open audio device: %s", pa_strerror(error));
    }
    if (!dev) {
        return SS4S_AUDIO_OPEN_ERROR;
    }
    SS4S_AudioInstance *newInstance = calloc(1, sizeof(SS4S_AudioInstance));
    newInstance->dev = dev;
    *instance = newInstance;
    return SS4S_AUDIO_OPEN_OK;
}

static SS4S_AudioFeedResult Feed(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    int error;
    pa_simple_write(instance->dev, data, size, &error);
    return SS4S_AUDIO_FEED_OK;
}

static void Close(SS4S_AudioInstance *instance) {
    pa_simple_free(instance->dev);
    free(instance);
}

static const SS4S_AudioDriver PulseDriver = {
        .Open = Open,
        .Feed = Feed,
        .Close = Close,
};

SS4S_EXPORTED bool SS4S_ModuleOpen_PULSE(SS4S_Module *module, const SS4S_LibraryContext *context) {
    (void) context;
    module->Name = "pulse";
    module->AudioDriver = &PulseDriver;
    LibContext = context;
    return true;
}
