#include "ndl_common.h"

#include <stdlib.h>

static SS4S_PlayerContext *CreatePlayerContext();

static void DestroyPlayerContext(SS4S_PlayerContext *context);

static void UnloadMedia(const SS4S_PlayerContext *context);

const SS4S_PlayerDriver SS4S_LGNC_PlayerDriver = {
        .Create = CreatePlayerContext,
        .Destroy = DestroyPlayerContext,
};

static SS4S_PlayerContext *CreatePlayerContext() {
    return calloc(1, sizeof(SS4S_PlayerContext));
}

static void DestroyPlayerContext(SS4S_PlayerContext *context) {
    UnloadMedia(context);
    free(context);
}

static void UnloadMedia(const SS4S_PlayerContext *context) {

}