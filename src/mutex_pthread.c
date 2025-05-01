#include "mutex.h"
#include "lib_logging.h"

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>

struct SS4S_Mutex {
    pthread_mutex_t inner;
};

SS4S_Mutex *SS4S_MutexCreate() {
    SS4S_Mutex *mutex = malloc(sizeof(SS4S_Mutex));
    assert(mutex != NULL);
    pthread_mutex_init(&mutex->inner, NULL);
    return mutex;
}

void SS4S_MutexLockEx(SS4S_Mutex *mutex, const char *caller) {
    assert(mutex != NULL);
    if (caller != NULL) {
        SS4S_Log(SS4S_LogLevelVerbose, "Mutex", "Locking mutex %p from %s", mutex, caller);
    }
    pthread_mutex_lock(&mutex->inner);
}

void SS4S_MutexUnlockEx(SS4S_Mutex *mutex, const char *caller) {
    assert(mutex != NULL);
    if (caller != NULL) {
        SS4S_Log(SS4S_LogLevelVerbose, "Mutex", "Unlocking mutex %p from %s", mutex, caller);
    }
    pthread_mutex_unlock(&mutex->inner);
}

void SS4S_MutexDestroy(SS4S_Mutex *mutex) {
    assert(mutex != NULL);
    pthread_mutex_destroy(&mutex->inner);
    free(mutex);
}
