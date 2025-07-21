#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memory.h>

// Power Management System - Advanced power management for RaeenOS
// Provides battery optimization, energy saving, and power monitoring

// Power states
typedef enum {
    POWER_STATE_ACTIVE = 0,      // Fully active
    POWER_STATE_IDLE,            // Idle state
    POWER_STATE_STANDBY,         // Standby/sleep
    POWER_STATE_HIBERNATE,       // Hibernation
    POWER_STATE_SHUTDOWN,        // Shutdown
    POWER_STATE_HYBRID_SLEEP,    // Hybrid sleep
    POWER_STATE_FAST_STARTUP,    // Fast startup
} power_state_t;

// Power sources
typedef enum {
    POWER_SOURCE_UNKNOWN = 0,    // Unknown power source
    POWER_SOURCE_AC,             // AC power
    POWER_SOURCE_BATTERY,        // Battery power
    POWER_SOURCE_UPS,            // UPS power
    POWER_SOURCE_SOLAR,          // Solar power
    POWER_SOURCE_WIRELESS,       // Wireless charging
} power_source_t;

// Battery types
typedef enum {
    BATTERY_TYPE_UNKNOWN = 0,    // Unknown battery type
    BATTERY_TYPE_LITHIUM_ION,    // Lithium-ion
    BATTERY_TYPE_LITHIUM_POLYMER, // Lithium-polymer
    BATTERY_TYPE_NICKEL_METAL_HYDRIDE, // NiMH
    BATTERY_TYPE_NICKEL_CADMIUM, // NiCd
    BATTERY_TYPE_LEAD_ACID,      // Lead-acid
    BATTERY_TYPE_ALKALINE,       // Alkaline
} battery_type_t;

// Battery health
typedef enum {
    BATTERY_HEALTH_UNKNOWN = 0,  // Unknown health
    BATTERY_HEALTH_EXCELLENT,    // Excellent (>90%)
    BATTERY_HEALTH_GOOD,         // Good (70-90%)
    BATTERY_HEALTH_FAIR,         // Fair (50-70%)
    BATTERY_HEALTH_POOR,         // Poor (30-50%)
    BATTERY_HEALTH_CRITICAL,     // Critical (<30%)
    BATTERY_HEALTH_DEAD,         // Dead
} battery_health_t;

// Power plans
typedef enum {
    POWER_PLAN_BALANCED = 0,     // Balanced power plan
    POWER_PLAN_POWER_SAVER,      // Power saver plan
    POWER_PLAN_HIGH_PERFORMANCE, // High performance plan
    POWER_PLAN_ULTIMATE_PERFORMANCE, // Ultimate performance
    POWER_PLAN_GAMING,           // Gaming optimized
    POWER_PLAN_CREATIVE,         // Creative work optimized
    POWER_PLAN_QUIET,            // Quiet operation
    POWER_PLAN_CUSTOM,           // Custom power plan
} power_plan_t;

// CPU power states
typedef enum {
    CPU_POWER_STATE_C0 = 0,      // Active state
    CPU_POWER_STATE_C1,          // Halt state
    CPU_POWER_STATE_C2,          // Stop clock state
    CPU_POWER_STATE_C3,          // Sleep state
    CPU_POWER_STATE_C4,          // Deep sleep state
    CPU_POWER_STATE_C5,          // Deeper sleep state
    CPU_POWER_STATE_C6,          // Deepest sleep state
    CPU_POWER_STATE_C7,          // Deepest sleep state (SMT)
    CPU_POWER_STATE_C8,          // Deepest sleep state (package)
    CPU_POWER_STATE_C9,          // Deepest sleep state (package + SMT)
    CPU_POWER_STATE_C10,         // Deepest sleep state (package + SMT + cache)
} cpu_power_state_t;

// GPU power states
typedef enum {
    GPU_POWER_STATE_P0 = 0,      // Maximum performance
    GPU_POWER_STATE_P1,          // High performance
    GPU_POWER_STATE_P2,          // Medium performance
    GPU_POWER_STATE_P3,          // Low performance
    GPU_POWER_STATE_P4,          // Minimum performance
    GPU_POWER_STATE_P5,          // Idle
    GPU_POWER_STATE_P6,          // Sleep
    GPU_POWER_STATE_P7,          // Deep sleep
    GPU_POWER_STATE_P8,          // Deepest sleep
} gpu_power_state_t;

