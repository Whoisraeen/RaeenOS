#ifndef RAEENOS_SYSTEM_MONITOR_H
#define RAEENOS_SYSTEM_MONITOR_H

#include "types.h"

// System monitor constants
#define MAX_HISTORICAL_METRICS 1000
#define MAX_SYSTEM_ALERTS 100
#define MAX_SYSTEM_DIAGNOSTICS 50
#define MAX_ALERT_MESSAGE_LENGTH 256
#define MAX_DIAGNOSTIC_MESSAGE_LENGTH 512

// System alert types
typedef enum {
    SYSTEM_ALERT_CPU_HIGH = 0,
    SYSTEM_ALERT_CPU_TEMP = 1,
    SYSTEM_ALERT_MEMORY_HIGH = 2,
    SYSTEM_ALERT_MEMORY_PRESSURE = 3,
    SYSTEM_ALERT_DISK_HIGH = 4,
    SYSTEM_ALERT_DISK_FULL = 5,
    SYSTEM_ALERT_NETWORK_ERRORS = 6,
    SYSTEM_ALERT_NETWORK_DROPS = 7,
    SYSTEM_ALERT_PROCESS_HIGH = 8,
    SYSTEM_ALERT_SYSTEM_OVERLOAD = 9,
    SYSTEM_ALERT_SECURITY_THREAT = 10,
    SYSTEM_ALERT_HARDWARE_FAILURE = 11
} system_alert_type_t;

// System diagnostic types
typedef enum {
    SYSTEM_DIAGNOSTIC_CPU = 0,
    SYSTEM_DIAGNOSTIC_MEMORY = 1,
    SYSTEM_DIAGNOSTIC_DISK = 2,
    SYSTEM_DIAGNOSTIC_NETWORK = 3,
    SYSTEM_DIAGNOSTIC_PROCESS = 4,
    SYSTEM_DIAGNOSTIC_SECURITY = 5,
    SYSTEM_DIAGNOSTIC_HARDWARE = 6,
    SYSTEM_DIAGNOSTIC_PERFORMANCE = 7
} system_diagnostic_type_t;

// System diagnostic severity levels
typedef enum {
    SYSTEM_DIAGNOSTIC_INFO = 0,
    SYSTEM_DIAGNOSTIC_WARNING = 1,
    SYSTEM_DIAGNOSTIC_ERROR = 2,
    SYSTEM_DIAGNOSTIC_CRITICAL = 3
} system_diagnostic_severity_t;

// CPU metrics structure
typedef struct {
    float usage_percent;
    float temperature;
    uint32_t frequency;
    float load_average_1min;
    float load_average_5min;
    float load_average_15min;
    int core_count;
    uint64_t l1_cache_size;
    uint64_t l2_cache_size;
    uint64_t l3_cache_size;
} cpu_metrics_t;

// Memory metrics structure
typedef struct {
    uint64_t total_memory;
    uint64_t used_memory;
    uint64_t free_memory;
    float usage_percent;
    uint64_t swap_total;
    uint64_t swap_used;
    uint64_t swap_free;
    float memory_pressure;
} memory_metrics_t;

// Disk metrics structure
typedef struct {
    uint64_t total_space;
    uint64_t used_space;
    uint64_t free_space;
    float usage_percent;
    uint64_t read_bytes;
    uint64_t write_bytes;
    uint64_t read_operations;
    uint64_t write_operations;
    float average_latency;
} disk_metrics_t;

// Network metrics structure
typedef struct {
    uint64_t bytes_received;
    uint64_t bytes_sent;
    uint64_t packets_received;
    uint64_t packets_sent;
    uint64_t errors_received;
    uint64_t errors_sent;
    uint64_t drops_received;
    uint64_t drops_sent;
    float bandwidth_in;
    float bandwidth_out;
} network_metrics_t;

// Process metrics structure
typedef struct {
    int total_processes;
    int running_processes;
    int sleeping_processes;
    int stopped_processes;
    int zombie_processes;
    int total_threads;
    uint64_t context_switches;
    float process_creation_rate;
} process_metrics_t;

// System metrics structure
typedef struct {
    uint64_t timestamp;
    cpu_metrics_t cpu;
    memory_metrics_t memory;
    disk_metrics_t disk;
    network_metrics_t network;
    process_metrics_t process;
} system_metrics_t;

// System alert structure
typedef struct {
    system_alert_type_t type;
    char message[MAX_ALERT_MESSAGE_LENGTH];
    float value;
    uint64_t timestamp;
    bool acknowledged;
} system_alert_t;

// System diagnostic structure
typedef struct {
    system_diagnostic_type_t type;
    char message[MAX_DIAGNOSTIC_MESSAGE_LENGTH];
    system_diagnostic_severity_t severity;
    uint64_t timestamp;
} system_diagnostic_t;

// System monitor functions
error_t system_monitor_init(void);
void system_monitor_shutdown(void);
error_t system_monitor_collect_metrics(void);
error_t system_monitor_get_metrics(system_metrics_t* metrics);
error_t system_monitor_get_historical_metrics(system_metrics_t* metrics, int* count);

// CPU monitoring functions
error_t system_monitor_cpu_init(void);
void system_monitor_cpu_shutdown(void);
error_t system_monitor_collect_cpu_metrics(cpu_metrics_t* metrics);

// Memory monitoring functions
error_t system_monitor_memory_init(void);
void system_monitor_memory_shutdown(void);
error_t system_monitor_collect_memory_metrics(memory_metrics_t* metrics);

