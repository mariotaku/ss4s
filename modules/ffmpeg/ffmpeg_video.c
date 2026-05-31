#include "ffmpeg_common.h"

#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavutil/cpu.h>
#include <libavutil/hwcontext.h>

struct SS4S_VideoInstance {
    SS4S_PlayerContext *context;
    AVPacket *packet;
    AVCodecContext *decoder_ctx;
    AVBufferRef *hw_device_ctx;
    size_t frames_count, next_frame;
    AVFrame **frames;
    SS4S_VideoFrameCallback *callback;
    void *callbackUserdata;
};

static bool InitFrames(SS4S_VideoInstance *instance, size_t max_size) {
    instance->frames = calloc(max_size, sizeof(AVFrame *));
    if (instance->frames == NULL) {
        return false;
    }
    for (size_t i = 0; i < max_size; i++) {
        instance->frames[i] = av_frame_alloc();
        if (instance->frames[i] == NULL) {
            for (size_t j = 0; j < i; j++) {
                av_frame_free(&instance->frames[j]);
            }
            free(instance->frames);
            instance->frames = NULL;
            return false;
        }
    }
    instance->next_frame = 0;
    instance->frames_count = max_size;
    return true;
}

static const AVCodec *FindCodec(enum SS4S_VideoCodec codec) {
    switch (codec) {
        case SS4S_VIDEO_H264:
            return avcodec_find_decoder(AV_CODEC_ID_H264);
        case SS4S_VIDEO_H265:
            return avcodec_find_decoder(AV_CODEC_ID_HEVC);
        case SS4S_VIDEO_VP9:
            return avcodec_find_decoder(AV_CODEC_ID_VP9);
        case SS4S_VIDEO_AV1:
            return avcodec_find_decoder(AV_CODEC_ID_AV1);
        default:
            return NULL;
    }
}

static int NumberOfThreads(void) {
    int cores = av_cpu_count() / 2;
    if (cores < 1) {
        cores = 1;
    } else if (cores > 4) {
        cores = 4;
    }
    return cores;
}

static SS4S_VideoOpenResult Open(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                 SS4S_VideoInstance **instance, SS4S_PlayerContext *context) {
    (void) extraInfo;
    const AVCodec *codec = FindCodec(info->codec);
    if (codec == NULL) {
        return SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
    }

    AVBufferRef *hw_device_ctx = NULL;
    const AVCodecHWConfig *hw_config = NULL;
    for (int i = 0; (hw_config = avcodec_get_hw_config(codec, i)) != NULL; i++) {
        if (!(hw_config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)) {
            continue;
        }
        if (av_hwdevice_ctx_create(&hw_device_ctx, hw_config->device_type, NULL, NULL, 0) < 0) {
            continue;
        }
        SS4S_FFMPEG_LibContext->Log(SS4S_LogLevelDebug, "FFMPEG", "hw_device: %s",
                                    av_hwdevice_get_type_name(hw_config->device_type));
        break;
    }

    AVCodecContext *decoder_ctx = avcodec_alloc_context3(codec);
    if (decoder_ctx == NULL) {
        av_buffer_unref(&hw_device_ctx);
        return SS4S_VIDEO_OPEN_ERROR;
    }

    AVPacket *packet = av_packet_alloc();
    if (packet == NULL) {
        av_buffer_unref(&hw_device_ctx);
        avcodec_free_context(&decoder_ctx);
        return SS4S_VIDEO_OPEN_ERROR;
    }

    decoder_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    decoder_ctx->flags |= AV_CODEC_FLAG_OUTPUT_CORRUPT;
    decoder_ctx->flags2 |= AV_CODEC_FLAG2_SHOW_ALL;
    decoder_ctx->hwaccel_flags |= AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH;
    decoder_ctx->err_recognition |= AV_EF_EXPLODE;
    decoder_ctx->width = info->width;
    decoder_ctx->height = info->height;
    decoder_ctx->thread_count = NumberOfThreads();
    decoder_ctx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
    if (hw_device_ctx != NULL) {
        decoder_ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    }

    if (avcodec_open2(decoder_ctx, codec, NULL) < 0) {
        av_packet_free(&packet);
        avcodec_free_context(&decoder_ctx);
        av_buffer_unref(&hw_device_ctx);
        return SS4S_VIDEO_OPEN_ERROR;
    }

    SS4S_VideoInstance *inst = calloc(1, sizeof(SS4S_VideoInstance));
    if (inst == NULL) {
        av_packet_free(&packet);
        avcodec_free_context(&decoder_ctx);
        av_buffer_unref(&hw_device_ctx);
        return SS4S_VIDEO_OPEN_ERROR;
    }
    if (!InitFrames(inst, decoder_ctx->thread_count)) {
        free(inst);
        av_packet_free(&packet);
        avcodec_free_context(&decoder_ctx);
        av_buffer_unref(&hw_device_ctx);
        return SS4S_VIDEO_OPEN_ERROR;
    }
    inst->context = context;
    inst->packet = packet;
    inst->decoder_ctx = decoder_ctx;
    inst->hw_device_ctx = hw_device_ctx;
    *instance = inst;
    return SS4S_VIDEO_OPEN_OK;
}

static bool FrameRetain(SS4S_VideoOutputFrame *frame);

