#include "ss4s/modapi.h"
#include "ndl_common.h"

#include <stdlib.h>
#include <assert.h>

pthread_mutex_t SS4S_NDL_Esplayer_Lock = PTHREAD_MUTEX_INITIALIZER;
SS4S_LoggingFunction *SS4S_NDL_Esplayer_Log = NULL;

SS4S_EXPORTED bool SS4S_ModuleOpen_NDL_ESPLAYER(SS4S_Module *module, const SS4S_LibraryContext *context) {
    (void) context;
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
    return 0;
}

void SS4S_NDL_Esplayer_Driver_Quit() {
    pthread_mutex_lock(&SS4S_NDL_Esplayer_Lock);
    SS4S_NDL_Esplayer_Log(SS4S_LogLevelInfo, "NDL", "Driver quit.");
    pthread_mutex_unlock(&SS4S_NDL_Esplayer_Lock);
}

