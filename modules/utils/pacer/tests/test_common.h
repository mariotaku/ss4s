#pragma once

#include <time.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static int callback(void *arg, const unsigned char *data, size_t size, const struct timespec *ts) {
    SS4S_Pacer *pacer = *((SS4S_Pacer **) arg);
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (size == 0) {
        printf("Feed underflow. bufferCount=%u, ts=%ld.%06ld\n", (uint32_t) SS4S_PacerGetBufferCount(pacer), now.tv_sec,
               now.tv_nsec / 1000);
    } else {
        long latency = (now.tv_sec - ts->tv_sec) * 1000000 + (now.tv_nsec - ts->tv_nsec) / 1000;
        printf("Feed counter %lu. bufferCount=%u, latency=%ld us, ts=%ld.%06ld\n", *((const uint64_t *) data),
               (uint32_t) SS4S_PacerGetBufferCount(pacer), latency, now.tv_sec, now.tv_nsec / 1000);
    }
    usleep(1000 + rand() % 400);
    return 0;
}
