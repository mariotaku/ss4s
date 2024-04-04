#include "smp_resource.h"
#include "smp_player.h"

#include <SDL.h>

struct StarfishResource {
    const char *windowId;
    int maxRefreshRate;
};

StarfishResource *StarfishResourceCreate(const char *appId) {
    (void) appId;
    StarfishResource *res = calloc(1, sizeof(StarfishResource));
    assert(res != NULL);
    res->windowId = SDL_webOSCreateExportedWindow(0);
    if (res->windowId == NULL || res->windowId[0] == '\0') {
        StarfishLibContext->Log(SS4S_LogLevelError, "SMP", "Didn't get a valid windowId: %s", res->windowId);
        free(res);
        return NULL;
    }
    if (!SDL_webOSGetRefreshRate(&res->maxRefreshRate)) {
        res->maxRefreshRate = 60;
    }
    return res;
}

void StarfishResourceDestroy(StarfishResource *res) {
    if (res->windowId != NULL) {
        SDL_webOSDestroyExportedWindow(res->windowId);
    }
    free(res);
}

bool StarfishResourcePopulateLoadPayload(StarfishResource *resource, jvalue_ref arg,
                                         const SS4S_AudioInfo *audioInfo, const SS4S_VideoInfo *videoInfo) {
    (void) audioInfo;
    (void) videoInfo;
    jvalue_ref option = jobject_get(arg, J_CSTR_TO_BUF("option"));
    jobject_set(option, J_CSTR_TO_BUF("windowId"), j_cstr_to_jval(resource->windowId));
    return true;
}

bool StarfishResourceSetMediaVideoData(StarfishResource *resource, const char *data, bool hdr) {
    (void) resource;
    (void) data;
    (void) hdr;
    return true;
}

bool StarfishResourceLoadCompleted(StarfishResource *resource, const char *mediaId) {
    (void) resource;
    (void) mediaId;
    return true;
}

bool StarfishResourcePostLoad(StarfishResource *resource, const SS4S_VideoInfo *info) {
    if (resource->windowId == NULL) {
        return false;
    }
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    SDL_Rect src = {0, 0, info->width, info->height};
    SDL_Rect dst = {0, 0, dm.w, dm.h};
    SDL_webOSSetExportedWindow(resource->windowId, &src, &dst);
    return true;
}

bool StarfishResourceStartPlaying(StarfishResource *resource) {
    (void) resource;
    return true;
}

bool StarfishResourcePostUnload(StarfishResource *resource) {
    (void) resource;
    return true;
}