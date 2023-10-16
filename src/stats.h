#pragma once

typedef struct SS4S_StatsItem {
    uint64_t frameTimeUs;
    uint32_t latencyUs;
} SS4S_StatsItem;

typedef struct SS4S_StatsCounter {
    SS4S_StatsItem *items;
    size_t capacity;
    size_t index;
    unsigned char size;
} SS4S_StatsCounter;

void SS4S_StatsCounterInit(SS4S_StatsCounter *counter, size_t capacity);

void SS4S_StatsCounterDeinit(SS4S_StatsCounter *counter);

uint32_t SS4S_StatsCounterBeginFrame(SS4S_StatsCounter *counter);

void SS4S_StatsCounterEndFrame(SS4S_StatsCounter *counter, uint32_t beginFrameResult);

void SS4S_StatsCounterReportFrame(SS4S_StatsCounter *counter, uint32_t latencyUs);

int32_t SS4S_StatsCounterGetAverageLatencyUs(const SS4S_StatsCounter *counter, uint32_t intervalUs);
