#include "smp_resource.h"

#include <SDL.h>

struct StarfishResource {
    const char *windowId;
    int maxRefreshRate;
};

StarfishResource *StarfishResourceCreate(const char *appId) {
    (void) appId;
    StarfishResource *res = calloc(1, sizeof(StarfishResource));
    res->windowId = SDL_webOSCreateExportedWindow(0);
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

bool StarfishResourceUpdateLoadPayload(StarfishResource *resource, jvalue_ref payload, const SS4S_VideoInfo *info) {
    if (resource->windowId == NULL) {
        return false;
    }
    jvalue_ref args = jobject_get(payload, J_CSTR_TO_BUF("args"));
    jvalue_ref arg = jarray_get(args, 0);
    jvalue_ref option = jobject_get(arg, J_CSTR_TO_BUF("option"));
    jvalue_ref adaptiveStreaming = jobject_get(arg, J_CSTR_TO_BUF("adaptiveStreaming"));
    jobject_set(adaptiveStreaming, J_CSTR_TO_BUF("maxFrameRate"), jnumber_create_i32(resource->maxRefreshRate));
    return jobject_set(option, J_CSTR_TO_BUF("windowId"), j_cstr_to_jval(resource->windowId));
}

bool StarfishResourceSetMediaVideoData(StarfishResource *resource, const char *data) {
    return true;
}

bool StarfishResourceLoadCompleted(StarfishResource *resource, const char *mediaId) {
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
    return true;
}

bool StarfishResourcePostUnload(StarfishResource *resource) {
    return true;
}