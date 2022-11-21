#include <stddef.h>

#include "driver.h"

void SS4S_DriverInit(const SS4S_DriverBase *base, int argc, char *argv[]) {
    if (base->Init == NULL) {
        return;
    }
    base->Init(argc, argv);
}

void SS4S_DriverPostInit(const SS4S_DriverBase *base, int argc, char *argv[]) {
    if (base->PostInit == NULL) {
        return;
    }
    base->PostInit(argc, argv);
}

void SS4S_DriverQuit(const SS4S_DriverBase *base) {
    if (base->Quit == NULL) {
        return;
    }
    base->Quit();
}