// Battery information
typedef struct {
    uint32_t id;
    char name[64];               // Battery name
    battery_type_t type;         // Battery type
    battery_health_t health;     // Battery health
    uint32_t design_capacity;    // Design capacity in mWh
    uint32_t full_charge_capacity; // Full charge capacity in mWh
    uint32_t current_capacity;   // Current capacity in mWh
    uint32_t voltage;            // Current voltage in mV
    uint32_t current;            // Current draw in mA
    uint32_t temperature;        // Temperature in 0.1°C
    uint32_t cycle_count;        // Charge cycle count
    uint32_t charge_rate;        // Charge rate in mW
    bool is_charging;            // Is currently charging
    bool is_present;             // Is battery present
    bool is_critical;            // Is battery critical
    uint64_t last_update_time;   // Last update timestamp
} battery_info_t;

// Power source information
typedef struct {
    power_source_t type;         // Power source type
    bool is_connected;           // Is power source connected
    uint32_t voltage;            // Input voltage in mV
    uint32_t current;            // Input current in mA
    uint32_t power;              // Input power in mW
    bool is_charging;            // Is charging battery
    uint32_t efficiency;         // Power efficiency percentage
    char name[64];               // Power source name
} power_source_info_t;

// CPU power information
typedef struct {
    uint32_t core_count;         // Number of CPU cores
    uint32_t active_cores;       // Number of active cores
    cpu_power_state_t* core_states; // Power state of each core
    uint32_t frequency;          // Current frequency in MHz
    uint32_t max_frequency;      // Maximum frequency in MHz
    uint32_t min_frequency;      // Minimum frequency in MHz
    uint32_t power_consumption;  // Power consumption in mW
    uint32_t temperature;        // CPU temperature in 0.1°C
    float utilization;           // CPU utilization percentage
    bool turbo_enabled;          // Is turbo boost enabled
    bool power_limit_enabled;    // Is power limiting enabled
    uint32_t power_limit;        // Power limit in mW
} cpu_power_info_t;

// GPU power information
typedef struct {
    uint32_t id;
    char name[64];               // GPU name
    gpu_power_state_t state;     // Current power state
    uint32_t frequency;          // Current frequency in MHz
    uint32_t max_frequency;      // Maximum frequency in MHz
    uint32_t memory_frequency;   // Memory frequency in MHz
    uint32_t power_consumption;  // Power consumption in mW
    uint32_t temperature;        // GPU temperature in 0.1°C
    float utilization;           // GPU utilization percentage
    uint32_t memory_usage;       // Memory usage in MB
    bool power_limit_enabled;    // Is power limiting enabled
    uint32_t power_limit;        // Power limit in mW
    bool dynamic_boost_enabled;  // Is dynamic boost enabled
} gpu_power_info_t;

// System power information
typedef struct {
    power_state_t state;         // Current power state
    power_source_t source;       // Current power source
    uint32_t total_power;        // Total system power in mW
    uint32_t cpu_power;          // CPU power consumption in mW
    uint32_t gpu_power;          // GPU power consumption in mW
    uint32_t memory_power;       // Memory power consumption in mW
    uint32_t storage_power;      // Storage power consumption in mW
    uint32_t network_power;      // Network power consumption in mW
    uint32_t display_power;      // Display power consumption in mW
    uint32_t fan_power;          // Fan power consumption in mW
    uint32_t other_power;        // Other components power in mW
    uint32_t temperature;        // System temperature in 0.1°C
    uint64_t uptime;             // System uptime in seconds
    uint64_t last_update_time;   // Last update timestamp
} system_power_info_t;

// Power plan configuration
typedef struct {
    power_plan_t plan;           // Power plan type
    char name[64];               // Plan name
    char description[256];       // Plan description
    uint32_t cpu_min_percent;    // Minimum CPU percentage
    uint32_t cpu_max_percent;    // Maximum CPU percentage
    uint32_t cpu_boost_enabled;  // CPU boost enabled
    uint32_t gpu_power_limit;    // GPU power limit in mW
    uint32_t display_brightness; // Display brightness percentage
    uint32_t sleep_timeout;      // Sleep timeout in minutes
    uint32_t hibernate_timeout;  // Hibernate timeout in minutes
    bool fast_startup_enabled;   // Fast startup enabled
    bool hybrid_sleep_enabled;   // Hybrid sleep enabled
    uint32_t fan_curve[10];      // Fan curve settings
    bool is_active;              // Is this plan active
} power_plan_config_t;

