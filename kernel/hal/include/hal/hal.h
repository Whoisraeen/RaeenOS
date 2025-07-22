#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../../core/include/types.h"
#include "../../core/include/error.h"

// HAL initialization and basic functions
error_t hal_init(void);
void hal_disable_interrupts(void);
void hal_enable_interrupts(void);
void hal_halt_cpu(void);

// Console functions
void hal_early_console_init(void);
void hal_console_print(const char* format, ...);
void hal_console_vprint(const char* format, va_list args);
void hal_console_set_color(uint32_t color);
bool hal_is_console_ready(void);

// Timing functions
uint64_t hal_get_timestamp(void);
void hal_sleep_ms(uint32_t ms);

// Debug functions
void hal_dump_registers(void);
void hal_dump_stack_trace(void);

#endif