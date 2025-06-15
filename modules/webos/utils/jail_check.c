#include "jail_check.h"

#include <string.h>
#include <unistd.h>

bool SS4S_webOS_IsJailConfigBroken(const char *machine_name) {
    // Only k3lp/k5lp needs to be checked
    if (memcmp(machine_name, "k5lp", 4) == 0 || memcmp(machine_name, "k3lp", 4) == 0) {
        // Make sure /dev/rtkmem is readable, otherwise we have a broken jailer config
        return access("/dev/rtkmem", R_OK) != 0;
    }
    return false;
}