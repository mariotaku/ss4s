#include <stdlib.h>
#include <stdio.h>

#include "StarfishMediaAPIs_C.h"
#include "ss4s/modapi.h"

#include "smp_resource.h"
#include "smp_player.h"

#include <pbnjson.h>

const char *StarfishVideoCodecName(SS4S_VideoCodec codec) {
    switch (codec) {
        case SS4S_VIDEO_H264:
            return "H264";
        case SS4S_VIDEO_H265:
            return "H265";
        case SS4S_VIDEO_AV1:
            return "AV1";
        default:
            return NULL;
    }
}


static bool GetVideoCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264 | SS4S_VIDEO_H265;
    capabilities->transform = SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING;
    capabilities->hdr = true;
    return true;
}

static SS4S_VideoOpenResult VideoOpen(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                      SS4S_VideoInstance **instance, SS4S_PlayerContext *context) {
    (void) extraInfo;
    if (context == NULL) {
        return SS4S_VIDEO_OPEN_ERROR;
    }
    StarfishPlayerLock(context);
    context->videoInfo = *info;
    context->hasVideo = true;
    *instance = (void *) context;
    if (!context->hasAudio && context->waitAudioVideoReady) {
        StarfishLibContext->Log(SS4S_LogLevelInfo, "SMP", "VideoOpen: defer loading until audio is ready");
        StarfishPlayerUnlock(context);
        return SS4S_VIDEO_OPEN_OK;
    }
    if (context->state != SMP_STATE_UNLOADED) {
        StarfishPlayerUnloadInner(context);
    }
    if (StarfishPlayerLoadInner(context)) {
        StarfishPlayerUnlock(context);
        return SS4S_VIDEO_OPEN_OK;
    }
    StarfishPlayerUnlock(context);
    return SS4S_VIDEO_OPEN_ERROR;
}

static void VideoClose(SS4S_VideoInstance *instance) {
    assert(instance != NULL);
    SS4S_PlayerContext *context = (SS4S_PlayerContext *) instance;
    StarfishPlayerLock(context);
    StarfishPlayerUnloadInner(context);
    context->hasVideo = false;
    StarfishPlayerUnlock(context);
}


static SS4S_VideoFeedResult VideoFeed(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                      SS4S_VideoFeedFlags flags) {
    (void) flags;
    switch (StarfishPlayerFeed((SS4S_PlayerContext *) instance, data, size, 1)) {
        case SMP_FEED_OK:
            return SS4S_VIDEO_FEED_OK;
        case SMP_FEED_NOT_READY:
            return SS4S_VIDEO_FEED_NOT_READY;
        case SMP_FEED_BUFFER_FULL:
            return SS4S_VIDEO_FEED_REQUEST_KEYFRAME;
        default:
            return SS4S_VIDEO_FEED_ERROR;
    }
}

static bool SizeChanged(SS4S_VideoInstance *instance, int width, int height) {
    SS4S_PlayerContext *ctx = (SS4S_PlayerContext *) instance;
    int aspectRatio = width * 100 / height;
    StarfishPlayerLock(ctx);
    if (ctx->aspectRatio == aspectRatio) {
        StarfishPlayerUnlock(ctx);
        return true;
    }
    StarfishPlayerUnloadInner(ctx);
    if (!StarfishPlayerLoadInner(ctx)) {
        StarfishPlayerUnlock(ctx);
        return false;
    }
    ctx->aspectRatio = aspectRatio;
    StarfishPlayerUnlock(ctx);
    return true;
}

static bool SetHDRInfo(SS4S_VideoInstance *instance, const SS4S_VideoHDRInfo *info) {
    SS4S_PlayerContext *ctx = (SS4S_PlayerContext *) instance;
    bool hdr = info != NULL;
    StarfishPlayerLock(ctx);
    bool shouldReload = ctx->hdr && !hdr;
    ctx->hdr = hdr;
    if (shouldReload) {
        ctx->shouldStop = true;
        StarfishPlayerUnlock(ctx);
        return true;
    }
    if (!hdr) {
        StarfishPlayerUnlock(ctx);
        return true;
    }
    assert(info != NULL);
    jvalue_ref info_json = jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("hdrType"), J_CSTR_TO_JVAL("HDR10")),
            jkeyval(J_CSTR_TO_JVAL("sei"), jobject_create_var(
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesX0"), jnumber_create_i32(info->displayPrimariesX.b)),
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesY0"), jnumber_create_i32(info->displayPrimariesY.b)),
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesX1"), jnumber_create_i32(info->displayPrimariesX.g)),
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesY1"), jnumber_create_i32(info->displayPrimariesY.g)),
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesX2"), jnumber_create_i32(info->displayPrimariesX.r)),
                    jkeyval(J_CSTR_TO_JVAL("displayPrimariesY2"), jnumber_create_i32(info->displayPrimariesY.r)),
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
    StarfishLibContext->Log(SS4S_LogLevelInfo, "SMP", "SetHDRInfo(payload=%s)", payload);
    bool result = StarfishMediaAPIs_setHdrInfo(ctx->api, payload);
    j_release(&info_json);
    StarfishPlayerUnlock(ctx);
    return result;
}

static bool SetDisplayArea(SS4S_VideoInstance *ctx, const SS4S_VideoRect *src, const SS4S_VideoRect *dst) {
    return true;
}

const SS4S_VideoDriver StarfishVideoDriver = {
        .Base = {},
        .GetCapabilities = GetVideoCapabilities,
        .Open = VideoOpen,
        .Feed = VideoFeed,
        .SizeChanged = SizeChanged,
        .SetHDRInfo = SetHDRInfo,
        .SetDisplayArea = SetDisplayArea,
        .Close = VideoClose,
};