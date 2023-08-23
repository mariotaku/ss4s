#include "smp_resource.h"

#include <AcbAPI.h>

struct StarfishResource {
    long acbId;
    SS4S_LoggingFunction *log;
};

static void AcbCallback(long acbId, long taskId, long eventType, long appState, long playState,
                        const char *reply);

StarfishResource *StarfishResourceCreate(const char *appId, SS4S_LoggingFunction *log) {
    StarfishResource *res = calloc(1, sizeof(StarfishResource));
    res->acbId = AcbAPI_create();
    res->log = log;
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

bool StarfishResourceSetMediaVideoData(StarfishResource *resource, const char *data, bool hdr) {
    if (resource->acbId == 0) {
        return false;
    }
    JSchemaInfo schema_info;
    jschema_info_init(&schema_info, jschema_all(), NULL, NULL);
    jdomparser_ref parser = jdomparser_create(&schema_info, 0);

    if (!jdomparser_feed(parser, data, (int) strlen(data))) {
        jdomparser_release(&parser);
        return false;
    }
    if (!jdomparser_end(parser)) {
        jdomparser_release(&parser);
        return false;
    }
    jvalue_ref request = jdomparser_get_result(parser);
    if (!jis_valid(request)) {
        jdomparser_release(&parser);
        return false;
    }
    jvalue_ref video = jobject_get(request, J_CSTR_TO_BUF("video"));
    if (hdr && !jobject_containskey(video, J_CSTR_TO_BUF("hdrType"))) {
        jobject_set(video, J_CSTR_TO_BUF("hdrType"), jstring_create("HDR10"));
    }

    const char *modified_info = jvalue_stringify(request);
    resource->log(SS4S_LogLevelInfo, "StarfishResource", "SetMediaVideoData: %s", modified_info);
    AcbAPI_setMediaVideoData(resource->acbId, modified_info);

    jdomparser_release(&parser);
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