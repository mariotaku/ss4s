#include "pacer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include "ringbuf.h"

struct SS4S_Pacer {
    /// Component fields
    pthread_mutex_t lock;
    ss4s_ringbuf_t *buffer;
    SS4S_PacerCallback callback;
    void *callbackContext;

    /// Configuration fields
    size_t healthyBufferCount;
    size_t bufferSizeLimit;
    uint32_t intervalUs;

    /// Context fields
    pthread_t consumerThread;

    /// State fields
    size_t bufferCount;
    bool underflow;
    bool running;
    struct timespec lastCallbackTime;

};

typedef struct FramePreamble {
    size_t size;
    struct timespec time;
} FramePreamble;

static void *ConsumerThread(void *arg);

static size_t QueuePop(SS4S_Pacer *pacer, uint8_t *data, size_t dataCap, FramePreamble *preamble, size_t *bufRemaining);

static int QueuePush(SS4S_Pacer *pacer, const uint8_t *data, size_t dataLength);

static inline time_t TimeDiff(const struct timespec *a, const struct timespec *b);

SS4S_Pacer *SS4S_PacerCreate(size_t healthyBufferCount, size_t bufferSizeLimit, uint32_t intervalUs,
                             SS4S_PacerCallback callback, void *callbackContext) {
    SS4S_Pacer *pacer = (SS4S_Pacer *) malloc(sizeof(SS4S_Pacer));
    pthread_mutex_init(&pacer->lock, NULL);
    pacer->buffer = ringbuf_new((sizeof(FramePreamble) + bufferSizeLimit) * healthyBufferCount * 4);
    pacer->bufferCount = 0;
    pacer->underflow = true;
    pacer->healthyBufferCount = healthyBufferCount;
    pacer->bufferSizeLimit = bufferSizeLimit;
    pacer->intervalUs = intervalUs;
    pacer->callback = callback;
    pacer->callbackContext = callbackContext;

    pacer->running = true;

    pthread_mutex_lock(&pacer->lock);
    pthread_create(&pacer->consumerThread, NULL, ConsumerThread, pacer);
    pthread_mutex_unlock(&pacer->lock);
    return pacer;
}

int SS4S_PacerFeed(SS4S_Pacer *pacer, const unsigned char *data, size_t size) {
    return QueuePush(pacer, (const uint8_t *) data, size);
}

size_t SS4S_PacerGetBufferCount(SS4S_Pacer *pacer) {
    pthread_mutex_lock(&pacer->lock);
    size_t count = pacer->bufferCount;
    pthread_mutex_unlock(&pacer->lock);
    return count;
}

void SS4S_PacerClear(SS4S_Pacer *pacer) {
    pthread_mutex_lock(&pacer->lock);
    ringbuf_clear(pacer->buffer);
    pacer->bufferCount = 0;
    pacer->underflow = true;
    pthread_mutex_unlock(&pacer->lock);
}

void SS4S_PacerDestroy(SS4S_Pacer *pacer) {
    pthread_mutex_lock(&pacer->lock);
    pacer->running = false;
    pthread_mutex_unlock(&pacer->lock);

    pthread_join(pacer->consumerThread, NULL);

    pthread_mutex_lock(&pacer->lock);
    ringbuf_delete(pacer->buffer);
    pthread_mutex_unlock(&pacer->lock);
    pthread_mutex_destroy(&pacer->lock);
    free(pacer);
}

bool IsRunning(SS4S_Pacer *pacer) {
    pthread_mutex_lock(&pacer->lock);
    bool running = pacer->running;
    pthread_mutex_unlock(&pacer->lock);
    return running;
}

size_t QueuePop(SS4S_Pacer *pacer, uint8_t *data, size_t dataCap, FramePreamble *preamble, size_t *bufRemaining) {
    pthread_mutex_lock(&pacer->lock);
    // Don't pop if underflow or no frames
    if (pacer->underflow) {
        pthread_mutex_unlock(&pacer->lock);
        return 0;
    }
    if (pacer->bufferCount == 0) {
        pacer->underflow = true;
        pthread_mutex_unlock(&pacer->lock);
        return 0;
    }
    size_t sizeRead = ringbuf_read(pacer->buffer, (unsigned char *) preamble, sizeof(FramePreamble));
    assert(sizeRead == sizeof(FramePreamble));
    assert(preamble->size <= dataCap);
    pacer->bufferCount--;
    size_t read = ringbuf_read(pacer->buffer, data, preamble->size);
    *bufRemaining = pacer->bufferCount;
    pthread_mutex_unlock(&pacer->lock);
    return (int) read;
}

