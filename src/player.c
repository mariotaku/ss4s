#include "ss4s.h"
#include "library.h"

#include <malloc.h>
#include <string.h>

SS4S_Player *SS4S_PlayerOpen() {
    SS4S_Player *player = calloc(1, sizeof(SS4S_Player));
    return player;
}

void SS4S_PlayerClose(SS4S_Player *player) {


    free(player);
}

bool SS4S_PlayerGetInfo(SS4S_Player *player, SS4S_PlayerInfo *info) {
    memset(info, 0, sizeof(SS4S_PlayerInfo));
    const char *audioModule = SS4S_GetAudioModuleName();
    if (audioModule != NULL) {
        info->audioModule = audioModule;
    }
    const char *videoModule = SS4S_GetVideoModuleName();
    if (videoModule != NULL) {
        info->videoModule = videoModule;
    }
    return true;
}
