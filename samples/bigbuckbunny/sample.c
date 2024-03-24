#include <unistd.h>
#include <stdlib.h>
#include "esplayer-datasrc.h"
#include <SDL.h>

#include "ss4s.h"

static SS4S_Player *player = NULL;
static bool finished = false;

#define FOURCC_H264 0x34363248
#define FOURCC_H265 0x35363248

void frameCallback(SS4S_VideoOutputFrame *frame, void *userdata) {
    (void) userdata;

}

int videoPreroll(const char *codec, int width, int height, int framerate_numerator, int framerate_denominator) {
    SS4S_VideoInfo info = {
            .width = width,
            .height = height,
            .frameRateNumerator = framerate_numerator,
            .frameRateDenominator = framerate_denominator,
    };
    if (strcasecmp(codec, "h264") == 0) {
        info.codec = SS4S_VIDEO_H264;
    } else if (strcasecmp(codec, "hevc") == 0) {
        info.codec = SS4S_VIDEO_H265;
    } else {
        return -1;
    }
    SS4S_VideoOpenResult result = SS4S_PlayerVideoOpen(player, &info);
    if (result == SS4S_VIDEO_OPEN_OK) {
        SS4S_PlayerVideoSetFrameCallback(player, frameCallback, NULL);
        SS4S_VideoRect src = {0, 0, width, 840 * height / 1080};
        SS4S_VideoRect dst = {0, 0, 1920, 840};
        SS4S_PlayerVideoSetDisplayArea(player, &src, &dst);
    }
    return result;
}

int videoSample(const void *data, size_t size, int flags) {
    SS4S_VideoFeedFlags vflags = 0;
    if (flags & VIDEO_FLAG_FRAME_START) {
        vflags |= SS4S_VIDEO_FEED_DATA_FRAME_START;
    }
    if (flags & VIDEO_FLAG_FRAME_END) {
        vflags |= SS4S_VIDEO_FEED_DATA_FRAME_END;
    }
    if (flags & VIDEO_FLAG_FRAME_KEYFRAME) {
        vflags |= SS4S_VIDEO_FEED_DATA_KEYFRAME;
    }
    return SS4S_PlayerVideoFeed(player, data, size, vflags);
}

void videoEos() {
    SS4S_PlayerVideoClose(player);
}

int audioPreroll(int channels, int sampleFreq) {
    SS4S_AudioInfo info = {
            .codec = SS4S_AUDIO_PCM_S16LE,
            .numOfChannels = channels,
            .sampleRate = sampleFreq,
            .samplesPerFrame = 240,
            .appName = "SS4S_Sample",
            .streamName = "stream",
    };
    return SS4S_PlayerAudioOpen(player, &info);
}

int audioSample(const void *data, size_t size) {
    return SS4S_PlayerAudioFeed(player, data, size);
}

void audioEos() {
    SS4S_PlayerAudioClose(player);
}

void pipelineQuit(int error) {
    SDL_Event quit = {.type = SDL_QUIT};
    SDL_PushEvent(&quit);
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    const char *audioDriver = getenv("SS4S_AUDIO_DRIVER"), *videoDriver = getenv("SS4S_VIDEO_DRIVER");
    if (audioDriver == NULL) {
        audioDriver = "alsa";
    }
    if (videoDriver == NULL) {
        videoDriver = "mmal";
    }
    SS4S_Config config = {
            .audioDriver = audioDriver,
            .videoDriver = videoDriver,
    };
    SS4S_Init(argc, argv, &config);
    SS4S_PostInit(argc, argv);
    SS4S_VideoCapabilities caps;
    SS4S_GetVideoCapabilities(&caps);
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    if (caps.output != SS4S_VIDEO_OUTPUT_DIRECT) {
        window = SDL_CreateWindow("SS4S", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720,
                                  SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, 1280, 720);
    }
    player = SS4S_PlayerOpen();
    struct DATASRC_CALLBACKS dscb = {
            .videoPreroll = videoPreroll,
            .videoSample = videoSample,
            .videoEos = videoEos,
            .audioPreroll = audioPreroll,
            .audioSample = audioSample,
            .audioEos = audioEos,
            .pipelineQuit = pipelineQuit,
    };

    datasrc_run(&dscb);

    SS4S_PlayerClose(player);
    player = NULL;
    SS4S_Quit();
    if (texture != NULL) {
        SDL_DestroyTexture(texture);
    }
    if (renderer != NULL) {
        SDL_DestroyRenderer(renderer);
    }
    if (window != NULL) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}