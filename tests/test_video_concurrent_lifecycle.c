/* Stress test for the FeedGuard exclusive contract via the dummy module.
 *
 * Verifies end-to-end: ss4s wrapper (PlayerVideoFeed / SetHDRInfo /
 * SizeChanged) + FeedGuard + driver (dummy). The dummy is enhanced
 * to abort() if Feed ever reaches the driver during a SetHDRInfo /
 * SizeChanged / Close window. With FeedGuard's BeginExclusive
 * primitive in place this never happens; without it (or with a
 * broken implementation) the test aborts loudly within a few
 * iterations.
 *
 * The dummy's Open/Close each include a 500ms LoadMedia sleep, so
 * we hold off on a Close-during-feed scenario (would make the test
 * slow). The destructive ops (SizeChanged/SetHDRInfo) have a 5ms
 * sleep window — wide enough to expose races on multi-core but
 * short enough to keep the test under ~2s.
 */

#include "ss4s.h"
#include "test_common.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define FEEDERS 8
#define TEST_DURATION_SECS 1

static SS4S_Player *g_player;
static atomic_bool g_stop;
static atomic_int g_feeds_total;
static atomic_int g_feeds_not_ready;
static atomic_int g_toggles_total;
static atomic_int g_resizes_total;

static int64_t now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t) ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static void *feeder_thread(void *arg) {
    (void) arg;
    unsigned char dummy_buf[1024] = {0};
    while (!atomic_load(&g_stop)) {
        SS4S_VideoFeedResult r = SS4S_PlayerVideoFeed(g_player, dummy_buf, sizeof(dummy_buf),
                                                      SS4S_VIDEO_FEED_DATA_FRAME_START |
                                                      SS4S_VIDEO_FEED_DATA_FRAME_END);
        atomic_fetch_add(&g_feeds_total, 1);
        if (r == SS4S_VIDEO_FEED_NOT_READY) {
            atomic_fetch_add(&g_feeds_not_ready, 1);
        }
        /* Don't sched_yield — we want maximum contention. */
    }
    return NULL;
}

static void *hdr_toggle_thread(void *arg) {
    (void) arg;
    SS4S_VideoHDRInfo info = {
            .displayPrimariesX = {34000, 13250, 7500},
            .displayPrimariesY = {16000, 34500, 3000},
            .whitePointX = 15635,
            .whitePointY = 16450,
            .maxDisplayMasteringLuminance = 1000,
            .minDisplayMasteringLuminance = 50,
            .maxContentLightLevel = 1000,
            .maxPicAverageLightLevel = 400,
    };
    bool enable = true;
    while (!atomic_load(&g_stop)) {
        SS4S_PlayerVideoSetHDRInfo(g_player, enable ? &info : NULL);
        atomic_fetch_add(&g_toggles_total, 1);
        enable = !enable;
        usleep(1000);
    }
    return NULL;
}

static void *size_change_thread(void *arg) {
    (void) arg;
    int sizes[][2] = {{1920, 1080}, {3840, 2160}, {2560, 1440}, {1280, 720}};
    int idx = 0;
    while (!atomic_load(&g_stop)) {
        SS4S_PlayerVideoSizeChanged(g_player, sizes[idx][0], sizes[idx][1]);
        atomic_fetch_add(&g_resizes_total, 1);
        idx = (idx + 1) % (sizeof(sizes) / sizeof(sizes[0]));
        usleep(1500);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    char driver[16] = {'\0'};
    single_test_infer_module(driver, sizeof(driver), "ss4s_test_video_concurrent_lifecycle_",
                             argc, argv);
    printf("Request video driver: %s\n", driver);
    if (!SS4S_ModuleAvailable(driver, SS4S_MODULE_CHECK_VIDEO)) {
        printf("Skipping unsupported video driver: %s\n", driver);
        return 127;
    }

    SS4S_Config config = {.videoDriver = driver};
    SS4S_Init(argc, argv, &config);
    SS4S_PostInit(argc, argv);
    g_player = SS4S_PlayerOpen();
    assert(g_player != NULL);

    SS4S_VideoInfo info = {
            .codec = SS4S_VIDEO_H264,
            .width = 1920,
            .height = 1080,
            .frameRateNumerator = 60,
            .frameRateDenominator = 1,
    };
    SS4S_VideoOpenResult open_result = SS4S_PlayerVideoOpen(g_player, &info);
    assert(open_result == SS4S_VIDEO_OPEN_OK);

    atomic_store(&g_stop, false);
    atomic_store(&g_feeds_total, 0);
    atomic_store(&g_feeds_not_ready, 0);
    atomic_store(&g_toggles_total, 0);
    atomic_store(&g_resizes_total, 0);

    pthread_t feeders[FEEDERS];
    pthread_t hdr_thread, size_thread;
    for (int i = 0; i < FEEDERS; i++) {
        int rc = pthread_create(&feeders[i], NULL, feeder_thread, NULL);
        assert(rc == 0);
    }
    pthread_create(&hdr_thread, NULL, hdr_toggle_thread, NULL);
    pthread_create(&size_thread, NULL, size_change_thread, NULL);

    int64_t start = now_us();
    usleep(TEST_DURATION_SECS * 1000 * 1000);
    atomic_store(&g_stop, true);

    for (int i = 0; i < FEEDERS; i++) {
        pthread_join(feeders[i], NULL);
    }
    pthread_join(hdr_thread, NULL);
    pthread_join(size_thread, NULL);
    int64_t elapsed_us = now_us() - start;

    SS4S_PlayerVideoClose(g_player);
    SS4S_PlayerClose(g_player);
    SS4S_Quit();

    int feeds = atomic_load(&g_feeds_total);
    int not_ready = atomic_load(&g_feeds_not_ready);
    int toggles = atomic_load(&g_toggles_total);
    int resizes = atomic_load(&g_resizes_total);
    printf("ran %.2fs: %d feeds (%d returned NOT_READY), %d HDR toggles, %d size changes\n",
           elapsed_us / 1e6, feeds, not_ready, toggles, resizes);

    /* If FeedGuard correctly drained, the dummy would have aborted us
     * already if anything sneaked through. Assert we actually exercised
     * the race conditions we care about. */
    assert(feeds > 0);
    assert(toggles > 0);
    assert(resizes > 0);
    /* Some NOT_READY returns are expected — that's the FeedGuard
     * marking the exclusive window. If we got ZERO, it means the
     * exclusive ops never overlapped with Feeds and the test didn't
     * actually exercise the race. */
    if (not_ready == 0) {
        printf("WARNING: no NOT_READY returns observed — race window may not have triggered\n");
    } else {
        printf("OK: %d Feed calls hit the exclusive window and were correctly deflected\n",
               not_ready);
    }
    return 0;
}
