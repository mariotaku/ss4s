/* Unit tests for SS4S_FeedGuard.
 *
 * The guard is the new primitive that lets Feed() run without holding
 * a mutex, while ensuring Close() does not free the instance until
 * every in-flight Feed has finished. These tests cover the basic
 * lifecycle and the drain-on-Close behavior.
 *
 * pthread is used directly rather than SDL since the rest of the
 * ss4s tests are POSIX-only already (test_pcm_playback uses
 * unistd.h, etc.). */

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "feed_guard.h"

static int instance_a = 1;
static int instance_b = 2;

static void test_open_close_basic(void) {
    SS4S_FeedGuard g;
    SS4S_FeedGuardInit(&g);
    assert(SS4S_FeedGuardOpen(&g, &instance_a) == true);
    void *closed = SS4S_FeedGuardClose(&g);
    assert(closed == &instance_a);
    SS4S_FeedGuardDeinit(&g);
}

static void test_acquire_release_no_close(void) {
    SS4S_FeedGuard g;
    SS4S_FeedGuardInit(&g);
    assert(SS4S_FeedGuardOpen(&g, &instance_a) == true);
    void *got = SS4S_FeedGuardAcquire(&g);
    assert(got == &instance_a);
    SS4S_FeedGuardRelease(&g);
    void *closed = SS4S_FeedGuardClose(&g);
    assert(closed == &instance_a);
    SS4S_FeedGuardDeinit(&g);
}

static void test_double_open_rejected(void) {
    SS4S_FeedGuard g;
    SS4S_FeedGuardInit(&g);
    assert(SS4S_FeedGuardOpen(&g, &instance_a) == true);
    assert(SS4S_FeedGuardOpen(&g, &instance_b) == false);
    void *closed = SS4S_FeedGuardClose(&g);
    assert(closed == &instance_a);
    SS4S_FeedGuardDeinit(&g);
}

static void test_close_without_open(void) {
    SS4S_FeedGuard g;
    SS4S_FeedGuardInit(&g);
    assert(SS4S_FeedGuardClose(&g) == NULL);
    SS4S_FeedGuardDeinit(&g);
}

static void test_acquire_after_close_returns_null(void) {
    SS4S_FeedGuard g;
    SS4S_FeedGuardInit(&g);
    assert(SS4S_FeedGuardOpen(&g, &instance_a) == true);
    void *closed = SS4S_FeedGuardClose(&g);
    assert(closed == &instance_a);
    assert(SS4S_FeedGuardAcquire(&g) == NULL);
    SS4S_FeedGuardDeinit(&g);
}

static void test_reopen_after_close(void) {
    SS4S_FeedGuard g;
    SS4S_FeedGuardInit(&g);
    assert(SS4S_FeedGuardOpen(&g, &instance_a) == true);
    void *closed = SS4S_FeedGuardClose(&g);
    assert(closed == &instance_a);
    assert(SS4S_FeedGuardOpen(&g, &instance_b) == true);
    void *got = SS4S_FeedGuardAcquire(&g);
    assert(got == &instance_b);
    SS4S_FeedGuardRelease(&g);
    closed = SS4S_FeedGuardClose(&g);
    assert(closed == &instance_b);
    SS4S_FeedGuardDeinit(&g);
}

/* ---------- Drain semantics under contention ---------- */

typedef struct {
    SS4S_FeedGuard *guard;
    int hold_us;           /* how long to keep the reference */
    pthread_mutex_t *seq_mu;
    pthread_cond_t *seq_cv;
    bool *acquired;
    bool *should_release;
} feeder_ctx_t;

static void *feeder_thread(void *arg) {
    feeder_ctx_t *ctx = arg;
    void *inst = SS4S_FeedGuardAcquire(ctx->guard);
    if (inst == NULL) {
        return NULL;
    }
    /* Signal that we hold the reference; sleep; release. */
    pthread_mutex_lock(ctx->seq_mu);
    *ctx->acquired = true;
    pthread_cond_broadcast(ctx->seq_cv);
    while (!*ctx->should_release) {
        pthread_cond_wait(ctx->seq_cv, ctx->seq_mu);
    }
    pthread_mutex_unlock(ctx->seq_mu);
    SS4S_FeedGuardRelease(ctx->guard);
    return inst;
}

typedef struct {
    SS4S_FeedGuard *guard;
    void *result;
    bool finished;
    pthread_mutex_t *done_mu;
    pthread_cond_t *done_cv;
    bool *done;
} closer_ctx_t;

