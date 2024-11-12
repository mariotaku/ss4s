#include "ss4s/modapi.h"
#include "ndl_common.h"

#include <stdlib.h>
#include <assert.h>

pthread_mutex_t SS4S_NDL_Esplayer_Lock = PTHREAD_MUTEX_INITIALIZER;
NDL_EsplayerHandle SS4S_NDL_Esplayer_Handle = NULL;
SS4S_LoggingFunction *SS4S_NDL_Esplayer_Log = NULL;
const SS4S_LibraryContext *SS4S_NDL_Esplayer_Lib = NULL;

SS4S_EXPORTED bool SS4S_ModuleOpen_NDL_ESPLAYER(SS4S_Module *module, const SS4S_LibraryContext *context) {
    (void) context;
    SS4S_NDL_Esplayer_Lib = context;
    SS4S_NDL_Esplayer_Log = context->Log;
    assert(SS4S_NDL_Esplayer_Log != NULL);
    module->Name = "ndl-esplayer";
    module->PlayerDriver = &SS4S_NDL_Esplayer_PlayerDriver;
    module->AudioDriver = &SS4S_NDL_Esplayer_AudioDriver;
    module->VideoDriver = &SS4S_NDL_Esplayer_VideoDriver;
    return true;
}

int SS4S_NDL_Esplayer_Driver_PostInit(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    if (SS4S_NDL_Esplayer_Handle) {
        return 0;
    }
    if ((SS4S_NDL_Esplayer_Handle = NDL_EsplayerCreate(getenv("APPID"), NULL, NULL)) == 0) {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "Driver init.");
        return 0;
    } else {
        SS4S_NDL_Esplayer_Log(SS4S_LogLevelError, "NDL", "Failed to init");
        return -1;
    }
}

void SS4S_NDL_Esplayer_Driver_Quit() {
    pthread_mutex_lock(&SS4S_NDL_Esplayer_Lock);
    if (!SS4S_NDL_Esplayer_Handle) {
        pthread_mutex_unlock(&SS4S_NDL_Esplayer_Lock);
        return;
    }
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "Driver quit.");
    NDL_EsplayerDestroy(SS4S_NDL_Esplayer_Handle);
    SS4S_NDL_Esplayer_Handle = NULL;
    pthread_mutex_unlock(&SS4S_NDL_Esplayer_Lock);
}