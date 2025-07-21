#include "timer.h"
#include "../core/kernel.h"
#include "../core/interrupts.h"
#include "../hal/hal.h"
#include "../process/scheduler.h"
#include <string.h>

// Global timer subsystem
static timer_subsystem_t timer_subsystem = {0};
static bool timer_initialized = false;

// Timer interrupt handler
void timer_interrupt_handler(void) {
    if (!timer_initialized) {
        return;
    }
    
    // Increment system tick counter
    timer_subsystem.system_ticks++;
    timer_subsystem.system_time_ms = timer_subsystem.system_ticks;
    timer_subsystem.stats.total_ticks++;
    
    // Process active timers
    uint64_t current_time = timer_subsystem.system_time_ms;
    timer_t* timer = timer_subsystem.timer_list;
    timer_t* prev = NULL;
    
    while (timer) {
        if (timer->active && current_time >= timer->next_fire) {
            // Timer has fired
            if (timer->callback) {
                uint64_t callback_start = timer_get_timestamp();
                timer->callback(timer->data);
                timer_subsystem.stats.callback_time += timer_get_timestamp() - callback_start;
            }
            
            if (timer->periodic) {
                // Reschedule periodic timer
                timer->next_fire = current_time + timer->interval;
            } else {
                // Remove one-shot timer
                timer->active = false;
                if (prev) {
                    prev->next = timer->next;
                } else {
                    timer_subsystem.timer_list = timer->next;
                }
                timer_subsystem.timer_count--;
                timer_subsystem.stats.timer_count--;
            }
        }
        
        prev = timer;
        timer = timer->next;
    }
    
    // Call scheduler tick
    scheduler_tick();
    
    // Send EOI to PIC
    pic_send_eoi(0);
}

