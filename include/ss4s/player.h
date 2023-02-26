#pragma once
#include "video.h"

#ifndef SS4S_MODAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

typedef struct SS4S_Player SS4S_Player;

typedef struct SS4S_PlayerInfo {
    struct {
        bool enabled;
        const char *module;
        SS4S_VideoCapabilities capabilities;
    } audio;
    struct {
        bool enabled;
        const char *module;
        SS4S_VideoCapabilities capabilities;
    } video;
} SS4S_PlayerInfo;

SS4S_Player *SS4S_PlayerOpen();

void SS4S_PlayerClose(SS4S_Player *player);

bool SS4S_PlayerGetInfo(SS4S_Player *player, SS4S_PlayerInfo *info);

#ifdef __cplusplus
}
#endif

#endif // SS4S_MODAPI_H
