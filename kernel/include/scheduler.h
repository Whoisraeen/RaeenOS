#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "core/include/error.h"
#include "process.h"

error_t scheduler_init(void);
void scheduler_tick(void);
void scheduler_yield(void);
process_t* scheduler_get_current_process(void);
uint32_t scheduler_get_thread_count(void);
void scheduler_dump_info(void);

typedef struct {
    uint64_t context_switches;
    uint64_t processes_created;
} scheduler_stats_t;

void scheduler_get_stats(scheduler_stats_t* stats);

#endif