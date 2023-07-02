#include "sl_common.h"

#include <stdlib.h>

static SS4S_PlayerContext *CreatePlayerContext();

static void DestroyPlayerContext(SS4S_PlayerContext *context);

const SS4S_PlayerDriver SS4S_STEAMLINK_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
};

static SS4S_PlayerContext *CreatePlayerContext() {
    return calloc(1, sizeof(SS4S_PlayerContext));
}

static void DestroyPlayerContext(SS4S_PlayerContext *context) {
    if (context->videoStream != NULL) {
        SLVideo_FreeStream(context->videoStream);
    }
    if (context->videoContext != NULL) {
        SLVideo_FreeContext(context->videoContext);
    }
    if (context->audioStream) {
        SLAudio_FreeStream(context->audioStream);
    }
    if (context->audioContext) {
        SLAudio_FreeContext(context->audioContext);
    }
    free(context);
}
