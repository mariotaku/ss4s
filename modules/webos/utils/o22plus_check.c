#include "o22plus_check.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool SS4S_webOS_Is_O22_And_Above() {
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
    if (machine_name[0] != 'o') {
        return false;
    }
    int year = strtol(machine_name + 1, NULL, 10);
    return year >= 22;
}