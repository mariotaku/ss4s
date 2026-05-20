#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "stats.h"

/* BeginFrame returns a 32-bit token that encodes the slot index in
 * the low 16 bits and a hash of the frame timestamp in the high
 * 16 bits, so EndFrame can detect that the slot has been recycled
 * for a different frame. This caps the supported ring-buffer
 * capacity at 65536, which is more than enough for any plausible
 * frame rate.
 */
#define STATS_INDEX_MASK  0x0000FFFFu
#define STATS_STAMP_MASK  0xFFFF0000u

static size_t NextIndex(const SS4S_StatsCounter *counter);

static uint64_t GetTimeUs();

void SS4S_StatsCounterInit(SS4S_StatsCounter *counter, size_t capacity) {
    assert(capacity > 0);
    assert(capacity <= STATS_INDEX_MASK + 1);
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
    size_t index = NextIndex(counter);
    counter->index = index;
    counter->items[index].frameTimeUs = GetTimeUs();
    counter->items[index].latencyUs = -1;
    if (counter->size < counter->capacity) {
        counter->size++;
    } else {
        counter->size = counter->capacity;
    }
    return (uint32_t) index | ((uint32_t) counter->items[index].frameTimeUs & STATS_STAMP_MASK);
}

void SS4S_StatsCounterEndFrame(SS4S_StatsCounter *counter, uint32_t beginFrameResult) {
    size_t index = beginFrameResult & STATS_INDEX_MASK;
    if (index >= counter->capacity) {
        return;
    }
    uint64_t frameTimeUs = counter->items[index].frameTimeUs;
    if (((uint32_t) frameTimeUs & STATS_STAMP_MASK) != (beginFrameResult & STATS_STAMP_MASK)) {
        return;
    }
    counter->items[index].latencyUs = GetTimeUs() - frameTimeUs;
}

void SS4S_StatsCounterReportFrame(SS4S_StatsCounter *counter, uint32_t latencyUs) {
    size_t index = NextIndex(counter);
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
    size_t remSize = counter->size;
    size_t index = counter->index;
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

size_t NextIndex(const SS4S_StatsCounter *counter) {
    assert(counter->capacity > 0);
    return (counter->index + 1) % counter->capacity;
}

static uint64_t GetTimeUs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}
