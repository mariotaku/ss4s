#include "ss4s/modapi.h"
#include "lgnc_common.h"

#include <lgnc_plugin.h>
#include <stdlib.h>

bool SS4S_LGNC_Initialized = false;

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_LGNC(SS4S_Module *module) {
    module->Name = "lgnc";
    module->PlayerDriver = &SS4S_LGNC_PlayerDriver;
    module->AudioDriver = &SS4S_LGNC_AudioDriver;
    module->VideoDriver = &SS4S_LGNC_VideoDriver;
    return true;
}

void SS4S_LGNC_Driver_Init(int argc, char *argv[]) {
    if (SS4S_LGNC_Initialized) {
        return;
    }
    LGNC_CALLBACKS_T callbacks = {.msgHandler=NULL};
    if (LGNC_PLUGIN_Initialize(&callbacks)) {
        LGNC_PLUGIN_SetAppId(getenv("APPID"));
        SS4S_LGNC_Initialized = true;
    }
}

void SS4S_LGNC_Driver_Quit() {
    if (!SS4S_LGNC_Initialized) {
        return;
    }
    LGNC_PLUGIN_Finalize();
}