// Power management configuration
typedef struct {
    bool enable_power_management;
    bool enable_battery_optimization;
    bool enable_cpu_power_management;
    bool enable_gpu_power_management;
    bool enable_display_power_management;
    bool enable_storage_power_management;
    bool enable_network_power_management;
    bool enable_fan_control;
    bool enable_thermal_management;
    bool enable_power_monitoring;
    uint32_t update_interval_ms;
    uint32_t max_batteries;
    uint32_t max_gpus;
    uint32_t max_power_plans;
    float critical_battery_level;
    float low_battery_level;
    uint32_t battery_warning_interval;
} power_management_config_t;

// Power management context
typedef struct {
    power_management_config_t config;
    battery_info_t* batteries;
    uint32_t battery_count;
    uint32_t max_batteries;
    power_source_info_t power_source;
    cpu_power_info_t cpu_power;
    gpu_power_info_t* gpu_power;
    uint32_t gpu_count;
    uint32_t max_gpus;
    system_power_info_t system_power;
    power_plan_config_t* power_plans;
    uint32_t power_plan_count;
    uint32_t max_power_plans;
    power_plan_t active_plan;
    bool initialized;
    bool power_management_enabled;
    bool battery_optimization_enabled;
    uint64_t last_update_time;
    uint32_t next_battery_id;
    uint32_t next_gpu_id;
    uint32_t next_plan_id;
} power_management_t;

// Function prototypes

// Initialization and shutdown
power_management_t* power_management_init(power_management_config_t* config);
void power_management_shutdown(power_management_t* pm);
bool power_management_is_initialized(power_management_t* pm);

// Power state management
void power_management_set_power_state(power_management_t* pm, power_state_t state);
power_state_t power_management_get_power_state(power_management_t* pm);
bool power_management_sleep(power_management_t* pm);
bool power_management_hibernate(power_management_t* pm);
bool power_management_shutdown_system(power_management_t* pm);
bool power_management_restart_system(power_management_t* pm);
bool power_management_hybrid_sleep(power_management_t* pm);

// Battery management
uint32_t power_management_add_battery(power_management_t* pm, battery_info_t* battery);
void power_management_remove_battery(power_management_t* pm, uint32_t battery_id);
battery_info_t* power_management_get_battery(power_management_t* pm, uint32_t battery_id);
battery_info_t* power_management_get_batteries(power_management_t* pm, uint32_t* count);
uint32_t power_management_get_battery_count(power_management_t* pm);
float power_management_get_battery_percentage(power_management_t* pm, uint32_t battery_id);
uint32_t power_management_get_battery_time_remaining(power_management_t* pm, uint32_t battery_id);
bool power_management_is_battery_charging(power_management_t* pm, uint32_t battery_id);
battery_health_t power_management_get_battery_health(power_management_t* pm, uint32_t battery_id);
uint32_t power_management_get_battery_cycle_count(power_management_t* pm, uint32_t battery_id);

// Power source management
void power_management_update_power_source(power_management_t* pm, power_source_info_t* source);
power_source_info_t* power_management_get_power_source(power_management_t* pm);
bool power_management_is_ac_connected(power_management_t* pm);
bool power_management_is_battery_powered(power_management_t* pm);
uint32_t power_management_get_input_power(power_management_t* pm);

// CPU power management
void power_management_update_cpu_power(power_management_t* pm, cpu_power_info_t* cpu_info);
cpu_power_info_t* power_management_get_cpu_power(power_management_t* pm);
void power_management_set_cpu_frequency(power_management_t* pm, uint32_t frequency);
uint32_t power_management_get_cpu_frequency(power_management_t* pm);
void power_management_set_cpu_power_limit(power_management_t* pm, uint32_t power_limit);
uint32_t power_management_get_cpu_power_limit(power_management_t* pm);
void power_management_enable_cpu_turbo(power_management_t* pm, bool enable);
bool power_management_is_cpu_turbo_enabled(power_management_t* pm);
void power_management_set_cpu_cores(power_management_t* pm, uint32_t active_cores);
uint32_t power_management_get_active_cpu_cores(power_management_t* pm);

// GPU power management
uint32_t power_management_add_gpu(power_management_t* pm, gpu_power_info_t* gpu_info);
void power_management_remove_gpu(power_management_t* pm, uint32_t gpu_id);
gpu_power_info_t* power_management_get_gpu_power(power_management_t* pm, uint32_t gpu_id);
gpu_power_info_t* power_management_get_gpus(power_management_t* pm, uint32_t* count);
uint32_t power_management_get_gpu_count(power_management_t* pm);
void power_management_set_gpu_power_state(power_management_t* pm, uint32_t gpu_id, gpu_power_state_t state);
gpu_power_state_t power_management_get_gpu_power_state(power_management_t* pm, uint32_t gpu_id);
void power_management_set_gpu_power_limit(power_management_t* pm, uint32_t gpu_id, uint32_t power_limit);
uint32_t power_management_get_gpu_power_limit(power_management_t* pm, uint32_t gpu_id);
void power_management_enable_gpu_dynamic_boost(power_management_t* pm, uint32_t gpu_id, bool enable);
bool power_management_is_gpu_dynamic_boost_enabled(power_management_t* pm, uint32_t gpu_id);

