#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include "test_common.h"
#include "ss4s.h"
#include "nalu_reader.h"

int ss4s_nalu_cb(void *ctx, const unsigned char *nalu, size_t size);

void *audio_proc(void *arg);

void *video_proc(void *arg);

void *session_proc(void *arg);

int main(int argc, char *argv[]) {
    setenv("APPID", "com.example.test", 0);
    char drivers[32] = {'\0'};
    single_test_infer_module(drivers, sizeof(drivers), "ss4s_test_av_threaded_", argc, argv);
    char *vid_driver = strchr(drivers, '_'), *aud_driver = drivers;
    if (vid_driver == NULL) {
        vid_driver = drivers;
        aud_driver = drivers;
    } else {
        *vid_driver++ = '\0';
    }
    printf("Request audio driver: %s\n", aud_driver);
    printf("Request video driver: %s\n", vid_driver);

    SS4S_Config config = {
            .audioDriver = aud_driver,
            .videoDriver = vid_driver,
    };
    SS4S_Init(argc, argv, &config);
    SS4S_PostInit(argc, argv);
    pthread_t session_thread;
    pthread_create(&session_thread, NULL, session_proc, NULL);
    pthread_join(session_thread, NULL);
    SS4S_Quit();
    return 0;
}

void *session_proc(void *arg) {
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
    pthread_t audio_thread, video_thread;
    pthread_create(&audio_thread, NULL, audio_proc, player);
    pthread_create(&video_thread, NULL, video_proc, player);
    pthread_join(audio_thread, NULL);
    pthread_join(video_thread, NULL);
    SS4S_PlayerClose(player);
    return NULL;
}

void *audio_proc(void *arg) {
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
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (SS4S_PlayerAudioFeed(player, (unsigned char *) buf, samplesRead * unitSize) != SS4S_AUDIO_FEED_OK) {
            break;
        }
        numOfSamples += samplesRead;
        clock_gettime(CLOCK_MONOTONIC, &end);
        int64_t delay = ((int64_t) samplesRead * 1000000 / audioInfo.sampleRate) -
                        ((end.tv_sec * 1000000 + end.tv_nsec / 1000) - (start.tv_sec * 1000000 + start.tv_nsec / 1000));
        if (delay > 0) {
            usleep(delay);
        }
    }
    fclose(sampleFile);
    return NULL;
}

void *video_proc(void *arg) {
    SS4S_Player *player = arg;
    SS4S_VideoInfo videoInfo = {
            .codec = SS4S_VIDEO_H264,
            .width = 1280,
            .height = 800,
    };
    assert(SS4S_PlayerVideoOpen(player, &videoInfo) == SS4S_VIDEO_OPEN_OK);

    FILE *sampleFile = fopen("sample.h264", "rb");
    assert(sampleFile != NULL);
    nalu_read(sampleFile, ss4s_nalu_cb, player);

    fclose(sampleFile);

    assert(SS4S_PlayerVideoClose(player));
    return NULL;
}

int ss4s_nalu_cb(void *ctx, const unsigned char *nalu, size_t size) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    SS4S_Player *player = ctx;
    SS4S_VideoFeedResult result = SS4S_PlayerVideoFeed(player, nalu, size, 0);
    clock_gettime(CLOCK_MONOTONIC, &end);
    int64_t delay = ((int64_t) 33000) -
                    ((end.tv_sec * 1000000 + end.tv_nsec / 1000) - (start.tv_sec * 1000000 + start.tv_nsec / 1000));
    if (delay > 0) {
        usleep(delay);
    }
    return result;
}
