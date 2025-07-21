#include "kernel.h"
#include "process/process.h"
#include "memory/memory.h"
#include "drivers/driver_framework.h"
#include "hal/hal.h"
#include <string.h>

// Performance optimization configuration
#define OPTIMIZER_TICK_INTERVAL 1000  // 1 second
#define CPU_USAGE_THRESHOLD 80        // 80% CPU usage threshold
#define MEMORY_USAGE_THRESHOLD 85     // 85% memory usage threshold
#define I/O_THROTTLE_THRESHOLD 1000   // 1000 I/O operations per second
#define POWER_SAVE_THRESHOLD 30       // 30% CPU usage for power save mode
#define THERMAL_THRESHOLD 85          // 85°C thermal threshold

// Performance metrics
typedef struct {
    u64 cpu_usage_total;
    u64 cpu_usage_user;
    u64 cpu_usage_system;
    u64 cpu_usage_idle;
    u64 memory_usage_total;
    u64 memory_usage_used;
    u64 memory_usage_free;
    u64 memory_usage_cached;
    u64 io_operations_read;
    u64 io_operations_write;
    u64 io_operations_total;
    u64 network_packets_in;
    u64 network_packets_out;
    u64 network_bytes_in;
    u64 network_bytes_out;
    u32 temperature;
    u32 power_consumption;
    u64 timestamp;
} performance_metrics_t;

// CPU frequency scaling
typedef struct {
    u32 current_frequency;
    u32 min_frequency;
    u32 max_frequency;
    u32 available_frequencies[16];
    u32 num_frequencies;
    bool frequency_scaling_enabled;
    bool turbo_boost_enabled;
} cpu_frequency_t;

// Memory optimization
typedef struct {
    u64 page_cache_size;
    u64 swap_usage;
    u64 swap_total;
    u32 page_reclaim_threshold;
    u32 page_reclaim_target;
    bool memory_compression_enabled;
    bool swap_enabled;
} memory_optimization_t;

// I/O optimization
typedef struct {
    u32 read_ahead_size;
    u32 write_behind_size;
    u32 io_scheduler;
    u32 io_priority;
    bool io_optimization_enabled;
    u64 last_io_operation;
} io_optimization_t;

// Power management
typedef struct {
    u32 power_state;
    u32 cpu_power_limit;
    u32 gpu_power_limit;
    u32 thermal_threshold;
    bool power_save_enabled;
    bool thermal_protection_enabled;
    u32 idle_timeout;
} power_management_t;

// Global optimization state
static performance_metrics_t current_metrics = {0};
static performance_metrics_t previous_metrics = {0};
static cpu_frequency_t cpu_freq = {0};
static memory_optimization_t mem_opt = {0};
static io_optimization_t io_opt = {0};
static power_management_t power_mgmt = {0};
static bool optimizer_active = false;
static u64 last_optimization_tick = 0;

// Forward declarations
static void optimizer_collect_metrics(void);
static void optimizer_analyze_performance(void);
static void optimizer_apply_optimizations(void);
static void optimizer_cpu_frequency_scale(void);
static void optimizer_memory_optimize(void);
static void optimizer_io_optimize(void);
static void optimizer_power_manage(void);
static void optimizer_thermal_protect(void);
static u32 optimizer_calculate_cpu_usage(void);
static u32 optimizer_calculate_memory_usage(void);
static u32 optimizer_calculate_io_load(void);
static void optimizer_adjust_scheduler(void);
static void optimizer_cache_optimize(void);
static void optimizer_network_optimize(void);