// System power monitoring
void power_management_update_system_power(power_management_t* pm, system_power_info_t* system_info);
system_power_info_t* power_management_get_system_power(power_management_t* pm);
uint32_t power_management_get_total_power_consumption(power_management_t* pm);
uint32_t power_management_get_cpu_power_consumption(power_management_t* pm);
uint32_t power_management_get_gpu_power_consumption(power_management_t* pm);
uint32_t power_management_get_memory_power_consumption(power_management_t* pm);
uint32_t power_management_get_storage_power_consumption(power_management_t* pm);
uint32_t power_management_get_display_power_consumption(power_management_t* pm);
uint32_t power_management_get_system_temperature(power_management_t* pm);

// Power plan management
uint32_t power_management_create_power_plan(power_management_t* pm, power_plan_config_t* plan);
void power_management_destroy_power_plan(power_management_t* pm, uint32_t plan_id);
void power_management_set_active_power_plan(power_management_t* pm, power_plan_t plan);
power_plan_t power_management_get_active_power_plan(power_management_t* pm);
power_plan_config_t* power_management_get_power_plan(power_management_t* pm, power_plan_t plan);
power_plan_config_t* power_management_get_power_plans(power_management_t* pm, uint32_t* count);
uint32_t power_management_get_power_plan_count(power_management_t* pm);
void power_management_edit_power_plan(power_management_t* pm, power_plan_t plan, power_plan_config_t* config);

// Power optimization
void power_management_enable_power_management(power_management_t* pm, bool enable);
bool power_management_is_power_management_enabled(power_management_t* pm);
void power_management_enable_battery_optimization(power_management_t* pm, bool enable);
bool power_management_is_battery_optimization_enabled(power_management_t* pm);
void power_management_optimize_for_battery(power_management_t* pm);
void power_management_optimize_for_performance(power_management_t* pm);
void power_management_optimize_for_quiet(power_management_t* pm);
void power_management_optimize_for_gaming(power_management_t* pm);
void power_management_optimize_for_creative(power_management_t* pm);

// Thermal management
void power_management_set_fan_curve(power_management_t* pm, uint32_t* fan_curve);
uint32_t* power_management_get_fan_curve(power_management_t* pm);
void power_management_set_fan_speed(power_management_t* pm, uint32_t fan_speed);
uint32_t power_management_get_fan_speed(power_management_t* pm);
void power_management_enable_thermal_management(power_management_t* pm, bool enable);
bool power_management_is_thermal_management_enabled(power_management_t* pm);
void power_management_set_thermal_threshold(power_management_t* pm, uint32_t threshold);
uint32_t power_management_get_thermal_threshold(power_management_t* pm);

// Display power management
void power_management_set_display_brightness(power_management_t* pm, uint32_t brightness);
uint32_t power_management_get_display_brightness(power_management_t* pm);
void power_management_set_display_timeout(power_management_t* pm, uint32_t timeout_seconds);
uint32_t power_management_get_display_timeout(power_management_t* pm);
void power_management_enable_display_power_management(power_management_t* pm, bool enable);
bool power_management_is_display_power_management_enabled(power_management_t* pm);
void power_management_display_off(power_management_t* pm);
void power_management_display_on(power_management_t* pm);

// Storage power management
void power_management_enable_storage_power_management(power_management_t* pm, bool enable);
bool power_management_is_storage_power_management_enabled(power_management_t* pm);
void power_management_set_storage_power_state(power_management_t* pm, uint32_t power_state);
uint32_t power_management_get_storage_power_state(power_management_t* pm);

// Network power management
void power_management_enable_network_power_management(power_management_t* pm, bool enable);
bool power_management_is_network_power_management_enabled(power_management_t* pm);
void power_management_set_network_power_state(power_management_t* pm, uint32_t power_state);
uint32_t power_management_get_network_power_state(power_management_t* pm);

