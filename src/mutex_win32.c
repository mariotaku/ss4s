#include "mutex.h"
#include "lib_logging.h"

#include <synchapi.h>
#include <windows.h>
#include <assert.h>
#include <stdlib.h>

struct SS4S_Mutex {
    HANDLE inner;
};

SS4S_Mutex *SS4S_MutexCreate() {
    SS4S_Mutex *mutex = malloc(sizeof(SS4S_Mutex));
    assert(mutex != NULL);
    mutex->inner = CreateMutex(NULL, FALSE, NULL);
    return mutex;
}

void SS4S_MutexLockEx(SS4S_Mutex *mutex, const char *caller) {
    assert(mutex != NULL);
    if (caller != NULL) {
        SS4S_Log(SS4S_LogLevelDebug, "Mutex", "Locking mutex %p from %s", mutex, caller);
    }
    WaitForSingleObject(mutex->inner, INFINITE);
}

void SS4S_MutexUnlockEx(SS4S_Mutex *mutex, const char *caller) {
    assert(mutex != NULL);
    if (caller != NULL) {
        SS4S_Log(SS4S_LogLevelDebug, "Mutex", "Unlocking mutex %p from %s", mutex, caller);
    }
    ReleaseMutex(mutex->inner);
}

void SS4S_MutexDestroy(SS4S_Mutex *mutex) {
    assert(mutex != NULL);
    CloseHandle(mutex->inner);
    free(mutex);
}

/* Win32 has CRITICAL_SECTION + CONDITION_VARIABLE for proper condvar support,
 * but SS4S_Mutex wraps HANDLE (which can be waited on cross-process). For
 * compatibility, the condvar uses a manual-reset event broadcast pattern:
 * Wait releases the mutex, waits on the event, re-acquires; Broadcast pulses
 * the event so all waiters wake up. */
struct SS4S_Cond {
    HANDLE event;
};

SS4S_Cond *SS4S_CondCreate() {
    SS4S_Cond *cond = malloc(sizeof(SS4S_Cond));
    assert(cond != NULL);
    cond->event = CreateEvent(NULL, TRUE, FALSE, NULL);
    return cond;
}

void SS4S_CondWait(SS4S_Cond *cond, SS4S_Mutex *mutex) {
    assert(cond != NULL);
    assert(mutex != NULL);
    ResetEvent(cond->event);
    ReleaseMutex(mutex->inner);
    WaitForSingleObject(cond->event, INFINITE);
    WaitForSingleObject(mutex->inner, INFINITE);
}

void SS4S_CondBroadcast(SS4S_Cond *cond) {
    assert(cond != NULL);
    SetEvent(cond->event);
}

void SS4S_CondDestroy(SS4S_Cond *cond) {
    assert(cond != NULL);
    CloseHandle(cond->event);
    free(cond);
}