static void *closer_thread(void *arg) {
    closer_ctx_t *ctx = arg;
    ctx->result = SS4S_FeedGuardClose(ctx->guard);
    pthread_mutex_lock(ctx->done_mu);
    *ctx->done = true;
    pthread_cond_broadcast(ctx->done_cv);
    pthread_mutex_unlock(ctx->done_mu);
    return NULL;
}

static int64_t now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t) ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static void test_close_blocks_until_release(void) {
    SS4S_FeedGuard g;
    SS4S_FeedGuardInit(&g);
    assert(SS4S_FeedGuardOpen(&g, &instance_a) == true);

    pthread_mutex_t seq_mu = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t seq_cv = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t done_mu = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t done_cv = PTHREAD_COND_INITIALIZER;
    bool feeder_acquired = false, feeder_should_release = false, closer_done = false;

    feeder_ctx_t feeder_ctx = {
        .guard = &g,
        .seq_mu = &seq_mu,
        .seq_cv = &seq_cv,
        .acquired = &feeder_acquired,
        .should_release = &feeder_should_release,
    };
    closer_ctx_t closer_ctx = {
        .guard = &g,
        .done_mu = &done_mu,
        .done_cv = &done_cv,
        .done = &closer_done,
    };

    pthread_t feeder, closer;
    pthread_create(&feeder, NULL, feeder_thread, &feeder_ctx);

    /* Wait until feeder has acquired the reference. */
    pthread_mutex_lock(&seq_mu);
    while (!feeder_acquired) {
        pthread_cond_wait(&seq_cv, &seq_mu);
    }
    pthread_mutex_unlock(&seq_mu);

    /* Spawn the closer; it should block on the in-flight reference. */
    int64_t close_start = now_us();
    pthread_create(&closer, NULL, closer_thread, &closer_ctx);

    /* Sleep ~50ms to give the closer a chance to be observed blocked. */
    usleep(50 * 1000);
    pthread_mutex_lock(&done_mu);
    bool done_during_block = closer_done;
    pthread_mutex_unlock(&done_mu);
    assert(done_during_block == false && "Close returned while a Feed was still in flight");

    /* Now let the feeder release its reference. */
    pthread_mutex_lock(&seq_mu);
    feeder_should_release = true;
    pthread_cond_broadcast(&seq_cv);
    pthread_mutex_unlock(&seq_mu);

    /* Wait for closer to complete. */
    pthread_mutex_lock(&done_mu);
    while (!closer_done) {
        pthread_cond_wait(&done_cv, &done_mu);
    }
    pthread_mutex_unlock(&done_mu);
    int64_t elapsed = now_us() - close_start;
    assert(elapsed >= 50 * 1000 && "Close returned too fast — it should have waited");

    pthread_join(feeder, NULL);
    pthread_join(closer, NULL);
    assert(closer_ctx.result == &instance_a);

    SS4S_FeedGuardDeinit(&g);
}

static void test_close_waits_for_all_feeders(void) {
    SS4S_FeedGuard g;
    SS4S_FeedGuardInit(&g);
    assert(SS4S_FeedGuardOpen(&g, &instance_a) == true);

    enum { N = 8 };
    pthread_mutex_t seq_mu = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t seq_cv = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t done_mu = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t done_cv = PTHREAD_COND_INITIALIZER;
    bool acquired_flags[N] = {false};
    bool should_release[N] = {false};
    bool closer_done = false;

    feeder_ctx_t feeder_ctxs[N];
    pthread_t feeders[N];
    for (int i = 0; i < N; i++) {
        feeder_ctxs[i].guard = &g;
        feeder_ctxs[i].seq_mu = &seq_mu;
        feeder_ctxs[i].seq_cv = &seq_cv;
        feeder_ctxs[i].acquired = &acquired_flags[i];
        feeder_ctxs[i].should_release = &should_release[i];
        pthread_create(&feeders[i], NULL, feeder_thread, &feeder_ctxs[i]);
    }

    /* Wait until all feeders have acquired. */
    pthread_mutex_lock(&seq_mu);
    for (int i = 0; i < N; i++) {
        while (!acquired_flags[i]) {
            pthread_cond_wait(&seq_cv, &seq_mu);
        }
    }
    pthread_mutex_unlock(&seq_mu);

    closer_ctx_t closer_ctx = {
        .guard = &g,
        .done_mu = &done_mu,
        .done_cv = &done_cv,
        .done = &closer_done,
    };
    pthread_t closer;
    pthread_create(&closer, NULL, closer_thread, &closer_ctx);

    /* Release feeders one at a time. Closer must not finish until the last
     * one releases. */
    for (int i = 0; i < N - 1; i++) {
        pthread_mutex_lock(&seq_mu);
        should_release[i] = true;
        pthread_cond_broadcast(&seq_cv);
        pthread_mutex_unlock(&seq_mu);
        usleep(5 * 1000);
        pthread_mutex_lock(&done_mu);
        bool partial_done = closer_done;
        pthread_mutex_unlock(&done_mu);
        assert(partial_done == false && "Close returned before all feeders released");
    }
    /* Release the last feeder. */
    pthread_mutex_lock(&seq_mu);
    should_release[N - 1] = true;
    pthread_cond_broadcast(&seq_cv);
    pthread_mutex_unlock(&seq_mu);

    pthread_mutex_lock(&done_mu);
    while (!closer_done) {
        pthread_cond_wait(&done_cv, &done_mu);
    }
    pthread_mutex_unlock(&done_mu);

    for (int i = 0; i < N; i++) {
        pthread_join(feeders[i], NULL);
    }
    pthread_join(closer, NULL);
    assert(closer_ctx.result == &instance_a);

    SS4S_FeedGuardDeinit(&g);
}

