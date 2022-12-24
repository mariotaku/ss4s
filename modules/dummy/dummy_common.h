#pragma once

#include "ss4s/modapi.h"

extern bool SS4S_Dummy_Initialized;
extern SS4S_LoggingFunction *SS4S_Dummy_Log;

struct SS4S_PlayerContext {
    bool mediaLoaded;
    int aspectRatio;
};

extern const SS4S_PlayerDriver SS4S_Dummy_PlayerDriver;
extern const SS4S_AudioDriver SS4S_Dummy_AudioDriver;
extern const SS4S_VideoDriver SS4S_Dummy_VideoDriver;

int SS4S_Dummy_ReloadMedia(SS4S_PlayerContext *context);

int SS4S_Dummy_Driver_PostInit(int argc, char *argv[]);

void SS4S_Dummy_Driver_Quit();
