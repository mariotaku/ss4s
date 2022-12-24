#include "dummy_common.h"

static SS4S_AudioOpenResult OpenAudio(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    if (SS4S_Dummy_ReloadMedia(context) != 0) {
        return SS4S_AUDIO_OPEN_ERROR;
    }
    *instance = (SS4S_AudioInstance *) context;
    return SS4S_AUDIO_OPEN_OK;
}

static SS4S_AudioFeedResult FeedAudio(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    SS4S_PlayerContext *context = (void *) instance;
    if (!context->mediaLoaded) {
        return SS4S_AUDIO_FEED_NOT_READY;
    }
    return SS4S_AUDIO_FEED_OK;
}

static void CloseAudio(SS4S_AudioInstance *instance) {
    SS4S_PlayerContext *context = (void *) instance;
    SS4S_Dummy_ReloadMedia(context);
}

const SS4S_AudioDriver SS4S_Dummy_AudioDriver = {
        .Base = {
                .PostInit = SS4S_Dummy_Driver_PostInit,
                .Quit = SS4S_Dummy_Driver_Quit,
        },
        .Open = OpenAudio,
        .Feed = FeedAudio,
        .Close = CloseAudio,
};