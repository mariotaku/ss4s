#include "modules_ini.h"

#ifdef HAS_DLADDR

#include <dlfcn.h>
#include <string.h>
#include <limits.h>

#else
#include <unistd.h>

#endif

FILE *SS4S_ModulesListFileOpen() {
#ifdef HAS_DLADDR
    Dl_info info;
    if (dladdr(SS4S_ModulesListFileOpen, &info) == 0) {
        return NULL;
    }
    const char *dirnameEnd = strrchr(info.dli_fname, '/');
    if (dirnameEnd == NULL) {
        return NULL;
    }
    char iniPath[PATH_MAX];
    snprintf(iniPath, PATH_MAX, "%.*s/ss4s_modules.ini", (int) (dirnameEnd - info.dli_fname), info.dli_fname);
    return fopen(iniPath, "r");
#else
    char iniPath[255];
    snprintf(iniPath, 255, "%.*s/ss4s_modules.ini", getcwd());
    return fopen(iniPath, "r");
#endif
}

void SS4S_ModulesListFileClose(FILE *file) {
    fclose(file);
}