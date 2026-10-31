#pragma once

#include <stdbool.h>
#include <stdint.h>

// Allocations and per-frame logging are avoided in the decoder hot path.
typedef struct {
    uint64_t windowStartUs;
    uint64_t totalUs;
    uint64_t maxUs;
    unsigned int calls;
    unsigned int failures;
} SS4S_NdlBenchStats;

static inline void ss4s_ndl_bench_reset(SS4S_NdlBenchStats *stats) {
    *stats = (SS4S_NdlBenchStats){0};
}

static inline void ss4s_ndl_bench_record(SS4S_NdlBenchStats *stats, uint64_t nowUs,
                                          uint64_t durationUs, bool failed) {
    if (stats->calls == 0) {
        stats->windowStartUs = nowUs;
    }
    stats->calls++;
    stats->failures += failed;
    stats->totalUs += durationUs;
    if (durationUs > stats->maxUs) {
        stats->maxUs = durationUs;
    }
}

static inline bool ss4s_ndl_bench_ready(const SS4S_NdlBenchStats *stats, uint64_t nowUs) {
    return stats->calls > 0 && nowUs >= stats->windowStartUs &&
           nowUs - stats->windowStartUs >= UINT64_C(1000000);
}