static void test_acquire_during_close_drain_returns_null(void) {
    /* While Close is draining (in_flight > 0), a fresh Acquire must
     * see the guard as already closed and return NULL — otherwise
     * Close could be defeated by a tight feed-spam loop. */
    SS4S_FeedGuard g;
    SS4S_FeedGuardInit(&g);
    assert(SS4S_FeedGuardOpen(&g, &instance_a) == true);

    pthread_mutex_t seq_mu = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t seq_cv = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t done_mu = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t done_cv = PTHREAD_COND_INITIALIZER;
    bool feeder_acquired = false, feeder_should_release = false, closer_done = false;

    feeder_ctx_t feeder_ctx = {
        .guard = &g,
        .seq_mu = &seq_mu,
        .seq_cv = &seq_cv,
        .acquired = &feeder_acquired,
        .should_release = &feeder_should_release,
    };
    closer_ctx_t closer_ctx = {
        .guard = &g,
        .done_mu = &done_mu,
        .done_cv = &done_cv,
        .done = &closer_done,
    };

    pthread_t feeder, closer;
    pthread_create(&feeder, NULL, feeder_thread, &feeder_ctx);
    pthread_mutex_lock(&seq_mu);
    while (!feeder_acquired) {
        pthread_cond_wait(&seq_cv, &seq_mu);
    }
    pthread_mutex_unlock(&seq_mu);

    pthread_create(&closer, NULL, closer_thread, &closer_ctx);
    usleep(20 * 1000);  /* let closer reach the wait */

    /* Closer is now waiting for the feeder to release. A fresh Acquire
     * must return NULL because the instance pointer has been detached. */
    void *late = SS4S_FeedGuardAcquire(&g);
    assert(late == NULL);

    pthread_mutex_lock(&seq_mu);
    feeder_should_release = true;
    pthread_cond_broadcast(&seq_cv);
    pthread_mutex_unlock(&seq_mu);

    pthread_mutex_lock(&done_mu);
    while (!closer_done) {
        pthread_cond_wait(&done_cv, &done_mu);
    }
    pthread_mutex_unlock(&done_mu);

    pthread_join(feeder, NULL);
    pthread_join(closer, NULL);

    SS4S_FeedGuardDeinit(&g);
}

int main(void) {
    test_open_close_basic();
    printf("test_open_close_basic: OK\n");
    test_acquire_release_no_close();
    printf("test_acquire_release_no_close: OK\n");
    test_double_open_rejected();
    printf("test_double_open_rejected: OK\n");
    test_close_without_open();
    printf("test_close_without_open: OK\n");
    test_acquire_after_close_returns_null();
    printf("test_acquire_after_close_returns_null: OK\n");
    test_reopen_after_close();
    printf("test_reopen_after_close: OK\n");
    test_close_blocks_until_release();
    printf("test_close_blocks_until_release: OK\n");
    test_close_waits_for_all_feeders();
    printf("test_close_waits_for_all_feeders: OK\n");
    test_acquire_during_close_drain_returns_null();
    printf("test_acquire_during_close_drain_returns_null: OK\n");
    return 0;
}
