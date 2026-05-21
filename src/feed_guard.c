#include "feed_guard.h"

#include <assert.h>
#include <stddef.h>

void SS4S_FeedGuardInit(SS4S_FeedGuard *g) {
    g->mutex = SS4S_MutexCreate();
    g->drained = SS4S_CondCreate();
    g->instance = NULL;
    g->in_flight = 0;
}

void SS4S_FeedGuardDeinit(SS4S_FeedGuard *g) {
    assert(g->instance == NULL);
    assert(g->in_flight == 0);
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
    void *inst = g->instance;
    if (inst != NULL) {
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
