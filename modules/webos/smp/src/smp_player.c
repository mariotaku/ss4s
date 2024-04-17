#include "smp_player.h"
#include "smp_resource.h"
#include "StarfishMediaAPIs_C.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

static void LoadCallback(int type, int64_t numValue, const char *strValue, void *data);

static jvalue_ref MakeLoadPayload(SS4S_PlayerContext *ctx, const SS4S_AudioInfo *audioInfo,
                                  const SS4S_VideoInfo *videoInfo);

static jvalue_ref AudioCreatePcmInfo(const SS4S_AudioInfo *audioInfo);

static jvalue_ref AudioCreateOpusInfo(const SS4S_AudioInfo *audioInfo);

static jvalue_ref AudioCreateAacInfo(const SS4S_AudioInfo *audioInfo);

static jvalue_ref AudioCreateAc3PlusInfo(const SS4S_AudioInfo *audioInfo);

static SS4S_PlayerContext *CreatePlayer(SS4S_Player *player) {
    const char *appId = getenv("APPID");
    if (appId == NULL) {
        return NULL;
    }
    SS4S_PlayerContext *context = calloc(1, sizeof(SS4S_PlayerContext));
    context->appId = strdup(appId);
    pthread_mutex_init(&context->lock, NULL);
    context->player = player;

    context->res = StarfishResourceCreate(context->appId);
    if (context->res == NULL) {
        StarfishLibContext->Log(SS4S_LogLevelError, "SMP", "Failed to allocate resource");
        free(context->appId);
        pthread_mutex_destroy(&context->lock);
        free(context);
        return NULL;
    }
    context->openTime = StarfishPlayerGetTime();
    context->state = SMP_STATE_UNLOADED;
    return context;
}

static void DestroyPlayer(SS4S_PlayerContext *context) {
    pthread_mutex_lock(&context->lock);
    if (context->api != NULL) {
        StarfishMediaAPIs_destroy(context->api);
    }
    StarfishResourceDestroy(context->res);
    free(context->appId);
    pthread_mutex_unlock(&context->lock);
    pthread_mutex_destroy(&context->lock);
    free(context);
}

static void SetWaitAudioVideoReady(SS4S_PlayerContext *context, bool wait) {
    context->waitAudioVideoReady = wait;
}

bool StarfishPlayerLoadInner(SS4S_PlayerContext *ctx) {
    if (ctx->state != SMP_STATE_UNLOADED) {
        StarfishLibContext->Log(SS4S_LogLevelError, "SMP", "Media already loaded");
        return false;
    }
    if (ctx->api == NULL && (ctx->api = StarfishMediaAPIs_create(NULL)) == NULL) {
        StarfishLibContext->Log(SS4S_LogLevelError, "SMP", "Failed to instantiate media APIs");
        return false;
    }
    bool result = false;
    StarfishMediaAPIs_notifyForeground(ctx->api);
    jvalue_ref payload = MakeLoadPayload(ctx, ctx->hasAudio ? &ctx->audioInfo : NULL,
                                         ctx->hasVideo ? &ctx->videoInfo : NULL);
    const char *payload_str = jvalue_stringify(payload);
    StarfishLibContext->Log(SS4S_LogLevelInfo, "SMP", "Load(payload=%s)", payload_str);
    if (StarfishMediaAPIs_load(ctx->api, payload_str, LoadCallback, ctx)) {
        result = true;
        StarfishLibContext->Log(SS4S_LogLevelInfo, "SMP", "Media loaded");
        ctx->state = SMP_STATE_LOADED;
        StarfishResourcePostLoad(ctx->res, &ctx->videoInfo);
    } else {
        StarfishLibContext->Log(SS4S_LogLevelError, "SMP", "Media load failed");
    }
    j_release(&payload);
    return result;
}

bool StarfishPlayerUnloadInner(SS4S_PlayerContext *ctx) {
    PlayerState state = ctx->state;
    if (state != SMP_STATE_LOADED && state != SMP_STATE_PLAYING) {
        return false;
    }
    ctx->state = SMP_STATE_UNLOADED;
    if (state == SMP_STATE_PLAYING) {
        StarfishMediaAPIs_pushEOS(ctx->api);
    }
    StarfishMediaAPIs_unload(ctx->api);
    StarfishResourcePostUnload(ctx->res);
    return true;
}

