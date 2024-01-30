#include "ss4s/modapi.h"

extern const SS4S_PlayerDriver SS4S_FFMPEG_PlayerDriver;
extern const SS4S_LibraryContext *SS4S_FFMPEG_LibContext;
extern const SS4S_VideoDriver SS4S_FFMPEG_VideoDriver;

struct SS4S_PlayerContext {
    SS4S_Player *player;
};
