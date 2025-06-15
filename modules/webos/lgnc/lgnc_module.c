#include "ss4s/modapi.h"
#include "lgnc_common.h"

#include <lgnc_plugin.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "jail_check.h"
#include "m3_kadp_fix.h"
#include "read_machine_name.h"

bool SS4S_LGNC_Initialized = false;
SS4S_LoggingFunction *SS4S_LGNC_Log = NULL;

SS4S_EXPORTED bool SS4S_ModuleOpen_LGNC(SS4S_Module *module, const SS4S_LibraryContext *context) {
    (void) context;
    SS4S_LGNC_Log = context->Log;
    assert(SS4S_LGNC_Log != NULL);
    module->Name = "lgnc";
    module->PlayerDriver = &SS4S_LGNC_PlayerDriver;
    module->AudioDriver = &SS4S_LGNC_AudioDriver;
    module->VideoDriver = &SS4S_LGNC_VideoDriver;
    return true;
}

SS4S_EXPORTED SS4S_ModuleCheckFlag SS4S_ModuleCheck_LGNC(SS4S_ModuleCheckFlag flags) {
    if (flags & SS4S_MODULE_CHECK_AUDIO) {
        char machine_name[16] = {0};
        if (SS4S_webOS_ReadMachineName(machine_name, sizeof(machine_name)) != 0) {
            return 0;
        }
        // SoC m16p has issues with direct audio, see https://github.com/mariotaku/moonlight-tv/issues/456
        if (strcmp(machine_name, "m16p") == 0 || SS4S_webOS_IsJailConfigBroken(machine_name)) {
            flags &= ~SS4S_MODULE_CHECK_AUDIO;
        }
    }
    return flags;
}

int SS4S_LGNC_Driver_Init() {
    if (SS4S_LGNC_Initialized) {
        return 0;
    }
    LGNC_CALLBACKS_T callbacks = {.msgHandler=NULL};
    int ret;
    if ((ret = LGNC_PLUGIN_Initialize(&callbacks)) == 0) {
        LGNC_PLUGIN_SetAppId(getenv("APPID"));
        SS4S_LGNC_Log(SS4S_LogLevelInfo, "LGNC", "Driver init.");
        SS4S_LGNC_Initialized = true;
    }
    SS4S_webOS_M3_KADP_Fix();
    return ret;
}

void SS4S_LGNC_Driver_Quit() {
    if (!SS4S_LGNC_Initialized) {
        return;
    }
    SS4S_LGNC_Log(SS4S_LogLevelInfo, "LGNC", "Driver quit.");
    LGNC_PLUGIN_Finalize();
    SS4S_LGNC_Initialized = false;
}