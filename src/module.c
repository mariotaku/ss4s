#include "module.h"

bool SS4S_ModuleAvailable(const char *name, SS4S_ModuleCheckFlag flags) {
    return SS4S_ModuleCheck(name, flags) == flags;
}