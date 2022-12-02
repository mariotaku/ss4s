#include "ss4s/modapi.h"
#include "ndl_common.h"

#include <NDL_directmedia.h>
#include <stdlib.h>

bool SS4S_LGNC_Initialized = false;

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_LGNC(SS4S_Module *module) {
    module->Name = "ndl-webos4";
    module->PlayerDriver = &SS4S_LGNC_PlayerDriver;
    module->AudioDriver = &SS4S_LGNC_AudioDriver;
    module->VideoDriver = &SS4S_LGNC_VideoDriver;
    return true;
}

void SS4S_LGNC_Driver_Init(int argc, char *argv[]) {
    if (SS4S_LGNC_Initialized) {
        return;
    }
    if (NDL_DirectMediaInit(getenv("APPID"), NULL)) {
        SS4S_LGNC_Initialized = true;
    }
}

void SS4S_LGNC_Driver_Quit() {
    if (!SS4S_LGNC_Initialized) {
        return;
    }
    NDL_DirectMediaQuit();
}