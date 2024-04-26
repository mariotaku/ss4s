#include <assert.h>
#include "ss4s.h"
#include "library.h"

bool SS4S_GetAudioCapabilitiesByCodecs(SS4S_AudioCapabilities *capabilities, SS4S_AudioCodec codecs) {
    const SS4S_AudioDriver *driver = SS4S_GetAudioDriver();
    if (driver == NULL || driver->GetCapabilities == NULL) {
        return false;
    }
    return driver->GetCapabilities(capabilities, codecs);
}

bool SS4S_GetAudioCapabilities(SS4S_AudioCapabilities *capabilities) {
    return SS4S_GetAudioCapabilitiesByCodecs(capabilities, SS4S_AUDIO_NONE);
}

SS4S_AudioCodec SS4S_GetAudioPreferredCodecs(const SS4S_AudioInfo *info) {
    const SS4S_AudioDriver *driver = SS4S_GetAudioDriver();
    if (driver == NULL || driver->GetPreferredCodecs == NULL) {
        return SS4S_AUDIO_NONE;
    }
    return driver->GetPreferredCodecs(info);
}

SS4S_AudioOpenResult SS4S_PlayerAudioOpen(SS4S_Player *player, const SS4S_AudioInfo *info) {
    const SS4S_AudioDriver *driver = SS4S_GetAudioDriver();
    if (driver == NULL) {
        return SS4S_AUDIO_OPEN_ERROR;
    }
    SS4S_MutexLock(player->mutex);
    SS4S_AudioOpenResult result = driver->Open(info, &player->audio, player->context.audio);
    if (result == SS4S_AUDIO_OPEN_OK) {
        assert(player->audio != NULL);
    }
    SS4S_MutexUnlock(player->mutex);
    return result;
}

SS4S_AudioFeedResult SS4S_PlayerAudioFeed(SS4S_Player *player, const unsigned char *data, size_t size) {
    SS4S_MutexLockEx(player->mutex, NULL);
    if (player->audio == NULL) {
        SS4S_MutexUnlockEx(player->mutex, NULL);
        return SS4S_AUDIO_FEED_NOT_READY;
    }
    const SS4S_AudioDriver *driver = SS4S_GetAudioDriver();
    assert(driver != NULL);
    SS4S_AudioInstance *audio = player->audio;
    SS4S_MutexUnlockEx(player->mutex, NULL);
    return driver->Feed(audio, data, size);
}

bool SS4S_PlayerAudioClose(SS4S_Player *player) {
    SS4S_MutexLock(player->mutex);
    if (player->audio == NULL) {
        SS4S_MutexUnlock(player->mutex);
        return false;
    }
    const SS4S_AudioDriver *driver = SS4S_GetAudioDriver();
    assert(driver != NULL);
    driver->Close(player->audio);
    player->audio = NULL;
    SS4S_MutexUnlock(player->mutex);
    return true;
}