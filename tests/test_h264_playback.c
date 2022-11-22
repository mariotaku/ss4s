#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "ss4s.h"

int main(int argc, char *argv[]) {
    char *driver = "mmal";
    if (argc > 1) {
        driver = argv[1];
    }
    printf("Request video driver: %s\n", driver);

    SS4S_Config config = {.videoDriver = driver};
    SS4S_Init(argc, argv, &config);
    SS4S_PostInit(argc, argv);
    SS4S_Player *player = SS4S_PlayerOpen();
    assert(player != NULL);
    SS4S_PlayerInfo playerInfo;
    assert(SS4S_PlayerGetInfo(player, &playerInfo));
    if (playerInfo.videoModule == NULL) {
        return 127;
    }
    assert(strcmp(playerInfo.videoModule, driver) == 0);

    SS4S_VideoInfo videoInfo = {
            .codec = SS4S_VIDEO_H264,
            .width = 1280,
            .height = 800,
    };
    assert(SS4S_PlayerVideoOpen(player, &videoInfo) == SS4S_VIDEO_OPEN_OK);

    FILE *sampleFile = fopen("sample.h264", "rb");
    assert(sampleFile != NULL);
    int c, headIdx = 0, naluCount = 0;
    unsigned char *buf = malloc(1024 * 1024);
    size_t bufSize = 0;
    while ((c = fgetc(sampleFile)) >= 0) {
        buf[bufSize++] = c;
        switch (c) {
            case 0: {
                headIdx++;
                break;
            }
            case 1: {
                if (headIdx == 3) {
                    naluCount++;
                    if (bufSize > 4) {
//                        SS4S_PlayerVideoFeed(player, buf, bufSize, 0);
                        usleep(1000);
                        bufSize = 4;
                    }
                }
                headIdx = 0;
                break;
            }
            default: {
                headIdx = 0;
                break;
            }
        }
    }
//    SS4S_PlayerVideoFeed(player, buf, bufSize, 0);

    fclose(sampleFile);

    assert(SS4S_PlayerVideoClose(player));
    SS4S_PlayerClose(player);
    SS4S_Quit();
    return 0;
}