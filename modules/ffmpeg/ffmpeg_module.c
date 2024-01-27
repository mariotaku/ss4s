#include "ss4s/modapi.h"

#include <stdlib.h>
#include <libavcodec/avcodec.h>

static const SS4S_LibraryContext *LibContext;

struct SS4S_VideoInstance {
    AVPacket *packet;
    AVCodecContext *decoder_ctx;
    size_t frames_count, next_frame;
    AVFrame **frames;
    SS4S_VideoFrameCallback callback;
    void *callbackUserdata;
};

static bool InitFrames(SS4S_VideoInstance *instance, size_t max_size) {
    instance->frames = malloc(sizeof(AVFrame *) * max_size);
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
            return false;
        }
    }
    instance->next_frame = 0;
    instance->frames_count = max_size;
    return true;
}

static SS4S_VideoOpenResult Open(const SS4S_VideoInfo *info, const SS4S_VideoExtraInfo *extraInfo,
                                 SS4S_VideoInstance **instance, SS4S_PlayerContext *context) {
    enum AVCodecID codecId;
    switch (info->codec) {
        case SS4S_VIDEO_H264:
            codecId = AV_CODEC_ID_H264;
            break;
        case SS4S_VIDEO_H265:
            codecId = AV_CODEC_ID_HEVC;
            break;
        default:
            return SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
    }
    const AVCodec *codec = avcodec_find_decoder(codecId);
    if (codec == NULL) {
        return SS4S_VIDEO_OPEN_UNSUPPORTED_CODEC;
    }
    AVPacket *packet = av_packet_alloc();
    if (packet == NULL) {
        return SS4S_VIDEO_OPEN_ERROR;
    }

    AVCodecContext *decoder_ctx = avcodec_alloc_context3(codec);
    if (decoder_ctx == NULL) {
        av_packet_free(&packet);
        return SS4S_VIDEO_OPEN_ERROR;
    }

    // Use low delay decoding
    decoder_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;

    // Allow display of corrupted frames and frames with missing reference frames
    decoder_ctx->flags |= AV_CODEC_FLAG_OUTPUT_CORRUPT;
    decoder_ctx->flags2 |= AV_CODEC_FLAG2_SHOW_ALL;

    decoder_ctx->hwaccel_flags |= AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH;

    // Report decoding errors to allow us to request a key frame
    decoder_ctx->err_recognition |= AV_EF_EXPLODE;

    decoder_ctx->width = info->width;
    decoder_ctx->height = info->height;

    if (avcodec_open2(decoder_ctx, codec, NULL) < 0) {
        av_packet_free(&packet);
        avcodec_free_context(&decoder_ctx);
        return SS4S_VIDEO_OPEN_ERROR;
    }

    *instance = malloc(sizeof(SS4S_VideoInstance));
    if (!InitFrames(*instance, 3)) {
        av_packet_free(&packet);
        avcodec_free_context(&decoder_ctx);
        free(*instance);
        return SS4S_VIDEO_OPEN_ERROR;
    }
    (*instance)->packet = packet;
    (*instance)->decoder_ctx = decoder_ctx;
    return SS4S_VIDEO_OPEN_OK;
}

static SS4S_VideoFeedResult Feed(SS4S_VideoInstance *instance, const unsigned char *data, size_t size,
                                 SS4S_VideoFeedFlags flags) {
    void *buf = av_malloc(size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (buf == NULL) {
        return SS4S_VIDEO_FEED_ERROR;
    }
    memcpy(buf, data, size);
    if (av_packet_from_data(instance->packet, buf, (int) size) != 0) {
        av_free(buf);
        return SS4S_VIDEO_FEED_ERROR;
    }
    if (flags & SS4S_VIDEO_FEED_DATA_KEYFRAME) {
        instance->packet->flags |= AV_PKT_FLAG_KEY;
    }
    int err;
    err = avcodec_send_packet(instance->decoder_ctx, instance->packet);
    if (err != 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(err, errbuf, sizeof(errbuf));
        LibContext->Log(SS4S_LogLevelError, "avcodec_send_packet: %s", errbuf);
        return SS4S_VIDEO_FEED_ERROR;
    }
    while ((err = avcodec_receive_frame(instance->decoder_ctx, instance->frames[instance->next_frame])) == 0) {
        AVFrame *frame = instance->frames[instance->next_frame];
        SS4S_VideoOutputFrame output = {
                .yuv.data = frame->data,
                .yuv.linesize = frame->linesize,
                .yuv.width = frame->width,
                .yuv.height = frame->height,
                .yuv.pts = frame->pts,
        };
        if (instance->callback != NULL) {
            instance->callback(&output, instance->callbackUserdata);
        }
        instance->next_frame = (instance->next_frame + 1) % instance->frames_count;
    }
    if (err == AVERROR(EAGAIN)) {
        return SS4S_VIDEO_FEED_OK;
    } else {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(err, errbuf, sizeof(errbuf));
        LibContext->Log(SS4S_LogLevelError, "avcodec_receive_frame: %s", errbuf);
        return SS4S_VIDEO_FEED_ERROR;
    }
}

static void Close(SS4S_VideoInstance *instance) {
    for (size_t i = 0; i < instance->frames_count; i++) {
        av_frame_free(&instance->frames[i]);
    }
    free(instance->frames);
    av_packet_free(&instance->packet);
    avcodec_free_context(&instance->decoder_ctx);
    free(instance);
}

static bool GetCapabilities(SS4S_VideoCapabilities *capabilities) {
    capabilities->codecs = SS4S_VIDEO_H264 | SS4S_VIDEO_H265;
    capabilities->output = SS4S_VIDEO_OUTPUT_YUV;
    return true;
}

static bool SetFrameCallback(SS4S_VideoInstance *instance, SS4S_VideoFrameCallback callback, void *userdata) {
    instance->callback = callback;
    instance->callbackUserdata = userdata;
    return true;
}

static const SS4S_VideoDriver FFMPEGDriver = {
        .Open = Open,
        .Feed = Feed,
        .Close = Close,
        .GetCapabilities = GetCapabilities,
        .SetFrameCallback = SetFrameCallback,
};

SS4S_EXPORTED bool SS4S_ModuleOpen_FFMPEG(SS4S_Module *module, const SS4S_LibraryContext *context) {
    module->Name = "ffmpeg";
    module->VideoDriver = &FFMPEGDriver;
    LibContext = context;
    return true;
}
