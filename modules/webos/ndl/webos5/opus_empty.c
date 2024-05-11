#include "opus_empty.h"
#include "opus_empty_samples.h"
#include "ndl_common.h"

#include <stdlib.h>
#include <pthread.h>

#include <NDL_directmedia_v2.h>
#include <assert.h>
#include <unistd.h>

struct SS4S_NDLOpusEmpty {
    int channels, streams, coupled;
    pthread_t thread;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    bool ready, stop;
};

static void *ThreadProc(void *arg);

static bool IsStopped(SS4S_NDLOpusEmpty *instance);

static int PlayEmpty(const SS4S_NDLOpusEmpty *instance);

SS4S_NDLOpusEmpty *SS4S_NDLOpusEmptyCreate(int channels, int streams, int coupled) {
    SS4S_NDLOpusEmpty *instance = calloc(1, sizeof(SS4S_NDLOpusEmpty));
    instance->channels = channels;
    instance->streams = streams;
    instance->coupled = coupled;
    pthread_mutex_init(&instance->lock, NULL);
    pthread_cond_init(&instance->cond, NULL);
    return instance;
}

void SS4S_NDLOpusEmptyDestroy(SS4S_NDLOpusEmpty *instance) {
    pthread_mutex_destroy(&instance->lock);
    pthread_cond_destroy(&instance->cond);
    free(instance);
}

void SS4S_NDLOpusEmptyMediaLoaded(SS4S_NDLOpusEmpty *instance) {
    pthread_mutex_lock(&instance->lock);
    instance->ready = false;
    instance->stop = false;
    if (pthread_create(&instance->thread, NULL, ThreadProc, instance) != 0) {
        instance->thread = 0;
    }
    assert(instance->thread != 0);
    pthread_mutex_unlock(&instance->lock);
}

void SS4S_NDLOpusEmptyMediaUnloaded(SS4S_NDLOpusEmpty *instance) {
    pthread_mutex_lock(&instance->lock);
    instance->stop = true;
    if (instance->thread != 0) {
        pthread_cancel(instance->thread);
        pthread_detach(instance->thread);
        instance->thread = 0;
    }
    pthread_mutex_unlock(&instance->lock);
}

void SS4S_NDLOpusEmptyMediaVideoReady(SS4S_NDLOpusEmpty *instance) {
    assert(instance->thread != 0);
    pthread_mutex_lock(&instance->lock);
    if (!instance->ready) {
        SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "Start empty audio because real video is ready");
        instance->ready = true;
        pthread_cond_signal(&instance->cond);
    }
    pthread_mutex_unlock(&instance->lock);
}

void SS4S_NDLOpusEmptyMediaAudioReady(SS4S_NDLOpusEmpty *instance) {
    assert(instance->thread != 0);
    pthread_mutex_lock(&instance->lock);
    if (!instance->stop) {
        SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "Stop empty audio because real audio is ready");
        instance->stop = true;
    }
    pthread_mutex_unlock(&instance->lock);
}

void *ThreadProc(void *arg) {
    SS4S_NDLOpusEmpty *instance = arg;

    pthread_mutex_lock(&instance->lock);
    while (!instance->ready) {
        if (pthread_cond_wait(&instance->cond, &instance->lock) != 0) {
            break;
        }
    }
    pthread_mutex_unlock(&instance->lock);

    for (int count = 0; !IsStopped(instance) && count < 20; count++) {
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        PlayEmpty(instance);
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        if (elapsed_us < 5000) {
            if (usleep(5000 - elapsed_us) != 0) {
                break;
            }
        }
    }
    return NULL;
}

bool IsStopped(SS4S_NDLOpusEmpty *instance) {
    pthread_mutex_lock(&instance->lock);
    bool stopped = instance->stop;
    pthread_mutex_unlock(&instance->lock);
    return stopped;
}

int PlayEmpty(const SS4S_NDLOpusEmpty *instance) {
    switch (instance->channels * 100 + instance->streams * 10 + instance->coupled) {
        case 220:
            return NDL_DirectAudioPlay((void *) opus_empty_frame_220, sizeof(opus_empty_frame_220), 0);
        case 211:
            return NDL_DirectAudioPlay((void *) opus_empty_frame_211, sizeof(opus_empty_frame_211), 0);
        case 660:
            // Fallthrough because it will be converted to 642
        case 642:
            return NDL_DirectAudioPlay((void *) opus_empty_frame_642, sizeof(opus_empty_frame_642), 0);
        case 880:
            return NDL_DirectAudioPlay((void *) opus_empty_frame_880, sizeof(opus_empty_frame_880), 0);
        case 853:
            return NDL_DirectAudioPlay((void *) opus_empty_frame_853, sizeof(opus_empty_frame_853), 0);
        default:
            return 0;
    }
}
