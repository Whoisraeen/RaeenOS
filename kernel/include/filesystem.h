#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "core/include/error.h"

error_t filesystem_init(void);

typedef struct {
    uint64_t files_open;
    uint64_t bytes_read;
    uint64_t bytes_written;
} filesystem_stats_t;

void vfs_get_stats(filesystem_stats_t* stats);

#endif