int QueuePush(SS4S_Pacer *pacer, const uint8_t *data, size_t dataLength) {
    pthread_mutex_lock(&pacer->lock);
    if (dataLength > pacer->bufferSizeLimit) {
        pthread_mutex_unlock(&pacer->lock);
        return -1;
    }
    FramePreamble preamble = {dataLength};
    clock_gettime(CLOCK_MONOTONIC, &preamble.time);
    size_t sizeWritten = ringbuf_write(pacer->buffer, (const unsigned char *) &preamble, sizeof(preamble));
    if (sizeWritten == 0) {
        pthread_mutex_unlock(&pacer->lock);
        return 0;
    }
    assert(sizeWritten == sizeof(preamble));
    sizeWritten = ringbuf_write(pacer->buffer, data, dataLength);
    if (sizeWritten == 0) {
        size_t rewindSize = ringbuf_rewind(pacer->buffer, sizeof(preamble));
        assert(rewindSize == sizeof(preamble));
        pthread_mutex_unlock(&pacer->lock);
        return 0;
    }
    assert(sizeWritten == dataLength);
    pacer->bufferCount++;
    // Reset underflow flag if we have enough frames
    if (pacer->bufferCount >= pacer->healthyBufferCount) {
        pacer->underflow = false;
    }
    pthread_mutex_unlock(&pacer->lock);
    return (int) sizeWritten;
}

void GetLastFrameTime(SS4S_Pacer *pacer, struct timespec *ts) {
    pthread_mutex_lock(&pacer->lock);
    *ts = pacer->lastCallbackTime;
    pthread_mutex_unlock(&pacer->lock);
}

void UpdateLastFrameTime(SS4S_Pacer *pacer) {
    pthread_mutex_lock(&pacer->lock);
    clock_gettime(CLOCK_MONOTONIC, &pacer->lastCallbackTime);
    pthread_mutex_unlock(&pacer->lock);
}

void *ConsumerThread(void *arg) {
    SS4S_Pacer *pacer = (SS4S_Pacer *) arg;
    uint32_t intervalUs = pacer->intervalUs;
    size_t healthyBufferCount = pacer->healthyBufferCount;
    size_t frameSizeLimit = pacer->bufferSizeLimit;
    SS4S_PacerCallback callback = pacer->callback;
    void *callbackContext = pacer->callbackContext;
    uint8_t *frame = (uint8_t *) malloc(frameSizeLimit);
    UpdateLastFrameTime(pacer);
    while (IsRunning(pacer)) {
        struct timespec last;
        GetLastFrameTime(pacer, &last);
        FramePreamble preamble = {0};
        size_t remainingCount;
        size_t frameSize = QueuePop(pacer, frame, frameSizeLimit, &preamble, &remainingCount);

        callback(callbackContext, frame, frameSize, &preamble.time);

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        // Calculate time elapsed after callback since last frame
        int64_t diff = TimeDiff(&now, &last);
        // Wait if elapsed time is less than interval
        int64_t sleepUs = intervalUs - diff;

        // If buffer is larger than healthy amount, reduce sleep time by latency
        if (frameSize > 0 && (remainingCount + 1) >= healthyBufferCount) {
            time_t lag = TimeDiff(&now, &preamble.time) - diff;
            int64_t drift = lag - (int64_t) (healthyBufferCount * intervalUs);
            if (drift > 0) {
                sleepUs -= drift;
            }
        }

        if (sleepUs > 0) {
            usleep(sleepUs);
        }

        UpdateLastFrameTime(pacer);
    }
    free(frame);
    return NULL;
}

time_t TimeDiff(const struct timespec *a, const struct timespec *b) {
    return (a->tv_sec - b->tv_sec) * 1000000 + (a->tv_nsec - b->tv_nsec) / 1000 + 100;
}
