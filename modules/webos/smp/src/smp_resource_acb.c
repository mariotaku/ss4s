#include "smp_resource.h"
#include "smp_player.h"

#include <AcbAPI.h>

struct StarfishResource {
    const char *appId;
    long acbId;
};

static void AcbCallback(long acbId, long taskId, long eventType, long appState, long playState,
                        const char *reply);

StarfishResource *StarfishResourceCreate(const char *appId) {
    StarfishResource *res = calloc(1, sizeof(StarfishResource));
    res->appId = appId;
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

jvalue_ref StarfishResourceMakeLoadPayload(StarfishResource *resource, const SS4S_AudioInfo *audioInfo,
                                           const SS4S_VideoInfo *videoInfo) {
    const char *audioCodec, *videoCodec;
    if (audioInfo != NULL) {
        audioCodec = StarfishAudioCodecName(audioInfo->codec);
        if (audioCodec == NULL) {
            return NULL;
        }
    }
    if (videoInfo != NULL) {
        videoCodec = StarfishVideoCodecName(videoInfo->codec);
        if (videoCodec == NULL) {
            return NULL;
        }
    }
    jvalue_ref codec = jobject_create();
    if (audioInfo) {
        jobject_set(codec, J_CSTR_TO_BUF("audio"), jstring_create(audioCodec));
    }
    if (videoInfo) {
        jobject_set(codec, J_CSTR_TO_BUF("video"), jstring_create(videoCodec));
    }

    jvalue_ref option = jobject_create();
    jobject_set(option, J_CSTR_TO_BUF("externalStreamingInfo"), jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("contents"), jobject_create_var(
                    jkeyval(J_CSTR_TO_JVAL("codec"), codec),
                    jkeyval(J_CSTR_TO_JVAL("esInfo"), jobject_create_var(
                            jkeyval(J_CSTR_TO_JVAL("ptsToDecode"), jnumber_create_i32(0)),
                            jkeyval(J_CSTR_TO_JVAL("seperatedPTS"), jboolean_create(true)),
                            jkeyval(J_CSTR_TO_JVAL("pauseAtDecodeTime"), jboolean_create(true)),
                            J_END_OBJ_DECL
                    )),
                    jkeyval(J_CSTR_TO_JVAL("format"), J_CSTR_TO_JVAL("RAW")),
                    J_END_OBJ_DECL
            )),
            jkeyval(J_CSTR_TO_JVAL("bufferingCtrInfo"), jobject_create_var(
                    jkeyval(J_CSTR_TO_JVAL("srcBufferLevelVideo"), jobject_create_var(
                            jkeyval(J_CSTR_TO_JVAL("maximum"), jnumber_create_i32(2097152)),
                            jkeyval(J_CSTR_TO_JVAL("minimum"), jnumber_create_i32(1)),
                            J_END_OBJ_DECL
                    )),
                    jkeyval(J_CSTR_TO_JVAL("qBufferLevelVideo"), jnumber_create_i32(0)),
                    jkeyval(J_CSTR_TO_JVAL("qBufferLevelAudio"), jnumber_create_i32(0)),
                    jkeyval(J_CSTR_TO_JVAL("bufferMinLevel"), jnumber_create_i32(0)),
                    jkeyval(J_CSTR_TO_JVAL("bufferMaxLevel"), jnumber_create_i32(0)),
                    jkeyval(J_CSTR_TO_JVAL("preBufferByte"), jnumber_create_i32(0)),
                    jkeyval(J_CSTR_TO_JVAL("srcBufferLevelAudio"), jobject_create_var(
                            jkeyval(J_CSTR_TO_JVAL("maximum"), jnumber_create_i32(10)),
                            jkeyval(J_CSTR_TO_JVAL("minimum"), jnumber_create_i32(1)),
                            J_END_OBJ_DECL
                    )),
                    J_END_OBJ_DECL
            )),
            J_END_OBJ_DECL
    ));
    jobject_set(option, J_CSTR_TO_BUF("appId"), jstring_create(resource->appId));
    jobject_set(option, J_CSTR_TO_BUF("transmission"), jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("contentsType"), J_CSTR_TO_JVAL("WEBRTC")),
            J_END_OBJ_DECL
    ));
    if (videoInfo) {
        int frameRate = 6000;
        if (videoInfo->frameRateNumerator != 0 && videoInfo->frameRateDenominator != 0) {
            frameRate = videoInfo->frameRateNumerator * 100 / videoInfo->frameRateDenominator;
        }
        jobject_set(option, J_CSTR_TO_BUF("adaptiveStreaming"), jobject_create_var(
                jkeyval(J_CSTR_TO_JVAL("maxWidth"), jnumber_create_i32(videoInfo->width)),
                jkeyval(J_CSTR_TO_JVAL("maxHeight"), jnumber_create_i32(videoInfo->height)),
                jkeyval(J_CSTR_TO_JVAL("maxFrameRate"), jnumber_create_f64(frameRate / 100.0)),
                J_END_OBJ_DECL
        ));
    }
    jobject_set(option, J_CSTR_TO_BUF("needAudio"), jboolean_create(false));
    jobject_set(option, J_CSTR_TO_BUF("lowDelayMode"), jboolean_create(true));

    jvalue_ref arg = jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("mediaTransportType"), J_CSTR_TO_JVAL("BUFFERSTREAM")),
            jkeyval(J_CSTR_TO_JVAL("option"), option),
            J_END_OBJ_DECL
    );
    return jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("args"), jarray_create_var(NULL, arg, J_END_ARRAY_DECL)),
            J_END_OBJ_DECL
    );
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