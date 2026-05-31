#include "feed_guard.h"

#include <assert.h>
#include <stddef.h>

void SS4S_FeedGuardInit(SS4S_FeedGuard *g) {
    g->mutex = SS4S_MutexCreate();
    g->drained = SS4S_CondCreate();
    g->instance = NULL;
    g->in_flight = 0;
    g->exclusive = false;
}

void SS4S_FeedGuardDeinit(SS4S_FeedGuard *g) {
    assert(g->instance == NULL);
    assert(g->in_flight == 0);
    assert(!g->exclusive);
    SS4S_CondDestroy(g->drained);
    SS4S_MutexDestroy(g->mutex);
}

bool SS4S_FeedGuardOpen(SS4S_FeedGuard *g, void *instance) {
    assert(instance != NULL);
    SS4S_MutexLockEx(g->mutex, NULL);
    if (g->instance != NULL) {
        SS4S_MutexUnlockEx(g->mutex, NULL);
        return false;
    }
    g->instance = instance;
    SS4S_MutexUnlockEx(g->mutex, NULL);
    return true;
}

void *SS4S_FeedGuardAcquire(SS4S_FeedGuard *g) {
    SS4S_MutexLockEx(g->mutex, NULL);
    void *inst = NULL;
    if (!g->exclusive && g->instance != NULL) {
        inst = g->instance;
        g->in_flight++;
    }
    SS4S_MutexUnlockEx(g->mutex, NULL);
    return inst;
}

void SS4S_FeedGuardRelease(SS4S_FeedGuard *g) {
    SS4S_MutexLockEx(g->mutex, NULL);
    assert(g->in_flight > 0);
    if (--g->in_flight == 0) {
        SS4S_CondBroadcast(g->drained);
    }
    SS4S_MutexUnlockEx(g->mutex, NULL);
}

void *SS4S_FeedGuardClose(SS4S_FeedGuard *g) {
    SS4S_MutexLockEx(g->mutex, NULL);
    /* Serialize with any in-progress exclusive op so we don't yank the
     * instance out from under it. */
    while (g->exclusive) {
        SS4S_CondWait(g->drained, g->mutex);
    }
    void *inst = g->instance;
    if (inst == NULL) {
        SS4S_MutexUnlockEx(g->mutex, NULL);
        return NULL;
    }
    g->instance = NULL;
    while (g->in_flight > 0) {
        SS4S_CondWait(g->drained, g->mutex);
    }
    SS4S_MutexUnlockEx(g->mutex, NULL);
    return inst;
}

void *SS4S_FeedGuardBeginExclusive(SS4S_FeedGuard *g) {
    SS4S_MutexLockEx(g->mutex, NULL);
    /* Only one exclusive op at a time. */
    while (g->exclusive) {
        SS4S_CondWait(g->drained, g->mutex);
    }
    if (g->instance == NULL) {
        SS4S_MutexUnlockEx(g->mutex, NULL);
        return NULL;
    }
    g->exclusive = true;
    /* Wait for any in-flight Feeds (started before we set exclusive)
     * to finish. New Acquires from here on return NULL because
     * exclusive is now true. */
    while (g->in_flight > 0) {
        SS4S_CondWait(g->drained, g->mutex);
    }
    void *inst = g->instance;
    SS4S_MutexUnlockEx(g->mutex, NULL);
    return inst;
}

void SS4S_FeedGuardEndExclusive(SS4S_FeedGuard *g) {
    SS4S_MutexLockEx(g->mutex, NULL);
    assert(g->exclusive);
    g->exclusive = false;
    SS4S_CondBroadcast(g->drained);
    SS4S_MutexUnlockEx(g->mutex, NULL);
}
