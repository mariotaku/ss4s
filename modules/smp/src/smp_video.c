#include <stdlib.h>
#include <stdio.h>

#include "smp_video.h"
#include "smp_resource.h"

#include "pbnjson.h"

struct SS4S_VideoInstance {
    StarfishMediaAPIs_C *api;
    StarfishResource *res;
    char *appId;

    SS4S_LoggingFunction *log;
};

static const char *CodecName(SS4S_VideoCodec codec);

static jvalue_ref MakePayload(StarfishVideo *ctx, const SS4S_VideoInfo *info);

static void LoadCallback(int type, int64_t numValue, const char *strValue, void *data);

StarfishVideo *StarfishVideoCreate(SS4S_LoggingFunction *log) {
    const char *appId = getenv("APPID");
    if (appId == NULL) {
        return NULL;
    }
    StarfishVideo *ctx = calloc(1, sizeof(StarfishVideo));
    ctx->log = log;
    ctx->appId = strdup(appId);
    ctx->res = StarfishResourceCreate();
    return ctx;
}

void StarfishVideoDestroy(StarfishVideo *ctx) {
    if (ctx->api != NULL) {
        StarfishMediaAPIs_destroy(ctx->api);
    }
    StarfishResourceDestroy(ctx->res);
    free(ctx->appId);
    free(ctx);
}

bool StarfishVideoCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264 | SS4S_VIDEO_H265;
    capabilities->transform = SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
    return true;
}

SS4S_VideoOpenResult StarfishVideoLoad(StarfishVideo *ctx, const SS4S_VideoInfo *info) {
    SS4S_VideoOpenResult result = SS4S_VIDEO_OPEN_ERROR;
    if (ctx->api == NULL) {
        ctx->api = StarfishMediaAPIs_create(NULL);
    }
    StarfishMediaAPIs_notifyForeground(ctx->api);
    jvalue_ref payload = MakePayload(ctx, info);
    const char *payload_str = jvalue_stringify(payload);
    ctx->log(SS4S_LogLevelInfo, "SMP", "Load(payload=%s)", payload_str);
    if (StarfishMediaAPIs_load(ctx->api, payload_str, LoadCallback, ctx)) {
        result = SS4S_VIDEO_OPEN_OK;
        ctx->log(SS4S_LogLevelInfo, "SMP", "Media loaded");
        StarfishResourcePostLoad(ctx->res, info);
    }
    j_release(&payload);

    return result;
}

bool StarfishVideoUnload(StarfishVideo *ctx) {
    StarfishMediaAPIs_pushEOS(ctx->api);
    StarfishMediaAPIs_unload(ctx->api);
    return true;
}

SS4S_VideoFeedResult StarfishVideoFeed(StarfishVideo *ctx, const unsigned char *data, size_t size,
                                       SS4S_VideoFeedFlags flags) {

    char payload[256], result[256];
    snprintf(payload, sizeof(payload), "{\"bufferAddr\":\"%p\",\"bufferSize\":%u,\"pts\":%llu,\"esData\":%d}",
             data, size, 0LL, 1);
    StarfishMediaAPIs_feed(ctx->api, payload, result, 256);
    if (strstr(result, "Ok") == NULL) {
        if (strstr(result, "BufferFull") != NULL) {
            return SS4S_VIDEO_FEED_OK;
        }
        return SS4S_VIDEO_FEED_ERROR;
    }
    return SS4S_VIDEO_FEED_OK;
}

bool StarfishVideoSizeChanged(StarfishVideo *ctx, int width, int height) {
    return true;
}

bool StarfishVideoSetHDRInfo(StarfishVideo *ctx, const SS4S_VideoHDRInfo *info) {
    return true;
}

bool StarfishVideoSetDisplayArea(StarfishVideo *ctx, const SS4S_VideoRect *src, const SS4S_VideoRect *dst) {
    return true;
}

static const char *CodecName(SS4S_VideoCodec codec) {
    switch (codec) {
        case SS4S_VIDEO_H264:
            return "H264";
        case SS4S_VIDEO_H265:
            return "H265";
        default:
            return NULL;
    }
}

