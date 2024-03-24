#define NDL_DIRECTMEDIA_API_VERSION 1

#include <unistd.h>
#include <stdio.h>

#include "NDL_directmedia.h"
#include "ndl_directmedia_mock.h"

static NDLVideoPlayCallback video_callback = NULL;

int NDL_DirectAudioOpen(NDL_DIRECTAUDIO_DATA_INFO_T *data) {
    mock_ndl_lock(__func__);
    if (audio_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    usleep(200000);
    audio_opened = true;
    printf("[NDL] DirectAudio opened\n");
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectAudioPlay(void *buffer, unsigned int size) {
    mock_ndl_lock(__func__);
    if (!audio_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectAudioClose(void) {
    mock_ndl_lock(__func__);
    if (!audio_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    usleep(200000);
    audio_opened = false;
    printf("[NDL] DirectAudio closed\n");
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoOpen(NDL_DIRECTVIDEO_DATA_INFO_T *data) {
    mock_ndl_lock(__func__);
    if (video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    usleep(200000);
    video_opened = true;
    printf("[NDL] DirectVideo opened\n");
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoSetCallback(NDLVideoPlayCallback cb) {
    mock_ndl_lock(__func__);
    video_callback = cb;
    printf("[NDL] DirectVideo callback set\n");
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoPlay(void *buffer, unsigned int size) {
    mock_ndl_lock(__func__);
    if (!video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoPlayWithCallback(const void *buffer, unsigned int size, unsigned long long dataStruct) {
    mock_ndl_lock(__func__);
    if (!video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    if (video_callback != NULL) {
        video_callback(dataStruct);
    }
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoSetArea(int left, int top, int width, int height) {
    mock_ndl_lock(__func__);
    if (!video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoClose(void) {
    mock_ndl_lock(__func__);
    if (!video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    usleep(200000);
    video_opened = false;
    printf("[NDL] DirectVideo closed\n");
    mock_ndl_unlock(__func__);
    return 0;
}