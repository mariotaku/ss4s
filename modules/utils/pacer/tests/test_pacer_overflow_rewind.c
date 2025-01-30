#include <assert.h>
#include <time.h>

#include "pacer.h"
#include "test_common.h"

int main() {
    int interval = 50000;
    SS4S_Pacer *pacerRef[1];
    SS4S_Pacer *pacer = SS4S_PacerCreate(3, 32, interval, callback,
                                         pacerRef);
    pacerRef[0] = pacer;
    uint8_t data[32];
    for(int i = 0; i < 11; i++) {
        assert(SS4S_PacerFeed(pacer, data, 32) == 32);
    }
    assert(SS4S_PacerFeed(pacer, data, 8) == 8);
    assert(SS4S_PacerFeed(pacer, data, 32) == 0);
    SS4S_PacerDestroy(pacer);
}