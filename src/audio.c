#include <assert.h>
#include "ss4s.h"
#include "library.h"

SS4S_AudioCapabilities SS4S_GetAudioCapabilities() {
    const SS4S_AudioDriver *driver = SS4S_GetAudioDriver();
    if (driver == NULL || driver->GetCapabilities == NULL) {
        return 0;
    }
    return driver->GetCapabilities();

}

SS4S_AudioOpenResult SS4S_PlayerAudioOpen(SS4S_Player *player, const SS4S_AudioInfo *info) {
    const SS4S_AudioDriver *driver = SS4S_GetAudioDriver();
    if (driver == NULL) {
        return false;
    }
    SS4S_AudioOpenResult result = driver->Open(info, &player->audio, player->context.audio);
    if (result == SS4S_AUDIO_OPEN_OK) {
        assert(player->audio != NULL);
    }
    return result;
}

SS4S_AudioFeedResult SS4S_PlayerAudioFeed(SS4S_Player *player, const unsigned char *data, size_t size) {
    if (player->audio == NULL) {
        return false;
    }
    const SS4S_AudioDriver *driver = SS4S_GetAudioDriver();
    assert(driver != NULL);
    return driver->Feed(player->audio, data, size);
}

bool SS4S_PlayerAudioClose(SS4S_Player *player) {
    if (player->audio == NULL) {
        return false;
    }
    const SS4S_AudioDriver *driver = SS4S_GetAudioDriver();
    assert(driver != NULL);
    driver->Close(player->audio);
    return true;
}