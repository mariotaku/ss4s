#include <stdlib.h>
#include "ss4s/modapi.h"
#include "ringbuf.h"

#include <SDL2/SDL.h>

struct SS4S_AudioInstance {
    sdlaud_ringbuf *ringbuf;
    unsigned char *readbuf;
    int readbuf_size;
};

static const SS4S_LibraryContext *LibContext;

static void Callback(void *userdata, Uint8 *stream, int len);

static int Init(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    return 0;
}

static void Quit() {
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

static SS4S_AudioOpenResult Open(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                 SS4S_PlayerContext *context) {
    (void) context;
    if (info->codec != SS4S_AUDIO_PCM_S16LE) {
        return SS4S_AUDIO_OPEN_UNSUPPORTED_CODEC;
    }
    size_t bufSize = sizeof(short) * info->numOfChannels * info->samplesPerFrame;

    SS4S_AudioInstance *newInstance = calloc(1, sizeof(SS4S_AudioInstance));

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.callback = Callback;
    want.userdata = newInstance;
    want.freq = info->sampleRate;
    want.format = AUDIO_S16LSB;
    want.channels = info->numOfChannels;
    want.samples = info->samplesPerFrame;

    if (SDL_OpenAudio(&want, &have) != 0) {
        free(newInstance);
        return SS4S_AUDIO_OPEN_ERROR;
    }
    if (have.format != want.format) {
    }
    newInstance->ringbuf = sdlaud_ringbuf_new(bufSize * 1024);
    *instance = newInstance;
    SDL_PauseAudio(0); // start audio playing.
    return SS4S_AUDIO_OPEN_OK;
}

static int Feed(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    size_t write_size = sdlaud_ringbuf_write(instance->ringbuf, data, size);
    if (!write_size) {
        LibContext->Log(SS4S_LogLevelWarn, "SDLAudio", "ring buffer overflow, clean the whole buffer");
        sdlaud_ringbuf_clear(instance->ringbuf);
    }
    return 0;
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


static void Close(SS4S_AudioInstance *instance) {
    SDL_CloseAudio();
    if (instance->ringbuf != NULL) {
        sdlaud_ringbuf_delete(instance->ringbuf);
        instance->ringbuf = NULL;
    }
    if (instance->readbuf != NULL) {
        SDL_free(instance->readbuf);
        instance->readbuf_size = 0;
        instance->readbuf = NULL;
    }
    free(instance);
}

static const SS4S_AudioDriver SDLDriver = {
        .Base = {
                .Init = Init,
                .Quit = Quit,
        },
        .Open = Open,
        .Feed = Feed,
        .Close = Close,
};

SS4S_EXPORTED bool SS4S_ModuleOpen_SDL(SS4S_Module *module, const SS4S_LibraryContext *context) {
    module->Name = "sdl";
    module->AudioDriver = &SDLDriver;
    LibContext = context;
    return true;
}