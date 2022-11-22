#include "ss4s/modapi.h"

#include <pulse/simple.h>
#include <stdlib.h>

struct SS4S_AudioInstance {
    pa_simple *dev;
};

static SS4S_AudioOpenResult Open(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance) {
    if (info->codec != SS4S_AUDIO_PCM_S16LE) {
        return SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
    }
    pa_sample_spec spec = {
            .format = PA_SAMPLE_S16LE,
            .rate = info->sampleRate,
            .channels = info->numOfChannels,
    };
    pa_buffer_attr buffer_attr = {
            .maxlength = -1,
            .tlength = sizeof(short) * info->numOfChannels,
            .prebuf = -1,
            .minreq = -1,
            .fragsize = -1,
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

    int error = 0;
    pa_simple *dev = pa_simple_new(NULL, info->appName, PA_STREAM_PLAYBACK, NULL, "Streaming", &spec, pchannel_map,
                                   &buffer_attr, &error);

    if (!dev) {
        return SS4S_AUDIO_OPEN_ERROR;
    }
    SS4S_AudioInstance *newInstance = calloc(1, sizeof(SS4S_AudioInstance));
    newInstance->dev = dev;
    *instance = newInstance;
    return SS4S_AUDIO_OPEN_OK;
}

static int Feed(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    int error;
    pa_simple_write(instance->dev, data, size, &error);
    return 0;
}

static void Close(SS4S_AudioInstance *instance) {
    pa_simple_free(instance->dev);
    free(instance);
}

const static SS4S_AudioDriver PulseDriver = {
        .Open = Open,
        .Feed = Feed,
        .Close = Close,
};

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_PULSE(SS4S_Module *module) {
    module->Name = "pulse";
    module->AudioDriver = &PulseDriver;
    return true;
}
