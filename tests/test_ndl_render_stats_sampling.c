#include "../modules/webos/ndl/webos5/render_stats_sampling.h"

#include <assert.h>
#include <stdint.h>

int main(void) {
    const uint64_t first = UINT64_C(1000000);
    assert(ndl_should_sample_render_stats(first, 0));
    assert(!ndl_should_sample_render_stats(first + 1, first));
    assert(!ndl_should_sample_render_stats(first + 99999, first));
    assert(ndl_should_sample_render_stats(first + 100000, first));
    assert(ndl_should_sample_render_stats(first + 200000, first));
    assert(ndl_should_sample_render_stats(1, first)); // monotonic clock anomaly
    assert(ndl_should_sample_render_stats(first, 0)); // new session or media reload
    return 0;
}
