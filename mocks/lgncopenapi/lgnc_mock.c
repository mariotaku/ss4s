#include "lgnc_mock.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static pthread_mutex_t pipeline_lock = PTHREAD_MUTEX_INITIALIZER;

void mock_pipeline_lock(const char *func) {
    if (pthread_mutex_trylock(&pipeline_lock) != 0) {
        fprintf(stderr, "[LGNC] mock_pipeline_lock failed on %s\n", func);
        abort();
    }
}

void mock_pipeline_unlock(const char *func) {
    if (pthread_mutex_unlock(&pipeline_lock) != 0) {
        fprintf(stderr, "[LGNC] mock_pipeline_unlock failed on %s\n", func);
        abort();
    }
}