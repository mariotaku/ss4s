#include <stdlib.h>
#include "ss4s/modapi.h"
#include "ringbuf.h"

#include <SDL2/SDL.h>

#define CHECK_RETURN(f) if ((f) < 0) { return NULL; }

#define FRAME_SIZE 240
#define FRAME_BUFFER 2

struct SS4S_AudioInstance {
    sdlaud_ringbuf *ringbuf;
    unsigned char *readbuf;
    size_t readbuf_size;
};

static SS4S_AudioInstance *Open(const SS4S_AudioInfo *info);

static int Feed(SS4S_AudioInstance *instance, const unsigned char *data, size_t size);

static void Close(SS4S_AudioInstance *instance);


static void Callback(void *userdata, Uint8 *stream, int len);

const static SS4S_AudioDriver SDLDriver = {
        .Open = Open,
        .Feed = Feed,
        .Close = Close,
};

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_SDL(SS4S_Module *module) {
    module->Name = "sdl";
    module->AudioDriver = &SDLDriver;
    return true;
}

static SS4S_AudioInstance *Open(const SS4S_AudioInfo *info) {
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    size_t bufSize = sizeof(short) * info->numOfChannels * info->samplesPerFrame;

    SS4S_AudioInstance *instance = calloc(1, sizeof(SS4S_AudioInstance));

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.callback = Callback;
    want.userdata = instance;
    want.freq = info->sampleRate;
    want.format = AUDIO_S16LSB;
    want.channels = info->numOfChannels;
    want.samples = info->samplesPerFrame;

    if (SDL_OpenAudio(&want, &have) != 0) {
        free(instance);
        return NULL;
    } else {
        if (have.format != want.format) {
        }
        instance->ringbuf = sdlaud_ringbuf_new(bufSize * 1024);
        SDL_PauseAudio(0); // start audio playing.
        return instance;
    }
}

static int Feed(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    size_t write_size = sdlaud_ringbuf_write(instance->ringbuf, data, size);
    if (!write_size) {
        fprintf(stderr, "ring buffer overflow, clean the whole buffer\b");
        sdlaud_ringbuf_clear(instance->ringbuf);
    }
    return 0;
}

static void Close(SS4S_AudioInstance *instance) {
    if (instance->ringbuf != NULL) {
        sdlaud_ringbuf_delete(instance->ringbuf);
        instance->ringbuf = NULL;
    }
    if (instance->readbuf != NULL) {
        SDL_free(instance->readbuf);
        instance->readbuf_size = 0;
        instance->readbuf = NULL;
    }
    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    free(instance);
}

static void Callback(void *userdata, Uint8 *stream, int len) {
    SS4S_AudioInstance *instance = userdata;
    if (instance->readbuf_size < len) {
        instance->readbuf = SDL_realloc(instance->readbuf, len);
        instance->readbuf_size = len;
    }
    size_t read_size = sdlaud_ringbuf_read(instance->ringbuf, instance->readbuf, len);
    if (read_size > 0) {
        SDL_memset(stream, 0, len);
        SDL_MixAudio(stream, instance->readbuf, read_size, SDL_MIX_MAXVOLUME);
    }
}
