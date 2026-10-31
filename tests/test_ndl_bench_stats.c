#include "../modules/webos/ndl/webos5/ndl_bench_stats.h"

#include <assert.h>

int main(void) {
    SS4S_NdlBenchStats stats = {0};
    assert(!ss4s_ndl_bench_ready(&stats, 1000000));

    // Both operations share a window but retain independent counters.
    ss4s_ndl_bench_record(&stats, SS4S_NDL_BENCH_VIDEO_PLAY, 1000000, 200, false);
    ss4s_ndl_bench_record(&stats, SS4S_NDL_BENCH_RENDER_BUFFER, 1100000, 500, true);
    ss4s_ndl_bench_record(&stats, SS4S_NDL_BENCH_VIDEO_PLAY, 1500000, 300, true);
    assert(stats.windowStartUs == 1000000);
    assert(stats.videoPlay.calls == 2);
    assert(stats.videoPlay.failures == 1);
    assert(stats.videoPlay.totalUs == 500);
    assert(stats.videoPlay.maxUs == 300);
    assert(stats.renderBuffer.calls == 1);
    assert(stats.renderBuffer.failures == 1);
    assert(stats.renderBuffer.totalUs == 500);
    assert(stats.renderBuffer.maxUs == 500);
    assert(!ss4s_ndl_bench_ready(&stats, 1999999));
    assert(ss4s_ndl_bench_ready(&stats, 2000000));
    assert(!ss4s_ndl_bench_ready(&stats, 500000)); // backwards clock

    ss4s_ndl_bench_reset(&stats);
    assert(!ss4s_ndl_bench_ready(&stats, 3000000));
    assert(stats.videoPlay.calls == 0 && stats.renderBuffer.calls == 0);

    // A window is still reportable when the render-buffer query is skipped.
    ss4s_ndl_bench_record(&stats, SS4S_NDL_BENCH_VIDEO_PLAY, 3100000, 400, false);
    assert(stats.renderBuffer.calls == 0);
    assert(ss4s_ndl_bench_ready(&stats, 4100000));
    ss4s_ndl_bench_reset(&stats);

    // Failed video submission is recorded even if the query never occurs.
    ss4s_ndl_bench_record(&stats, SS4S_NDL_BENCH_VIDEO_PLAY, 5000000, 100, true);
    assert(stats.videoPlay.failures == 1);
    assert(stats.renderBuffer.calls == 0);
    return 0;
}
