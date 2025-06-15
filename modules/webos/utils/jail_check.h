#pragma once

#include <stdbool.h>

/**
 * On k3lp/k5lp machines, default jailer config is broken, causing /dev/rtkmem to be missing.
 * This function checks if /dev/rtkmem is missing.
 */
bool SS4S_webOS_IsJailConfigBroken(const char *machine_name);