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
