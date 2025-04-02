#pragma once

#include <stdio.h>

static int SS4S_webOS_ReadMachineName(char *machine_name, size_t size) {
    // Read SoC name
    FILE *f = fopen("/etc/prefs/properties/machineName", "r");
    if (f == NULL) {
        return -1;
    }
    size_t read_len = fread(machine_name, 1, size, f);
    fclose(f);
    if (read_len <= 0) {
        return -1;
    }
    return 0;
}