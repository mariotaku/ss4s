#include <stdlib.h>
#include <stdio.h>

#include "smp_video.h"
#include "smp_resource.h"

#include <pbnjson.h>

typedef enum PlayerState {
    SMP_STATE_UNLOADED,
    SMP_STATE_LOADED,
    SMP_STATE_PLAYING,
} PlayerState;

struct SS4S_VideoInstance {
    pthread_mutex_t lock;
    StarfishMediaAPIs_C *api;
    StarfishResource *res;
    char *appId;
    SS4S_VideoInfo info;
    PlayerState state;
    int aspectRatio;
    bool hdr, shouldStop;
    SS4S_LoggingFunction *log;
};

static const char *CodecName(SS4S_VideoCodec codec);

static jvalue_ref MakePayload(StarfishVideo *ctx, const SS4S_VideoInfo *info);

static void LoadCallback(int type, int64_t numValue, const char *strValue, void *data);

static void StarfishLock(StarfishVideo *ctx);

static void StarfishUnlock(StarfishVideo *ctx);

StarfishVideo *StarfishVideoCreate(SS4S_LoggingFunction *log) {
    const char *appId = getenv("APPID");
    if (appId == NULL) {
        return NULL;
    }
    StarfishVideo *ctx = calloc(1, sizeof(StarfishVideo));
    log(SS4S_LogLevelInfo, "SMP", "StarfishVideo = %p", ctx);
    pthread_mutex_init(&ctx->lock, NULL);
    ctx->log = log;
    ctx->appId = strdup(appId);
    ctx->res = StarfishResourceCreate(ctx->appId, log);
    ctx->state = SMP_STATE_UNLOADED;
    return ctx;
}

void StarfishVideoDestroy(StarfishVideo *ctx) {
    if (ctx->api != NULL) {
        StarfishMediaAPIs_destroy(ctx->api);
    }
    StarfishResourceDestroy(ctx->res);
    free(ctx->appId);
    pthread_mutex_destroy(&ctx->lock);
    free(ctx);
}

bool StarfishVideoCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264 | SS4S_VIDEO_H265;
    capabilities->transform = SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
    capabilities->hdr = true;
    return true;
}

SS4S_VideoOpenResult StarfishVideoLoad(StarfishVideo *ctx, const SS4S_VideoInfo *info) {
    StarfishLock(ctx);
    if (ctx->state != SMP_STATE_UNLOADED) {
        StarfishUnlock(ctx);
        return SS4S_VIDEO_OPEN_ERROR;
    }
    if (ctx->api == NULL && (ctx->api = StarfishMediaAPIs_create(NULL)) == NULL) {
        StarfishUnlock(ctx);
        return SS4S_VIDEO_OPEN_ERROR;
    }
    SS4S_VideoOpenResult result = SS4S_VIDEO_OPEN_ERROR;
    StarfishMediaAPIs_notifyForeground(ctx->api);
    jvalue_ref payload = MakePayload(ctx, info);
    const char *payload_str = jvalue_stringify(payload);
    ctx->log(SS4S_LogLevelInfo, "SMP", "Load(payload=%s)", payload_str);
    if (StarfishMediaAPIs_load(ctx->api, payload_str, LoadCallback, ctx)) {
        result = SS4S_VIDEO_OPEN_OK;
        ctx->log(SS4S_LogLevelInfo, "SMP", "Media loaded");
        ctx->state = SMP_STATE_LOADED;
        StarfishResourcePostLoad(ctx->res, info);
    } else {
        ctx->log(SS4S_LogLevelError, "SMP", "Media load failed");
    }
    j_release(&payload);
    ctx->info = *info;

    StarfishUnlock(ctx);
    return result;
}

bool StarfishVideoUnload(StarfishVideo *ctx) {
    StarfishLock(ctx);
    PlayerState state = ctx->state;
    if (state != SMP_STATE_LOADED && state != SMP_STATE_PLAYING) {
        StarfishUnlock(ctx);
        return false;
    }
    ctx->state = SMP_STATE_UNLOADED;
    if (state == SMP_STATE_PLAYING) {
        StarfishMediaAPIs_pushEOS(ctx->api);
    }
    StarfishMediaAPIs_unload(ctx->api);
    StarfishResourcePostUnload(ctx->res);
    StarfishUnlock(ctx);
    return true;
}

SS4S_VideoFeedResult StarfishVideoFeed(StarfishVideo *ctx, const unsigned char *data, size_t size,
                                       SS4S_VideoFeedFlags flags) {
    StarfishLock(ctx);
    if (ctx->shouldStop) {
        StarfishUnlock(ctx);
        return SS4S_VIDEO_FEED_ERROR;
    }
    if (ctx->state == SMP_STATE_UNLOADED) {
        StarfishUnlock(ctx);
        return SS4S_VIDEO_FEED_NOT_READY;
    }
    char payload[256], result[256];
    snprintf(payload, sizeof(payload), "{\"bufferAddr\":\"%p\",\"bufferSize\":%u,\"pts\":%llu,\"esData\":%d}",
             data, size, 0LL, 1);
    StarfishMediaAPIs_feed(ctx->api, payload, result, 256);
    if (strstr(result, "Ok") == NULL) {
        StarfishUnlock(ctx);
        if (strstr(result, "BufferFull") != NULL) {
            return SS4S_VIDEO_FEED_OK;
        }
        return SS4S_VIDEO_FEED_ERROR;
    }
    if (ctx->state == SMP_STATE_LOADED) {
        ctx->state = SMP_STATE_PLAYING;
        StarfishResourceStartPlaying(ctx->res);
    }
    StarfishUnlock(ctx);
    return SS4S_VIDEO_FEED_OK;
}

