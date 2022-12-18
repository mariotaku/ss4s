#include "ss4s/modapi.h"
#include "ndl_common.h"

bool SS4S_NDL_webOS5_Initialized;

SS4S_MODULE_ENTRY bool SS4S_ModuleOpen_NDL_WEBOS5(SS4S_Module *module) {
    module->Name = "ndl-webos5";
    module->PlayerDriver = &SS4S_NDL_webOS5_PlayerDriver;
    module->AudioDriver = &SS4S_NDL_webOS5_AudioDriver;
    module->VideoDriver = &SS4S_NDL_webOS5_VideoDriver;
    return true;
}