#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "ss4s.h"
#include "test_common.h"
#include "nalu_reader.h"

static int ss4s_nalu_cb(void *ctx, const unsigned char *nalu, size_t size) {
    SS4S_Player *player = ctx;
    SS4S_VideoFeedResult result = SS4S_PlayerVideoFeed(player, nalu, size, 0);
    usleep(30000);
    return result;
}

int main(int argc, char *argv[]) {
    setenv("APPID", "com.example.test", 0);
    char driver[16] = {'\0'};
    single_test_infer_module(driver, sizeof(driver), "ss4s_test_h264_playback_", argc, argv);
    printf("Request video driver: %s\n", driver);

    SS4S_Config config = {.videoDriver = driver};
    SS4S_Init(argc, argv, &config);
    SS4S_PostInit(argc, argv);
    SS4S_Player *player = SS4S_PlayerOpen();
    assert(player != NULL);
    SS4S_PlayerInfo playerInfo;
    assert(SS4S_PlayerGetInfo(player, &playerInfo));
    if (playerInfo.video.module == NULL) {
        return 127;
    }
    assert(strcmp(playerInfo.video.module, driver) == 0);

    SS4S_VideoInfo videoInfo = {
            .codec = SS4S_VIDEO_H264,
            .width = 1280,
            .height = 800,
    };
    unsigned char tmp[16] = {0};
    assert(SS4S_PlayerVideoFeed(player, tmp, 16, 0) == SS4S_VIDEO_FEED_NOT_READY);
    assert(SS4S_PlayerVideoOpen(player, &videoInfo) == SS4S_VIDEO_OPEN_OK);

    FILE *sampleFile = fopen("sample.h264", "rb");
    assert(sampleFile != NULL);
    assert(nalu_read(sampleFile, ss4s_nalu_cb, player) == 0);

    fclose(sampleFile);

    assert(SS4S_PlayerVideoClose(player));
    SS4S_PlayerClose(player);
    SS4S_Quit();
    return 0;
}