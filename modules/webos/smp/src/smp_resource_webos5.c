#include "smp_resource.h"
#include "smp_player.h"

#include <SDL.h>

struct StarfishResource {
    const char *appId;
    const char *windowId;
    int maxRefreshRate;
};

jvalue_ref AudioCreatePcmInfo(const SS4S_AudioInfo *audioInfo);

jvalue_ref AudioCreateAacInfo(const SS4S_AudioInfo *audioInfo);

jvalue_ref AudioCreateAc3PlusInfo(const SS4S_AudioInfo *audioInfo);

StarfishResource *StarfishResourceCreate(const char *appId) {
    (void) appId;
    StarfishResource *res = calloc(1, sizeof(StarfishResource));
    assert(res != NULL);
    res->appId = appId;
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
    jvalue_ref contents = jobject_create();
    jvalue_ref codec = jobject_create();
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

    jobject_set(contents, J_CSTR_TO_BUF("codec"), codec);
    jvalue_ref option = jobject_create();
    jobject_set(option, J_CSTR_TO_BUF("externalStreamingInfo"), jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("contents"), contents),
            J_END_OBJ_DECL
    ));
    jobject_set(option, J_CSTR_TO_BUF("appId"), j_cstr_to_jval(resource->appId));
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
    jobject_set(option, J_CSTR_TO_BUF("videoInfo"), jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("isGameMode"), jboolean_create(true)),
            J_END_OBJ_DECL
    ));
    jobject_set(option, J_CSTR_TO_BUF("windowId"), j_cstr_to_jval(resource->windowId));
    jobject_set(option, J_CSTR_TO_BUF("lowDelayMode"), jboolean_create(true));
    return jobject_create_var(
            jkeyval(J_CSTR_TO_JVAL("args"), jarray_create_var(
                    NULL,
                    jobject_create_var(
                            jkeyval(J_CSTR_TO_JVAL("mediaTransportType"), J_CSTR_TO_JVAL("DIRECTMEDIA-ES-PLAYER")),
                            jkeyval(J_CSTR_TO_JVAL("option"), option),
                            J_END_OBJ_DECL
                    ),
                    J_END_ARRAY_DECL
            )),
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