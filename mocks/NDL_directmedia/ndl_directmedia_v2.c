#define NDL_DIRECTMEDIA_API_VERSION 2

#include <unistd.h>
#include "NDL_directmedia.h"

#include "ndl_directmedia_mock.h"


int NDL_DirectMediaUnload(void) {
    mock_ndl_lock(__func__);
    if (!audio_opened && !video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    audio_opened = false;
    video_opened = false;
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectMediaLoad(NDL_DIRECTMEDIA_DATA_INFO_T *data, NDLMediaLoadCallback callback) {
    mock_ndl_lock(__func__);
    if (audio_opened || video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    usleep(1000000);
    audio_opened = data->audio.type != 0;
    video_opened = data->video.type != 0;
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectAudioSupportMultiChannel(int *isSupported) {
    mock_ndl_lock(__func__);
    usleep(100000);
    *isSupported = 0;
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoGetRenderBufferLength(int *length) {
    mock_ndl_lock(__func__);
    if (!video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    *length = 0;
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoSetHDRInfo(NDL_DIRECTVIDEO_HDR_INFO_T hdrInfo) {
    (void) hdrInfo;
    mock_ndl_lock(__func__);
    if (!video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    mock_ndl_unlock(__func__);
    return 0;
}