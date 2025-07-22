#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "core/include/error.h"

error_t performance_init(void);
void performance_update(void);
void performance_dump_counters(void);
void performance_shutdown(void);

typedef struct {
    uint64_t cpu_usage;
    uint64_t memory_usage;
} performance_stats_t;

void performance_get_stats(performance_stats_t* stats);

#endif