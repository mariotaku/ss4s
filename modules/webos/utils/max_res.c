#include <dlfcn.h>
#include "max_res.h"

static bool (*MRCGetMaxVideoResolution)(int codec, int *w, int *h, int *fps) = NULL;

int SS4S_webOS_GetMaxVideoResolution(SS4S_VideoCodec codec, int *fps, int *w, int *h) {
    int mrc_codec;
    switch (codec) {
        case SS4S_VIDEO_H264:
            mrc_codec = (1 << 1);//kVideoH264;
            break;
        case SS4S_VIDEO_H265:
            mrc_codec = (1 << 2);//kVideoH265;
            break;
        default:
            return -1;
    }
    if (MRCGetMaxVideoResolution == NULL) {
        void *handle = dlopen("libmedia-resource-calculator.so.1", RTLD_LAZY);
        if (handle == NULL) {
            return -1;
        }
        MRCGetMaxVideoResolution = dlsym(handle, "MRCGetMaxVideoResolution");
        if (MRCGetMaxVideoResolution == NULL) {
            dlclose(handle);
            return -1;
        }
    }
    return MRCGetMaxVideoResolution(mrc_codec, w, h, fps) ? 0 : -1;
}