static jvalue_ref MakePayload(StarfishVideo *ctx, const SS4S_VideoInfo *info) {
    const char *videoCodec = CodecName(info->codec);
    if (videoCodec == NULL) {
        return NULL;
    }
    jvalue_ref option = jobject_create();
    jobject_set(option, J_CSTR_TO_BUF("externalStreamingInfo"), jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("contents"), jobject_create_var(
                    jkeyval(J_CSTR_TO_JVAL("codec"), jobject_create_var(
                            jkeyval(J_CSTR_TO_JVAL("video"), jstring_create(videoCodec)),
                            J_END_OBJ_DECL
                    )),
                    jkeyval(J_CSTR_TO_JVAL("esInfo"), jobject_create_var(
                            jkeyval(J_CSTR_TO_JVAL("ptsToDecode"), jnumber_create_i32(0)),
                            jkeyval(J_CSTR_TO_JVAL("seperatedPTS"), jboolean_true()),
                            jkeyval(J_CSTR_TO_JVAL("pauseAtDecodeTime"), jboolean_true()),
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
    jobject_set(option, J_CSTR_TO_BUF("appId"), jstring_create(ctx->appId));
    jobject_set(option, J_CSTR_TO_BUF("transmission"), jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("contentsType"), J_CSTR_TO_JVAL("WEBRTC")),
            J_END_OBJ_DECL
    ));
    jobject_set(option, J_CSTR_TO_BUF("adaptiveStreaming"), jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("maxWidth"), jnumber_create_i32(info->width)),
            jkeyval(J_CSTR_TO_JVAL("maxHeight"), jnumber_create_i32(info->height)),
            jkeyval(J_CSTR_TO_JVAL("maxFrameRate"), jnumber_create_i32(60)),
            J_END_OBJ_DECL
    ));
    jobject_set(option, J_CSTR_TO_BUF("needAudio"), jboolean_false());
    jobject_set(option, J_CSTR_TO_BUF("lowDelayMode"), jboolean_true());

    jvalue_ref arg = jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("mediaTransportType"), J_CSTR_TO_JVAL("BUFFERSTREAM")),
            jkeyval(J_CSTR_TO_JVAL("option"), option),
            J_END_OBJ_DECL
    );
    jvalue_ref payload = jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("args"), jarray_create_var(NULL, arg, J_END_ARRAY_DECL)),
            J_END_OBJ_DECL
    );
    StarfishResourceUpdateLoadPayload(ctx->res, payload, info);
    return payload;
}

static void LoadCallback(int type, int64_t numValue, const char *strValue, void *data) {
    StarfishVideo *ctx = data;
    switch (type) {
        case 0:
            break;
        case STARFISH_EVENT_STR_ERROR:
            ctx->log(SS4S_LogLevelWarn, "SMP", "LoadCallback STARFISH_EVENT_STR_ERROR, numValue: %lld, strValue: %p\n",
                     numValue, strValue);
            break;
        case STARFISH_EVENT_INT_ERROR: {
            ctx->log(SS4S_LogLevelWarn, "SMP", "LoadCallback STARFISH_EVENT_INT_ERROR, numValue: %lld, strValue: %p\n",
                     numValue, strValue);
            break;
        }
        case STARFISH_EVENT_STR_BUFFERFULL:
            ctx->log(SS4S_LogLevelWarn, "SMP", "LoadCallback STARFISH_EVENT_STR_BUFFERFULL\n");
            break;
        case STARFISH_EVENT_STR_STATE_UPDATE_LOADCOMPLETED:
            StarfishMediaAPIs_play(ctx->api);
            break;
        case STARFISH_EVENT_STR_STATE_UPDATE_PLAYING:
            break;
        case STARFISH_EVENT_STR_VIDEO_INFO:
            break;
        case STARFISH_EVENT_INT_SVP_VDEC_READY:
            break;
        default:
            ctx->log(SS4S_LogLevelWarn, "SMP", "LoadCallback unhandled 0x%02x\n", type);
            break;
    }
}