bool StarfishVideoSizeChanged(StarfishVideo *ctx, int width, int height) {
    int aspectRatio = width * 100 / height;
    StarfishLock(ctx);
    if (ctx->aspectRatio == aspectRatio) {
        StarfishUnlock(ctx);
        return true;
    }

    SS4S_VideoInfo newInfo = ctx->info;
    newInfo.width = width;
    newInfo.height = width;
    StarfishUnlock(ctx);
    StarfishVideoUnload(ctx);
    if (!StarfishVideoLoad(ctx, &newInfo)) {
        return false;
    }
    StarfishLock(ctx);
    ctx->aspectRatio = aspectRatio;
    StarfishUnlock(ctx);
    return true;
}

bool StarfishVideoSetHDRInfo(StarfishVideo *ctx, const SS4S_VideoHDRInfo *info) {
    bool hdr = info != NULL;
    StarfishLock(ctx);
    bool shouldReload = ctx->hdr && !hdr;
    ctx->hdr = hdr;
    if (shouldReload) {
        ctx->shouldStop = true;
        StarfishUnlock(ctx);
        return true;
    }
    if (!hdr) {
        StarfishUnlock(ctx);
        return true;
    }
    jvalue_ref info_json = jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("hdrType"), J_CSTR_TO_JVAL("HDR10")),
            jkeyval(J_CSTR_TO_JVAL("sei"), jobject_create_var(
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesX0"), jnumber_create_i32(info->displayPrimariesX[0])),
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesY0"), jnumber_create_i32(info->displayPrimariesY[0])),
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesX1"), jnumber_create_i32(info->displayPrimariesX[1])),
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesY1"), jnumber_create_i32(info->displayPrimariesY[1])),
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesX2"), jnumber_create_i32(info->displayPrimariesX[2])),
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesY2"), jnumber_create_i32(info->displayPrimariesY[2])),
                    jkeyval(J_CSTR_TO_JVAL("whitePointX"), jnumber_create_i32(info->whitePointX)),
                    jkeyval(J_CSTR_TO_JVAL("whitePointY"), jnumber_create_i32(info->whitePointY)),
                    jkeyval(J_CSTR_TO_JVAL("minDisplayMasteringLuminance"),
                            jnumber_create_i32(info->minDisplayMasteringLuminance)),
                    jkeyval(J_CSTR_TO_JVAL("maxDisplayMasteringLuminance"),
                            jnumber_create_i32(info->maxDisplayMasteringLuminance)),
                    jkeyval(J_CSTR_TO_JVAL("maxContentLightLevel"), jnumber_create_i32(info->maxContentLightLevel)),
                    jkeyval(J_CSTR_TO_JVAL("maxPicAverageLightLevel"),
                            jnumber_create_i32(info->maxPicAverageLightLevel)),
                    J_END_OBJ_DECL)),
            J_END_OBJ_DECL
    );
    const char *payload = jvalue_stringify(info_json);
    ctx->log(SS4S_LogLevelInfo, "SMP", "SetHDRInfo(payload=%s)", payload);
    bool result = StarfishMediaAPIs_setHdrInfo(ctx->api, payload);
    j_release(&info_json);
    StarfishUnlock(ctx);
    return result;
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
    jobject_set(option, J_CSTR_TO_BUF("needAudio"), jboolean_create(false));
    jobject_set(option, J_CSTR_TO_BUF("lowDelayMode"), jboolean_create(true));

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
            StarfishLock(ctx);
            StarfishResourceLoadCompleted(ctx->res, StarfishMediaAPIs_getMediaID(ctx->api));
            StarfishMediaAPIs_play(ctx->api);
            StarfishUnlock(ctx);
            break;
        case STARFISH_EVENT_STR_STATE_UPDATE_UNLOADCOMPLETED:
            ctx->log(SS4S_LogLevelWarn, "SMP", "LoadCallback STARFISH_EVENT_STR_STATE_UPDATE_UNLOADCOMPLETED\n");
            break;
        case STARFISH_EVENT_STR_STATE_UPDATE_PLAYING:
            break;
        case STARFISH_EVENT_STR_VIDEO_INFO:
            StarfishLock(ctx);
            StarfishResourceSetMediaVideoData(ctx->res, strValue, ctx->hdr);
            StarfishUnlock(ctx);
            break;
        case STARFISH_EVENT_INT_SVP_VDEC_READY:
            break;
        default:
            ctx->log(SS4S_LogLevelWarn, "SMP", "LoadCallback unhandled 0x%02x\n", type);
            break;
    }
}

static void StarfishLock(StarfishVideo *ctx) {
    pthread_mutex_lock(&ctx->lock);
}

static void StarfishUnlock(StarfishVideo *ctx) {
    pthread_mutex_unlock(&ctx->lock);
}
