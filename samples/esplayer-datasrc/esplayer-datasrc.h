#pragma

#include <stddef.h>

enum {
    VIDEO_FLAG_FRAME_START = 1 << 0,
    VIDEO_FLAG_FRAME_END = 1 << 1,
    VIDEO_FLAG_FRAME_KEYFRAME = 1 << 2,
    VIDEO_FLAG_FRAME = VIDEO_FLAG_FRAME_START | VIDEO_FLAG_FRAME_END,
};

struct DATASRC_CALLBACKS {
    int (*videoPreroll)(const char *codec, int width, int height, int framerate_numerator, int framerate_denominator);

    int (*videoSample)(const void *data, size_t size, int flags);

    void (*videoEos)();

    int (*audioPreroll)(int channels, int sampleFreq);

    int (*audioSample)(const void *data, size_t size);

    void (*audioEos)();

    void (*pipelineQuit)(int error);
};

int datasrc_run(struct DATASRC_CALLBACKS *callbacks);
