# Optional NDL render-buffer API profiler

Configure ss4s with `-DSS4S_NDL_BENCH_STATS=ON` when building
Moonlight TV against this revision. With this option disabled (default),
the NDL hot path remains unchanged.

In the webOS5 backend, each successful video Feed ordinarily calls
`NDL_DirectVideoGetRenderBufferLength()`. This build records each call's
wall-clock duration (monotonic clock), reporting approximately once per
second as `NDL_BENCH_V1` log entries. The entries contain window duration,
number of calls, failures, sum of call times, and largest call duration.
Compute mean query latency as `total_us / calls`; this is **not p95**.

Use the exact same profiler in both unthrottled and throttled builds.
Compare total query time/s and call rate as well as overall received FPS and
SS4S decoder latency. The added clock reads themselves incur some overhead.
Do not infer a streaming FPS improvement from fewer API calls alone.

No device access or automated streaming is added. A real webOS5 device is
required to measure NDL, and TV usage requires operator authorization.