// Power monitoring
void power_management_start_power_monitoring(power_management_t* pm);
void power_management_stop_power_monitoring(power_management_t* pm);
bool power_management_is_power_monitoring_enabled(power_management_t* pm);
void power_management_set_monitoring_interval(power_management_t* pm, uint32_t interval_ms);
uint32_t power_management_get_monitoring_interval(power_management_t* pm);

// Power events
void power_management_handle_power_event(power_management_t* pm, uint32_t event_type);
void power_management_handle_battery_event(power_management_t* pm, uint32_t battery_id, uint32_t event_type);
void power_management_handle_thermal_event(power_management_t* pm, uint32_t event_type);
void power_management_handle_ac_event(power_management_t* pm, bool connected);

// Power statistics
uint64_t power_management_get_total_uptime(power_management_t* pm);
uint64_t power_management_get_battery_uptime(power_management_t* pm);
uint64_t power_management_get_ac_uptime(power_management_t* pm);
float power_management_get_average_power_consumption(power_management_t* pm);
float power_management_get_peak_power_consumption(power_management_t* pm);
float power_management_get_average_temperature(power_management_t* pm);
float power_management_get_peak_temperature(power_management_t* pm);

// Power alerts
void power_management_set_low_battery_alert(power_management_t* pm, float percentage);
float power_management_get_low_battery_alert(power_management_t* pm);
void power_management_set_critical_battery_alert(power_management_t* pm, float percentage);
float power_management_get_critical_battery_alert(power_management_t* pm);
void power_management_set_high_temperature_alert(power_management_t* pm, uint32_t temperature);
uint32_t power_management_get_high_temperature_alert(power_management_t* pm);
void power_management_set_power_consumption_alert(power_management_t* pm, uint32_t power_limit);
uint32_t power_management_get_power_consumption_alert(power_management_t* pm);

// Utility functions
bool power_management_is_battery_low(power_management_t* pm);
bool power_management_is_battery_critical(power_management_t* pm);
bool power_management_is_temperature_high(power_management_t* pm);
bool power_management_is_power_consumption_high(power_management_t* pm);
uint32_t power_management_estimate_battery_life(power_management_t* pm);
uint32_t power_management_estimate_charge_time(power_management_t* pm);
float power_management_calculate_power_efficiency(power_management_t* pm);

// Callbacks
typedef void (*power_state_change_callback_t)(power_management_t* pm, power_state_t old_state, power_state_t new_state, void* user_data);
typedef void (*battery_status_change_callback_t)(power_management_t* pm, uint32_t battery_id, float old_percentage, float new_percentage, void* user_data);
typedef void (*power_source_change_callback_t)(power_management_t* pm, power_source_t old_source, power_source_t new_source, void* user_data);
typedef void (*thermal_event_callback_t)(power_management_t* pm, uint32_t temperature, uint32_t threshold, void* user_data);

void power_management_set_power_state_change_callback(power_management_t* pm, power_state_change_callback_t callback, void* user_data);
void power_management_set_battery_status_change_callback(power_management_t* pm, battery_status_change_callback_t callback, void* user_data);
void power_management_set_power_source_change_callback(power_management_t* pm, power_source_change_callback_t callback, void* user_data);
void power_management_set_thermal_event_callback(power_management_t* pm, thermal_event_callback_t callback, void* user_data);

// Preset configurations
power_management_config_t power_management_preset_balanced_style(void);
power_management_config_t power_management_preset_power_saver_style(void);
power_management_config_t power_management_preset_high_performance_style(void);
power_management_config_t power_management_preset_gaming_style(void);
power_management_config_t power_management_preset_quiet_style(void);

// Error handling
typedef enum {
    POWER_MANAGEMENT_SUCCESS = 0,
    POWER_MANAGEMENT_ERROR_INVALID_CONTEXT,
    POWER_MANAGEMENT_ERROR_INVALID_BATTERY,
    POWER_MANAGEMENT_ERROR_INVALID_GPU,
    POWER_MANAGEMENT_ERROR_INVALID_POWER_PLAN,
    POWER_MANAGEMENT_ERROR_OUT_OF_MEMORY,
    POWER_MANAGEMENT_ERROR_HARDWARE_NOT_SUPPORTED,
    POWER_MANAGEMENT_ERROR_OPERATION_FAILED,
    POWER_MANAGEMENT_ERROR_INVALID_PARAMETER,
    POWER_MANAGEMENT_ERROR_TIMEOUT,
} power_management_error_t;

power_management_error_t power_management_get_last_error(void);
const char* power_management_get_error_string(power_management_error_t error);

#endif // POWER_MANAGEMENT_H 