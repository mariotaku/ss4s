#include "ss4s/modapi.h"
#include "ndl_common.h"

#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <memory.h>

pthread_mutex_t SS4S_NDL_webOS4_Lock = PTHREAD_MUTEX_INITIALIZER;
bool SS4S_NDL_webOS4_Initialized = false;
SS4S_LoggingFunction *SS4S_NDL_webOS4_Log = NULL;
const SS4S_LibraryContext *SS4S_NDL_webOS4_LibContext = NULL;

SS4S_EXPORTED bool SS4S_ModuleOpen_NDL_WEBOS4(SS4S_Module *module, const SS4S_LibraryContext *context) {
    SS4S_NDL_webOS4_LibContext = context;
    SS4S_NDL_webOS4_Log = context->Log;
    assert(SS4S_NDL_webOS4_Log != NULL);
    module->Name = "ndl-webos4";
    module->PlayerDriver = &SS4S_NDL_webOS4_PlayerDriver;
    module->AudioDriver = &SS4S_NDL_webOS4_AudioDriver;
    module->VideoDriver = &SS4S_NDL_webOS4_VideoDriver;
    return true;
}

SS4S_EXPORTED SS4S_ModuleCheckFlag SS4S_ModuleCheck_NDL_WEBOS4(SS4S_ModuleCheckFlag flags) {
    FILE *f = fopen("/etc/prefs/properties/machineName", "r");
    char machine_name[16] = {0};
    if (f != NULL) {
        size_t read_len = fread(machine_name, 1, sizeof(machine_name), f);
        fclose(f);
        if (read_len > 0 && memcmp(machine_name, "k5lp", 4) == 0) {
            // k5lp SoC is not supported
            return 0;
        }
    }
    return flags;
}

int SS4S_NDL_webOS4_Driver_Init(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    if (SS4S_NDL_webOS4_Initialized) {
        return 0;
    }
    int ret;
    if ((ret = NDL_DirectMediaInit(getenv("APPID"), NULL)) == 0) {
        SS4S_NDL_webOS4_Initialized = true;
        SS4S_NDL_webOS4_Log(SS4S_LogLevelInfo, "NDL", "Driver init.");
    } else {
        SS4S_NDL_webOS4_Log(SS4S_LogLevelError, "NDL", "Failed to init: ret=%d, error=%s", ret,
                            NDL_DirectMediaGetError());
    }
    return ret;
}

void SS4S_NDL_webOS4_Driver_Quit() {
    if (!SS4S_NDL_webOS4_Initialized) {
        return;
    }
    SS4S_NDL_webOS4_Log(SS4S_LogLevelInfo, "NDL", "Driver quit.");
    NDL_DirectMediaQuit();
    SS4S_NDL_webOS4_Initialized = false;
}