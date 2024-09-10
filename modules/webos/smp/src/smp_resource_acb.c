#include "smp_resource.h"
#include "smp_player.h"

#include <AcbAPI.h>

struct StarfishResource {
    long acbId;
    long taskId;
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
    if (!AcbAPI_initialize(res->acbId, PLAYER_TYPE_MSE, appId, AcbCallback)) {
        free(res);
        return NULL;
    }
    return res;
}

void StarfishResourceDestroy(StarfishResource *res) {
    if (res->acbId != 0) {
        AcbAPI_destroy(res->acbId);
    }
    free(res);
}

bool StarfishResourcePopulateLoadPayload(StarfishResource *resource, jvalue_ref arg, const SS4S_AudioInfo *audioInfo,
                                         const SS4S_VideoInfo *videoInfo) {
    (void) resource;
    (void) arg;
    (void) audioInfo;
    (void) videoInfo;
    return true;
}

bool StarfishResourceSetMediaId(StarfishResource *resource, const char *connId) {
    if (resource->acbId == 0) {
        return false;
    }
    StarfishLibContext->Log(SS4S_LogLevelInfo, "StarfishResource", "SetMediaId: %s", connId);
    AcbAPI_setMediaId(resource->acbId, connId);
    return true;
}

bool StarfishResourceSetMediaAudioData(StarfishResource *resource, const char *data) {
    if (resource->acbId == 0) {
        return false;
    }
    StarfishLibContext->Log(SS4S_LogLevelInfo, "StarfishResource", "SetMediaAudioData: %s", data);
    AcbAPI_setMediaAudioData(resource->acbId, data, &resource->taskId);
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
    StarfishLibContext->Log(SS4S_LogLevelInfo, "StarfishResource", "SetMediaVideoData: %s", modified_info);
#ifdef HAVE_SETMEDIAAUDIODATA
    AcbAPI_setMediaVideoData(resource->acbId, modified_info, NULL);
#else
    AcbAPI_setMediaVideoData(resource->acbId, modified_info, &resource->taskId);
#endif

    jdomparser_release(&parser);
    return true;
}

bool StarfishResourceLoadCompleted(StarfishResource *resource, const char *mediaId) {
    if (resource->acbId == 0) {
        return false;
    }
    StarfishLibContext->Log(SS4S_LogLevelInfo, "StarfishResource", "LoadCompleted: %s", mediaId);
    AcbAPI_setSinkType(resource->acbId, SINK_TYPE_MAIN);
    AcbAPI_setState(resource->acbId, APPSTATE_FOREGROUND, PLAYSTATE_LOADED, &resource->taskId);
    return true;
}

bool StarfishResourcePostLoad(StarfishResource *resource, const SS4S_VideoInfo *info) {
    if (resource->acbId == 0) {
        return false;
    }
    AcbAPI_setDisplayWindow(resource->acbId, 0, 0, info->width, info->height, true, &resource->taskId);
    return true;
}

bool StarfishResourceStartPlaying(StarfishResource *resource) {
    if (resource->acbId == 0) {
        return false;
    }
    AcbAPI_setState(resource->acbId, APPSTATE_FOREGROUND, PLAYSTATE_PLAYING, &resource->taskId);
    return true;
}

bool StarfishResourcePostUnload(StarfishResource *resource) {
    if (resource->acbId == 0) {
        return false;
    }
    AcbAPI_setState(resource->acbId, APPSTATE_FOREGROUND, PLAYSTATE_UNLOADED, &resource->taskId);
    return true;
}

static void AcbCallback(long acbId, long taskId, long eventType, long appState, long playState,
                        const char *reply) {
    StarfishLibContext->Log(SS4S_LogLevelInfo, "AcbCallback",
                            "acbId: %ld, taskId: %ld, eventType: %ld, appState: %ld, playState: %ld, reply: %s",
                            acbId, taskId, eventType, appState, playState, reply);
}