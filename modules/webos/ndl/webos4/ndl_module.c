#include "ss4s/modapi.h"
#include "ndl_common.h"

#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>

pthread_mutex_t SS4S_NDL_webOS4_Lock = PTHREAD_MUTEX_INITIALIZER;
bool SS4S_NDL_webOS4_Initialized = false;
SS4S_LoggingFunction *SS4S_NDL_webOS4_Log = NULL;

SS4S_EXPORTED bool SS4S_ModuleOpen_NDL_WEBOS4(SS4S_Module *module, const SS4S_LibraryContext *context) {
    (void) context;
    SS4S_NDL_webOS4_Log = context->Log;
    assert(SS4S_NDL_webOS4_Log != NULL);
    module->Name = "ndl-webos4";
    module->PlayerDriver = &SS4S_NDL_webOS4_PlayerDriver;
    module->AudioDriver = &SS4S_NDL_webOS4_AudioDriver;
    module->VideoDriver = &SS4S_NDL_webOS4_VideoDriver;
    return true;
}

SS4S_EXPORTED SS4S_ModuleCheckFlag SS4S_ModuleCheck_NDL_WEBOS4(SS4S_ModuleCheckFlag flags) {
    if (flags & SS4S_MODULE_CHECK_AUDIO) {
        /*
         * A simple check for unsupported hardware. A partial solution is to use ALSA/PulseAudio, so we fail the check
         * if we have the problematic library, and let SS4S choose other modules
         * Related issues:
         * https://github.com/mariotaku/moonlight-tv/issues/110
         * https://github.com/mariotaku/moonlight-tv/issues/184
         * https://github.com/mariotaku/ihsplay/issues/13
         */
        void *lib = dlopen("libkadaptor.so.1", RTLD_LAZY);
        if (lib != NULL) {
            bool has_flow = dlsym(lib, "_Z8new_flowv") != NULL;
            dlclose(lib);
            if (has_flow) {
                // If we have new_flow function, report audio model is not compatible
                return flags & ~SS4S_MODULE_CHECK_AUDIO;
            }
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