#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "stats.h"

static uint8_t NextIndex(const SS4S_StatsCounter *counter);

static uint64_t GetTimeUs();

void SS4S_StatsCounterInit(SS4S_StatsCounter *counter, size_t capacity) {
    assert(capacity > 0);
    counter->items = (SS4S_StatsItem *) malloc(sizeof(SS4S_StatsItem) * capacity);
    counter->capacity = capacity;
    counter->index = 0;
    counter->size = 0;
}

void SS4S_StatsCounterDeinit(SS4S_StatsCounter *counter) {
    free(counter->items);
    counter->items = NULL;
    counter->size = 0;
    counter->capacity = 0;
    counter->index = 0;
}

uint32_t SS4S_StatsCounterBeginFrame(SS4S_StatsCounter *counter) {
    uint8_t index = NextIndex(counter);
    counter->index = index;
    counter->items[index].frameTimeUs = GetTimeUs();
    counter->items[index].latencyUs = -1;
    if (counter->size < counter->capacity) {
        counter->size++;
    } else {
        counter->size = counter->capacity;
    }
    return index | (counter->items[index].frameTimeUs & 0xFFFFFF00);
}

void SS4S_StatsCounterEndFrame(SS4S_StatsCounter *counter, uint32_t beginFrameResult) {
    uint8_t index = beginFrameResult & 0xFF;
    if (index >= counter->capacity) {
        return;
    }
    uint64_t frameTimeUs = counter->items[index].frameTimeUs;
    if ((frameTimeUs & 0xFFFFFF00) != (beginFrameResult & 0xFFFFFF00)) {
        return;
    }
    counter->items[index].latencyUs = GetTimeUs() - frameTimeUs;
}

void SS4S_StatsCounterReportFrame(SS4S_StatsCounter *counter, uint32_t latencyUs) {
    uint8_t index = NextIndex(counter);
    counter->index = index;
    counter->items[index].latencyUs = latencyUs;
    counter->items[index].frameTimeUs = GetTimeUs();
    if (counter->size < counter->capacity) {
        counter->size++;
    } else {
        counter->size = counter->capacity;
    }
}

int32_t SS4S_StatsCounterGetAverageLatencyUs(const SS4S_StatsCounter *counter, uint32_t intervalUs) {
    if (counter->size == 0) {
        return -1;
    }
    uint64_t now = GetTimeUs();
    uint8_t remSize = counter->size, index = counter->index;
    uint32_t count = 0, sum = 0;
    while (remSize > 0) {
        index = (index + counter->capacity - 1) % counter->capacity;
        if (now - counter->items[index].frameTimeUs > intervalUs) {
            break;
        }
        uint32_t latency = counter->items[index].latencyUs;
        remSize--;
        if (latency == (uint32_t) -1) {
            continue;
        }
        sum += latency;
        count++;
    }
    if (count == 0) {
        return -1;
    }
    return (int32_t) (sum / count);
}

uint8_t NextIndex(const SS4S_StatsCounter *counter) {
    assert(counter->capacity > 0);
    return (counter->index + 1) % counter->capacity;
}

static uint64_t GetTimeUs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}
