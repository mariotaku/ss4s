#include <assert.h>
#include "ss4s.h"
#include "library.h"

bool SS4S_GetVideoCapabilities(SS4S_VideoCapabilities *capabilities) {
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL || driver->GetCapabilities == NULL) {
        return false;
    }
    return driver->GetCapabilities(capabilities);
}

SS4S_VideoOpenResult SS4S_PlayerVideoOpen(SS4S_Player *player, const SS4S_VideoInfo *info) {
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL) {
        return SS4S_VIDEO_OPEN_ERROR;
    }
    assert(driver->Open != NULL);
    SS4S_VideoExtraInfo extraInfo = {
            .viewportWidth = player->viewportWidth,
            .viewportHeight = player->viewportHeight,
    };
    SS4S_VideoOpenResult result = driver->Open(info, &extraInfo, &player->video, player->context.video);
    if (result == SS4S_VIDEO_OPEN_OK) {
        assert(player->video != NULL);
    }
    size_t statsCapacity = 120;
    if (info->frameRateNumerator > 0 && info->frameRateNumerator > 0) {
        statsCapacity = info->frameRateNumerator / info->frameRateDenominator * 2;
    }
    SS4S_StatsCounterInit(&player->stats.video, statsCapacity);
    return result;
}

SS4S_VideoFeedResult SS4S_PlayerVideoFeed(SS4S_Player *player, const unsigned char *data, size_t size,
                                          SS4S_VideoFeedFlags flags) {
    if (player->video == NULL) {
        return SS4S_VIDEO_FEED_NOT_READY;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    assert(driver != NULL);
    assert(driver->Feed != NULL);
    return driver->Feed(player->video, data, size, flags);
}

bool SS4S_PlayerVideoSizeChanged(SS4S_Player *player, int width, int height) {
    if (player->video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL || driver->SizeChanged == NULL) {
        return false;
    }
    return driver->SizeChanged(player->video, width, height);
}

bool SS4S_PlayerVideoSetHDRInfo(SS4S_Player *player, const SS4S_VideoHDRInfo *info) {
    if (player->video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL || driver->SetHDRInfo == NULL) {
        return false;
    }
    return driver->SetHDRInfo(player->video, info);

}

bool SS4S_PlayerVideoSetDisplayArea(SS4S_Player *player, const SS4S_VideoRect *src, const SS4S_VideoRect *dst) {
    if (player->video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL || driver->SetDisplayArea == NULL) {
        return false;
    }
    return driver->SetDisplayArea(player->video, src, dst);
}

bool SS4S_PlayerVideoClose(SS4S_Player *player) {
    if (player->video == NULL) {
        return false;
    }
    SS4S_StatsCounterDeinit(&player->stats.video);
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    assert(driver != NULL);
    assert(driver->Close != NULL);
    driver->Close(player->video);
    return true;
}