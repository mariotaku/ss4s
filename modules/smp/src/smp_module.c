#include <assert.h>
#include <stdlib.h>

#include "ss4s/modapi.h"
#include "smp_video.h"

static SS4S_LoggingFunction *Log = NULL;

struct SS4S_PlayerContext {
    SS4S_LoggingFunction *log;
    StarfishVideo *videoPlayer;
};

static SS4S_PlayerContext *CreatePlayer() {
    SS4S_PlayerContext *context = calloc(1, sizeof(SS4S_PlayerContext));
    context->log = Log;
    return context;
}

static void DestroyPlayer(SS4S_PlayerContext *context) {
    if (context->videoPlayer != NULL) {
        StarfishVideoDestroy(context->videoPlayer);
    }
    free(context);
}

static SS4S_VideoOpenResult VideoOpen(const SS4S_VideoInfo *info, SS4S_VideoInstance **instance,
                                      SS4S_PlayerContext *context) {
    if (context->videoPlayer == NULL) {
        context->videoPlayer = StarfishVideoCreate(context->log);
        if (context->videoPlayer == NULL) {
            return SS4S_VIDEO_OPEN_ERROR;
        }
    }
    StarfishVideo *player = context->videoPlayer;
    SS4S_VideoOpenResult result;
    if ((result = StarfishVideoLoad(player, info)) != SS4S_VIDEO_OPEN_OK) {
        return result;
    }
    *instance = player;
    return SS4S_VIDEO_OPEN_OK;
}

static void VideoClose(SS4S_VideoInstance *instance) {
    assert(instance != NULL);
    StarfishVideoUnload(instance);
}


static const SS4S_VideoDriver VideoDriver = {
        .Base = {},
        .GetCapabilities = StarfishVideoCapabilities,
        .Open = VideoOpen,
        .Feed = StarfishVideoFeed,
        .SizeChanged = StarfishVideoSizeChanged,
        .SetHDRInfo = StarfishVideoSetHDRInfo,
        .SetDisplayArea = StarfishVideoSetDisplayArea,
        .Close = VideoClose,
};

static const SS4S_PlayerDriver PlayerDriver = {
        .Create = CreatePlayer,
        .Destroy = DestroyPlayer,
};

SS4S_EXPORTED bool SS4S_MODULE_ENTRY(SS4S_Module *module, const SS4S_LibraryContext *context) {
    Log = context->Log;
    assert(Log != NULL);
    module->Name = SS4S_MODULE_NAME;
    module->PlayerDriver = &PlayerDriver;
    module->VideoDriver = &VideoDriver;
    return true;
}