//
// Created by Mariotaku on 2024/11/18.
//

#include "knlp_check.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

bool SS4S_webOS_KNLP_IsJailConfigBroken() {
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
    // We're fine with non-k5lp/k3lp machines
    if (memcmp(machine_name, "k5lp", 4) != 0 && memcmp(machine_name, "k3lp", 4) != 0) {
        return false;
    }
    // Make sure /dev/rtkmem is readable, otherwise we have a broken jailer config
    return access("/dev/rtkmem", R_OK) != 0;
}