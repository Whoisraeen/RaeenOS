#include "filesystem/include/filesystem.h"
#include "kernel.h"

// Placeholder ProcFS implementation
filesystem_operations_t procfs_ops = {
    .mount = NULL,
    .unmount = NULL,
    .read_super = NULL
};

// Placeholder for RaeenFS
filesystem_operations_t raeenfs_ops = {
    .mount = NULL,
    .unmount = NULL,
    .read_super = NULL
};