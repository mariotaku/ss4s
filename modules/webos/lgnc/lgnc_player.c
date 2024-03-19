#include "lgnc_common.h"

#include <stdlib.h>

static SS4S_PlayerContext *CreatePlayerContext();

static void DestroyPlayerContext(SS4S_PlayerContext *context);

const SS4S_PlayerDriver SS4S_LGNC_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
};

static SS4S_PlayerContext *CreatePlayerContext() {
    return calloc(1, sizeof(SS4S_PlayerContext));
}

static void DestroyPlayerContext(SS4S_PlayerContext *context) {
    if (context->videoOpened) {
        LGNC_DIRECTVIDEO_Close();
    }
    if (context->audioOpened) {
        LGNC_DIRECTAUDIO_Close();
    }
    free(context);
}