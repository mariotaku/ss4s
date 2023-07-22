#include "ss4s.h"
#include "library.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

SS4S_Player *SS4S_PlayerOpen() {
    SS4S_Player *player = calloc(1, sizeof(SS4S_Player));
    assert(player != NULL);
    const SS4S_PlayerDriver *audioPlayerDriver = SS4S_GetAudioPlayerDriver();
    const SS4S_PlayerDriver *videoPlayerDriver = SS4S_GetVideoPlayerDriver();
    if (audioPlayerDriver != videoPlayerDriver) {
        if (audioPlayerDriver != NULL) {
            player->context.audio = audioPlayerDriver->Create();
        }
        if (videoPlayerDriver != NULL) {
            player->context.video = videoPlayerDriver->Create();
        }
    } else if (videoPlayerDriver != NULL) {
        player->context.video = player->context.audio = videoPlayerDriver->Create();
    }
    return player;
}

void SS4S_PlayerClose(SS4S_Player *player) {
    assert(player != NULL);
    if (player->context.audio != player->context.video) {
        if (player->context.audio != NULL) {
            const SS4S_PlayerDriver *audioPlayerDriver = SS4S_GetAudioPlayerDriver();
            audioPlayerDriver->Destroy(player->context.audio);
        }
        if (player->context.video != NULL) {
            const SS4S_PlayerDriver *audioPlayerDriver = SS4S_GetAudioPlayerDriver();
            audioPlayerDriver->Destroy(player->context.audio);
        }
    } else if (player->context.video != NULL) {
        const SS4S_PlayerDriver *videoPlayerDriver = SS4S_GetVideoPlayerDriver();
        videoPlayerDriver->Destroy(player->context.video);
    }

    free(player);
}

void SS4S_PlayerSetWaitAudioVideoReady(SS4S_Player *player, bool value) {
    assert(player != NULL);
    if (player->context.video != NULL && player->context.video == player->context.audio) {
        const SS4S_PlayerDriver *videoPlayerDriver = SS4S_GetVideoPlayerDriver();
        if (videoPlayerDriver->SetWaitAudioVideoReady != NULL) {
            videoPlayerDriver->SetWaitAudioVideoReady(player->context.video, value);
        }
    }
}

bool SS4S_PlayerGetInfo(SS4S_Player *player, SS4S_PlayerInfo *info) {
    assert(player != NULL);
    assert(info != NULL);
    memset(info, 0, sizeof(SS4S_PlayerInfo));
    const char *audioModule = SS4S_GetAudioModuleName();
    if (audioModule != NULL) {
        info->audio.enabled = true;
        info->audio.module = audioModule;
        SS4S_GetVideoCapabilities(&info->audio.capabilities);
    }
    const char *videoModule = SS4S_GetVideoModuleName();
    if (videoModule != NULL) {
        info->video.enabled = true;
        info->video.module = videoModule;
        SS4S_GetVideoCapabilities(&info->video.capabilities);
    }
    info->viewportWidth = player->viewportWidth;
    info->viewportHeight = player->viewportHeight;
    return true;
}

void SS4S_PlayerSetUserdata(SS4S_Player *player, void *userdata) {
    assert(player != NULL);
    player->userdata = userdata;
}

void SS4S_PlayerSetViewportSize(SS4S_Player *player, int width, int height) {
    assert(player != NULL);
    player->viewportWidth = width;
    player->viewportHeight = height;
}

void *SS4S_PlayerGetUserdata(SS4S_Player *player) {
    assert(player != NULL);
    return player->userdata;
}