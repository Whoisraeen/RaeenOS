#ifndef EXECUTABLE_LOADER_H
#define EXECUTABLE_LOADER_H

#include "core/include/types.h"
#include "core/include/error.h"

// Executable loader functions
error_t ramdisk_init(void);
error_t executable_loader_init(void);

#endif