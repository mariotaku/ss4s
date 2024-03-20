#include "mutex.h"

#include <pthread.h>
#include <stdlib.h>

struct SS4S_Mutex {
    pthread_mutex_t inner;
};

SS4S_Mutex *SS4S_MutexCreate() {
    SS4S_Mutex *mutex = malloc(sizeof(SS4S_Mutex));
    pthread_mutex_init(&mutex->inner, NULL);
    return mutex;
}

void SS4S_MutexLock(SS4S_Mutex *mutex) {
    pthread_mutex_lock(&mutex->inner);
}

void SS4S_MutexUnlock(SS4S_Mutex *mutex) {
    pthread_mutex_unlock(&mutex->inner);
}

void SS4S_MutexDestroy(SS4S_Mutex *mutex) {
    pthread_mutex_destroy(&mutex->inner);
    free(mutex);
}
