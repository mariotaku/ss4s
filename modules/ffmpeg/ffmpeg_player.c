#include "ffmpeg_common.h"

#include <stdlib.h>

static SS4S_PlayerContext *CreatePlayerContext(SS4S_Player *player);

static void DestroyPlayerContext(SS4S_PlayerContext *context);

const SS4S_PlayerDriver SS4S_FFMPEG_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
};

static SS4S_PlayerContext *CreatePlayerContext(SS4S_Player *player) {
    SS4S_PlayerContext *context = calloc(1, sizeof(SS4S_PlayerContext));
    context->player = player;
    return context;
}

static void DestroyPlayerContext(SS4S_PlayerContext *context) {
    free(context);
}