// Initialize performance optimizer
error_t performance_optimizer_init(void) {
    KINFO("Initializing performance optimizer");
    
    // Initialize CPU frequency scaling
    cpu_freq.min_frequency = 800;  // 800 MHz
    cpu_freq.max_frequency = 3500; // 3.5 GHz
    cpu_freq.current_frequency = cpu_freq.max_frequency;
    cpu_freq.frequency_scaling_enabled = true;
    cpu_freq.turbo_boost_enabled = true;
    
    // Common CPU frequencies
    cpu_freq.available_frequencies[0] = 800;
    cpu_freq.available_frequencies[1] = 1200;
    cpu_freq.available_frequencies[2] = 1600;
    cpu_freq.available_frequencies[3] = 2000;
    cpu_freq.available_frequencies[4] = 2400;
    cpu_freq.available_frequencies[5] = 2800;
    cpu_freq.available_frequencies[6] = 3200;
    cpu_freq.available_frequencies[7] = 3500;
    cpu_freq.num_frequencies = 8;
    
    // Initialize memory optimization
    mem_opt.page_cache_size = 0;
    mem_opt.swap_usage = 0;
    mem_opt.swap_total = 0;
    mem_opt.page_reclaim_threshold = 80;  // 80%
    mem_opt.page_reclaim_target = 60;     // 60%
    mem_opt.memory_compression_enabled = true;
    mem_opt.swap_enabled = true;
    
    // Initialize I/O optimization
    io_opt.read_ahead_size = 64;  // 64 KB
    io_opt.write_behind_size = 32; // 32 KB
    io_opt.io_scheduler = 0;      // Default scheduler
    io_opt.io_priority = 5;       // Normal priority
    io_opt.io_optimization_enabled = true;
    io_opt.last_io_operation = 0;
    
    // Initialize power management
    power_mgmt.power_state = 0;  // Normal power state
    power_mgmt.cpu_power_limit = 100; // 100% power limit
    power_mgmt.gpu_power_limit = 100; // 100% power limit
    power_mgmt.thermal_threshold = THERMAL_THRESHOLD;
    power_mgmt.power_save_enabled = false;
    power_mgmt.thermal_protection_enabled = true;
    power_mgmt.idle_timeout = 30000; // 30 seconds
    
    // Initialize metrics
    memset(&current_metrics, 0, sizeof(current_metrics));
    memset(&previous_metrics, 0, sizeof(previous_metrics));
    
    optimizer_active = true;
    last_optimization_tick = hal_get_timestamp();
    
    KINFO("Performance optimizer initialized");
    KINFO("CPU: %u-%u MHz, Memory: %u%% threshold, I/O: %u KB read-ahead",
          cpu_freq.min_frequency, cpu_freq.max_frequency,
          mem_opt.page_reclaim_threshold, io_opt.read_ahead_size);
    
    return SUCCESS;
}

// Performance optimizer tick
void performance_optimizer_tick(void) {
    if (!optimizer_active) {
        return;
    }
    
    u64 current_time = hal_get_timestamp();
    if (current_time - last_optimization_tick < OPTIMIZER_TICK_INTERVAL) {
        return;
    }
    
    // Collect current metrics
    optimizer_collect_metrics();
    
    // Analyze performance
    optimizer_analyze_performance();
    
    // Apply optimizations
    optimizer_apply_optimizations();
    
    // Update previous metrics
    memcpy(&previous_metrics, &current_metrics, sizeof(performance_metrics_t));
    last_optimization_tick = current_time;
}

// Collect performance metrics
static void optimizer_collect_metrics(void) {
    current_metrics.timestamp = hal_get_timestamp();
    
    // CPU metrics
    current_metrics.cpu_usage_total = hal_get_cpu_usage();
    current_metrics.cpu_usage_user = hal_get_cpu_user_time();
    current_metrics.cpu_usage_system = hal_get_cpu_system_time();
    current_metrics.cpu_usage_idle = hal_get_cpu_idle_time();
    
    // Memory metrics
    memory_stats_t mem_stats = memory_get_stats();
    current_metrics.memory_usage_total = mem_stats.total_memory;
    current_metrics.memory_usage_used = mem_stats.used_memory;
    current_metrics.memory_usage_free = mem_stats.free_memory;
    current_metrics.memory_usage_cached = mem_stats.cached_memory;
    
    // I/O metrics
    current_metrics.io_operations_read = hal_get_io_read_count();
    current_metrics.io_operations_write = hal_get_io_write_count();
    current_metrics.io_operations_total = current_metrics.io_operations_read + current_metrics.io_operations_write;
    
    // Network metrics
    current_metrics.network_packets_in = hal_get_network_packets_in();
    current_metrics.network_packets_out = hal_get_network_packets_out();
    current_metrics.network_bytes_in = hal_get_network_bytes_in();
    current_metrics.network_bytes_out = hal_get_network_bytes_out();
    
    // Hardware metrics
    current_metrics.temperature = hal_get_cpu_temperature();
    current_metrics.power_consumption = hal_get_power_consumption();
}

