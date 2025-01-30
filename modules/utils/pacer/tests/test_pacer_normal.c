#include <unistd.h>
#include <assert.h>
#include <time.h>

#include "pacer.h"
#include "test_common.h"


int main() {
    int iterations = 100;
    int interval = 5000;
    SS4S_Pacer *pacerRef[1];
    SS4S_Pacer *pacer = SS4S_PacerCreate(3, sizeof(uint64_t), interval, callback, pacerRef);
    pacerRef[0] = pacer;
    for (uint64_t i = 0; i < iterations; i++) {
        int writeLen = SS4S_PacerFeed(pacer, (const uint8_t *) &i, sizeof(uint64_t));
        assert(writeLen == sizeof(uint64_t));
        usleep(interval - 100);
    }
    usleep(10000);
    SS4S_PacerDestroy(pacer);
}