#include <assert.h>
#include <time.h>

#include "pacer.h"
#include "test_common.h"

int main() {
    int interval = 5000;
    SS4S_Pacer *pacerRef[1];
    SS4S_Pacer *pacer = SS4S_PacerCreate(3, sizeof(struct timespec), interval, callback,
                                         pacerRef);
    pacerRef[0] = pacer;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(SS4S_PacerFeed(pacer, (const uint8_t *) &ts, sizeof(struct timespec)) == sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(SS4S_PacerFeed(pacer, (const uint8_t *) &ts, sizeof(struct timespec)) == sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(SS4S_PacerFeed(pacer, (const uint8_t *) &ts, sizeof(struct timespec)) == 0);
    SS4S_PacerDestroy(pacer);
}