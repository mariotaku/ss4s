#include "ss4s.h"
#include "library.h"

#include <malloc.h>
#include <string.h>

SS4S_Player *SS4S_PlayerOpen() {
    SS4S_Player *player = calloc(1, sizeof(SS4S_Player));
    const SS4S_PlayerDriver *audioPlayerDriver = SS4S_GetAudioPlayerDriver();
    const SS4S_PlayerDriver *videoPlayerDriver = SS4S_GetVideoPlayerDriver();
    if (audioPlayerDriver != videoPlayerDriver) {
        if (audioPlayerDriver != NULL) {
            player->context.audio = audioPlayerDriver->Create();
        }
        if (videoPlayerDriver != NULL) {
            player->context.video = videoPlayerDriver->Create();
        }
    } else if (audioPlayerDriver != NULL) {
        player->context.audio = player->context.video = audioPlayerDriver->Create();
    }
    return player;
}

void SS4S_PlayerClose(SS4S_Player *player) {
    if (player->context.audio != player->context.video) {
        if (player->context.audio != NULL) {
            const SS4S_PlayerDriver *audioPlayerDriver = SS4S_GetAudioPlayerDriver();
            audioPlayerDriver->Destroy(player->context.audio);
        }
        if (player->context.video != NULL) {
            const SS4S_PlayerDriver *audioPlayerDriver = SS4S_GetAudioPlayerDriver();
            audioPlayerDriver->Destroy(player->context.audio);
        }
    } else if (player->context.audio != NULL) {
        const SS4S_PlayerDriver *audioPlayerDriver = SS4S_GetAudioPlayerDriver();
        audioPlayerDriver->Destroy(player->context.audio);
    }

    free(player);
}

bool SS4S_PlayerGetInfo(SS4S_Player *player, SS4S_PlayerInfo *info) {
    (void) player;
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
    return true;
}
