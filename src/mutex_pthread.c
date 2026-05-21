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

struct SS4S_Cond {
    pthread_cond_t inner;
};

SS4S_Cond *SS4S_CondCreate() {
    SS4S_Cond *cond = malloc(sizeof(SS4S_Cond));
    assert(cond != NULL);
    pthread_cond_init(&cond->inner, NULL);
    return cond;
}

void SS4S_CondWait(SS4S_Cond *cond, SS4S_Mutex *mutex) {
    assert(cond != NULL);
    assert(mutex != NULL);
    pthread_cond_wait(&cond->inner, &mutex->inner);
}

void SS4S_CondBroadcast(SS4S_Cond *cond) {
    assert(cond != NULL);
    pthread_cond_broadcast(&cond->inner);
}

void SS4S_CondDestroy(SS4S_Cond *cond) {
    assert(cond != NULL);
    pthread_cond_destroy(&cond->inner);
    free(cond);
}
