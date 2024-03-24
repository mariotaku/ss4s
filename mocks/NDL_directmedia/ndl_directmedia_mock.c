#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "NDL_directmedia.h"
#include "ndl_directmedia_mock.h"

bool ndl_mock_init;
bool audio_opened = false, video_opened = false;

static pthread_mutex_t pipeline_lock = PTHREAD_MUTEX_INITIALIZER;

void mock_ndl_lock(const char *func) {
    if (pthread_mutex_trylock(&pipeline_lock) != 0) {
        fprintf(stderr, "[LGNC] mock_pipeline_lock failed on %s\n", func);
        abort();
    }
}

void mock_ndl_unlock(const char *func) {
    if (pthread_mutex_unlock(&pipeline_lock) != 0) {
        fprintf(stderr, "[LGNC] mock_pipeline_unlock failed on %s\n", func);
        abort();
    }
}

int NDL_DirectMediaInit(const char *app_id, ResourceReleased cb) {
    assert(app_id != NULL);
    mock_ndl_lock(__func__);
    if (ndl_mock_init) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    ndl_mock_init = true;
    mock_ndl_unlock(__func__);
    return 0;
}

const char *NDL_DirectMediaGetError() {
    return "";
}

int NDL_DirectMediaQuit() {
    mock_ndl_lock(__func__);
    if (!ndl_mock_init) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    ndl_mock_init = false;
    mock_ndl_unlock(__func__);
    return 0;
}