// Analyze performance patterns
static void optimizer_analyze_performance(void) {
    u32 cpu_usage = optimizer_calculate_cpu_usage();
    u32 memory_usage = optimizer_calculate_memory_usage();
    u32 io_load = optimizer_calculate_io_load();
    
    KDEBUG("Performance analysis: CPU=%u%%, Memory=%u%%, I/O=%u ops/s, Temp=%u°C",
           cpu_usage, memory_usage, io_load, current_metrics.temperature);
    
    // Check for performance bottlenecks
    if (cpu_usage > CPU_USAGE_THRESHOLD) {
        KDEBUG("High CPU usage detected: %u%%", cpu_usage);
    }
    
    if (memory_usage > MEMORY_USAGE_THRESHOLD) {
        KDEBUG("High memory usage detected: %u%%", memory_usage);
    }
    
    if (io_load > I/O_THROTTLE_THRESHOLD) {
        KDEBUG("High I/O load detected: %u ops/s", io_load);
    }
    
    if (current_metrics.temperature > THERMAL_THRESHOLD) {
        KDEBUG("High temperature detected: %u°C", current_metrics.temperature);
    }
}

// Apply performance optimizations
static void optimizer_apply_optimizations(void) {
    u32 cpu_usage = optimizer_calculate_cpu_usage();
    u32 memory_usage = optimizer_calculate_memory_usage();
    u32 io_load = optimizer_calculate_io_load();
    
    // CPU frequency scaling
    optimizer_cpu_frequency_scale();
    
    // Memory optimization
    optimizer_memory_optimize();
    
    // I/O optimization
    optimizer_io_optimize();
    
    // Power management
    optimizer_power_manage();
    
    // Thermal protection
    optimizer_thermal_protect();
    
    // Scheduler adjustments
    optimizer_adjust_scheduler();
    
    // Cache optimization
    optimizer_cache_optimize();
    
    // Network optimization
    optimizer_network_optimize();
}

// CPU frequency scaling optimization
static void optimizer_cpu_frequency_scale(void) {
    if (!cpu_freq.frequency_scaling_enabled) {
        return;
    }
    
    u32 cpu_usage = optimizer_calculate_cpu_usage();
    u32 target_frequency = cpu_freq.current_frequency;
    
    // Scale frequency based on CPU usage
    if (cpu_usage > 90) {
        // High load - use maximum frequency
        target_frequency = cpu_freq.max_frequency;
        if (cpu_freq.turbo_boost_enabled) {
            target_frequency = cpu_freq.max_frequency + 500; // Turbo boost
        }
    } else if (cpu_usage > 70) {
        // Medium-high load - use high frequency
        target_frequency = cpu_freq.max_frequency;
    } else if (cpu_usage > 50) {
        // Medium load - use medium frequency
        target_frequency = cpu_freq.available_frequencies[4]; // 2400 MHz
    } else if (cpu_usage > 30) {
        // Medium-low load - use low-medium frequency
        target_frequency = cpu_freq.available_frequencies[2]; // 1600 MHz
    } else if (cpu_usage > 10) {
        // Low load - use low frequency
        target_frequency = cpu_freq.available_frequencies[1]; // 1200 MHz
    } else {
        // Very low load - use minimum frequency
        target_frequency = cpu_freq.min_frequency;
    }
    
    // Apply frequency change
    if (target_frequency != cpu_freq.current_frequency) {
        error_t result = hal_set_cpu_frequency(target_frequency);
        if (result == SUCCESS) {
            KDEBUG("CPU frequency scaled: %u MHz -> %u MHz", 
                   cpu_freq.current_frequency, target_frequency);
            cpu_freq.current_frequency = target_frequency;
        }
    }
}

// Memory optimization
static void optimizer_memory_optimize(void) {
    u32 memory_usage = optimizer_calculate_memory_usage();
    
    // Page cache management
    if (memory_usage > mem_opt.page_reclaim_threshold) {
        // Reclaim pages from cache
        u64 target_cache_size = current_metrics.memory_usage_total * mem_opt.page_reclaim_target / 100;
        if (current_metrics.memory_usage_cached > target_cache_size) {
            u64 reclaim_size = current_metrics.memory_usage_cached - target_cache_size;
            memory_reclaim_cache(reclaim_size);
            KDEBUG("Memory cache reclaimed: %llu bytes", reclaim_size);
        }
    }
    
    // Memory compression
    if (mem_opt.memory_compression_enabled && memory_usage > 75) {
        memory_compress_pages();
        KDEBUG("Memory compression applied");
    }
    
    // Swap management
    if (mem_opt.swap_enabled && memory_usage > 90) {
        // Increase swap usage
        memory_swap_out_pages();
        KDEBUG("Pages swapped out due to high memory usage");
    } else if (mem_opt.swap_enabled && memory_usage < 50 && mem_opt.swap_usage > 0) {
        // Reduce swap usage
        memory_swap_in_pages();
        KDEBUG("Pages swapped in due to low memory usage");
    }
}

