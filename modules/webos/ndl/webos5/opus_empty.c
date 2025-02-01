#include "opus_empty.h"
#include "opus_empty_samples.h"

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

struct SS4S_OpusEmpty {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t thread;
    SS4S_NDLOpusEmptyFeedFunc feed;
    void *feedArg;

    uint32_t channels, streams, coupled, frameDurationUs;
    bool started, running;
    struct timespec feedEmptyAfter;
};

static void *OpusEmptyProc(void *arg);


SS4S_OpusEmpty *SS4S_OpusEmptyCreate(uint32_t channels, uint32_t streams, uint32_t coupled, uint32_t frameDurationUs) {
    SS4S_OpusEmpty *instance = calloc(1, sizeof(SS4S_OpusEmpty));
    pthread_mutex_init(&instance->lock, NULL);
    pthread_mutex_lock(&instance->lock);
    pthread_cond_init(&instance->cond, NULL);
    instance->channels = channels;
    instance->streams = streams;
    instance->coupled = coupled;
    instance->frameDurationUs = frameDurationUs;
    pthread_create(&instance->thread, NULL, OpusEmptyProc, instance);
    pthread_mutex_unlock(&instance->lock);
    return instance;
}

void SS4S_OpusEmptyStart(SS4S_OpusEmpty *instance, SS4S_NDLOpusEmptyFeedFunc feed, void *feedArg) {
    pthread_mutex_lock(&instance->lock);
    instance->feed = feed;
    instance->feedArg = feedArg;
    instance->started = true;
    instance->running = true;
    clock_gettime(CLOCK_MONOTONIC_RAW, &instance->feedEmptyAfter);
    pthread_cond_signal(&instance->cond);
    pthread_mutex_unlock(&instance->lock);
}

void SS4S_OpusEmptyFrameArrived(SS4S_OpusEmpty *instance) {
    pthread_mutex_lock(&instance->lock);
    clock_gettime(CLOCK_MONOTONIC_RAW, &instance->feedEmptyAfter);
    instance->feedEmptyAfter.tv_nsec += instance->frameDurationUs * 1000 * 3;
    while (instance->feedEmptyAfter.tv_nsec >= 1000000000) {
        instance->feedEmptyAfter.tv_sec++;
        instance->feedEmptyAfter.tv_nsec -= 1000000000;
    }
    pthread_mutex_unlock(&instance->lock);
}

void SS4S_OpusEmptyDestroy(SS4S_OpusEmpty *instance) {
    pthread_mutex_lock(&instance->lock);
    instance->started = true;
    instance->running = false;
    pthread_mutex_unlock(&instance->lock);
    pthread_join(instance->thread, NULL);
    pthread_cond_destroy(&instance->cond);
    pthread_mutex_destroy(&instance->lock);
    free(instance);
}

static int OpusEmptyPlay(const SS4S_OpusEmpty *instance);

static void WaitForStart(SS4S_OpusEmpty *instance) {
    pthread_mutex_lock(&instance->lock);
    while (!instance->started) {
        pthread_cond_wait(&instance->cond, &instance->lock);
    }
    pthread_mutex_unlock(&instance->lock);
}

static bool IsRunning(SS4S_OpusEmpty *instance) {
    pthread_mutex_lock(&instance->lock);
    bool running = instance->running;
    pthread_mutex_unlock(&instance->lock);
    return running;
}

static time_t TimeDiff(const struct timespec *a, const struct timespec *b) {
    return (a->tv_sec - b->tv_sec) * 1000000 + (a->tv_nsec - b->tv_nsec) / 1000 + 100;
}

static bool IsPastFeedEmptyAfter(SS4S_OpusEmpty *instance) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    pthread_mutex_lock(&instance->lock);
    bool ret = TimeDiff(&now, &instance->feedEmptyAfter) >= 0;
    pthread_mutex_unlock(&instance->lock);
    return ret;
}

void *OpusEmptyProc(void *arg) {
    SS4S_OpusEmpty *instance = (SS4S_OpusEmpty *) arg;
    WaitForStart(instance);
    while (IsRunning(instance)) {
        if (IsPastFeedEmptyAfter(instance)) {
            OpusEmptyPlay(instance);
        }
        usleep(instance->frameDurationUs);
    }
    return NULL;
}

int OpusEmptyPlay(const SS4S_OpusEmpty *instance) {
    switch (instance->channels * 100 + instance->streams * 10 + instance->coupled) {
        case 220:
            return instance->feed(instance->feedArg, opus_empty_frame_220, sizeof(opus_empty_frame_220));
        case 211:
            return instance->feed(instance->feedArg, opus_empty_frame_211, sizeof(opus_empty_frame_211));
        case 660:
            // Fallthrough because it will be converted to 642
        case 642:
            return instance->feed(instance->feedArg, opus_empty_frame_642, sizeof(opus_empty_frame_642));
        case 880:
            return instance->feed(instance->feedArg, opus_empty_frame_880, sizeof(opus_empty_frame_880));
        case 853:
            return instance->feed(instance->feedArg, opus_empty_frame_853, sizeof(opus_empty_frame_853));
        default:
            return 0;
    }
}
