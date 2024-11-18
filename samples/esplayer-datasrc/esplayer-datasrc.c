#include "esplayer-datasrc.h"

#include <gst/gst.h>
#include <gst/app/app.h>
#include <stdio.h>

static GstElement *pipeline = NULL;
struct DATASRC_CALLBACKS *callbacks;

static void audioEos(GstAppSink *appsink, gpointer user_data) {
    callbacks->audioEos();
}

static GstFlowReturn audioNewPreroll(GstAppSink *appsink, gpointer user_data) {
    GstSample *preroll = gst_app_sink_pull_preroll(appsink);
    GstCaps *caps = gst_sample_get_caps(preroll);
    GstStructure *cap = gst_caps_get_structure(caps, 0);
    int channels = 0, rate = 0;
    gst_structure_get_int(cap, "channels", &channels);
    gst_structure_get_int(cap, "rate", &rate);

    gst_sample_unref(preroll);

    if (callbacks->audioPreroll(channels, rate) != 0) {
        return GST_FLOW_ERROR;
    }

    return GST_FLOW_OK;
}

static GstFlowReturn audioNewSample(GstAppSink *appsink, gpointer user_data) {
    GstSample *sample = gst_app_sink_pull_sample(appsink);

    GstBuffer *buf = gst_sample_get_buffer(sample);

    GstMapInfo info;
    gst_buffer_map(buf, &info, GST_MAP_READ);

    if (callbacks->audioSample(info.data, info.size) != 0) {
        gst_buffer_unmap(buf, &info);
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    gst_buffer_unmap(buf, &info);
    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

static void videoEos(GstAppSink *appsink, gpointer user_data) {
    callbacks->videoEos();
}

static GstFlowReturn videoNewPreroll(GstAppSink *appsink, gpointer user_data) {
    GstSample *preroll = gst_app_sink_pull_preroll(appsink);
    GstCaps *caps = gst_sample_get_caps(preroll);
    GstStructure *cap = gst_caps_get_structure(caps, 0);

    int width, height;
    g_assert(gst_structure_get_int(cap, "width", &width));
    g_assert(gst_structure_get_int(cap, "height", &height));

    gst_sample_unref(preroll);

    if (callbacks->videoPreroll(width, height, 0) != 0) {
        return GST_FLOW_ERROR;
    }
    return GST_FLOW_OK;
}

static GstFlowReturn videoNewSample(GstAppSink *appsink, gpointer user_data) {
    GstSample *sample = gst_app_sink_pull_sample(appsink);

    GstBuffer *buf = gst_sample_get_buffer(sample);

    GstMapInfo info;
    gst_buffer_map(buf, &info, GST_MAP_READ);

    int flags = VIDEO_FLAG_FRAME;
    if (!gst_buffer_has_flags(buf, GST_BUFFER_FLAG_DELTA_UNIT)) {
        flags |= VIDEO_FLAG_FRAME_KEYFRAME;
    }

    if (callbacks->videoSample(info.data, info.size, flags) != 0) {
        gst_buffer_unmap(buf, &info);
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }
    return GST_FLOW_OK;
}

/* called when a new message is posted on the bus */
static void cb_message(GstBus *bus, GstMessage *message, gpointer user_data) {
    GstElement *pipeline = GST_ELEMENT(user_data);

    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR:
            g_print("we received an error!\n");
            callbacks->pipelineQuit(GST_MESSAGE_ERROR);
            break;
        case GST_MESSAGE_EOS:
            g_print("we reached EOS\n");
            callbacks->pipelineQuit(0);
            break;
        default:
            break;
    }
}

void datasrc_init(int argc, char *argv[]) {
    gst_init(0, NULL);
}

int datasrc_start(struct DATASRC_CALLBACKS *cb) {
    callbacks = cb;
    GstStateChangeReturn ret;

    GstElement *audiosink, *videosink;
    char gst_args[8192];
    const char *url = "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4";
    snprintf(gst_args, 8192,
             "curlhttpsrc location=%s ! qtdemux name=demux "
             "demux.audio_0 ! queue ! aacparse ! avdec_aac ! audioconvert ! audio/x-raw,format=S16LE ! appsink name=audsink "
             "demux.video_0 ! queue ! h264parse config-interval=-1 ! video/x-h264,stream-format=byte-stream,alignment=nal ! appsink name=vidsink",
             url);
    pipeline = gst_parse_launch(gst_args, NULL);

    g_assert_nonnull(pipeline);

    audiosink = gst_bin_get_by_name(GST_BIN(pipeline), "audsink");
    g_assert_nonnull(audiosink);

    videosink = gst_bin_get_by_name(GST_BIN(pipeline), "vidsink");
    g_assert_nonnull(videosink);

    GstAppSinkCallbacks audioCallbacks = {
            .eos = audioEos,
            .new_preroll = audioNewPreroll,
            .new_sample = audioNewSample,
    };
    gst_app_sink_set_callbacks(GST_APP_SINK(audiosink), &audioCallbacks, NULL, NULL);

    GstAppSinkCallbacks videoCallbacks = {
            .eos = videoEos,
            .new_preroll = videoNewPreroll,
            .new_sample = videoNewSample,
    };
    gst_app_sink_set_callbacks(GST_APP_SINK(videosink), &videoCallbacks, NULL, NULL);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message", (GCallback) cb_message, pipeline);
    gst_object_unref(bus);

    /* Start playing */
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    return 0;
}

int datasrc_stop() {
    g_print("Stopping pipeline\n");
    gst_element_send_event(pipeline, gst_event_new_eos());
    return 0;
}

int datasrc_destroy() {
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline = NULL;
    callbacks = NULL;
    return 0;
}