// Disk monitoring functions
error_t system_monitor_disk_init(void);
void system_monitor_disk_shutdown(void);
error_t system_monitor_collect_disk_metrics(disk_metrics_t* metrics);

// Network monitoring functions
error_t system_monitor_network_init(void);
void system_monitor_network_shutdown(void);
error_t system_monitor_collect_network_metrics(network_metrics_t* metrics);

// Process monitoring functions
error_t system_monitor_process_init(void);
void system_monitor_process_shutdown(void);
error_t system_monitor_collect_process_metrics(process_metrics_t* metrics);

// Alert management functions
void system_monitor_check_alerts(void);
error_t system_monitor_add_alert(system_alert_type_t type, const char* message, float value);
error_t system_monitor_get_alerts(system_alert_t* alert_list, int* count);
error_t system_monitor_acknowledge_alert(int alert_index);
void system_monitor_clear_alerts(void);

// Diagnostic functions
error_t system_monitor_run_diagnostics(void);
error_t system_monitor_get_diagnostics(system_diagnostic_t* diagnostic_list, int* count);
error_t system_monitor_add_diagnostic(system_diagnostic_type_t type, const char* message, system_diagnostic_severity_t severity);

// Monitoring thread functions
error_t system_monitor_start_monitoring(void);
void system_monitor_stop_monitoring(void);

// Utility functions
uint64_t system_monitor_get_timestamp(void);
bool system_monitor_is_initialized(void);

// Metric collection functions
float system_monitor_get_cpu_usage(void);
float system_monitor_get_cpu_temperature(void);
uint32_t system_monitor_get_cpu_frequency(void);
float system_monitor_get_load_average(int minutes);
int system_monitor_get_cpu_core_count(void);
uint64_t system_monitor_get_cpu_cache_size(int level);

uint64_t system_monitor_get_total_memory(void);
uint64_t system_monitor_get_used_memory(void);
uint64_t system_monitor_get_free_memory(void);
uint64_t system_monitor_get_swap_total(void);
uint64_t system_monitor_get_swap_used(void);
uint64_t system_monitor_get_swap_free(void);
float system_monitor_get_memory_pressure(void);

uint64_t system_monitor_get_disk_total_space(const char* path);
uint64_t system_monitor_get_disk_used_space(const char* path);
uint64_t system_monitor_get_disk_free_space(const char* path);
uint64_t system_monitor_get_disk_read_bytes(void);
uint64_t system_monitor_get_disk_write_bytes(void);
uint64_t system_monitor_get_disk_read_ops(void);
uint64_t system_monitor_get_disk_write_ops(void);
float system_monitor_get_disk_latency(void);

uint64_t system_monitor_get_network_bytes_received(void);
uint64_t system_monitor_get_network_bytes_sent(void);
uint64_t system_monitor_get_network_packets_received(void);
uint64_t system_monitor_get_network_packets_sent(void);
uint64_t system_monitor_get_network_errors_received(void);
uint64_t system_monitor_get_network_errors_sent(void);
uint64_t system_monitor_get_network_drops_received(void);
uint64_t system_monitor_get_network_drops_sent(void);
float system_monitor_get_network_bandwidth_in(void);
float system_monitor_get_network_bandwidth_out(void);

int system_monitor_get_process_count(void);
int system_monitor_get_running_process_count(void);
int system_monitor_get_sleeping_process_count(void);
int system_monitor_get_stopped_process_count(void);
int system_monitor_get_zombie_process_count(void);
int system_monitor_get_thread_count(void);
uint64_t system_monitor_get_context_switches(void);
float system_monitor_get_process_creation_rate(void);

// Diagnostic helper functions
void system_monitor_diagnose_cpu(void);
void system_monitor_diagnose_memory(void);
void system_monitor_diagnose_disk(void);
void system_monitor_diagnose_network(void);
void system_monitor_diagnose_processes(void);

// CPU performance monitoring functions
void system_monitor_cpu_perf_init(void);
void system_monitor_cpu_perf_shutdown(void);

// System health status
typedef enum {
    SYSTEM_HEALTH_EXCELLENT = 0,
    SYSTEM_HEALTH_GOOD = 1,
    SYSTEM_HEALTH_FAIR = 2,
    SYSTEM_HEALTH_POOR = 3,
    SYSTEM_HEALTH_CRITICAL = 4
} system_health_status_t;

// System performance score
typedef struct {
    float overall_score;
    float cpu_score;
    float memory_score;
    float disk_score;
    float network_score;
    system_health_status_t health_status;
} system_performance_score_t;

// Performance scoring functions
error_t system_monitor_calculate_performance_score(system_performance_score_t* score);
system_health_status_t system_monitor_get_system_health(void);

// Real-time monitoring
typedef struct {
    bool enabled;
    uint32_t collection_interval_ms;
    void (*metrics_callback)(const system_metrics_t* metrics);
    void (*alert_callback)(const system_alert_t* alert);
} real_time_monitoring_config_t;

error_t system_monitor_start_real_time_monitoring(const real_time_monitoring_config_t* config);
void system_monitor_stop_real_time_monitoring(void);

// Performance optimization recommendations
typedef struct {
    char recommendation[256];
    system_diagnostic_severity_t priority;
    float expected_improvement;
} performance_recommendation_t;

error_t system_monitor_get_performance_recommendations(performance_recommendation_t* recommendations, int* count);

#endif // RAEENOS_SYSTEM_MONITOR_H 