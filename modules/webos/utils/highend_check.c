#include "highend_check.h"
#include "read_machine_name.h"

#include <string.h>
#include <stdlib.h>

static bool Is_Machine_HighEnd(const char *machine_name);


bool SS4S_webOS_Is_HighEnd_SoC() {
    char machine_name[16] = {0};
    if (SS4S_webOS_ReadMachineName(machine_name, sizeof(machine_name)) != 0) {
        return false;
    }
    return Is_Machine_HighEnd(machine_name);
}

bool Is_Machine_HighEnd(const char *machine_name) {
    if (machine_name[0] == 'o') {
        int year = strtol(machine_name + 1, NULL, 10);
        return year >= 22;
    } else if (machine_name[0] == 'k') {
        char *remaining = NULL;
        int generation = strtol(machine_name + 1, &remaining, 10);
        if (generation < 8) {
            return false;
        }
        return remaining != NULL && strncmp(remaining, "hp", 2) == 0;
    }
    return false;
}