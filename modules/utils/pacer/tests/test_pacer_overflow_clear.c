#include <unistd.h>
#include <assert.h>

#include "pacer.h"
#include "test_common.h"

int main() {
    int iterations = 100;
    int interval = 50000;
    SS4S_Pacer *pacerRef[1];
    SS4S_Pacer *pacer = SS4S_PacerCreate(3, sizeof(struct timespec), interval, callback,
                                         pacerRef);
    pacerRef[0] = pacer;
    for (uint64_t i = 0; i < iterations; i++) {
        int writeLen = SS4S_PacerFeed(pacer, (const uint8_t *) &i, sizeof(uint64_t));
        if (writeLen == 0) {
            fprintf(stderr, "Buffer overflow. Clear the buffer\n");
            SS4S_PacerClear(pacer);
            continue;
        }
    }
    assert(SS4S_PacerFeed(pacer, (const uint8_t *) "01234567890123456789", 20) == -1);
    usleep(interval * iterations / 2);
    SS4S_PacerDestroy(pacer);
}