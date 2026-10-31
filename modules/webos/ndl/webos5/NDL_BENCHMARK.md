# Optional NDL video API profiler

Configure ss4s with `-DSS4S_NDL_BENCH_STATS=ON` when building Moonlight TV
against this revision. With the option disabled (default), the video hot path
uses the existing uninstrumented NDL calls.

The webOS5 backend measures two separate, synchronous NDL operations:

- `NDL_DirectVideoPlay()`: the wall-clock time to submit a compressed frame to
  the NDL API, **not** its eventual decoder/panel presentation time. Failed
  submissions are included.
- `NDL_DirectVideoGetRenderBufferLength()`: the wall-clock time spent querying
  render-buffer occupancy. This query normally happens on each successful
  Feed after the initial frame, and is skipped on a failed submission.

One `NDL_BENCH_V2` log line per approximately one-second window reports
`duration_us`, `play_calls`, `play_failures`, `play_total_us`,
`play_max_us`, `query_calls`, `query_failures`, `query_total_us`,
and `query_max_us`. Mean duration is `*_total_us / *_calls`; the reported
maximum is **not p95**. Samples use the monotonic clock, fixed-size
counters, no allocations, and no per-frame logging.

To evaluate [ss4s #22](https://github.com/mariotaku/ss4s/pull/22), apply the
**same** profiler to both unthrottled and 10 Hz versions. Compare NDL call
rate and total query time/s, video submission time, and client FPS/latency.
Identical instrumentation and session settings matter: the extra clock
reads have measurable overhead themselves. The combined profiler deliberately
does not claim displayed FPS or decode completion.

Unit test:
`ss4s_add_test(ndl_bench_stats ...)` exercises separate counters, failures,
window boundaries, reset, skipped queries, and clock rollback.

No device access or automatic benchmarking is added. Actual NDL timing
requires a physical webOS5 TV and permission to use it.
