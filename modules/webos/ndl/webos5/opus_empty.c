#include "opus_empty.h"
#include "opus_empty_samples.h"
#include "ndl_common.h"

#include <stdlib.h>
#include <pthread.h>

#include <NDL_directmedia_v2.h>
#include <assert.h>
#include <unistd.h>

struct SS4S_NDLOpusEmpty {
    int channels, streams, coupled;
};

SS4S_NDLOpusEmpty *SS4S_NDLOpusEmptyCreate(int channels, int streams, int coupled) {
    SS4S_NDLOpusEmpty *instance = calloc(1, sizeof(SS4S_NDLOpusEmpty));
    instance->channels = channels;
    instance->streams = streams;
    instance->coupled = coupled;
    return instance;
}

void SS4S_NDLOpusEmptyDestroy(SS4S_NDLOpusEmpty *instance) {
    free(instance);
}


int SS4S_NDLOpusEmptyPlay(const SS4S_NDLOpusEmpty *instance) {
    switch (instance->channels * 100 + instance->streams * 10 + instance->coupled) {
        case 220:
            return NDL_DirectAudioPlay((void *) opus_empty_frame_220, sizeof(opus_empty_frame_220), 0);
        case 211:
            return NDL_DirectAudioPlay((void *) opus_empty_frame_211, sizeof(opus_empty_frame_211), 0);
        case 660:
            // Fallthrough because it will be converted to 642
        case 642:
            return NDL_DirectAudioPlay((void *) opus_empty_frame_642, sizeof(opus_empty_frame_642), 0);
        case 880:
            return NDL_DirectAudioPlay((void *) opus_empty_frame_880, sizeof(opus_empty_frame_880), 0);
        case 853:
            return NDL_DirectAudioPlay((void *) opus_empty_frame_853, sizeof(opus_empty_frame_853), 0);
        default:
            return 0;
    }
}
