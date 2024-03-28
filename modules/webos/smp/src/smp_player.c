#include "smp_player.h"
#include "smp_resource.h"
#include "StarfishMediaAPIs_C.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

static void LoadCallback(int type, int64_t numValue, const char *strValue, void *data);

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
    jvalue_ref payload = StarfishResourceMakeLoadPayload(ctx->res, ctx->hasAudio ? &ctx->audioInfo : NULL,
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
        case STARFISH_EVENT_STR_BUFFERFULL:
            StarfishLibContext->Log(SS4S_LogLevelWarn, "SMP", "LoadCallback STARFISH_EVENT_STR_BUFFERFULL\n");
            break;
        case STARFISH_EVENT_STR_STATE_UPDATE_LOADCOMPLETED: {
            StarfishPlayerLock(ctx);
            StarfishResourceLoadCompleted(ctx->res, StarfishMediaAPIs_getMediaID(ctx->api));
            StarfishMediaAPIs_play(ctx->api);
            StarfishPlayerUnlock(ctx);
            break;
        }
        case STARFISH_EVENT_STR_STATE_UPDATE_UNLOADCOMPLETED: {
            StarfishLibContext->Log(SS4S_LogLevelWarn, "SMP",
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
            StarfishLibContext->Log(SS4S_LogLevelWarn, "SMP", "LoadCallback unhandled 0x%02x\n", type);
            break;
    }
}


const SS4S_PlayerDriver StarfishPlayerDriver = {
        .Create = CreatePlayer,
        .Destroy = DestroyPlayer,
        .SetWaitAudioVideoReady = SetWaitAudioVideoReady,
};
