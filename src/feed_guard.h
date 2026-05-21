#pragma once

#include "mutex.h"
#include <stdbool.h>

/* SS4S_FeedGuard coordinates the lifecycle of an audio/video instance
 * against in-flight driver->Feed() calls. The driver's Feed routine
 * can block for arbitrary durations, so we cannot hold a mutex across
 * it — but we still need to guarantee that Close() does not free the
 * instance while a Feed() is running.
 *
 * Pattern:
 *   Open():   SS4S_FeedGuardOpen(g, instance)
 *   Feed():   inst = SS4S_FeedGuardAcquire(g);  // NULL if closed
 *             if (inst) { driver->Feed(inst); SS4S_FeedGuardRelease(g); }
 *   Close():  inst = SS4S_FeedGuardClose(g);    // blocks until Feeds drain
 *             if (inst) driver->Close(inst);
 *
 * Acquire/Release does not hold the mutex during the caller's Feed
 * work; the mutex is taken only briefly to bump/drop the in-flight
 * counter. Close swaps the instance pointer to NULL first (so any
 * Acquire that races after the swap returns NULL immediately) and
 * then waits on a condvar until all already-acquired refs are
 * released.
 *
 * Type-erased on void* so the same helper covers audio and video
 * without templates. Callers cast back to their concrete instance
 * type. */
typedef struct SS4S_FeedGuard {
    SS4S_Mutex *mutex;
    SS4S_Cond *drained;
    void *instance;
    int in_flight;
} SS4S_FeedGuard;

void SS4S_FeedGuardInit(SS4S_FeedGuard *g);

void SS4S_FeedGuardDeinit(SS4S_FeedGuard *g);

/* Register a freshly-opened instance. Returns false if the guard was
 * already open (caller error: must Close before reopening). */
bool SS4S_FeedGuardOpen(SS4S_FeedGuard *g, void *instance);

/* Try to take a reference to the live instance. Returns the instance
 * pointer (guaranteed valid until Release) or NULL if the guard is
 * closed. Every successful Acquire must be paired with a Release. */
void *SS4S_FeedGuardAcquire(SS4S_FeedGuard *g);

/* Release the reference taken by Acquire. */
void SS4S_FeedGuardRelease(SS4S_FeedGuard *g);

/* Atomically swap the instance to NULL (new Acquires now see NULL),
 * wait for all in-flight refs to be Released, and return the
 * detached instance. Returns NULL if the guard was not open.
 * Caller is responsible for calling driver->Close on the result. */
void *SS4S_FeedGuardClose(SS4S_FeedGuard *g);