// Initialize timer subsystem
error_t timer_init(void) {
    if (timer_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing timer subsystem");
    
    // Initialize timer subsystem structure
    memset(&timer_subsystem, 0, sizeof(timer_subsystem_t));
    timer_subsystem.initialized = true;
    timer_subsystem.stats.frequency = TIMER_FREQUENCY;
    
    // Initialize PIT
    error_t result = pit_init(TIMER_FREQUENCY);
    if (result != SUCCESS) {
        KERROR("Failed to initialize PIT");
        return result;
    }
    
    // Register timer interrupt handler
    register_interrupt_handler(IRQ_TIMER, timer_interrupt_handler);
    
    // Unmask timer IRQ
    pic_unmask_irq(0);
    
    timer_initialized = true;
    
    KINFO("Timer subsystem initialized at %u Hz", TIMER_FREQUENCY);
    return SUCCESS;
}

// Initialize PIT
error_t pit_init(uint32_t frequency) {
    if (frequency == 0 || frequency > PIT_FREQUENCY) {
        return E_INVAL;
    }
    
    KDEBUG("Initializing PIT at %u Hz", frequency);
    
    // Calculate divisor
    uint32_t divisor = PIT_FREQUENCY / frequency;
    if (divisor > 65535) {
        divisor = 65535;
    }
    
    // Send command to PIT
    uint8_t command = PIT_CHANNEL0 | PIT_ACCESS_BOTH | PIT_MODE2 | PIT_BINARY;
    hal_outb(PIT_COMMAND_PORT, command);
    
    // Send divisor
    hal_outb(PIT_CHANNEL0_DATA, divisor & 0xFF);        // Low byte
    hal_outb(PIT_CHANNEL0_DATA, (divisor >> 8) & 0xFF); // High byte
    
    KDEBUG("PIT initialized with divisor %u", divisor);
    return SUCCESS;
}

// Create a timer
timer_t* timer_create(uint64_t interval_ms, timer_callback_t callback, void* data, bool periodic) {
    if (interval_ms == 0 || !callback) {
        return NULL;
    }
    
    timer_t* timer = (timer_t*)kernel_heap_alloc(sizeof(timer_t), ALLOC_FLAG_ZERO);
    if (!timer) {
        return NULL;
    }
    
    timer->interval = interval_ms;
    timer->callback = callback;
    timer->data = data;
    timer->periodic = periodic;
    timer->active = false;
    timer->next = NULL;
    
    KDEBUG("Created timer: interval=%llu ms, periodic=%s", 
           interval_ms, periodic ? "yes" : "no");
    
    return timer;
}

// Start a timer
error_t timer_start(timer_t* timer) {
    if (!timer || !timer_initialized) {
        return E_INVAL;
    }
    
    if (timer->active) {
        return E_ALREADY;
    }
    
    // Set next fire time
    timer->next_fire = timer_subsystem.system_time_ms + timer->interval;
    timer->active = true;
    
    // Add to timer list
    timer->next = timer_subsystem.timer_list;
    timer_subsystem.timer_list = timer;
    timer_subsystem.timer_count++;
    timer_subsystem.stats.timer_count++;
    
    KDEBUG("Started timer: next_fire=%llu", timer->next_fire);
    return SUCCESS;
}

// Stop a timer
error_t timer_stop(timer_t* timer) {
    if (!timer || !timer_initialized) {
        return E_INVAL;
    }
    
    if (!timer->active) {
        return E_INVAL;
    }
    
    // Remove from timer list
    timer_t* current = timer_subsystem.timer_list;
    timer_t* prev = NULL;
    
    while (current) {
        if (current == timer) {
            if (prev) {
                prev->next = current->next;
            } else {
                timer_subsystem.timer_list = current->next;
            }
            timer_subsystem.timer_count--;
            timer_subsystem.stats.timer_count--;
            break;
        }
        prev = current;
        current = current->next;
    }
    
    timer->active = false;
    
    KDEBUG("Stopped timer");
    return SUCCESS;
}

// Destroy a timer
error_t timer_destroy(timer_t* timer) {
    if (!timer) {
        return E_INVAL;
    }
    
    // Stop timer if active
    if (timer->active) {
        timer_stop(timer);
    }
    
    // Free memory
    kernel_heap_free(timer);
    
    KDEBUG("Destroyed timer");
    return SUCCESS;
}

// Create one-shot timer
timer_t* timer_create_oneshot(uint64_t delay_ms, timer_callback_t callback, void* data) {
    return timer_create(delay_ms, callback, data, false);
}

// Create periodic timer
timer_t* timer_create_periodic(uint64_t interval_ms, timer_callback_t callback, void* data) {
    return timer_create(interval_ms, callback, data, true);
}

// Schedule one-shot timer
error_t timer_schedule_oneshot(uint64_t delay_ms, timer_callback_t callback, void* data) {
    timer_t* timer = timer_create_oneshot(delay_ms, callback, data);
    if (!timer) {
        return E_NOMEM;
    }
    
    error_t result = timer_start(timer);
    if (result != SUCCESS) {
        timer_destroy(timer);
        return result;
    }
    
    return SUCCESS;
}

// Get system ticks
uint64_t timer_get_ticks(void) {
    return timer_subsystem.system_ticks;
}

// Get system time in milliseconds
uint64_t timer_get_time_ms(void) {
    return timer_subsystem.system_time_ms;
}

// Get system time in microseconds
uint64_t timer_get_time_us(void) {
    return timer_subsystem.system_time_ms * 1000;
}

// Get high precision timestamp
uint64_t timer_get_timestamp(void) {
    // Use RDTSC for high precision timing
    uint32_t low, high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ((uint64_t)high << 32) | low;
}

// Get uptime in milliseconds
uint64_t timer_get_uptime_ms(void) {
    return timer_subsystem.system_time_ms;
}

// Get uptime in seconds
uint64_t timer_get_uptime_seconds(void) {
    return timer_subsystem.system_time_ms / 1000;
}

// Sleep for milliseconds
void timer_sleep_ms(uint64_t milliseconds) {
    if (!timer_initialized || milliseconds == 0) {
        return;
    }
    
    uint64_t start_time = timer_get_time_ms();
    uint64_t end_time = start_time + milliseconds;
    
    while (timer_get_time_ms() < end_time) {
        // Yield to other processes
        scheduler_yield();
    }
}

// Sleep for microseconds
void timer_sleep_us(uint64_t microseconds) {
    timer_sleep_ms(microseconds / 1000);
}

// Busy wait delay in milliseconds
void timer_delay_ms(uint64_t milliseconds) {
    if (!timer_initialized || milliseconds == 0) {
        return;
    }
    
    uint64_t start_time = timer_get_time_ms();
    uint64_t end_time = start_time + milliseconds;
    
    while (timer_get_time_ms() < end_time) {
        // Busy wait
        __asm__ volatile ("pause");
    }
}

// Busy wait delay in microseconds
void timer_delay_us(uint64_t microseconds) {
    if (microseconds < 1000) {
        // For very short delays, use busy wait
        uint64_t start = timer_get_timestamp();
        uint64_t cycles = microseconds * 3000; // Approximate cycles per microsecond
        while ((timer_get_timestamp() - start) < cycles) {
            __asm__ volatile ("pause");
        }
    } else {
        timer_delay_ms(microseconds / 1000);
    }
}

// Set PIT frequency
void pit_set_frequency(uint32_t frequency) {
    if (frequency == 0 || frequency > PIT_FREQUENCY) {
        return;
    }
    
    pit_init(frequency);
    timer_subsystem.stats.frequency = frequency;
}

// Get PIT frequency
uint32_t pit_get_frequency(void) {
    return timer_subsystem.stats.frequency;
}

// Get timer statistics
error_t timer_get_stats(timer_stats_t* stats) {
    if (!stats || !timer_initialized) {
        return E_INVAL;
    }
    
    *stats = timer_subsystem.stats;
    return SUCCESS;
}

// Reset timer statistics
void timer_reset_stats(void) {
    if (!timer_initialized) {
        return;
    }
    
    timer_subsystem.stats.total_ticks = 0;
    timer_subsystem.stats.missed_ticks = 0;
    timer_subsystem.stats.callback_time = 0;
}

// Get active timer count
uint32_t timer_get_active_count(void) {
    return timer_subsystem.timer_count;
}

// Get timer info
const char* timer_get_info(void) {
    static char info[256];
    snprintf(info, sizeof(info), 
             "Timer: %s, Freq: %u Hz, Ticks: %llu, Active: %u",
             timer_initialized ? "Initialized" : "Not initialized",
             timer_subsystem.stats.frequency,
             timer_subsystem.system_ticks,
             timer_subsystem.timer_count);
    return info;
}

// Check if timer is initialized
bool timer_is_initialized(void) {
    return timer_initialized;
}

// Calibrate timer
error_t timer_calibrate(void) {
    if (!timer_initialized) {
        return E_INVAL;
    }
    
    KINFO("Calibrating timer...");
    
    // Simple calibration - measure actual frequency
    uint64_t start_ticks = timer_get_ticks();
    timer_delay_ms(1000); // Wait 1 second
    uint64_t end_ticks = timer_get_ticks();
    
    uint32_t measured_frequency = (uint32_t)(end_ticks - start_ticks);
    
    KINFO("Timer calibration: expected %u Hz, measured %u Hz", 
          TIMER_FREQUENCY, measured_frequency);
    
    return SUCCESS;
}

// Measure CPU frequency
uint64_t timer_measure_cpu_frequency(void) {
    if (!timer_initialized) {
        return 0;
    }
    
    uint64_t start_tsc = timer_get_timestamp();
    uint64_t start_time = timer_get_time_ms();
    
    timer_delay_ms(100); // Wait 100ms
    
    uint64_t end_tsc = timer_get_timestamp();
    uint64_t end_time = timer_get_time_ms();
    
    uint64_t cycles = end_tsc - start_tsc;
    uint64_t time_ms = end_time - start_time;
    
    if (time_ms == 0) {
        return 0;
    }
    
    // Calculate frequency in Hz
    uint64_t frequency = (cycles * 1000) / time_ms;
    
    KINFO("Measured CPU frequency: %llu Hz (%llu MHz)", 
          frequency, frequency / 1000000);
    
    return frequency;
}

// Shutdown timer subsystem
void timer_shutdown(void) {
    if (!timer_initialized) {
        return;
    }
    
    KINFO("Shutting down timer subsystem");
    
    // Stop all active timers
    timer_t* timer = timer_subsystem.timer_list;
    while (timer) {
        timer_t* next = timer->next;
        timer_destroy(timer);
        timer = next;
    }
    
    // Mask timer IRQ
    pic_mask_irq(0);
    
    timer_initialized = false;
    
    KINFO("Timer subsystem shut down");
}