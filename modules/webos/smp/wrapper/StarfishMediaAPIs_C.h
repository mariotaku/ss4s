#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct StarfishMediaAPIs_C StarfishMediaAPIs_C;

typedef enum StarfishMediaAPIs_Event {
    STARFISH_EVENT_FRAMEREADY = 0x0,
    STARFISH_EVENT_STR_STREAMING_INFO_PERI = 0x1,
    STARFISH_EVENT_INT_BUFFER_RANGE_INFO = 0x2,
    STARFISH_EVENT_INT_DURATION = 0x3,
    STARFISH_EVENT_STR_VIDEO_INFO = 0x4,
    STARFISH_EVENT_STR_VIDEO_TRACK_INFO = 0x5,
    STARFISH_EVENT_STR_AUDIO_INFO = 0x7,
    STARFISH_EVENT_STR_AUDIO_TRACK_INFO = 0x8,
    STARFISH_EVENT_STR_SUBT_TRACK_INFO = 0x9,
    STARFISH_EVENT_STR_BUFF_EVENT = 0xa,
    STARFISH_EVENT_STR_SOURCE_INFO = 0xb,
    STARFISH_EVENT_INT_NUM_PROGRAM = 0xd,
    STARFISH_EVENT_INT_NUM_VIDEO_TRACK = 0xe,
    STARFISH_EVENT_INT_NUM_AUDIO_TRACK = 0xf,
    STARFISH_EVENT_STR_RESOURCE_INFO = 0x11,
    STARFISH_EVENT_INT_ERROR = 0x12,
    STARFISH_EVENT_STR_ERROR = 0x13,
    STARFISH_EVENT_STR_STATE_UPDATE_PRELOADCOMPLETED = 0x15,
    STARFISH_EVENT_STR_STATE_UPDATE_LOADCOMPLETED = 0x16,
    STARFISH_EVENT_STR_STATE_UPDATE_UNLOADCOMPLETED = 0x17,
    STARFISH_EVENT_STR_STATE_UPDATE_TRACKSELECTED = 0x18,
    STARFISH_EVENT_STR_STATE_UPDATE_SEEKDONE = 0x19,
    STARFISH_EVENT_STR_STATE_UPDATE_PLAYING = 0x1a,
    STARFISH_EVENT_STR_STATE_UPDATE_PAUSED = 0x1b,
    STARFISH_EVENT_STR_STATE_UPDATE_ENDOFSTREAM = 0x1c,
    STARFISH_EVENT_STR_CUSTOM = 0x1d,
    STARFISH_EVENT_INT_NEED_DATA = 0x26,
    STARFISH_EVENT_INT_ENOUGH_DATA = 0x27,
    STARFISH_EVENT_INT_SVP_VDEC_READY = 0x2b,
    STARFISH_EVENT_INT_BUFFERLOW = 0x2c,
    STARFISH_EVENT_STR_BUFFERFULL = 0x2d,
    STARFISH_EVENT_STR_BUFFERLOW = 0x2e,
    STARFISH_EVENT_DROPPED_FRAME = 0x30,
    STARFISH_EVENT_USER_DEFINED = 0x270,
} StarfishMediaAPIs_Event;

typedef void(StarfishMediaAPIs_callbackWithData)(int type, int64_t numValue, const char *strValue,
                                                 void *data);

StarfishMediaAPIs_C *StarfishMediaAPIs_create(const char *uid);

bool StarfishMediaAPIs_feed(StarfishMediaAPIs_C *api, const char *payload, char *result, size_t resultLen);

const char *StarfishMediaAPIs_getMediaID(StarfishMediaAPIs_C *api);

bool StarfishMediaAPIs_load(StarfishMediaAPIs_C *api, const char *payload, StarfishMediaAPIs_callbackWithData *callback,
                            void *data);

bool StarfishMediaAPIs_notifyForeground(StarfishMediaAPIs_C *api);

bool StarfishMediaAPIs_notifyBackground(StarfishMediaAPIs_C *api);

bool StarfishMediaAPIs_pause(StarfishMediaAPIs_C *api);

bool StarfishMediaAPIs_play(StarfishMediaAPIs_C *api);

bool StarfishMediaAPIs_pushEOS(StarfishMediaAPIs_C *api);

bool StarfishMediaAPIs_setHdrInfo(StarfishMediaAPIs_C *api, const char *message);

bool StarfishMediaAPIs_unload(StarfishMediaAPIs_C *api);

void StarfishMediaAPIs_destroy(StarfishMediaAPIs_C *api);


#ifdef __cplusplus
}
#endif