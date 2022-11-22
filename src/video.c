#include <assert.h>
#include "ss4s.h"
#include "library.h"

SS4S_VideoOpenResult SS4S_PlayerVideoOpen(SS4S_Player *player, const SS4S_VideoInfo *info) {
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL) {
        return false;
    }
    SS4S_VideoOpenResult result = driver->Open(info, &player->video);
    if (result == SS4S_VIDEO_OPEN_OK) {
        assert(player->video != NULL);
    }
    return result;
}

SS4S_VideoFeedResult SS4S_PlayerVideoFeed(SS4S_Player *player, const unsigned char *data, size_t size,
                                          SS4S_VideoFeedFlags flags) {
    if (player->video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    assert(driver != NULL);
    return driver->Feed(player->video, data, size, flags);
}

bool SS4S_PlayerVideoSizeChanged(SS4S_Player *player, int width, int height) {
    if (player->video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL) {
        return false;
    }
    return driver->SizeChanged(player->video, width, height);
}

bool SS4S_PlayerVideoClose(SS4S_Player *player) {
    if (player->video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    assert(driver != NULL);
    driver->Close(player->video);
    return true;
}