#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "ss4s.h"

#include "library.h"
#include "module.h"
#include "driver.h"

static struct {
    char *AppName;
    struct {
        const char *ModuleName;
        const SS4S_AudioDriver *Driver;
        const SS4S_PlayerDriver *PlayerDriver;
    } Audio;
    struct {
        const char *ModuleName;
        const SS4S_VideoDriver *Driver;
        const SS4S_PlayerDriver *PlayerDriver;
    } Video;
} States;

static SS4S_LoggingFunction *SS4S_Log = NULL;
static SS4S_LibraryContext SS4S_LibContext = {
        .Log = NULL,
};

static void StdIOLoggingFunction(SS4S_LogLevel level, const char *tag, const char *fmt, ...);

int SS4S_Init(int argc, char *argv[], const SS4S_Config *config) {
    SS4S_Log = config->loggingFunction != NULL ? config->loggingFunction : StdIOLoggingFunction;
    SS4S_LibContext.Log = SS4S_Log;
    SS4S_Module module;
    if (config->audioDriver != NULL) {
        SS4S_Log(SS4S_LogLevelInfo, "Audio", "Opening driver %s", config->audioDriver);
        if (SS4S_ModuleOpen(config->audioDriver, &module, &SS4S_LibContext)) {
            assert(module.Name != NULL);
            if (module.AudioDriver != NULL && SS4S_DriverInit(&module.AudioDriver->Base, argc, argv) == 0) {
                States.Audio.Driver = module.AudioDriver;
                States.Audio.PlayerDriver = module.PlayerDriver;
                States.Audio.ModuleName = module.Name;
                SS4S_Log(SS4S_LogLevelInfo, "Audio", "Opened driver: %s", module.Name);
            }
        }
    }
    if (config->videoDriver != NULL) {
        SS4S_Log(SS4S_LogLevelInfo, "Video", "Opening driver %s", config->videoDriver);
        if (SS4S_ModuleOpen(config->videoDriver, &module, &SS4S_LibContext)) {
            assert(module.Name != NULL);
            if (module.VideoDriver != NULL && SS4S_DriverInit(&module.VideoDriver->Base, argc, argv) == 0) {
                States.Video.Driver = module.VideoDriver;
                States.Video.PlayerDriver = module.PlayerDriver;
                States.Video.ModuleName = module.Name;
                SS4S_Log(SS4S_LogLevelInfo, "Video", "Opened driver: %s", module.Name);
            }
        }
    }
    return 0;
}

int SS4S_PostInit(int argc, char *argv[]) {
    int aret = 0, vret = 0;
    if (States.Audio.Driver != NULL) {
        aret = SS4S_DriverPostInit(&States.Audio.Driver->Base, argc, argv);
    }
    if (States.Video.Driver != NULL) {
        vret = SS4S_DriverPostInit(&States.Video.Driver->Base, argc, argv);
    }
    return vret != 0 ? vret : aret;
}

void SS4S_Quit() {
    if (States.Audio.Driver != NULL) {
        SS4S_DriverQuit(&States.Audio.Driver->Base);
        States.Audio.Driver = NULL;
    }
    if (States.Video.Driver != NULL) {
        SS4S_DriverQuit(&States.Video.Driver->Base);
        States.Video.Driver = NULL;
    }
    if (States.AppName != NULL) {
        free(States.AppName);
    }
}

const SS4S_AudioDriver *SS4S_GetAudioDriver() {
    return States.Audio.Driver;
}

const SS4S_VideoDriver *SS4S_GetVideoDriver() {
    return States.Video.Driver;
}

const SS4S_PlayerDriver *SS4S_GetAudioPlayerDriver() {
    return States.Audio.PlayerDriver;
}

const SS4S_PlayerDriver *SS4S_GetVideoPlayerDriver() {
    return States.Video.PlayerDriver;
}

const char *SS4S_GetAudioModuleName() {
    return States.Audio.ModuleName;
}

const char *SS4S_GetVideoModuleName() {
    return States.Video.ModuleName;
}

static void StdIOLoggingFunction(SS4S_LogLevel level, const char *tag, const char *fmt, ...) {
    (void) level;
    fprintf(stderr, "[SS4S.%s] ", tag);
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    printf("\n");
}