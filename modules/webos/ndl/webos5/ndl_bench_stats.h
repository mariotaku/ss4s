#pragma once

#include <stdbool.h>
#include <stdint.h>

// One fixed-size aggregate per operation; no heap allocation or per-frame logging.
typedef struct {
    uint64_t totalUs;
    uint64_t maxUs;
    unsigned int calls;
    unsigned int failures;
} SS4S_NdlBenchCounter;

typedef enum {
    SS4S_NDL_BENCH_VIDEO_PLAY,
    SS4S_NDL_BENCH_RENDER_BUFFER,
} SS4S_NdlBenchOperation;

typedef struct {
    uint64_t windowStartUs;
    SS4S_NdlBenchCounter videoPlay;
    SS4S_NdlBenchCounter renderBuffer;
} SS4S_NdlBenchStats;

static inline void ss4s_ndl_bench_reset(SS4S_NdlBenchStats *stats) {
    *stats = (SS4S_NdlBenchStats){0};
}

static inline void ss4s_ndl_bench_record(SS4S_NdlBenchStats *stats,
                                         SS4S_NdlBenchOperation operation,
                                         uint64_t nowUs, uint64_t durationUs,
                                         bool failed) {
    if (stats->videoPlay.calls == 0 && stats->renderBuffer.calls == 0) {
        stats->windowStartUs = nowUs;
    }
    SS4S_NdlBenchCounter *counter = operation == SS4S_NDL_BENCH_VIDEO_PLAY
                                  ? &stats->videoPlay : &stats->renderBuffer;
    counter->calls++;
    counter->failures += failed;
    counter->totalUs += durationUs;
    if (durationUs > counter->maxUs) {
        counter->maxUs = durationUs;
    }
}

static inline bool ss4s_ndl_bench_ready(const SS4S_NdlBenchStats *stats,
                                         uint64_t nowUs) {
    return (stats->videoPlay.calls > 0 || stats->renderBuffer.calls > 0) &&
           nowUs >= stats->windowStartUs &&
           nowUs - stats->windowStartUs >= UINT64_C(1000000);
}
