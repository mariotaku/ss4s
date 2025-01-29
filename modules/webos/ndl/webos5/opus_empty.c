#include "opus_empty.h"
#include "opus_empty_samples.h"

#include <stdlib.h>
#include <stdint.h>

struct SS4S_OpusEmpty {
    int channels, streams, coupled;
};

SS4S_OpusEmpty *SS4S_OpusEmptyCreate(int channels, int streams, int coupled) {
    SS4S_OpusEmpty *instance = calloc(1, sizeof(SS4S_OpusEmpty));
    instance->channels = channels;
    instance->streams = streams;
    instance->coupled = coupled;
    return instance;
}

void SS4S_OpusEmptyDestroy(SS4S_OpusEmpty *instance) {
    free(instance);
}

bool SS4S_OpusIsEmptyFrame(const SS4S_OpusEmpty *instance, const unsigned char *data, size_t size) {
    size_t pos = 0;
    for (int i = 0, j = instance->streams - 1; i <= j; i++) {
        uint8_t toc = data[pos];
        uint8_t code = toc & 0b11;
        if (code != 0) {
            return false;
        }
        uint16_t frameLen;
        if (i < j) {
            frameLen = data[++pos];
            if (frameLen >= 252) {
                frameLen += data[++pos] * 4;
            }
            pos++;
        } else {
            pos++;
            frameLen = size - pos;
        }
        if (frameLen < 2) {
            return false;
        }
        if (data[pos] != 0xFF || data[pos + 1] != 0xFE) {
            return false;
        }
        if (frameLen >= 2) {
            // Rest of the frame is all zeros
            for (size_t k = pos + 2; k < pos + frameLen; k++) {
                if (data[k] != 0) {
                    return false;
                }
            }
        }
        pos += frameLen;
    }
    return true;
}

int SS4S_OpusEmptyPlay(const SS4S_OpusEmpty *instance, SS4S_NDLOpusEmptyFeedFunc feed) {
    switch (instance->channels * 100 + instance->streams * 10 + instance->coupled) {
        case 220:
            return feed(opus_empty_frame_220, sizeof(opus_empty_frame_220));
        case 211:
            return feed(opus_empty_frame_211, sizeof(opus_empty_frame_211));
        case 660:
            // Fallthrough because it will be converted to 642
        case 642:
            return feed(opus_empty_frame_642, sizeof(opus_empty_frame_642));
        case 880:
            return feed(opus_empty_frame_880, sizeof(opus_empty_frame_880));
        case 853:
            return feed(opus_empty_frame_853, sizeof(opus_empty_frame_853));
        default:
            return 0;
    }
}