FeedResult StarfishPlayerFeed(SS4S_PlayerContext *ctx, const unsigned char *data, size_t size, int esData) {
    StarfishPlayerLock(ctx);
    if (ctx->state == SMP_STATE_UNLOADED && ctx->waitAudioVideoReady) {
        StarfishPlayerUnlock(ctx);
        return SMP_FEED_OK;
    }
    if (ctx->shouldStop) {
        StarfishPlayerUnlock(ctx);
        return SMP_FEED_ERROR;
    }
    if (ctx->state == SMP_STATE_UNLOADED) {
        StarfishPlayerUnlock(ctx);
        return SMP_FEED_NOT_READY;
    }
    char payload[256], result[256];
    uint64_t diff = StarfishPlayerGetTime() - ctx->openTime;
    snprintf(payload, sizeof(payload), "{\"bufferAddr\":\"%p\",\"bufferSize\":%u,\"pts\":%llu,\"esData\":%d}",
             data, size, diff, esData);
    StarfishMediaAPIs_feed(ctx->api, payload, result, 256);
    if (strstr(result, "Ok") == NULL) {
        StarfishPlayerUnlock(ctx);
        if (strstr(result, "BufferFull") != NULL) {
            return SMP_FEED_BUFFER_FULL;
        }
        return SMP_FEED_ERROR;
    }
    if (ctx->state == SMP_STATE_LOADED) {
        ctx->state = SMP_STATE_PLAYING;
        StarfishResourceStartPlaying(ctx->res);
    }
    StarfishPlayerUnlock(ctx);
    return SMP_FEED_OK;
}

uint64_t StarfishPlayerGetTime() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec * 1000000000LL + now.tv_nsec);
}

void StarfishPlayerLock(SS4S_PlayerContext *ctx) {
    pthread_mutex_lock(&ctx->lock);
}

void StarfishPlayerUnlock(SS4S_PlayerContext *ctx) {
    pthread_mutex_unlock(&ctx->lock);
}


static void LoadCallback(int type, int64_t numValue, const char *strValue, void *data) {
    SS4S_PlayerContext *ctx = data;
    switch (type) {
        case STARFISH_EVENT_FRAMEREADY: {
            StarfishPlayerLock(ctx);
            uint64_t ptsNow = StarfishPlayerGetTime() - ctx->openTime;
            StarfishLibContext->VideoStats.ReportFrame(ctx->player, (ptsNow - numValue) / 1000);
            StarfishPlayerUnlock(ctx);
            break;
        }
        case STARFISH_EVENT_STR_ERROR:
            StarfishLibContext->Log(SS4S_LogLevelWarn, "SMP",
                                    "LoadCallback STARFISH_EVENT_STR_ERROR, numValue: %lld, strValue: %p\n",
                                    numValue, strValue);
            break;
        case STARFISH_EVENT_INT_ERROR: {
            StarfishLibContext->Log(SS4S_LogLevelWarn, "SMP",
                                    "LoadCallback STARFISH_EVENT_INT_ERROR, numValue: %lld, strValue: %p\n",
                                    numValue, strValue);
            break;
        }
        case STARFISH_EVENT_STR_AUDIO_INFO:
            StarfishLibContext->Log(SS4S_LogLevelInfo, "SMP", "LoadCallback STARFISH_EVENT_STR_AUDIO_INFO %s\n",
                                    strValue);
            break;
        case STARFISH_EVENT_INT_BUFFERLOW:
        case STARFISH_EVENT_STR_BUFFERLOW:
            break;
        case STARFISH_EVENT_STR_BUFFERFULL:
            StarfishLibContext->Log(SS4S_LogLevelWarn, "SMP", "LoadCallback STARFISH_EVENT_STR_BUFFERFULL %s",
                                    strValue);
            break;
        case STARFISH_EVENT_STR_STATE_UPDATE_LOADCOMPLETED: {
            StarfishPlayerLock(ctx);
            StarfishResourceLoadCompleted(ctx->res, StarfishMediaAPIs_getMediaID(ctx->api));
            StarfishMediaAPIs_play(ctx->api);
            StarfishPlayerUnlock(ctx);
            break;
        }
        case STARFISH_EVENT_STR_STATE_UPDATE_UNLOADCOMPLETED: {
            StarfishLibContext->Log(SS4S_LogLevelInfo, "SMP",
                                    "LoadCallback STARFISH_EVENT_STR_STATE_UPDATE_UNLOADCOMPLETED\n");
            break;
        }
        case STARFISH_EVENT_STR_STATE_UPDATE_PLAYING:
            break;
        case STARFISH_EVENT_STR_VIDEO_INFO: {
            StarfishPlayerLock(ctx);
            StarfishResourceSetMediaVideoData(ctx->res, strValue, ctx->hdr);
            StarfishPlayerUnlock(ctx);
            break;
        }
        case STARFISH_EVENT_INT_SVP_VDEC_READY:
            break;
        default:
            StarfishLibContext->Log(SS4S_LogLevelInfo, "SMP", "LoadCallback unhandled 0x%02x\n", type);
            break;
    }
}

