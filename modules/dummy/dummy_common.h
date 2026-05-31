#pragma once

#include "ss4s/modapi.h"

extern bool SS4S_Dummy_Initialized;
extern SS4S_LoggingFunction *SS4S_Dummy_Log;

struct SS4S_PlayerContext {
    bool mediaLoaded;
    int aspectRatio;
    bool in_destructive;
};

extern const SS4S_PlayerDriver SS4S_Dummy_PlayerDriver;
extern const SS4S_AudioDriver SS4S_Dummy_AudioDriver;
extern const SS4S_VideoDriver SS4S_Dummy_VideoDriver;

int SS4S_Dummy_ReloadMedia(SS4S_PlayerContext *context);

int SS4S_Dummy_Driver_PostInit(int argc, char *argv[]);

void SS4S_Dummy_Driver_Quit();

/* Concurrency-check helpers. The contract: while EnterDestructive ..
 * ExitDestructive is in progress, the ss4s wrapper layer must drain
 * all in-flight Feeds and block new ones (BeginExclusive). If a Feed
 * still reaches the driver during that window, that's a FeedGuard
 * contract violation and CheckFeedSafe aborts the process. */
void SS4S_Dummy_EnterDestructive(SS4S_PlayerContext *context, int sleep_ms);

void SS4S_Dummy_ExitDestructive(SS4S_PlayerContext *context);

/* Returns the value of mediaLoaded. Aborts if called while a
 * destructive op is in progress. */
bool SS4S_Dummy_CheckFeedSafe(SS4S_PlayerContext *context);
