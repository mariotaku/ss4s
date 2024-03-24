#include "lgnc_directvideo.h"
#include "lgnc_mock.h"

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

static bool opened = false;

int LGNC_DIRECTVIDEO_Open(const LGNC_VDEC_DATA_INFO_T *info) {
    mock_pipeline_lock(__FUNCTION__);
    if (opened) {
        mock_pipeline_unlock(__FUNCTION__);
        return -1;
    }
    usleep(2000000);
    opened = true;
    printf("[LGNC] DirectVideo opened\n");
    mock_pipeline_unlock(__FUNCTION__);
    return 0;
}

int LGNC_DIRECTVIDEO_Close() {
    mock_pipeline_lock(__FUNCTION__);
    if (!opened) {
        mock_pipeline_unlock(__FUNCTION__);
        return -1;
    }
    usleep(2000000);
    opened = false;
    printf("[LGNC] DirectVideo closed\n");
    mock_pipeline_unlock(__FUNCTION__);
    return 0;
}

int LGNC_DIRECTVIDEO_Play(const void *data, unsigned int size) {
    if (!opened) {
        return -1;
    }
    if (size < 4) {
        return -1;
    }
    char *buf = (char *) data;
    return buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1 ? 0 : -1;
}

int _LGNC_DIRECTVIDEO_SetDisplayWindow(int x, int y, int w, int h) {
    if (!opened) {
        return -1;
    }
    printf("[LGNC] DirectVideo display window set to %d, %d, %d, %d\n", x, y, w, h);
    return 0;
}

int _LGNC_DIRECTVIDEO_SetCustomDisplayWindow(int x, int y, int w, int h) {
    if (!opened) {
        return -1;
    }
    printf("[LGNC] DirectVideo custom display window set to %d, %d, %d, %d\n", x, y, w, h);
    return 0;
}