static inline jvalue_ref CreateMinMax(int minimum, int maximum) {
    return jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("minimum"), jnumber_create_i32(minimum)),
            jkeyval(J_CSTR_TO_JVAL("maximum"), jnumber_create_i32(maximum)),
            J_END_OBJ_DECL
    );
}

jvalue_ref MakeLoadPayload(SS4S_PlayerContext *ctx, const SS4S_AudioInfo *audioInfo,
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
    jvalue_ref contents = jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("codec"), codec),
            jkeyval(J_CSTR_TO_JVAL("esInfo"), jobject_create_var(
                    jkeyval(J_CSTR_TO_JVAL("pauseAtDecodeTime"), jboolean_create(true)),
                    jkeyval(J_CSTR_TO_JVAL("ptsToDecode"), jnumber_create_i64(0)),
                    J_END_OBJ_DECL
            )),
            jkeyval(J_CSTR_TO_JVAL("format"), J_CSTR_TO_JVAL("RAW")),
            jkeyval(J_CSTR_TO_JVAL("provider"), J_CSTR_TO_JVAL("Chrome")),
            J_END_OBJ_DECL
    );
    if (videoInfo) {
        jobject_set(codec, J_CSTR_TO_BUF("video"), jstring_create(videoCodec));
    }
    if (audioInfo) {
        jobject_set(codec, J_CSTR_TO_BUF("audio"), jstring_create(audioCodec));
        switch (audioInfo->codec) {
            case SS4S_AUDIO_PCM_S16LE: {
                jobject_set(contents, J_CSTR_TO_BUF("pcmInfo"), AudioCreatePcmInfo(audioInfo));
                break;
            }
            case SS4S_AUDIO_OPUS: {
                jobject_set(contents, J_CSTR_TO_BUF("opusInfo"), AudioCreateOpusInfo(audioInfo));
                break;
            }
            case SS4S_AUDIO_AAC: {
                jobject_set(contents, J_CSTR_TO_BUF("aacInfo"), AudioCreateAacInfo(audioInfo));
                break;
            }
            case SS4S_AUDIO_AC3: {
                jobject_set(contents, J_CSTR_TO_BUF("ac3PlusInfo"), AudioCreateAc3PlusInfo(audioInfo));
                break;
            }
            default: {
                break;
            }
        }
    }


    jvalue_ref option = jobject_create();
    jobject_set(option, J_CSTR_TO_BUF("appId"), jstring_create(ctx->appId));
    jobject_set(option, J_CSTR_TO_BUF("externalStreamingInfo"), jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("contents"), contents),
            jkeyval(J_CSTR_TO_JVAL("streamQualityInfo"), jboolean_true()),
            jkeyval(J_CSTR_TO_JVAL("audioSync"), jboolean_true()),
            jkeyval(J_CSTR_TO_JVAL("bufferingCtrInfo"), jobject_create_var(
                    jkeyval(J_CSTR_TO_JVAL("preBufferByte"), jnumber_create_i32(0)),
                    jkeyval(J_CSTR_TO_JVAL("qBufferLevelAudio"), jnumber_create_i32(0)),
                    jkeyval(J_CSTR_TO_JVAL("qBufferLevelVideo"), jnumber_create_i32(0)),
                    /* This affects pipeline appsrc.
                     * A very low maximum is meaningless because it only causes the pipeline to discard buffers
                     */
                    jkeyval(J_CSTR_TO_JVAL("srcBufferLevelAudio"), CreateMinMax(1, 32768)),
                    jkeyval(J_CSTR_TO_JVAL("srcBufferLevelVideo"), CreateMinMax(1, 1048576)),
                    J_END_OBJ_DECL
            )),
            J_END_OBJ_DECL
    ));
    // Setting contentsType to WEBRTC will reduce video latency significantly
    jobject_set(option, J_CSTR_TO_BUF("transmission"), jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("contentsType"), J_CSTR_TO_JVAL("WEBRTC")),
            J_END_OBJ_DECL
    ));
    // When queryPosition is set to true, STARFISH_EVENT_FRAMEREADY will not be sent
    jobject_set(option, J_CSTR_TO_BUF("queryPosition"), jboolean_false());
