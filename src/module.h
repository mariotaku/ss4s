#pragma once

#include "ss4s/modapi.h"

bool SS4S_ModuleOpen(const char *name, SS4S_Module *module, const SS4S_LibraryContext *context);

bool SS4S_ModuleAvailable(const char *name);
