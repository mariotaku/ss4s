#include "ss4s/modapi.h"
#include "lgnc_common.h"

#include <lgnc_plugin.h>
#include <stdlib.h>

bool SS4S_LGNC_Initialized = false;

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_LGNC(SS4S_Module *module, const SS4S_LibraryContext *context) {
    (void) context;
    module->Name = "lgnc";
    module->PlayerDriver = &SS4S_LGNC_PlayerDriver;
    module->AudioDriver = &SS4S_LGNC_AudioDriver;
    module->VideoDriver = &SS4S_LGNC_VideoDriver;
    return true;
}

int SS4S_LGNC_Driver_Init() {
    if (SS4S_LGNC_Initialized) {
        return 0;
    }
    LGNC_CALLBACKS_T callbacks = {.msgHandler=NULL};
    int ret;
    if ((ret = LGNC_PLUGIN_Initialize(&callbacks)) == 0) {
        LGNC_PLUGIN_SetAppId(getenv("APPID"));
        SS4S_LGNC_Initialized = true;
    }
    return ret;
}

void SS4S_LGNC_Driver_Quit() {
    if (!SS4S_LGNC_Initialized) {
        return;
    }
    LGNC_PLUGIN_Finalize();
}