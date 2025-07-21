#ifndef TIMER_H
#define TIMER_H

#include "../core/types.h"
#include "../core/error.h"

// Timer constants
#define PIT_FREQUENCY 1193182  // PIT base frequency in Hz
#define TIMER_FREQUENCY 1000   // Target timer frequency (1000 Hz = 1ms)
#define TIMER_DIVISOR (PIT_FREQUENCY / TIMER_FREQUENCY)

// PIT I/O ports
#define PIT_CHANNEL0_DATA 0x40
#define PIT_CHANNEL1_DATA 0x41
#define PIT_CHANNEL2_DATA 0x42
#define PIT_COMMAND_PORT  0x43

// PIT command register bits
#define PIT_CHANNEL0      0x00
#define PIT_CHANNEL1      0x40
#define PIT_CHANNEL2      0x80
#define PIT_ACCESS_LATCH  0x00
#define PIT_ACCESS_LOW    0x10
#define PIT_ACCESS_HIGH   0x20
#define PIT_ACCESS_BOTH   0x30
#define PIT_MODE0         0x00  // Interrupt on terminal count
#define PIT_MODE1         0x02  // Hardware re-triggerable one-shot
#define PIT_MODE2         0x04  // Rate generator
#define PIT_MODE3         0x06  // Square wave generator
#define PIT_MODE4         0x08  // Software triggered strobe
#define PIT_MODE5         0x0A  // Hardware triggered strobe
#define PIT_BINARY        0x00
#define PIT_BCD           0x01

// Timer callback function type
typedef void (*timer_callback_t)(void* data);

// Timer structure
typedef struct timer {
    uint64_t interval;        // Timer interval in milliseconds
    uint64_t next_fire;       // Next fire time
    timer_callback_t callback; // Callback function
    void* data;               // Callback data
    bool periodic;            // Is this a periodic timer?
    bool active;              // Is timer active?
    struct timer* next;       // Next timer in list
} timer_t;

// Timer statistics
typedef struct {
    uint64_t total_ticks;     // Total timer ticks
    uint64_t missed_ticks;    // Missed timer ticks
    uint64_t timer_count;     // Number of active timers
    uint64_t callback_time;   // Total time spent in callbacks
    uint32_t frequency;       // Current timer frequency
} timer_stats_t;

// Global timer state
typedef struct {
    bool initialized;
    uint64_t system_ticks;    // System tick counter
    uint64_t system_time_ms;  // System time in milliseconds
    timer_t* timer_list;      // Active timer list
    uint32_t timer_count;     // Number of active timers
    timer_stats_t stats;      // Timer statistics
    mutex_t timer_mutex;      // Timer list mutex
} timer_subsystem_t;

// Function prototypes

// Timer subsystem initialization
error_t timer_init(void);
void timer_shutdown(void);

// PIT (Programmable Interval Timer) functions
error_t pit_init(uint32_t frequency);
void pit_set_frequency(uint32_t frequency);
uint32_t pit_get_frequency(void);

// Timer management
timer_t* timer_create(uint64_t interval_ms, timer_callback_t callback, void* data, bool periodic);
error_t timer_destroy(timer_t* timer);
error_t timer_start(timer_t* timer);
error_t timer_stop(timer_t* timer);
error_t timer_reset(timer_t* timer);

// One-shot timer functions
timer_t* timer_create_oneshot(uint64_t delay_ms, timer_callback_t callback, void* data);
error_t timer_schedule_oneshot(uint64_t delay_ms, timer_callback_t callback, void* data);

// Periodic timer functions
timer_t* timer_create_periodic(uint64_t interval_ms, timer_callback_t callback, void* data);

// Timer interrupt handler
void timer_interrupt_handler(void);

// Time functions
uint64_t timer_get_ticks(void);
uint64_t timer_get_time_ms(void);
uint64_t timer_get_time_us(void);
void timer_sleep_ms(uint64_t milliseconds);
void timer_sleep_us(uint64_t microseconds);
void timer_delay_ms(uint64_t milliseconds);
void timer_delay_us(uint64_t microseconds);

// High precision timing
uint64_t timer_get_timestamp(void);
uint64_t timer_get_uptime_ms(void);
uint64_t timer_get_uptime_seconds(void);

// Timer statistics and monitoring
error_t timer_get_stats(timer_stats_t* stats);
void timer_reset_stats(void);
uint32_t timer_get_active_count(void);

// Utility functions
const char* timer_get_info(void);
bool timer_is_initialized(void);

// Calibration functions
error_t timer_calibrate(void);
uint64_t timer_measure_cpu_frequency(void);

// Advanced timer features
error_t timer_set_priority(timer_t* timer, uint8_t priority);
error_t timer_set_cpu_affinity(timer_t* timer, uint32_t cpu_mask);

#endif // TIMER_H