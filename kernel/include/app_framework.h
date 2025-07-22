#ifndef APP_FRAMEWORK_H
#define APP_FRAMEWORK_H

#include "core/include/error.h"

error_t app_framework_init(void);
void app_framework_dump_applications(void);

typedef struct {
    uint32_t apps_running;
    uint32_t apps_installed;
} app_stats_t;

void app_framework_get_stats(app_stats_t* stats);

#endif