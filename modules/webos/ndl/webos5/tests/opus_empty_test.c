#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "opus_empty.h"

static int EmptyFeed(void *arg, const unsigned char *data, size_t size) {
    static int count = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("EmptyFeed: count=%d, time=%lu.%06lu\n", ++count, ts.tv_sec, ts.tv_nsec / 1000);
    fflush(stdout);
    return 0;
}

int main() {
    SS4S_OpusEmpty *empty = SS4S_OpusEmptyCreate(2, 2, 0, 5000);
    SS4S_OpusEmptyStart(empty, EmptyFeed, NULL);
    usleep(30000);
    printf("Frame arrived 1\n");
    SS4S_OpusEmptyFrameArrived(empty);
    usleep(5000);
    printf("Frame arrived 2\n");
    SS4S_OpusEmptyFrameArrived(empty);
    usleep(50000);
    SS4S_OpusEmptyDestroy(empty);
}