#include "performance/system_monitor.h"
#include "kernel.h"
#include "memory/memory.h"
#include "process/process.h"
#include <string.h>

// System monitor constants
#define MAX_METRICS 64
#define MAX_ALERTS 32
#define METRIC_HISTORY_SIZE 100
#define ALERT_CHECK_INTERVAL 1000 // milliseconds

// Metric types
#define METRIC_TYPE_CPU 1
#define METRIC_TYPE_MEMORY 2
#define METRIC_TYPE_DISK 3
#define METRIC_TYPE_NETWORK 4
#define METRIC_TYPE_PROCESS 5
#define METRIC_TYPE_SYSTEM 6

// Alert severity levels
#define ALERT_SEVERITY_LOW 1
#define ALERT_SEVERITY_MEDIUM 2
#define ALERT_SEVERITY_HIGH 3
#define ALERT_SEVERITY_CRITICAL 4

// Global system monitor state
static system_monitor_state_t monitor_state = {0};
static bool monitor_initialized = false;
static system_metric_t metrics[MAX_METRICS];
static int metric_count = 0;
static system_alert_t alerts[MAX_ALERTS];
static int alert_count = 0;
static u64 last_update_time = 0;

// Initialize system monitor
error_t system_monitor_init(void) {
    if (monitor_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing system monitor...");
    
    // Initialize monitor state
    memset(&monitor_state, 0, sizeof(monitor_state));
    monitor_state.initialized = false;
    monitor_state.enabled = true;
    
    // Initialize metrics array
    memset(metrics, 0, sizeof(metrics));
    metric_count = 0;
    
    // Initialize alerts array
    memset(alerts, 0, sizeof(alerts));
    alert_count = 0;
    
    // Initialize default metrics
    system_monitor_init_default_metrics();
    
    // Initialize alert system
    system_monitor_init_alerts();
    
    monitor_initialized = true;
    monitor_state.initialized = true;
    
    KINFO("System monitor initialized with %d metrics", metric_count);
    
    return SUCCESS;
}

// Initialize default metrics
void system_monitor_init_default_metrics(void) {
    // CPU metrics
    system_monitor_add_metric("cpu_usage", "CPU Usage", METRIC_TYPE_CPU, "%");
    system_monitor_add_metric("cpu_load_1m", "CPU Load (1m)", METRIC_TYPE_CPU, "");
    system_monitor_add_metric("cpu_load_5m", "CPU Load (5m)", METRIC_TYPE_CPU, "");
    system_monitor_add_metric("cpu_load_15m", "CPU Load (15m)", METRIC_TYPE_CPU, "");
    
    // Memory metrics
    system_monitor_add_metric("memory_usage", "Memory Usage", METRIC_TYPE_MEMORY, "%");
    system_monitor_add_metric("memory_used", "Memory Used", METRIC_TYPE_MEMORY, "MB");
    system_monitor_add_metric("memory_free", "Memory Free", METRIC_TYPE_MEMORY, "MB");
    system_monitor_add_metric("memory_total", "Memory Total", METRIC_TYPE_MEMORY, "MB");
    
    // Process metrics
    system_monitor_add_metric("process_count", "Process Count", METRIC_TYPE_PROCESS, "");
    system_monitor_add_metric("thread_count", "Thread Count", METRIC_TYPE_PROCESS, "");
    system_monitor_add_metric("process_cpu_usage", "Process CPU Usage", METRIC_TYPE_PROCESS, "%");
    
    // System metrics
    system_monitor_add_metric("uptime", "System Uptime", METRIC_TYPE_SYSTEM, "seconds");
    system_monitor_add_metric("boot_time", "Boot Time", METRIC_TYPE_SYSTEM, "timestamp");
    system_monitor_add_metric("system_load", "System Load", METRIC_TYPE_SYSTEM, "");
}

// Initialize alert system
void system_monitor_init_alerts(void) {
    // CPU alerts
    system_monitor_add_alert("high_cpu_usage", "High CPU Usage", 
                            "cpu_usage", 80.0, ALERT_SEVERITY_MEDIUM);
    system_monitor_add_alert("critical_cpu_usage", "Critical CPU Usage", 
                            "cpu_usage", 95.0, ALERT_SEVERITY_CRITICAL);
    
    // Memory alerts
    system_monitor_add_alert("high_memory_usage", "High Memory Usage", 
                            "memory_usage", 85.0, ALERT_SEVERITY_MEDIUM);
    system_monitor_add_alert("critical_memory_usage", "Critical Memory Usage", 
                            "memory_usage", 95.0, ALERT_SEVERITY_CRITICAL);
    
    // Process alerts
    system_monitor_add_alert("too_many_processes", "Too Many Processes", 
                            "process_count", 1000.0, ALERT_SEVERITY_LOW);
}

// Add a metric
int system_monitor_add_metric(const char* name, const char* description, 
                             int type, const char* unit) {
    if (!name || !description || metric_count >= MAX_METRICS) {
        return -1;
    }
    
    system_metric_t* metric = &metrics[metric_count];
    strncpy(metric->name, name, sizeof(metric->name) - 1);
    strncpy(metric->description, description, sizeof(metric->description) - 1);
    strncpy(metric->unit, unit, sizeof(metric->unit) - 1);
    metric->type = type;
    metric->current_value = 0.0;
    metric->min_value = 0.0;
    metric->max_value = 0.0;
    metric->average_value = 0.0;
    metric->update_count = 0;
    metric->last_update = 0;
    
    metric_count++;
    
    KDEBUG("Added metric: %s (%s)", name, description);
    
    return metric_count - 1;
}

// Add an alert
int system_monitor_add_alert(const char* name, const char* description, 
                            const char* metric_name, double threshold, int severity) {
    if (!name || !description || !metric_name || alert_count >= MAX_ALERTS) {
        return -1;
    }
    
    system_alert_t* alert = &alerts[alert_count];
    strncpy(alert->name, name, sizeof(alert->name) - 1);
    strncpy(alert->description, description, sizeof(alert->description) - 1);
    strncpy(alert->metric_name, metric_name, sizeof(alert->metric_name) - 1);
    alert->threshold = threshold;
    alert->severity = severity;
    alert->triggered = false;
    alert->last_triggered = 0;
    alert->trigger_count = 0;
    
    alert_count++;
    
    KDEBUG("Added alert: %s (threshold: %.2f)", name, threshold);
    
    return alert_count - 1;
}

// Update a metric
void system_monitor_update_metric(const char* name, double value) {
    if (!name) {
        return;
    }
    
    for (int i = 0; i < metric_count; i++) {
        system_metric_t* metric = &metrics[i];
        if (strcmp(metric->name, name) == 0) {
            // Update metric values
            metric->current_value = value;
            
            if (metric->update_count == 0) {
                metric->min_value = value;
                metric->max_value = value;
            } else {
                if (value < metric->min_value) metric->min_value = value;
                if (value > metric->max_value) metric->max_value = value;
            }
            
            // Update average
            metric->average_value = (metric->average_value * metric->update_count + value) / 
                                   (metric->update_count + 1);
            
            metric->update_count++;
            metric->last_update = system_monitor_get_time();
            
            break;
        }
    }
}

// Get metric by name
system_metric_t* system_monitor_get_metric(const char* name) {
    if (!name) {
        return NULL;
    }
    
    for (int i = 0; i < metric_count; i++) {
        if (strcmp(metrics[i].name, name) == 0) {
            return &metrics[i];
        }
    }
    
    return NULL;
}

// Get all metrics
system_metric_t* system_monitor_get_metrics(int* count) {
    if (count) {
        *count = metric_count;
    }
    return metrics;
}

// Collect system metrics
void system_monitor_collect_metrics(void) {
    if (!monitor_initialized) {
        return;
    }
    
    // Collect CPU metrics
    system_monitor_collect_cpu_metrics();
    
    // Collect memory metrics
    system_monitor_collect_memory_metrics();
    
    // Collect process metrics
    system_monitor_collect_process_metrics();
    
    // Collect system metrics
    system_monitor_collect_system_metrics();
    
    last_update_time = system_monitor_get_time();
}

// Collect CPU metrics
void system_monitor_collect_cpu_metrics(void) {
    // Calculate CPU usage (simplified)
    static u64 last_cpu_time = 0;
    static u64 last_idle_time = 0;
    
    u64 current_time = system_monitor_get_time();
    u64 cpu_time = current_time; // Simplified
    u64 idle_time = current_time / 2; // Simplified
    
    if (last_cpu_time > 0) {
        u64 total_time = cpu_time - last_cpu_time;
        u64 idle_delta = idle_time - last_idle_time;
        double cpu_usage = 100.0 * (1.0 - (double)idle_delta / total_time);
        
        system_monitor_update_metric("cpu_usage", cpu_usage);
    }
    
    last_cpu_time = cpu_time;
    last_idle_time = idle_time;
    
    // Update load averages (simplified)
    system_monitor_update_metric("cpu_load_1m", 0.5);
    system_monitor_update_metric("cpu_load_5m", 0.3);
    system_monitor_update_metric("cpu_load_15m", 0.2);
}

// Collect memory metrics
void system_monitor_collect_memory_metrics(void) {
    // Get memory information
    memory_info_t* mem_info = memory_get_info();
    if (mem_info) {
        u64 total_mb = mem_info->total_pages * 4096 / (1024 * 1024);
        u64 used_mb = (mem_info->total_pages - mem_info->free_pages) * 4096 / (1024 * 1024);
        u64 free_mb = mem_info->free_pages * 4096 / (1024 * 1024);
        double usage_percent = 100.0 * used_mb / total_mb;
        
        system_monitor_update_metric("memory_total", (double)total_mb);
        system_monitor_update_metric("memory_used", (double)used_mb);
        system_monitor_update_metric("memory_free", (double)free_mb);
        system_monitor_update_metric("memory_usage", usage_percent);
    }
}

// Collect process metrics
void system_monitor_collect_process_metrics(void) {
    // Get process information
    int process_count = process_get_count();
    int thread_count = process_get_thread_count();
    
    system_monitor_update_metric("process_count", (double)process_count);
    system_monitor_update_metric("thread_count", (double)thread_count);
    
    // Calculate process CPU usage (simplified)
    double process_cpu_usage = 25.0; // Simplified
    system_monitor_update_metric("process_cpu_usage", process_cpu_usage);
}

// Collect system metrics
void system_monitor_collect_system_metrics(void) {
    u64 current_time = system_monitor_get_time();
    
    // Update uptime
    system_monitor_update_metric("uptime", (double)current_time);
    
    // Update boot time (simplified)
    system_monitor_update_metric("boot_time", 1700000000.0); // Placeholder
    
    // Update system load (simplified)
    system_monitor_update_metric("system_load", 0.5);
}

// Check alerts
void system_monitor_check_alerts(void) {
    if (!monitor_initialized) {
        return;
    }
    
    u64 current_time = system_monitor_get_time();
    
    for (int i = 0; i < alert_count; i++) {
        system_alert_t* alert = &alerts[i];
        system_metric_t* metric = system_monitor_get_metric(alert->metric_name);
        
        if (metric && metric->current_value >= alert->threshold) {
            if (!alert->triggered) {
                alert->triggered = true;
                alert->last_triggered = current_time;
                alert->trigger_count++;
                
                KWARN("Alert triggered: %s (%.2f >= %.2f)", 
                      alert->description, metric->current_value, alert->threshold);
            }
        } else {
            if (alert->triggered) {
                alert->triggered = false;
                KINFO("Alert cleared: %s", alert->description);
            }
        }
    }
}

// Get alerts
system_alert_t* system_monitor_get_alerts(int* count) {
    if (count) {
        *count = alert_count;
    }
    return alerts;
}

// Get system monitor state
system_monitor_state_t* system_monitor_get_state(void) {
    return &monitor_state;
}

// Check if system monitor is initialized
bool system_monitor_is_initialized(void) {
    return monitor_initialized;
}

// Get current time (simplified)
u64 system_monitor_get_time(void) {
    // TODO: Implement proper system time
    static u64 time_counter = 0;
    return time_counter++;
}

// Update system monitor (called periodically)
void system_monitor_update(void) {
    if (!monitor_initialized) {
        return;
    }
    
    // Collect metrics
    system_monitor_collect_metrics();
    
    // Check alerts
    system_monitor_check_alerts();
    
    // Update monitor state
    monitor_state.last_update = system_monitor_get_time();
    monitor_state.metric_count = metric_count;
    monitor_state.alert_count = alert_count;
}

// Generate system report
void system_monitor_generate_report(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return;
    }
    
    int offset = 0;
    
    // Header
    offset += snprintf(buffer + offset, buffer_size - offset, 
                      "=== RaeenOS System Monitor Report ===\n");
    offset += snprintf(buffer + offset, buffer_size - offset, 
                      "Generated: %llu\n\n", system_monitor_get_time());
    
    // Metrics
    offset += snprintf(buffer + offset, buffer_size - offset, "METRICS:\n");
    for (int i = 0; i < metric_count && offset < buffer_size - 100; i++) {
        system_metric_t* metric = &metrics[i];
        offset += snprintf(buffer + offset, buffer_size - offset,
                          "  %s: %.2f %s (min: %.2f, max: %.2f, avg: %.2f)\n",
                          metric->name, metric->current_value, metric->unit,
                          metric->min_value, metric->max_value, metric->average_value);
    }
    
    // Alerts
    offset += snprintf(buffer + offset, buffer_size - offset, "\nALERTS:\n");
    for (int i = 0; i < alert_count && offset < buffer_size - 100; i++) {
        system_alert_t* alert = &alerts[i];
        offset += snprintf(buffer + offset, buffer_size - offset,
                          "  %s: %s (triggered: %s, count: %d)\n",
                          alert->name, alert->description,
                          alert->triggered ? "YES" : "NO", alert->trigger_count);
    }
    
    // Footer
    offset += snprintf(buffer + offset, buffer_size - offset, 
                      "\n=== End Report ===\n");
}

// Shutdown system monitor
void system_monitor_shutdown(void) {
    if (!monitor_initialized) {
        return;
    }
    
    KINFO("Shutting down system monitor...");
    
    monitor_initialized = false;
    monitor_state.initialized = false;
    
    KINFO("System monitor shutdown complete");
} 