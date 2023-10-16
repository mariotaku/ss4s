#include "ndl_common.h"

#include <stdlib.h>

static SS4S_PlayerContext *CreatePlayerContext(SS4S_Player *player);

static void DestroyPlayerContext(SS4S_PlayerContext *context);

const SS4S_PlayerDriver SS4S_NDL_webOS4_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
};

static SS4S_PlayerContext *CreatePlayerContext(SS4S_Player *player) {
    SS4S_PlayerContext *context = calloc(1, sizeof(SS4S_PlayerContext));
    context->player = player;
    return context;
}

static void DestroyPlayerContext(SS4S_PlayerContext *context) {
    if (context->videoOpened) {
        NDL_DirectVideoClose();
    }
    if (context->audioOpened) {
        NDL_DirectAudioClose();
    }
    free(context);
}
