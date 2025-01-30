#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "pacer.h"
#include "test_common.h"

#define INTERVAL 5000

static int running = 0;

static void *FeedThreadProc(void *arg) {
    SS4S_Pacer *pacer = (SS4S_Pacer *) arg;
    while (running) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        int writeLen = SS4S_PacerFeed(pacer, (const uint8_t *) &ts, sizeof(struct timespec));
        if (writeLen == 0) {
            fprintf(stderr, "Buffer overflow. Clear the buffer\n");
            SS4S_PacerClear(pacer);
            continue;
        }
        usleep(INTERVAL - 220);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <duration>\n", argv[0]);
        return 127;
    }
    int duration = strtol(argv[1], NULL, 10);
    SS4S_Pacer *pacerRef[1];
    SS4S_Pacer *pacer = SS4S_PacerCreate(2, sizeof(struct timespec), INTERVAL, callback, pacerRef);
    pacerRef[0] = pacer;
    pthread_t feedThread;
    running = 1;
    pthread_create(&feedThread, NULL, FeedThreadProc, pacer);
    sleep(duration);
    running = 0;
    pthread_join(feedThread, NULL);
    SS4S_PacerDestroy(pacer);
}