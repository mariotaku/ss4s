#include "opus_empty.h"
#include "opus_empty_samples.h"

#include <stdlib.h>
#include <stdint.h>

struct SS4S_OpusEmpty {
    uint32_t channels, streams, coupled;
    SS4S_NDLOpusEmptyFeedFunc feed;
};

SS4S_OpusEmpty *SS4S_OpusEmptyCreate(uint32_t channels, uint32_t streams, uint32_t coupled) {
    SS4S_OpusEmpty *instance = calloc(1, sizeof(SS4S_OpusEmpty));
    instance->channels = channels;
    instance->streams = streams;
    instance->coupled = coupled;
    return instance;
}

int SS4S_OpusEmptyFeed(SS4S_OpusEmpty *instance, SS4S_NDLOpusEmptyFeedFunc feed, void *feedArg) {
    switch (instance->channels * 100 + instance->streams * 10 + instance->coupled) {
        case 220:
            return feed(feedArg, opus_empty_frame_220, sizeof(opus_empty_frame_220));
        case 211:
            return feed(feedArg, opus_empty_frame_211, sizeof(opus_empty_frame_211));
        case 660:
            // Fallthrough because it will be converted to 642
        case 642:
            return feed(feedArg, opus_empty_frame_642, sizeof(opus_empty_frame_642));
        case 880:
            return feed(feedArg, opus_empty_frame_880, sizeof(opus_empty_frame_880));
        case 853:
            return feed(feedArg, opus_empty_frame_853, sizeof(opus_empty_frame_853));
        default:
            return 0;
    }
}

void SS4S_OpusEmptyDestroy(SS4S_OpusEmpty *instance) {
    free(instance);
}
