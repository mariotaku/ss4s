#include <assert.h>
#include "ss4s.h"
#include "library.h"

bool SS4S_PlayerVideoOpen(SS4S_Player *player, const SS4S_VideoInfo *info) {
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL) {
        return false;
    }
    player->video = driver->Open(info);
    return true;
}

bool SS4S_PlayerVideoFeed(SS4S_Player *player, const unsigned char *data, size_t size) {
    if (player->video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    assert(driver != NULL);
    return driver->Feed(player->video, data, size) == 0;
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