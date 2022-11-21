#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "ss4s.h"

#include <SDL2/SDL.h>

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_AUDIO);

    SS4S_Config config = {.audioDriver = "sdl"};
    SS4S_Init(argc, argv, &config);
    SS4S_PostInit(argc, argv);
    SS4S_Player *player = SS4S_PlayerOpen();
    assert(player != NULL);
    SS4S_PlayerInfo playerInfo;
    assert(SS4S_PlayerGetInfo(player, &playerInfo));
    assert(playerInfo.audioModule != NULL);
    assert(strcmp(playerInfo.audioModule, "sdl") == 0);

    SS4S_AudioInfo audioInfo = {
            .numOfChannels = 2,
            .sampleRate = 48000,
            .codec = SS4S_AUDIO_PCM,
            .samplesPerFrame = 240,
    };
    assert(SS4S_PlayerAudioOpen(player, &audioInfo));

    FILE *sampleFile = fopen("sample.pcm", "rb");
    assert(sampleFile != NULL);

    int16_t buf[480 * 2];
    size_t bufSize;
    size_t unitSize = sizeof(int16_t) * 2;
    while ((bufSize = fread(buf, unitSize, 480, sampleFile)) > 0) {
        assert(SS4S_PlayerAudioFeed(player, (unsigned char *) buf, bufSize * unitSize));
        usleep(10000);
    }

    fclose(sampleFile);

    assert(SS4S_PlayerAudioClose(player));
    SS4S_PlayerClose(player);
    SS4S_Quit();

    SDL_Quit();
    return 0;
}