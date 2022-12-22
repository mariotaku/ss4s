#include "ss4s/modapi.h"
#include "ndl_common.h"

#include "NDL_directmedia.h"
#include <stdlib.h>

bool SS4S_NDL_webOS4_Initialized = false;

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_NDL_WEBOS4(SS4S_Module *module) {
    module->Name = "ndl-webos4";
    module->PlayerDriver = &SS4S_NDL_webOS4_PlayerDriver;
    module->AudioDriver = &SS4S_NDL_webOS4_AudioDriver;
    module->VideoDriver = &SS4S_NDL_webOS4_VideoDriver;
    return true;
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
    }
    return ret;
}

void SS4S_NDL_webOS4_Driver_Quit() {
    if (!SS4S_NDL_webOS4_Initialized) {
        return;
    }
    NDL_DirectMediaQuit();
}