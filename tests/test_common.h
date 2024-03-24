#pragma once

#include <string.h>

static void single_test_infer_module(char *name, size_t name_size, const char *prefix, int argc, char *argv[]) {
    if (argc > 1) {
        strncpy(name, argv[1], name_size - 1);
    } else {
        const char *basename = strstr(argv[0], prefix);
        if (basename != NULL) {
            const char *suffix = basename + strlen(prefix);
            const char *extname = strrchr(suffix, '.');
            size_t name_len = 0;
            if (extname != NULL) {
                name_len = extname - suffix;
            } else {
                name_len = strlen(suffix);
            }
            if (name_len > name_size - 1) {
                name_len = name_size - 1;
            }
            strncpy(name, suffix, name_len);
        }
    }
}