#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "ss4s.h"
#include "nalu_reader.h"

int ss4s_nalu_cb(void *ctx, const unsigned char *nalu, size_t size);

int audio_proc(void *arg);

int video_proc(void *arg);

int session_proc(void *arg);

void handle_event(SDL_Event *event);

enum {
    EVENT_START_PLAYBACK = 0,
};

int main(int argc, char *argv[]) {
    char *vid_driver = "lgnc", *aud_driver = "lgnc";
    printf("Request audio driver: %s\n", aud_driver);
    printf("Request video driver: %s\n", vid_driver);
    SDL_Init(SDL_INIT_VIDEO);

    SS4S_Config config = {
            .audioDriver = aud_driver,
            .videoDriver = vid_driver,
    };
    if (SS4S_Init(argc, argv, &config) != 0) {
        abort();
    }

    if (SS4S_GetAudioModuleName() == NULL) {
        fprintf(stderr, "No audio driver available\n");
        abort();
    }

    if (SS4S_GetVideoModuleName() == NULL) {
        fprintf(stderr, "No video driver available\n");
        abort();
    }


    SDL_Window *window = SDL_CreateWindow("SS4S", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080,
                                          SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SS4S_PostInit(argc, argv);

    SDL_Event startPlayback = {.user = {SDL_USEREVENT, EVENT_START_PLAYBACK}};
    SDL_PushEvent(&startPlayback);

    static int rendered = 0;
    while (!SDL_QuitRequested()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handle_event(&event);
        }
        if (!rendered) {
            rendered = 1;
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
        }
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SS4S_Quit();
    return 0;
}

int session_proc(void *arg) {
    SS4S_Player *player = SS4S_PlayerOpen();
    assert(player != NULL);
    SS4S_PlayerInfo playerInfo;
    assert(SS4S_PlayerGetInfo(player, &playerInfo));
    assert(playerInfo.audio.module != NULL);
    assert(playerInfo.video.module != NULL);
    SS4S_VideoCapabilities videoCapabilities;
    if (SS4S_GetVideoCapabilities(&videoCapabilities)) {
        assert(videoCapabilities.codecs & SS4S_VIDEO_H264);
    }

    SS4S_AudioCapabilities audioCapabilities;
    if (SS4S_GetAudioCapabilities(&audioCapabilities)) {
        assert(audioCapabilities.codecs & SS4S_AUDIO_PCM_S16LE);
    }

    SS4S_PlayerSetWaitAudioVideoReady(player, true);
    SDL_Thread *audio_thread = SDL_CreateThread(audio_proc, "audio_proc", player);
    SDL_Thread *video_thread = SDL_CreateThread(video_proc, "video_proc", player);
    SDL_WaitThread(audio_thread, NULL);
    SDL_WaitThread(video_thread, NULL);
    SS4S_PlayerClose(player);

    SDL_Event quit = {SDL_QUIT};
    SDL_PushEvent(&quit);
    return 0;
}

int audio_proc(void *arg) {
    SS4S_Player *player = arg;
    SS4S_AudioInfo audioInfo = {
            .numOfChannels = 2,
            .sampleRate = 48000,
            .codec = SS4S_AUDIO_PCM_S16LE,
            .samplesPerFrame = 240,
            .appName = "SS4S_Test",
            .streamName = "Music"
    };
    assert(SS4S_PlayerAudioOpen(player, &audioInfo) == SS4S_AUDIO_OPEN_OK);

    FILE *sampleFile = fopen("sample.pcm", "rb");
    assert(sampleFile != NULL);

    int16_t buf[240 * 2];
    size_t samplesRead;
    size_t unitSize = sizeof(int16_t) * 2;
    size_t numOfSamples = 0;
    while ((samplesRead = fread(buf, unitSize, 240, sampleFile)) > 0) {
        Uint32 start = SDL_GetTicks();
        if (SS4S_PlayerAudioFeed(player, (unsigned char *) buf, samplesRead * unitSize) != SS4S_AUDIO_FEED_OK) {
            break;
        }
        numOfSamples += samplesRead;
        Sint32 delay = (Sint32) ((samplesRead * 1000 / audioInfo.sampleRate) - (SDL_GetTicks() - start));
        if (delay > 0) {
            SDL_Delay(delay);
        }
    }
    fclose(sampleFile);
    return 0;
}

int video_proc(void *arg) {
    SS4S_Player *player = arg;
    SS4S_VideoInfo videoInfo = {
            .codec = SS4S_VIDEO_H264,
            .width = 1280,
            .height = 800,
            .frameRateNumerator = 30,
            .frameRateDenominator = 1,
    };
    assert(SS4S_PlayerVideoOpen(player, &videoInfo) == SS4S_VIDEO_OPEN_OK);

    FILE *sampleFile = fopen("sample.h264", "rb");
    assert(sampleFile != NULL);
    nalu_read(sampleFile, ss4s_nalu_cb, player);

    fclose(sampleFile);

    assert(SS4S_PlayerVideoClose(player));
    return 0;
}

int ss4s_nalu_cb(void *ctx, const unsigned char *nalu, size_t size) {
    Uint32 start = SDL_GetTicks();
    SS4S_Player *player = ctx;
    SS4S_VideoFeedResult result = SS4S_PlayerVideoFeed(player, nalu, size, 0);
    Sint32 delay = (Sint32) (33 - (SDL_GetTicks() - start));
    if (delay > 0) {
        SDL_Delay(delay);
    }
    return result;
}

void handle_event(SDL_Event *event) {
    switch (event->type) {
        case SDL_USEREVENT: {
            switch (event->user.code) {
                case EVENT_START_PLAYBACK: {
                    SDL_CreateThread(session_proc, "session_proc", NULL);
                }
            }
            break;
        }
    }
}