// I/O optimization
static void optimizer_io_optimize(void) {
    if (!io_opt.io_optimization_enabled) {
        return;
    }
    
    u32 io_load = optimizer_calculate_io_load();
    
    // Adjust read-ahead size based on I/O patterns
    if (io_load > 500) {
        // High I/O load - increase read-ahead
        if (io_opt.read_ahead_size < 128) {
            io_opt.read_ahead_size = 128;
            hal_set_read_ahead_size(io_opt.read_ahead_size);
            KDEBUG("Read-ahead size increased to %u KB", io_opt.read_ahead_size);
        }
    } else if (io_load < 100) {
        // Low I/O load - decrease read-ahead
        if (io_opt.read_ahead_size > 32) {
            io_opt.read_ahead_size = 32;
            hal_set_read_ahead_size(io_opt.read_ahead_size);
            KDEBUG("Read-ahead size decreased to %u KB", io_opt.read_ahead_size);
        }
    }
    
    // I/O scheduler optimization
    if (io_load > I/O_THROTTLE_THRESHOLD) {
        // Switch to deadline scheduler for high I/O
        if (io_opt.io_scheduler != 1) {
            io_opt.io_scheduler = 1;
            hal_set_io_scheduler(io_opt.io_scheduler);
            KDEBUG("I/O scheduler switched to deadline");
        }
    } else {
        // Switch to CFQ scheduler for normal I/O
        if (io_opt.io_scheduler != 0) {
            io_opt.io_scheduler = 0;
            hal_set_io_scheduler(io_opt.io_scheduler);
            KDEBUG("I/O scheduler switched to CFQ");
        }
    }
    
    // Write-behind optimization
    if (current_metrics.io_operations_write > current_metrics.io_operations_read * 2) {
        // Write-heavy workload - increase write-behind
        if (io_opt.write_behind_size < 64) {
            io_opt.write_behind_size = 64;
            hal_set_write_behind_size(io_opt.write_behind_size);
            KDEBUG("Write-behind size increased to %u KB", io_opt.write_behind_size);
        }
    }
}

// Power management optimization
static void optimizer_power_manage(void) {
    u32 cpu_usage = optimizer_calculate_cpu_usage();
    u32 memory_usage = optimizer_calculate_memory_usage();
    
    // Power save mode
    if (cpu_usage < POWER_SAVE_THRESHOLD && memory_usage < 60) {
        if (!power_mgmt.power_save_enabled) {
            power_mgmt.power_save_enabled = true;
            power_mgmt.cpu_power_limit = 70;  // 70% power limit
            power_mgmt.gpu_power_limit = 50;  // 50% power limit
            
            hal_set_cpu_power_limit(power_mgmt.cpu_power_limit);
            hal_set_gpu_power_limit(power_mgmt.gpu_power_limit);
            
            KDEBUG("Power save mode enabled");
        }
    } else {
        if (power_mgmt.power_save_enabled) {
            power_mgmt.power_save_enabled = false;
            power_mgmt.cpu_power_limit = 100; // 100% power limit
            power_mgmt.gpu_power_limit = 100; // 100% power limit
            
            hal_set_cpu_power_limit(power_mgmt.cpu_power_limit);
            hal_set_gpu_power_limit(power_mgmt.gpu_power_limit);
            
            KDEBUG("Power save mode disabled");
        }
    }
    
    // Idle timeout management
    if (cpu_usage < 5) {
        // Very low usage - reduce idle timeout
        if (power_mgmt.idle_timeout > 10000) {
            power_mgmt.idle_timeout = 10000; // 10 seconds
            hal_set_idle_timeout(power_mgmt.idle_timeout);
            KDEBUG("Idle timeout reduced to %u ms", power_mgmt.idle_timeout);
        }
    } else {
        // Normal usage - restore idle timeout
        if (power_mgmt.idle_timeout < 30000) {
            power_mgmt.idle_timeout = 30000; // 30 seconds
            hal_set_idle_timeout(power_mgmt.idle_timeout);
            KDEBUG("Idle timeout restored to %u ms", power_mgmt.idle_timeout);
        }
    }
}

