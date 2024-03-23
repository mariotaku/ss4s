#include "ss4s/modapi.h"
#include "ndl_common.h"

#include <stdlib.h>
#include <assert.h>

pthread_mutex_t SS4S_NDL_webOS5_Lock = PTHREAD_MUTEX_INITIALIZER;
bool SS4S_NDL_webOS5_Initialized = false;
SS4S_LoggingFunction *SS4S_NDL_webOS5_Log = NULL;
const SS4S_LibraryContext *SS4S_NDL_webOS5_Lib = NULL;

SS4S_EXPORTED bool SS4S_ModuleOpen_NDL_WEBOS5(SS4S_Module *module, const SS4S_LibraryContext *context) {
    (void) context;
    SS4S_NDL_webOS5_Lib = context;
    SS4S_NDL_webOS5_Log = context->Log;
    assert(SS4S_NDL_webOS5_Log != NULL);
    module->Name = "ndl-webos5";
    module->PlayerDriver = &SS4S_NDL_webOS5_PlayerDriver;
    module->AudioDriver = &SS4S_NDL_webOS5_AudioDriver;
    module->VideoDriver = &SS4S_NDL_webOS5_VideoDriver;
    return true;
}

int SS4S_NDL_webOS5_Driver_PostInit(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    if (SS4S_NDL_webOS5_Initialized) {
        return 0;
    }
    int ret;
    if ((ret = NDL_DirectMediaInit(getenv("APPID"), NULL)) == 0) {
        SS4S_NDL_webOS5_Initialized = true;
        SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "Driver init.");
    } else {
        SS4S_NDL_webOS5_Log(SS4S_LogLevelError, "NDL", "Failed to init: ret=%d, error=%s", ret,
                            NDL_DirectMediaGetError());
    }
    return ret;
}

void SS4S_NDL_webOS5_Driver_Quit() {
    pthread_mutex_lock(&SS4S_NDL_webOS5_Lock);
    if (!SS4S_NDL_webOS5_Initialized) {
        pthread_mutex_unlock(&SS4S_NDL_webOS5_Lock);
        return;
    }
    SS4S_NDL_webOS5_Log(SS4S_LogLevelInfo, "NDL", "Driver quit.");
    NDL_DirectMediaQuit();
    SS4S_NDL_webOS5_Initialized = false;
    pthread_mutex_unlock(&SS4S_NDL_webOS5_Lock);
}