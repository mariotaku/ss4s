#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ss4s/player.h"

typedef uint32_t (SS4S_VideoStatsBeginFrameFunction)(SS4S_Player *player);

typedef void(SS4S_VideoStatsEndFrameFunction)(SS4S_Player *player, uint32_t beginFrameResult);

typedef void(SS4S_VideoStatsReportFrameFunction)(SS4S_Player *player, uint32_t frameLatencyUs);

#ifdef __cplusplus
}
#endif