#include "highend_check.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static inline bool Is_Machine_HighEnd(const char *machine_name) {
    if (machine_name[0] == 'o') {
        int year = strtol(machine_name + 1, NULL, 10);
        return year >= 22;
    } else if (machine_name[0] != 'k') {
        char *remaining = NULL;
        int generation = strtol(machine_name + 1, &remaining, 10);
        if (generation < 8) {
            return false;
        }
        return remaining != NULL && strncmp(remaining, "hp", 2) == 0;
    }
    return false;
}

bool SS4S_webOS_Is_HighEnd_SoC() {
    // Read SoC name
    FILE *f = fopen("/etc/prefs/properties/machineName", "r");
    char machine_name[16] = {0};
    if (f == NULL) {
        return false;
    }
    size_t read_len = fread(machine_name, 1, sizeof(machine_name), f);
    fclose(f);
    if (read_len <= 0) {
        return false;
    }
    return Is_Machine_HighEnd(machine_name);
}