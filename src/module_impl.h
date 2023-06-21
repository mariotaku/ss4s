#pragma once

#include <stddef.h>
#include <stdio.h>
#include <string.h>

__attribute((unused)) static void ModuleFileName(char *out, size_t outLen, const char *name) {
    snprintf(out, outLen, "ss4s-%s%s", name, SS4S_LIBRARY_SUFFIX);
}

__attribute((unused)) static void ModuleFunctionName(char *out, size_t outLen, const char *fnName, const char *module) {
    int strLen = snprintf(out, outLen, "SS4S_Module%s_%s", fnName, module);
    /* Start transformation at strlen("SS4S_Module") + strlen(fnName) + strlen("_") */
    for (int i = 12 + (int) strlen(fnName); i < strLen; i++) {
        char ch = out[i];
        if (ch == '\0') {
            break;
        } else if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z')) {
            continue;
        } else if (ch >= 'a' && ch <= 'z') {
            out[i] = (char) (ch - 'a' + 'A');
        } else {
            out[i] = '_';
        }
    }
}