// Thermal protection
static void optimizer_thermal_protect(void) {
    if (!power_mgmt.thermal_protection_enabled) {
        return;
    }
    
    if (current_metrics.temperature > THERMAL_THRESHOLD) {
        // High temperature - apply thermal throttling
        u32 throttle_level = (current_metrics.temperature - THERMAL_THRESHOLD) / 10;
        if (throttle_level > 5) throttle_level = 5;
        
        u32 power_limit = 100 - (throttle_level * 10);
        if (power_limit < 30) power_limit = 30;
        
        if (power_mgmt.cpu_power_limit > power_limit) {
            power_mgmt.cpu_power_limit = power_limit;
            hal_set_cpu_power_limit(power_mgmt.cpu_power_limit);
            KDEBUG("Thermal throttling applied: CPU power limit %u%%", power_limit);
        }
        
        // Reduce CPU frequency
        if (cpu_freq.current_frequency > cpu_freq.min_frequency) {
            u32 target_freq = cpu_freq.current_frequency - (throttle_level * 200);
            if (target_freq < cpu_freq.min_frequency) {
                target_freq = cpu_freq.min_frequency;
            }
            
            if (target_freq != cpu_freq.current_frequency) {
                hal_set_cpu_frequency(target_freq);
                cpu_freq.current_frequency = target_freq;
                KDEBUG("Thermal throttling: CPU frequency reduced to %u MHz", target_freq);
            }
        }
    } else {
        // Normal temperature - restore power limits
        if (power_mgmt.cpu_power_limit < 100 && !power_mgmt.power_save_enabled) {
            power_mgmt.cpu_power_limit = 100;
            hal_set_cpu_power_limit(power_mgmt.cpu_power_limit);
            KDEBUG("Thermal throttling removed: CPU power limit restored to 100%%");
        }
    }
}

// Scheduler adjustments
static void optimizer_adjust_scheduler(void) {
    u32 cpu_usage = optimizer_calculate_cpu_usage();
    
    // Adjust scheduler quantum based on CPU usage
    if (cpu_usage > 90) {
        // High CPU usage - reduce quantum for better responsiveness
        scheduler_set_quantum(5); // 5ms quantum
        KDEBUG("Scheduler quantum reduced to 5ms for high CPU usage");
    } else if (cpu_usage > 70) {
        // Medium-high CPU usage - normal quantum
        scheduler_set_quantum(10); // 10ms quantum
        KDEBUG("Scheduler quantum set to 10ms");
    } else {
        // Low CPU usage - increase quantum for better throughput
        scheduler_set_quantum(20); // 20ms quantum
        KDEBUG("Scheduler quantum increased to 20ms for low CPU usage");
    }
    
    // Adjust process priorities based on I/O patterns
    u32 io_load = optimizer_calculate_io_load();
    if (io_load > 500) {
        // High I/O - boost I/O-bound processes
        scheduler_boost_io_bound_processes();
        KDEBUG("I/O-bound processes boosted due to high I/O load");
    }
}

// Cache optimization
static void optimizer_cache_optimize(void) {
    u32 memory_usage = optimizer_calculate_memory_usage();
    
    // Adjust cache size based on memory usage
    if (memory_usage < 50) {
        // Low memory usage - increase cache
        memory_increase_cache_size();
        KDEBUG("Cache size increased due to low memory usage");
    } else if (memory_usage > 85) {
        // High memory usage - decrease cache
        memory_decrease_cache_size();
        KDEBUG("Cache size decreased due to high memory usage");
    }
    
    // Prefetch optimization based on access patterns
    memory_optimize_prefetch();
}

// Network optimization
static void optimizer_network_optimize(void) {
    u64 network_load = current_metrics.network_packets_in + current_metrics.network_packets_out;
    
    // Adjust network buffer sizes based on load
    if (network_load > 1000) {
        // High network load - increase buffers
        hal_increase_network_buffers();
        KDEBUG("Network buffers increased due to high load");
    } else if (network_load < 100) {
        // Low network load - decrease buffers
        hal_decrease_network_buffers();
        KDEBUG("Network buffers decreased due to low load");
    }
    
    // TCP window scaling optimization
    if (current_metrics.network_bytes_in > 1000000) { // 1MB/s
        hal_optimize_tcp_window_scaling();
        KDEBUG("TCP window scaling optimized");
    }
}

