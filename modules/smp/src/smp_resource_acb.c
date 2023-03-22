#include "smp_resource.h"

#include <AcbAPI.h>

struct StarfishResource {
    long acbId;
};

static void AcbCallback(long acbId, long taskId, long eventType, long appState, long playState,
                        const char *reply);

StarfishResource *StarfishResourceCreate(const char *appId) {
    StarfishResource *res = calloc(1, sizeof(StarfishResource));
    res->acbId = AcbAPI_create();
    if (res->acbId == 0) {
        free(res);
        return NULL;
    }
    AcbAPI_initialize(res->acbId, PLAYER_TYPE_MSE, appId, AcbCallback);
    return res;
}

void StarfishResourceDestroy(StarfishResource *res) {
    if (res->acbId != 0) {
        AcbAPI_destroy(res->acbId);
    }
    free(res);
}

bool StarfishResourceUpdateLoadPayload(StarfishResource *resource, jvalue_ref payload, const SS4S_VideoInfo *info) {
    (void) resource;
    (void) payload;
    (void) info;
    return true;
}

bool StarfishResourceSetMediaVideoData(StarfishResource *resource, const char *data) {
    if (resource->acbId == 0) {
        return false;
    }
    AcbAPI_setMediaVideoData(resource->acbId, data);
    return true;
}

bool StarfishResourceLoadCompleted(StarfishResource *resource, const char *mediaId) {
    if (resource->acbId == 0) {
        return false;
    }
    AcbAPI_setSinkType(resource->acbId, SINK_TYPE_MAIN);
    AcbAPI_setMediaId(resource->acbId, mediaId);
    AcbAPI_setState(resource->acbId, APPSTATE_FOREGROUND, PLAYSTATE_LOADED, NULL);
    return true;
}

bool StarfishResourcePostLoad(StarfishResource *resource, const SS4S_VideoInfo *info) {
    if (resource->acbId == 0) {
        return false;
    }
    AcbAPI_setDisplayWindow(resource->acbId, 0, 0, info->width, info->height, true, NULL);
    return true;
}

bool StarfishResourceStartPlaying(StarfishResource *resource) {
    if (resource->acbId == 0) {
        return false;
    }
    AcbAPI_setState(resource->acbId, APPSTATE_FOREGROUND, PLAYSTATE_PLAYING, NULL);
    return true;
}

bool StarfishResourcePostUnload(StarfishResource *resource) {
    if (resource->acbId == 0) {
        return false;
    }
    AcbAPI_setState(resource->acbId, APPSTATE_FOREGROUND, PLAYSTATE_UNLOADED, NULL);
    return true;
}

static void AcbCallback(long acbId, long taskId, long eventType, long appState, long playState,
                        const char *reply) {

}