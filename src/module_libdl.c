#include "module.h"

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static void ModuleFileName(char *out, size_t outLen, const char *name);

static void ModuleFunctionName(char *out, size_t outLen, const char *fnName, const char *module);

bool SS4S_ModuleOpen(const char *name, SS4S_Module *module, const SS4S_LibraryContext *context) {
    if (name == NULL || name[0] == '\0') {
        return false;
    }
    char tmp[128];
    ModuleFileName(tmp, sizeof(tmp), name);
    void *lib = dlopen(tmp, RTLD_NOW);
    if (lib == NULL) {
        return false;
    }
    ModuleFunctionName(tmp, sizeof(tmp), "Open", name);
    SS4S_ModuleOpenFunction *fn = (SS4S_ModuleOpenFunction *) dlsym(lib, tmp);
    if (fn == NULL) {
        context->Log(SS4S_LogLevelError, "Module", "Module `%s` is not valid!", name);
        return false;
    }
    memset(module, 0, sizeof(SS4S_Module));
    return fn(module, context);
}

bool SS4S_ModuleAvailable(const char *name, SS4S_ModuleCheckFlag flags) {
    return SS4S_ModuleCheck(name, flags) == flags;
}

SS4S_ModuleCheckFlag SS4S_ModuleCheck(const char *id, SS4S_ModuleCheckFlag flags) {
    if (id == NULL || id[0] == '\0') {
        return 0;
    }
    char tmp[128];
    ModuleFileName(tmp, sizeof(tmp), id);
    void *lib = dlopen(tmp, RTLD_NOW);
    if (lib == NULL) {
        return 0;
    }
    ModuleFunctionName(tmp, sizeof(tmp), "Check", id);
    SS4S_ModuleCheckFlag result = flags;
    SS4S_ModuleCheckFunction *fn = (SS4S_ModuleCheckFunction *) dlsym(lib, tmp);
    if (fn != NULL) {
        result = fn(flags);
    }
    dlclose(lib);
    return result;
}

static void ModuleFileName(char *out, size_t outLen, const char *name) {
    snprintf(out, outLen, "libss4s-%s.so", name);
}

static void ModuleFunctionName(char *out, size_t outLen, const char *fnName, const char *module) {
    int strLen = snprintf(out, outLen, "SS4S_Module%s_%s", fnName, module);
    /* Start transformation at strlen("SS4S_Module") + strlen(fnName) + strlen("_") */
    for (int i = 12 + (int) strlen(fnName); i < strLen; i++) {
        char ch = out[i];
        if (ch == '\0') {
            break;
        } else if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z')) {
            continue;
        } else if (ch >= 'a' && ch <= 'z') {
            out[i] = ch - 'a' + 'A';
        } else {
            out[i] = '_';
        }
    }
}