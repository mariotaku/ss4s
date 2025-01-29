#include <unistd.h>
#include <assert.h>
#include <time.h>

#include "pacer.h"
#include "test_common.h"


int main() {
    int iterations = 100;
    int interval = 5000;
    SS4S_Pacer *pacerRef[1];
    SS4S_Pacer *pacer = SS4S_PacerCreate(3, sizeof(struct timespec), interval, callback, pacerRef);
    pacerRef[0] = pacer;
    for (int i = 0; i < iterations; i++) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        int writeLen = SS4S_PacerFeed(pacer, (const uint8_t *) &ts, sizeof(struct timespec));
        assert(writeLen == sizeof(struct timespec));
        usleep(interval - 100);
    }
    usleep(10000);
    SS4S_PacerDestroy(pacer);
}