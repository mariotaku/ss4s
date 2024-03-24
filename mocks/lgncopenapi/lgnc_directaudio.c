#include "lgnc_directaudio.h"
#include "lgnc_mock.h"

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

static bool opened = false;

int LGNC_DIRECTAUDIO_Open(LGNC_ADEC_DATA_INFO_T *info) {
    mock_pipeline_lock(__FUNCTION__);
    if (opened) {
        mock_pipeline_unlock(__FUNCTION__);
        return -1;
    }
    usleep(2000000);
    opened = true;
    printf("[LGNC] DirectAudio opened\n");
    mock_pipeline_unlock(__FUNCTION__);
    return 0;

}

int LGNC_DIRECTAUDIO_Close() {
    mock_pipeline_lock(__FUNCTION__);
    if (!opened) {
        mock_pipeline_unlock(__FUNCTION__);
        return -1;
    }
    usleep(2000000);
    opened = false;
    printf("[LGNC] DirectAudio closed\n");
    mock_pipeline_unlock(__FUNCTION__);
    return 0;

}

int LGNC_DIRECTAUDIO_Play(const void *data, unsigned int size) {
    if (!opened) {
        return -1;
    }
    return 0;
}