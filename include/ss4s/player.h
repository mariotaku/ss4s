#pragma once

#ifndef SS4S_MODAPI_H

#include <stdbool.h>
#include <stddef.h>

typedef struct SS4S_Player SS4S_Player;

typedef struct SS4S_PlayerInfo {
    const char *audioModule;
    const char *videoModule;
} SS4S_PlayerInfo;

SS4S_Player *SS4S_PlayerOpen();

void SS4S_PlayerClose(SS4S_Player *player);

bool SS4S_PlayerGetInfo(SS4S_Player *player, SS4S_PlayerInfo *info);

#endif // SS4S_MODAPI_H