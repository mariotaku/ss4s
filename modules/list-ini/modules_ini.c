#include "modules_ini.h"

#include <stdio.h>
#include <limits.h>

#ifdef HAS_DLADDR

#include <dlfcn.h>
#include <string.h>

#endif

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

FILE *SS4S_ModulesListFileOpen() {
    char iniPath[PATH_MAX];
#ifdef HAS_DLADDR
    Dl_info info;
    if (dladdr(SS4S_ModulesListFileOpen, &info) == 0) {
        return NULL;
    }
    const char *dirnameEnd = strrchr(info.dli_fname, '/');
    if (dirnameEnd == NULL) {
        return NULL;
    }
    snprintf(iniPath, PATH_MAX, "%.*s/ss4s_modules.ini", (int) (dirnameEnd - info.dli_fname), info.dli_fname);
#else
    snprintf(iniPath, 255, "ss4s_modules.ini");
#endif
    return fopen(iniPath, "r");
}

void SS4S_ModulesListFileClose(FILE *file) {
    fclose(file);
}