static void FrameRelease(SS4S_VideoOutputFrame *frame);

static void FillView(SS4S_VideoOutputFrame *out, AVFrame *src) {
    if (out->format == SS4S_VIDEO_OUTPUT_FORMAT_AVFRAME) {
        out->avframe.frame = src;
    } else {
        out->yuv.data = src->data;
        out->yuv.linesize = src->linesize;
        out->yuv.width = src->width;
        out->yuv.height = src->height;
        out->yuv.pts = src->pts;
    }
}

static void EmitFrame(SS4S_VideoInstance *instance, AVFrame *frame) {
    if (instance->callback == NULL) {
        return;
    }
    SS4S_VideoOutputFrame output;
    memset(&output, 0, sizeof(output));
    output.format = (frame->hw_frames_ctx != NULL) ? SS4S_VIDEO_OUTPUT_FORMAT_AVFRAME
                                                   : SS4S_VIDEO_OUTPUT_FORMAT_YUV;
    FillView(&output, frame);
    output._private.handle = frame;
    output._private.retain = FrameRetain;
    /* release stays NULL until Retain succeeds — consumer-side Release
     * on a non-retained frame must be a no-op, since the original frame
     * is owned by our Feed loop. */
    instance->callback(&output, instance->callbackUserdata);
}

static bool FrameRetain(SS4S_VideoOutputFrame *frame) {
    AVFrame *src = frame->_private.handle;
    AVFrame *clone = av_frame_clone(src);
    if (clone == NULL) {
        return false;
    }
    frame->_private.handle = clone;
    frame->_private.retain = NULL;
    frame->_private.release = FrameRelease;
    FillView(frame, clone);
    return true;
}

static void FrameRelease(SS4S_VideoOutputFrame *frame) {
    AVFrame *clone = frame->_private.handle;
    av_frame_free(&clone);
}

static SS4S_VideoFeedResult Feed(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                 SS4S_VideoFeedFlags flags) {
    uint32_t beginToken = SS4S_FFMPEG_LibContext->VideoStats.BeginFrame(instance->context->player);
    void *buf = av_malloc(size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (buf == NULL) {
        SS4S_FFMPEG_LibContext->VideoStats.EndFrame(instance->context->player, beginToken);
        return SS4S_VIDEO_FEED_ERROR;
    }
    memcpy(buf, data, size);
    if (av_packet_from_data(instance->packet, buf, (int) size) != 0) {
        av_free(buf);
        SS4S_FFMPEG_LibContext->VideoStats.EndFrame(instance->context->player, beginToken);
        return SS4S_VIDEO_FEED_ERROR;
    }
    if (flags & SS4S_VIDEO_FEED_DATA_KEYFRAME) {
        instance->packet->flags |= AV_PKT_FLAG_KEY;
    }

    int err = avcodec_send_packet(instance->decoder_ctx, instance->packet);
    if (err != 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(err, errbuf, sizeof(errbuf));
        SS4S_FFMPEG_LibContext->Log(SS4S_LogLevelError, "FFMPEG", "avcodec_send_packet: %s", errbuf);
        SS4S_FFMPEG_LibContext->VideoStats.EndFrame(instance->context->player, beginToken);
        return SS4S_VIDEO_FEED_ERROR;
    }

    while ((err = avcodec_receive_frame(instance->decoder_ctx, instance->frames[instance->next_frame])) == 0) {
        AVFrame *frame = instance->frames[instance->next_frame];
        EmitFrame(instance, frame);
        av_frame_unref(frame);
        instance->next_frame = (instance->next_frame + 1) % instance->frames_count;
    }
    SS4S_FFMPEG_LibContext->VideoStats.EndFrame(instance->context->player, beginToken);
    if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
        return SS4S_VIDEO_FEED_OK;
    }
    char errbuf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(err, errbuf, sizeof(errbuf));
    SS4S_FFMPEG_LibContext->Log(SS4S_LogLevelError, "FFMPEG", "avcodec_receive_frame: %s", errbuf);
    return SS4S_VIDEO_FEED_ERROR;
}

static void Close(SS4S_VideoInstance *instance) {
    if (instance->frames != NULL) {
        for (size_t i = 0; i < instance->frames_count; i++) {
            av_frame_free(&instance->frames[i]);
        }
        free(instance->frames);
    }
    av_packet_free(&instance->packet);
    avcodec_free_context(&instance->decoder_ctx);
    av_buffer_unref(&instance->hw_device_ctx);
    free(instance);
}

static bool GetCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264 | SS4S_VIDEO_H265 | SS4S_VIDEO_VP9 | SS4S_VIDEO_AV1;
    capabilities->output = SS4S_VIDEO_CAP_OUTPUT_YUV | SS4S_VIDEO_CAP_OUTPUT_AVFRAME;
    return true;
}

static bool SetFrameCallback(SS4S_VideoInstance *instance, SS4S_VideoFrameCallback *callback, void *userdata) {
    instance->callback = callback;
    instance->callbackUserdata = userdata;
    return true;
}

const SS4S_VideoDriver SS4S_FFMPEG_VideoDriver = {
        .GetCapabilities = GetCapabilities,
        .Open = Open,
        .Feed = Feed,
        .SetFrameCallback = SetFrameCallback,
        .Close = Close,
};
