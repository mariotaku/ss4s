#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "ss4s.h"
#include "test_common.h"

int main(int argc, char *argv[]) {
    char driver[16] = {'\0'};
    single_test_infer_module(driver, sizeof(driver), "ss4s_test_pcm_playback_", argc, argv);
    printf("Request audio driver: %s\n", driver);
    if (!SS4S_ModuleAvailable(driver, SS4S_MODULE_CHECK_AUDIO)) {
        printf("Skipping unsupported audio driver: %s\n", driver);
        return 127;
    }

    SS4S_Config config = {.audioDriver = driver};
    SS4S_Init(argc, argv, &config);
    SS4S_PostInit(argc, argv);
    SS4S_Player *player = SS4S_PlayerOpen();
    assert(player != NULL);
    SS4S_PlayerInfo playerInfo;
    assert(SS4S_PlayerGetInfo(player, &playerInfo));
    if (playerInfo.audio.module == NULL) {
        return 127;
    }
    assert(strcmp(playerInfo.audio.module, driver) == 0);

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
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        assert(SS4S_PlayerAudioFeed(player, (unsigned char *) buf, samplesRead * unitSize) == SS4S_AUDIO_FEED_OK);
        numOfSamples += samplesRead;
        clock_gettime(CLOCK_MONOTONIC, &end);
        int64_t delay = ((int64_t) samplesRead * 1000000 / audioInfo.sampleRate) -
                        ((end.tv_sec * 1000000 + end.tv_nsec / 1000) - (start.tv_sec * 1000000 + start.tv_nsec / 1000));
        if (delay > 0) {
            usleep(delay);
        }
    }
    fclose(sampleFile);
    printf("Sound duration: %.03f seconds\n", numOfSamples / (double) audioInfo.sampleRate);

    assert(SS4S_PlayerAudioClose(player));
    SS4S_PlayerClose(player);
    SS4S_Quit();
    return 0;
}