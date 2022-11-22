#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "ss4s.h"

int main(int argc, char *argv[]) {
    char *driver = "sdl";
    if (argc > 1) {
        driver = argv[1];
    }
    printf("Request audio driver: %s\n", driver);

    SS4S_Config config = {.audioDriver = driver};
    SS4S_Init(argc, argv, &config);
    SS4S_PostInit(argc, argv);
    SS4S_Player *player = SS4S_PlayerOpen();
    assert(player != NULL);
    SS4S_PlayerInfo playerInfo;
    assert(SS4S_PlayerGetInfo(player, &playerInfo));
    if (playerInfo.audioModule == NULL) {
        return 127;
    }
    assert(strcmp(playerInfo.audioModule, driver) == 0);

    SS4S_AudioInfo audioInfo = {
            .numOfChannels = 2,
            .sampleRate = 48000,
            .codec = SS4S_AUDIO_PCM_S16LE,
            .samplesPerFrame = 240,
    };
    assert(SS4S_PlayerAudioOpen(player, &audioInfo) == SS4S_AUDIO_OPEN_OK);

    FILE *sampleFile = fopen("sample.pcm", "rb");
    assert(sampleFile != NULL);

    int16_t buf[240 * 2];
    size_t samplesRead;
    size_t unitSize = sizeof(int16_t) * 2;
    size_t numOfSamples = 0;
    while ((samplesRead = fread(buf, unitSize, 240, sampleFile)) > 0) {
        assert(SS4S_PlayerAudioFeed(player, (unsigned char *) buf, samplesRead * unitSize));
        numOfSamples += samplesRead;
        usleep(samplesRead * 1000000 / audioInfo.sampleRate);
    }
    fclose(sampleFile);
    printf("Sound duration: %.03f seconds\n", numOfSamples / (double) audioInfo.sampleRate);

    assert(SS4S_PlayerAudioClose(player));
    SS4S_PlayerClose(player);
    SS4S_Quit();
    return 0;
}