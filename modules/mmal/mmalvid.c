/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  * Neither the name of the copyright holder nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Video decode on Raspberry Pi using MMAL
// Based upon example code from the Raspberry Pi

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <bcm_host.h>
#include <interface/mmal/mmal.h>
#include <interface/mmal/util/mmal_default_components.h>
#include <interface/mmal/util/mmal_util.h>
#include <interface/mmal/vc/mmal_vc_api.h>

#include "ss4s/modapi.h"

#define MAX_DECODE_UNIT_SIZE 262144

#define ALIGN(x, a) (((x)+(a)-1)&~((a)-1))

struct SS4S_VideoInstance {
    bool started;
    VCOS_SEMAPHORE_T semaphore;
    MMAL_COMPONENT_T *decoder, *renderer;
    MMAL_POOL_T *pool_in, *pool_out;
};

static const SS4S_LibraryContext *LibContext;

static int Init() {
    bcm_host_init();
    return mmal_vc_init();
}

static void Quit() {
    mmal_vc_deinit();
    bcm_host_deinit();
}

static SS4S_VideoOpenResult Start(SS4S_VideoInstance *instance, int width, int height);

static void input_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf) {
    SS4S_VideoInstance *instance = (SS4S_VideoInstance *) port->component->userdata;
    assert(instance != NULL);
    mmal_buffer_header_release(buf);

    if (port == instance->decoder->input[0]) {
        vcos_semaphore_post(&instance->semaphore);
    }
}

static void decoder_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf) {
    SS4S_VideoInstance *instance = (SS4S_VideoInstance *) port->component->userdata;
    (void) instance;
    if (buf->cmd == MMAL_EVENT_ERROR) {
        MMAL_STATUS_T status = *(uint32_t *) buf->data;
        LibContext->Log(SS4S_LogLevelWarn, "MMAL", "Video decode error MMAL_EVENT_ERROR:%d", status);
    }

    mmal_buffer_header_release(buf);
}

static void render_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf) {
    SS4S_VideoInstance *instance = (SS4S_VideoInstance *) port->component->userdata;
    (void) instance;
    if (buf->cmd == MMAL_EVENT_ERROR) {
        MMAL_STATUS_T status = *(uint32_t *) buf->data;
        LibContext->Log(SS4S_LogLevelWarn, "MMAL", "Video render error MMAL_EVENT_ERROR:%d", status);
    }

    mmal_buffer_header_release(buf);
}

static void output_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf) {
    SS4S_VideoInstance *instance = (SS4S_VideoInstance *) port->component->userdata;
    assert(instance != NULL);
    if (mmal_port_send_buffer(instance->renderer->input[0], buf) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelWarn, "MMAL", "Can't display decoded frame");
        mmal_buffer_header_release(buf);
    }
}

static bool GetCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264;
    capabilities->transform = SS4S_VIDEO_CAP_TRANSFORM_AREA_SRC | SS4S_VIDEO_CAP_TRANSFORM_AREA_DEST;
    return true;
}

static SS4S_VideoOpenResult Open(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                 SS4S_VideoInstance **instance, SS4S_PlayerContext *context) {
    (void) extraInfo;
    (void) context;
    if (info->codec != SS4S_VIDEO_H264) {
        return SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
    }
    SS4S_VideoInstance *newInstance = calloc(1, sizeof(SS4S_VideoInstance));
    SS4S_VideoOpenResult result = Start(newInstance, info->width, info->height);
    if (result == SS4S_VIDEO_OPEN_OK) {
        *instance = newInstance;
    } else {
        free(newInstance);
    }
    return result;
}

