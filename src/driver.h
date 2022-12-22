#pragma once

#include "ss4s/modapi.h"

int SS4S_DriverInit(const SS4S_DriverBase *base, int argc, char *argv[]);

int SS4S_DriverPostInit(const SS4S_DriverBase *base, int argc, char *argv[]);

void SS4S_DriverQuit(const SS4S_DriverBase *base);