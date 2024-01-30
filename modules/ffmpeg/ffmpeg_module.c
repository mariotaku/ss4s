#include "ffmpeg_common.h"


 const SS4S_LibraryContext *SS4S_FFMPEG_LibContext;


SS4S_EXPORTED bool SS4S_ModuleOpen_FFMPEG(SS4S_Module *module, const SS4S_LibraryContext *context) {
    module->Name = "ffmpeg";
    module->VideoDriver = &SS4S_FFMPEG_VideoDriver;
    module->PlayerDriver = &SS4S_FFMPEG_PlayerDriver;
    SS4S_FFMPEG_LibContext = context;
    return true;
}