//    jobject_set(option, J_CSTR_TO_BUF("bufferControl"), jobject_create_var(
//            jkeyval(J_CSTR_TO_JVAL("userBufferCtrl"), jboolean_true()),
//            jkeyval(J_CSTR_TO_JVAL("preBufferTime"), jnumber_create_i32(3)),
//            jkeyval(J_CSTR_TO_JVAL("bufferingMinTime"), jnumber_create_i32(1)),
//            jkeyval(J_CSTR_TO_JVAL("bufferingMaxTime"), jnumber_create_i32(3)),
//            J_END_OBJ_DECL
//    ));
    // Recognized on webOS 5+, doesn't seem to have any effect
    jobject_set(option, J_CSTR_TO_BUF("lowDelayMode"), jboolean_create(true));

    if (videoInfo) {
        int frameRate = 6000;
        if (videoInfo->frameRateNumerator != 0 && videoInfo->frameRateDenominator != 0) {
            frameRate = videoInfo->frameRateNumerator * 100 / videoInfo->frameRateDenominator;
        }
        jobject_set(option, J_CSTR_TO_BUF("adaptiveStreaming"), jobject_create_var(
                jkeyval(J_CSTR_TO_JVAL("audioOnly"), jboolean_false()),
                jkeyval(J_CSTR_TO_JVAL("maxWidth"), jnumber_create_i32(videoInfo->width)),
                jkeyval(J_CSTR_TO_JVAL("maxHeight"), jnumber_create_i32(videoInfo->height)),
                jkeyval(J_CSTR_TO_JVAL("maxFrameRate"), jnumber_create_f64(frameRate / 100.0)),
                J_END_OBJ_DECL
        ));
    }

    jvalue_ref arg = jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("mediaTransportType"), J_CSTR_TO_JVAL("BUFFERSTREAM")),
            jkeyval(J_CSTR_TO_JVAL("option"), option),
            J_END_OBJ_DECL
    );
    StarfishResourcePopulateLoadPayload(ctx->res, arg, ctx->hasAudio ? &ctx->audioInfo : NULL,
                                        ctx->hasVideo ? &ctx->videoInfo : NULL);
    return jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("args"), jarray_create_var(NULL, arg, J_END_ARRAY_DECL)),
            J_END_OBJ_DECL
    );
}

jvalue_ref AudioCreatePcmInfo(const SS4S_AudioInfo *audioInfo) {
    const char *channelMode = "stereo";
    if (audioInfo->numOfChannels == 1) {
        channelMode = "mono";
    } else if (audioInfo->numOfChannels == 6) {
        channelMode = "6-channel";
    }
    return jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("channelMode"), j_cstr_to_jval(channelMode)),
            jkeyval(J_CSTR_TO_JVAL("format"), j_cstr_to_jval("S16LE")),
            jkeyval(J_CSTR_TO_JVAL("sampleRate"), jnumber_create_i32(1)),
            jkeyval(J_CSTR_TO_JVAL("layout"), j_cstr_to_jval("interleaved")),
            J_END_OBJ_DECL
    );
}

jvalue_ref AudioCreateOpusInfo(const SS4S_AudioInfo *audioInfo) {
    jvalue_ref info = jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("channels"), jnumber_create_i32(audioInfo->numOfChannels)),
            jkeyval(J_CSTR_TO_JVAL("sampleRate"), jnumber_create_f64(audioInfo->sampleRate / 1000.0)),
            J_END_OBJ_DECL
    );
    if (audioInfo->codecData && audioInfo->codecDataLen > 0) {
        gchar *encoded = g_base64_encode(audioInfo->codecData, audioInfo->codecDataLen);
        jvalue_ref streamHeader = jstring_create(encoded);
        g_free(encoded);
        jobject_set(info, J_CSTR_TO_BUF("streamHeader"), streamHeader);
    }
    return info;
}

jvalue_ref AudioCreateAacInfo(const SS4S_AudioInfo *audioInfo) {
    return jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("channels"), jnumber_create_i32(audioInfo->numOfChannels)),
            jkeyval(J_CSTR_TO_JVAL("format"), j_cstr_to_jval("adts")),
            jkeyval(J_CSTR_TO_JVAL("frequency"), jnumber_create_i32(audioInfo->sampleRate / 1000)),
            jkeyval(J_CSTR_TO_JVAL("profile"), jnumber_create_i32(2)),
            J_END_OBJ_DECL
    );
}

jvalue_ref AudioCreateAc3PlusInfo(const SS4S_AudioInfo *audioInfo) {
    return jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("channels"), jnumber_create_i32(audioInfo->numOfChannels)),
            jkeyval(J_CSTR_TO_JVAL("frequency"), jnumber_create_i32(audioInfo->sampleRate / 1000)),
            J_END_OBJ_DECL
    );
}

const SS4S_PlayerDriver StarfishPlayerDriver = {
        .Create = CreatePlayer,
        .Destroy = DestroyPlayer,
        .SetWaitAudioVideoReady = SetWaitAudioVideoReady,
};
