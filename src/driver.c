#include <stddef.h>

#include "driver.h"

int SS4S_DriverInit(const SS4S_DriverBase *base, int argc, char *argv[]) {
    if (base->Init == NULL) {
        return 0;
    }
    return base->Init(argc, argv);
}

int SS4S_DriverPostInit(const SS4S_DriverBase *base, int argc, char *argv[]) {
    if (base->PostInit == NULL) {
        return 0;
    }
    return base->PostInit(argc, argv);
}

void SS4S_DriverQuit(const SS4S_DriverBase *base) {
    if (base->Quit == NULL) {
        return;
    }
    base->Quit();
}