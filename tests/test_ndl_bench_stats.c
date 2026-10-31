#include "../modules/webos/ndl/webos5/ndl_bench_stats.h"

#include <assert.h>

int main(void) {
    SS4S_NdlBenchStats stats = {0};
    assert(!ss4s_ndl_bench_ready(&stats, 1000000));
    ss4s_ndl_bench_record(&stats, 1000000, 200, false);
    ss4s_ndl_bench_record(&stats, 1100000, 500, true);
    assert(stats.calls == 2);
    assert(stats.failures == 1);
    assert(stats.totalUs == 700);
    assert(stats.maxUs == 500);
    assert(!ss4s_ndl_bench_ready(&stats, 1500000));
    assert(ss4s_ndl_bench_ready(&stats, 2000000));
    assert(!ss4s_ndl_bench_ready(&stats, 500000)); // backwards clock
    ss4s_ndl_bench_reset(&stats);
    assert(stats.calls == 0);
    assert(stats.totalUs == 0);
    return 0;
}