// Calculate CPU usage percentage
static u32 optimizer_calculate_cpu_usage(void) {
    if (previous_metrics.cpu_usage_total == 0) {
        return 0;
    }
    
    u64 total_diff = current_metrics.cpu_usage_total - previous_metrics.cpu_usage_total;
    u64 idle_diff = current_metrics.cpu_usage_idle - previous_metrics.cpu_usage_idle;
    
    if (total_diff == 0) {
        return 0;
    }
    
    u32 usage = 100 - ((idle_diff * 100) / total_diff);
    return usage > 100 ? 100 : usage;
}

// Calculate memory usage percentage
static u32 optimizer_calculate_memory_usage(void) {
    if (current_metrics.memory_usage_total == 0) {
        return 0;
    }
    
    u32 usage = (current_metrics.memory_usage_used * 100) / current_metrics.memory_usage_total;
    return usage > 100 ? 100 : usage;
}

// Calculate I/O load (operations per second)
static u32 optimizer_calculate_io_load(void) {
    u64 time_diff = current_metrics.timestamp - previous_metrics.timestamp;
    if (time_diff == 0) {
        return 0;
    }
    
    u64 io_diff = current_metrics.io_operations_total - previous_metrics.io_operations_total;
    u32 load = (io_diff * 1000) / time_diff; // ops per second
    return load;
}

// Get current performance metrics
performance_metrics_t* performance_optimizer_get_metrics(void) {
    return &current_metrics;
}

// Get CPU frequency information
cpu_frequency_t* performance_optimizer_get_cpu_freq(void) {
    return &cpu_freq;
}

// Set CPU frequency scaling
error_t performance_optimizer_set_cpu_frequency_scaling(bool enabled) {
    cpu_freq.frequency_scaling_enabled = enabled;
    KINFO("CPU frequency scaling %s", enabled ? "enabled" : "disabled");
    return SUCCESS;
}

// Set power save mode
error_t performance_optimizer_set_power_save(bool enabled) {
    power_mgmt.power_save_enabled = enabled;
    if (enabled) {
        power_mgmt.cpu_power_limit = 70;
        power_mgmt.gpu_power_limit = 50;
    } else {
        power_mgmt.cpu_power_limit = 100;
        power_mgmt.gpu_power_limit = 100;
    }
    
    hal_set_cpu_power_limit(power_mgmt.cpu_power_limit);
    hal_set_gpu_power_limit(power_mgmt.gpu_power_limit);
    
    KINFO("Power save mode %s", enabled ? "enabled" : "disabled");
    return SUCCESS;
}

// Set thermal protection
error_t performance_optimizer_set_thermal_protection(bool enabled) {
    power_mgmt.thermal_protection_enabled = enabled;
    KINFO("Thermal protection %s", enabled ? "enabled" : "disabled");
    return SUCCESS;
}

// Dump performance optimizer information
void performance_optimizer_dump_info(void) {
    KINFO("=== Performance Optimizer Information ===");
    KINFO("Active: %s", optimizer_active ? "Yes" : "No");
    
    if (optimizer_active) {
        KINFO("CPU Frequency: %u MHz (min: %u, max: %u)", 
              cpu_freq.current_frequency, cpu_freq.min_frequency, cpu_freq.max_frequency);
        KINFO("CPU Scaling: %s, Turbo Boost: %s",
              cpu_freq.frequency_scaling_enabled ? "enabled" : "disabled",
              cpu_freq.turbo_boost_enabled ? "enabled" : "disabled");
        
        KINFO("Memory: %u%% used, Cache: %llu bytes, Swap: %llu/%llu bytes",
              optimizer_calculate_memory_usage(),
              current_metrics.memory_usage_cached,
              mem_opt.swap_usage, mem_opt.swap_total);
        
        KINFO("I/O: %u ops/s, Read-ahead: %u KB, Write-behind: %u KB",
              optimizer_calculate_io_load(),
              io_opt.read_ahead_size, io_opt.write_behind_size);
        
        KINFO("Power: %s, CPU Limit: %u%%, GPU Limit: %u%%",
              power_mgmt.power_save_enabled ? "save" : "normal",
              power_mgmt.cpu_power_limit, power_mgmt.gpu_power_limit);
        
        KINFO("Thermal: %u°C, Protection: %s",
              current_metrics.temperature,
              power_mgmt.thermal_protection_enabled ? "enabled" : "disabled");
        
        KINFO("Network: %llu packets/s, %llu bytes/s",
              (current_metrics.network_packets_in + current_metrics.network_packets_out) / 10,
              (current_metrics.network_bytes_in + current_metrics.network_bytes_out) / 10);
    }
} 