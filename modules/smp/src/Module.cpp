#include <cassert>
#include "ss4s/modapi.h"
#include "AVStreamPlayer.h"

using namespace SMP_DECODER_NS;

static SS4S_PlayerContext *CreatePlayer() {
    auto *context = new SS4S_PlayerContext;
    context->player = new AVStreamPlayer();
    return context;
}

static void DestroyPlayer(SS4S_PlayerContext *context) {
    delete context->player;
    delete context;
}

static SS4S_AudioOpenResult AudioOpen(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    auto *player = context->player;
//    player->audioConfig = *info;
    if (!player->load()) {
        return SS4S_AUDIO_OPEN_ERROR;
    }
    *instance = reinterpret_cast<SS4S_AudioInstance *>(context);
    return SS4S_AUDIO_OPEN_OK;
}

static SS4S_AudioFeedResult AudioFeed(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    auto *player = reinterpret_cast<SS4S_PlayerContext *>(instance)->player;
    player->submitAudio(data, size);
    return SS4S_AUDIO_FEED_ERROR;
}

static void AudioClose(SS4S_AudioInstance *instance) {
    auto *player = reinterpret_cast<SS4S_PlayerContext *>(instance)->player;
    player->audioConfig.codec = SS4S_AUDIO_NONE;
    player->load();
}

static const SS4S_AudioDriver AudioDriver = {
        .Base = {},
        .GetCapabilities = nullptr,
        .Open = AudioOpen,
        .Feed = AudioFeed,
        .Close = AudioClose,
};

static SS4S_VideoOpenResult VideoOpen(const SS4S_VideoInfo *info, SS4S_VideoInstance **instance,
                                      SS4S_PlayerContext *context) {
    auto *player = context->player;
    assert(player != nullptr);
    player->videoConfig = *info;
    if (!player->load()) {
        return SS4S_VIDEO_OPEN_ERROR;
    }
    *instance = reinterpret_cast<SS4S_VideoInstance *>(context);
    return SS4S_VIDEO_OPEN_OK;
}

static SS4S_VideoFeedResult VideoFeed(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                      SS4S_VideoFeedFlags flags) {
    auto *player = reinterpret_cast<SS4S_PlayerContext *>(instance)->player;
    assert(player != nullptr);
    return player->submitVideo(data, size, flags);
}

static bool VideoSizeChanged(SS4S_VideoInstance *instance, int width, int height) {
    auto *player = reinterpret_cast<SS4S_PlayerContext *>(instance)->player;
    assert(player != nullptr);
    player->videoConfig.width = width;
    player->videoConfig.height = height;
    return player->load();
}

static bool VideoSetHDRInfo(SS4S_VideoInstance *instance, const SS4S_VideoHDRInfo *info) {
    auto *player = reinterpret_cast<SS4S_PlayerContext *>(instance)->player;
    assert(player != nullptr);
    player->setHdr(info != nullptr);
    return true;
}

static bool VideoSetDisplayArea(SS4S_VideoInstance *instance, const SS4S_VideoRect *src, const SS4S_VideoRect *dst) {
    return false;
}

static void VideoClose(SS4S_VideoInstance *instance) {
    auto *player = reinterpret_cast<SS4S_PlayerContext *>(instance)->player;
    assert(player != nullptr);
    player->videoConfig.codec = SS4S_VIDEO_NONE;
    player->load();
}

static SS4S_VideoCapabilities VideoCapabilities() {
    return (SS4S_VideoCapabilities) (SS4S_VIDEO_CAP_CODEC_H265 | SS4S_VIDEO_CAP_CODEC_H264 |
                                     SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING);
}

static const SS4S_VideoDriver VideoDriver = {
        .Base = {},
        .GetCapabilities = VideoCapabilities,
        .Open = VideoOpen,
        .Feed = VideoFeed,
        .SizeChanged = VideoSizeChanged,
        .SetHDRInfo = VideoSetHDRInfo,
        .SetDisplayArea = VideoSetDisplayArea,
        .Close = VideoClose,
};

static const SS4S_PlayerDriver PlayerDriver = {
        .Create = CreatePlayer,
        .Destroy = DestroyPlayer,
};

extern "C" SS4S_EXPORTED bool SS4S_MODULE_ENTRY(SS4S_Module *module, const SS4S_LibraryContext *context) {
    Log = context->Log;
    assert(Log != nullptr);
    module->Name = SS4S_MODULE_NAME;
    module->PlayerDriver = &PlayerDriver;
    module->AudioDriver = &AudioDriver;
    module->VideoDriver = &VideoDriver;
    return true;
}