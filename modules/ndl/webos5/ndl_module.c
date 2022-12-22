#include <stdlib.h>
#include "ss4s/modapi.h"
#include "ndl_common.h"

bool SS4S_NDL_webOS5_Initialized = false;

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_NDL_WEBOS5(SS4S_Module *module) {
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
    }
    return ret;
}

void SS4S_NDL_webOS5_Driver_Quit() {
    if (!SS4S_NDL_webOS5_Initialized) {
        return;
    }
    NDL_DirectMediaQuit();
}