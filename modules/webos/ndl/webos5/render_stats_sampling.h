#pragma once

#include <stdbool.h>
#include <stdint.h>

// The stats overlay refreshes far less frequently than the decoded frame rate.
#define NDL_RENDER_STATS_SAMPLE_INTERVAL_US UINT64_C(100000)

static inline bool ndl_should_sample_render_stats(uint64_t now_us, uint64_t last_sample_us) {
    if (last_sample_us == 0 || now_us < last_sample_us) {
        return true;
    }
    return now_us - last_sample_us >= NDL_RENDER_STATS_SAMPLE_INTERVAL_US;
}