static SS4S_VideoOpenResult Start(SS4S_VideoInstance *instance, int width, int height) {
    vcos_semaphore_create(&instance->semaphore, "video_decoder", 1);
    if (mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_DECODER, &instance->decoder) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't create decoder");
        return SS4S_VIDEO_OPEN_ERROR;
    }
    instance->decoder->userdata = (struct MMAL_COMPONENT_USERDATA_T *) instance;
    LibContext->Log(SS4S_LogLevelInfo, "MMAL", "create decoder %d x %d", width, height);

    MMAL_ES_FORMAT_T *format_in = instance->decoder->input[0]->format;
    format_in->type = MMAL_ES_TYPE_VIDEO;
    format_in->encoding = MMAL_ENCODING_H264;
    format_in->es->video.width = ALIGN(width, 32);
    format_in->es->video.height = ALIGN(height, 16);
    format_in->es->video.crop.width = width;
    format_in->es->video.crop.height = height;
    format_in->es->video.frame_rate.num = 0;
    format_in->es->video.frame_rate.den = 1;
    format_in->es->video.par.num = 1;
    format_in->es->video.par.den = 1;
    format_in->flags = MMAL_ES_FORMAT_FLAG_FRAMED;

    if (mmal_port_format_commit(instance->decoder->input[0]) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't commit input format to decoder");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    instance->decoder->input[0]->buffer_num = 5;
    instance->decoder->input[0]->buffer_size = MAX_DECODE_UNIT_SIZE;
    instance->pool_in = mmal_port_pool_create(instance->decoder->input[0], instance->decoder->input[0]->buffer_num,
                                              instance->decoder->output[0]->buffer_size);

    MMAL_ES_FORMAT_T *format_out = instance->decoder->output[0]->format;
    format_out->encoding = MMAL_ENCODING_OPAQUE;
    if (mmal_port_format_commit(instance->decoder->output[0]) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't commit output format to decoder");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    instance->decoder->output[0]->buffer_num = 3;
    instance->decoder->output[0]->buffer_size = instance->decoder->output[0]->buffer_size_recommended;
    instance->pool_out = mmal_port_pool_create(instance->decoder->output[0], instance->decoder->output[0]->buffer_num,
                                               instance->decoder->output[0]->buffer_size);

    if (mmal_port_enable(instance->decoder->control, decoder_callback) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't enable control port");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    if (mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &instance->renderer) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't create renderer");
        return SS4S_VIDEO_OPEN_ERROR;
    }
    instance->renderer->userdata = (struct MMAL_COMPONENT_USERDATA_T *) instance;

    format_in = instance->renderer->input[0]->format;
    format_in->encoding = MMAL_ENCODING_OPAQUE;
    format_in->es->video.width = width;
    format_in->es->video.height = height;
    format_in->es->video.crop.x = 0;
    format_in->es->video.crop.y = 0;
    format_in->es->video.crop.width = width;
    format_in->es->video.crop.height = height;
    if (mmal_port_format_commit(instance->renderer->input[0]) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't set output format");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    MMAL_DISPLAYREGION_T param;
    param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
    param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);
    param.set = MMAL_DISPLAY_SET_LAYER | MMAL_DISPLAY_SET_NUM | MMAL_DISPLAY_SET_FULLSCREEN |
                MMAL_DISPLAY_SET_TRANSFORM;
    if (getenv("DISPLAY") == NULL) {
        param.layer = 10000;
    } else {
        param.layer = 0;
    }
    param.display_num = 0;
    param.fullscreen = true;
    param.transform = MMAL_DISPLAY_ROT0;
//    int displayRotation = drFlags & DISPLAY_ROTATE_MASK;
//    switch (displayRotation) {
//        case DISPLAY_ROTATE_90:
//            param.transform = MMAL_DISPLAY_ROT90;
//            break;
//        case DISPLAY_ROTATE_180:
//            param.transform = MMAL_DISPLAY_ROT180;
//            break;
//        case DISPLAY_ROTATE_270:
//            param.transform = MMAL_DISPLAY_ROT270;
//            break;
//        default:
//            param.transform = MMAL_DISPLAY_ROT0;
//            break;
//    }

    if (mmal_port_parameter_set(instance->renderer->input[0], &param.hdr) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't set parameters");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    if (mmal_port_enable(instance->renderer->control, render_callback) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't enable control port");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    if (mmal_component_enable(instance->renderer) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't enable renderer");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    if (mmal_port_enable(instance->renderer->input[0], input_callback) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't enable renderer input port");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    if (mmal_port_enable(instance->decoder->input[0], input_callback) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't enable decoder input port");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    if (mmal_port_enable(instance->decoder->output[0], output_callback) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't enable decoder output port");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    if (mmal_component_enable(instance->decoder) != MMAL_SUCCESS) {
        LibContext->Log(SS4S_LogLevelError, "MMAL", "Can't enable decoder");
        return SS4S_VIDEO_OPEN_ERROR;
    }

    LibContext->Log(SS4S_LogLevelInfo, "MMAL", "mmal decoder initialized");
    instance->started = true;
    return 0;
}

static void Stop(SS4S_VideoInstance *instance) {
    instance->started = false;
    if (instance->decoder) {
        mmal_component_destroy(instance->decoder);
    }

    if (instance->renderer) {
        mmal_component_destroy(instance->renderer);
    }

    if (instance->pool_in) {
        mmal_pool_destroy(instance->pool_in);
    }

    if (instance->pool_out) {
        mmal_pool_destroy(instance->pool_out);
    }

    vcos_semaphore_delete(&instance->semaphore);
}

static void Close(SS4S_VideoInstance *instance) {
    Stop(instance);
    free(instance);
}

static SS4S_VideoFeedResult Feed(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                 SS4S_VideoFeedFlags flags) {
    if (!instance->started) {
        return SS4S_VIDEO_FEED_NOT_READY;
    }

    MMAL_STATUS_T status;
    MMAL_BUFFER_HEADER_T *buf;

    vcos_semaphore_wait(&instance->semaphore);
    if ((buf = mmal_queue_get(instance->pool_in->queue)) != NULL) {
        buf->flags = 0;
        buf->offset = 0;
        buf->pts = buf->dts = MMAL_TIME_UNKNOWN;
    } else {
        LibContext->Log(SS4S_LogLevelWarn, "MMAL", "Video buffer full");
        return SS4S_VIDEO_FEED_REQUEST_KEYFRAME;
    }

    if (flags & SS4S_VIDEO_FEED_DATA_FRAME_START) {
        buf->flags |= MMAL_BUFFER_HEADER_FLAG_FRAME_START;
    }
    if (flags & SS4S_VIDEO_FEED_DATA_FRAME_END) {
        buf->flags |= MMAL_BUFFER_HEADER_FLAG_FRAME_END;
    }
    if (flags & SS4S_VIDEO_FEED_DATA_KEYFRAME) {
        buf->flags |= MMAL_BUFFER_HEADER_FLAG_KEYFRAME;
    }

    if (size + buf->length > buf->alloc_size) {
        LibContext->Log(SS4S_LogLevelWarn, "MMAL", "Video decoder buffer too small");
        mmal_buffer_header_release(buf);
        return SS4S_VIDEO_FEED_REQUEST_KEYFRAME;
    }
    memcpy(buf->data + buf->length, data, size);
    buf->length += size;

    if ((status = mmal_port_send_buffer(instance->decoder->input[0], buf)) != MMAL_SUCCESS) {
        mmal_buffer_header_release(buf);
        return SS4S_VIDEO_FEED_REQUEST_KEYFRAME;
    }

    //Send available output buffers to decoder
    while ((buf = mmal_queue_get(instance->pool_out->queue))) {
        if ((status = mmal_port_send_buffer(instance->decoder->output[0], buf)) != MMAL_SUCCESS) {
            mmal_buffer_header_release(buf);
        }
    }

    if (status != MMAL_SUCCESS) {
        return SS4S_VIDEO_FEED_ERROR;
    }

    return SS4S_VIDEO_FEED_OK;
}

static bool SizeChanged(SS4S_VideoInstance *instance, int width, int height) {
    Stop(instance);
    return Start(instance, width, height) == SS4S_VIDEO_OPEN_OK;
}

static bool SetDisplayArea(SS4S_VideoInstance *instance, const SS4S_VideoRect *src, const SS4S_VideoRect *dst) {
    MMAL_DISPLAYREGION_T param;
    param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
    param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);
    param.set = MMAL_DISPLAY_SET_FULLSCREEN | MMAL_DISPLAY_SET_SRC_RECT | MMAL_DISPLAY_SET_DEST_RECT;
    param.display_num = 0;
    param.fullscreen = src == NULL && dst == NULL;
    if (src != NULL) {
        param.src_rect.width = src->width;
        param.src_rect.height = src->height;
        param.src_rect.x = src->x;
        param.src_rect.y = src->y;
    } else {
        memset(&param.src_rect, 0, sizeof(MMAL_RECT_T));
    }
    if (dst != NULL) {
        param.dest_rect.width = dst->width;
        param.dest_rect.height = dst->height;
        param.dest_rect.x = dst->x;
        param.dest_rect.y = dst->y;
    } else {
        memset(&param.dest_rect, 0, sizeof(MMAL_RECT_T));
    }

    return mmal_port_parameter_set(instance->renderer->input[0], &param.hdr) == MMAL_SUCCESS;
}

static const SS4S_VideoDriver MMALDriver = {
        .Base = {
                .Init = Init,
                .Quit = Quit,
        },
        .GetCapabilities = GetCapabilities,
        .Open = Open,
        .Feed = Feed,
        .SizeChanged = SizeChanged,
        .SetDisplayArea = SetDisplayArea,
        .Close = Close,
};

SS4S_EXPORTED bool SS4S_ModuleOpen_MMAL(SS4S_Module *module, const SS4S_LibraryContext *context) {
    module->Name = "mmal";
    module->VideoDriver = &MMALDriver;
    LibContext = context;
    return true;
}