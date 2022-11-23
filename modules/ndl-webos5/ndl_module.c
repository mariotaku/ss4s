#include "ss4s/modapi.h"
#include "ndl_common.h"

bool NDL_webOS5_Initialized;

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_NDL_WEBOS5(SS4S_Module *module) {
    module->Name = "ndl-webos5";
    module->PlayerDriver = &NDL_webOS5_PlayerDriver;
    module->AudioDriver = &NDL_webOS5_AudioDriver;
    module->VideoDriver = &NDL_webOS5_VideoDriver;
    return true;
}