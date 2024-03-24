#include "lgnc_plugin.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

int LGNC_PLUGIN_Initialize(LGNC_CALLBACKS_T *callbacks) {
    assert(callbacks != NULL);
    printf("[LGNC] Initialized\n");
    return 0;
}

int LGNC_PLUGIN_SetAppId(const char *appId) {
    assert(appId != NULL);
    printf("[LGNC] App ID: %s\n", appId);
    return 0;
}

int LGNC_PLUGIN_Finalize() {
    printf("[LGNC] Finalized\n");
    return 0;
}