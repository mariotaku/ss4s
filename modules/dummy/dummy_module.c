#include <assert.h>
#include "ss4s/modapi.h"
#include "dummy_common.h"

bool SS4S_Dummy_Initialized = false;
SS4S_LoggingFunction *SS4S_Dummy_Log = NULL;

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_NDL_DUMMY(SS4S_Module *module, const SS4S_LibraryContext *context) {
    (void) context;
    SS4S_Dummy_Log = context->Log;
    assert(SS4S_Dummy_Log != NULL);
    module->Name = "dummy";
    module->PlayerDriver = &SS4S_Dummy_PlayerDriver;
    module->AudioDriver = &SS4S_Dummy_AudioDriver;
    module->VideoDriver = &SS4S_Dummy_VideoDriver;
    return true;
}

int SS4S_Dummy_Driver_PostInit(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    if (SS4S_Dummy_Initialized) {
        return 0;
    }
    SS4S_Dummy_Initialized = true;
    SS4S_Dummy_Log(SS4S_LogLevelInfo, "Dummy", "Driver initialized.");
    return 0;
}

void SS4S_Dummy_Driver_Quit() {
    if (!SS4S_Dummy_Initialized) {
        return;
    }
    SS4S_Dummy_Log(SS4S_LogLevelInfo, "Dummy", "Driver quit.");
    SS4S_Dummy_Initialized = false;
}