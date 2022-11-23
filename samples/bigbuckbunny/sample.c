#include <unistd.h>
#include "esplayer-datasrc.h"

#include "ss4s.h"

static SS4S_Player *player = NULL;
static bool finished = false;

int videoPreroll(int width, int height, int framerate) {
    (void) framerate;
    SS4S_VideoInfo info = {
            .codec = SS4S_VIDEO_H264,
            .width = width,
            .height = height,
    };
    return SS4S_PlayerVideoOpen(player, &info);
}

int videoSample(const void *data, size_t size) {
    return SS4S_PlayerVideoFeed(player, data, size, 0);
}

void videoEos() {
    SS4S_PlayerVideoClose(player);
}

int audioPreroll(int channels, int sampleFreq) {
    SS4S_AudioInfo info = {
            .codec = SS4S_AUDIO_PCM_S16LE,
            .numOfChannels = channels,
            .sampleRate = sampleFreq,
            .samplesPerFrame = 240,
    };
    return SS4S_PlayerAudioOpen(player, &info);
}

int audioSample(const void *data, size_t size) {
    return SS4S_PlayerAudioFeed(player, data, size);
}

void audioEos() {
    SS4S_PlayerAudioClose(player);
}

void pipelineQuit(int error) {
    finished = true;
}

int main(int argc, char *argv[]) {
    datasrc_init(argc, argv);
    SS4S_Config config = {
            .audioDriver = "alsa",
            .videoDriver = "mmal",
    };
    SS4S_Init(argc, argv, &config);
    SS4S_PostInit(argc, argv);
    player = SS4S_PlayerOpen();
    struct DATASRC_CALLBACKS dscb = {
            .videoPreroll = videoPreroll,
            .videoSample = videoSample,
            .videoEos = videoEos,
            .audioPreroll = audioPreroll,
            .audioSample = audioSample,
            .audioEos = audioEos,
            .pipelineQuit = pipelineQuit,
    };
    datasrc_start(&dscb);
    while (!finished) {
        usleep(1000);
    }
    SS4S_PlayerClose(player);
    player = NULL;
    SS4S_Quit();
}