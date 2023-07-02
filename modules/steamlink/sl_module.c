#include "ss4s/modapi.h"

#include "sl_common.h"

pthread_mutex_t SS4S_SteamLink_Lock = PTHREAD_MUTEX_INITIALIZER;
SS4S_LoggingFunction *SS4S_SteamLink_Log = NULL;

SS4S_EXPORTED bool SS4S_ModuleOpen_STEAMLINK(SS4S_Module *module, const SS4S_LibraryContext *context) {
    SS4S_SteamLink_Log = context->Log;
    module->Name = "steamlink";
    module->PlayerDriver = &SS4S_STEAMLINK_PlayerDriver;
    module->AudioDriver = &SS4S_STEAMLINK_AudioDriver;
    module->VideoDriver = &SS4S_STEAMLINK_VideoDriver;
    return true;
}