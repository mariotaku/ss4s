#include <assert.h>
#include <string.h>
#include <stdlib.h>

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

void SS4S_Init(int argc, char *argv[], const SS4S_Config *config) {
    SS4S_Module module;
    if (SS4S_ModuleOpen(config->audioDriver, &module)) {
        assert(module.Name != NULL);
        States.Audio.Driver = module.AudioDriver;
        States.Audio.ModuleName = module.Name;
        if (States.Audio.Driver != NULL) {
            States.Audio.PlayerDriver = module.PlayerDriver;
            SS4S_DriverInit(&States.Audio.Driver->Base, argc, argv);
        }
    }
    if (SS4S_ModuleOpen(config->videoDriver, &module)) {
        assert(module.Name != NULL);
        States.Video.Driver = module.VideoDriver;
        States.Video.ModuleName = module.Name;
        if (States.Video.Driver != NULL) {
            States.Video.PlayerDriver = module.PlayerDriver;
            SS4S_DriverInit(&States.Video.Driver->Base, argc, argv);
        }
    }
}

void SS4S_PostInit(int argc, char *argv[]) {
    if (States.Audio.Driver != NULL) {
        SS4S_DriverPostInit(&States.Audio.Driver->Base, argc, argv);
    }
    if (States.Video.Driver != NULL) {
        SS4S_DriverPostInit(&States.Video.Driver->Base, argc, argv);
    }
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
    return States.Video.PlayerDriver;
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
