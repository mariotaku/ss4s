#include "../modules/webos/ndl/webos5/ndl_bench_stats.h"

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

int main(void) {
    SS4S_NdlBenchStats stats = {0};
    CHECK(!ss4s_ndl_bench_ready(&stats, 1000000));

    // Both operations share a window but retain independent counters.
    ss4s_ndl_bench_record(&stats, SS4S_NDL_BENCH_VIDEO_PLAY, 1000000, 200, false);
    ss4s_ndl_bench_record(&stats, SS4S_NDL_BENCH_RENDER_BUFFER, 1100000, 500, true);
    ss4s_ndl_bench_record(&stats, SS4S_NDL_BENCH_VIDEO_PLAY, 1500000, 300, true);
    CHECK(stats.windowStartUs == 1000000);
    CHECK(stats.videoPlay.calls == 2);
    CHECK(stats.videoPlay.failures == 1);
    CHECK(stats.videoPlay.totalUs == 500);
    CHECK(stats.videoPlay.maxUs == 300);
    CHECK(stats.renderBuffer.calls == 1);
    CHECK(stats.renderBuffer.failures == 1);
    CHECK(stats.renderBuffer.totalUs == 500);
    CHECK(stats.renderBuffer.maxUs == 500);
    CHECK(!ss4s_ndl_bench_ready(&stats, 1999999));
    CHECK(ss4s_ndl_bench_ready(&stats, 2000000));
    CHECK(!ss4s_ndl_bench_ready(&stats, 500000)); // backwards clock

    ss4s_ndl_bench_reset(&stats);
    CHECK(stats.windowStartUs == 0);
    CHECK(stats.videoPlay.failures == 0 && stats.renderBuffer.failures == 0);
    CHECK(stats.videoPlay.totalUs == 0 && stats.renderBuffer.totalUs == 0);
    CHECK(stats.videoPlay.maxUs == 0 && stats.renderBuffer.maxUs == 0);
    CHECK(!ss4s_ndl_bench_ready(&stats, 3000000));
    CHECK(stats.videoPlay.calls == 0 && stats.renderBuffer.calls == 0);

    // A window is still reportable when the render-buffer query is skipped.
    ss4s_ndl_bench_record(&stats, SS4S_NDL_BENCH_VIDEO_PLAY, 3100000, 400, false);
    CHECK(stats.renderBuffer.calls == 0);
    CHECK(ss4s_ndl_bench_ready(&stats, 4100000));
    ss4s_ndl_bench_reset(&stats);

    // Failed video submission is recorded even if the query never occurs.
    ss4s_ndl_bench_record(&stats, SS4S_NDL_BENCH_VIDEO_PLAY, 5000000, 100, true);
    CHECK(stats.videoPlay.failures == 1);
    CHECK(stats.renderBuffer.calls == 0);
    return 0;
}
