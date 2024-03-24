#include "esplayer-datasrc.h"

#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <unistd.h>

int datasrc_run(struct DATASRC_CALLBACKS *callbacks) {
    const char *url = getenv("SS4S_SAMPLE_SOURCE");
    if (!url) {
        url = "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4";
    }
    int result = 0;
    AVFormatContext *ic = NULL;
    int audio_stream = -1, video_stream = -1;
    const AVCodec *audio_codec = NULL, *video_codec = NULL;
    if ((result = avformat_open_input(&ic, url, NULL, NULL)) != 0) {
        printf("Failed to open input: %s\n", av_err2str(result));
        return result;
    }
    audio_stream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, &audio_codec, 0);
    video_stream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, &video_codec, 0);

    AVCodecContext *audio_codec_ctx = NULL, *video_codec_ctx = NULL;

    if (audio_stream >= 0) {
//        callbacks->audioPreroll(ic->streams[audio_stream]->codecpar->ch_layout.nb_channels,
//                                ic->streams[audio_stream]->codecpar->sample_rate);
    }
    if (video_stream >= 0) {
        callbacks->videoPreroll(avcodec_get_name(ic->streams[video_stream]->codecpar->codec_id),
                                ic->streams[video_stream]->codecpar->width,
                                ic->streams[video_stream]->codecpar->height,
                                ic->streams[video_stream]->avg_frame_rate.num,
                                ic->streams[video_stream]->avg_frame_rate.den);
    }

    AVPacket *pkt = av_packet_alloc();
    AVFrame *a_frame = av_frame_alloc();
    while ((result = av_read_frame(ic, pkt)) == 0) {
        if (pkt->stream_index == audio_stream) {
        } else if (pkt->stream_index == video_stream) {
            int flags = 0;
            if (pkt->flags & AV_PKT_FLAG_KEY) {
                flags |= VIDEO_FLAG_FRAME_KEYFRAME;
            }
            callbacks->videoSample(pkt->data, pkt->size, flags);
        }
        av_packet_unref(pkt);
    }
    av_frame_free(&a_frame);

    if (audio_stream >= 0) {
//        callbacks->audioEos();
    }

    if (video_stream >= 0) {
        callbacks->videoEos();
    }

    callbacks->pipelineQuit(result);

    av_packet_free(&pkt);
    avformat_close_input(&ic);

    return 0;
}

