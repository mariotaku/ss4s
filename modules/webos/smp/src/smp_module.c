#include <assert.h>
#include <stdlib.h>

#include "ss4s/modapi.h"
#include "smp_player.h"

extern const SS4S_PlayerDriver StarfishPlayerDriver;
extern const SS4S_AudioDriver StarfishAudioDriver;
extern const SS4S_VideoDriver StarfishVideoDriver;

const SS4S_LibraryContext *StarfishLibContext = NULL;

SS4S_EXPORTED bool SS4S_MODULE_ENTRY(SS4S_Module *module, const SS4S_LibraryContext *context) {
    StarfishLibContext = context;
    assert(StarfishLibContext != NULL);
    module->Name = SS4S_MODULE_NAME;
    module->PlayerDriver = &StarfishPlayerDriver;
    module->AudioDriver = &StarfishAudioDriver;
    module->VideoDriver = &StarfishVideoDriver;
    return true;
}