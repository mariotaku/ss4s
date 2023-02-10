#pragma once

#ifndef SS4S_MODAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include "ss4s/player.h"
#include "ss4s/audio.h"
#include "ss4s/video.h"
#include "ss4s/module.h"
#include "ss4s/logging.h"

typedef struct SS4S_Config {
    const char *audioDriver;
    const char *videoDriver;

    SS4S_LoggingFunction *loggingFunction;
} SS4S_Config;

int SS4S_Init(int argc, char *argv[], const SS4S_Config *config);

int SS4S_PostInit(int argc, char *argv[]);

void SS4S_Quit();

const char *SS4S_GetAudioModuleName();

const char *SS4S_GetVideoModuleName();

#ifdef __cplusplus
}
#endif

#endif // SS4S_MODAPI_H