/*
 * RaeenOS Performance Profiler
 * Advanced performance analysis tool for RaeenOS
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#define MAX_FUNCTIONS 1024
#define MAX_SAMPLES 10000
#define MAX_NAME_LEN 128

typedef struct {
    char name[MAX_NAME_LEN];
    uint64_t address;
    uint64_t total_time;
    uint64_t call_count;
    uint64_t min_time;
    uint64_t max_time;
    double avg_time;
} function_profile_t;

typedef struct {
    uint64_t timestamp;
    uint64_t address;
    char function[MAX_NAME_LEN];
    uint32_t cpu_usage;
    uint32_t memory_usage;
} sample_t;

static function_profile_t functions[MAX_FUNCTIONS];
static sample_t samples[MAX_SAMPLES];
static int function_count = 0;
static int sample_count = 0;
static int profiling_active = 0;
static uint64_t start_time = 0;

void print_banner(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                 RaeenOS Performance Profiler                ║\n");
    printf("║                         Version 1.0                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void print_usage(const char* program) {
    printf("Usage: %s [options] [command]\n", program);
    printf("\nOptions:\n");
    printf("  -f <file>     Load profile data from file\n");
    printf("  -o <file>     Save profile data to file\n");
    printf("  -t <seconds>  Profile for specified time (default: 10)\n");
    printf("  -s <rate>     Sampling rate in Hz (default: 1000)\n");
    printf("  -v            Verbose output\n");
    printf("  -h            Show this help\n");
    printf("\nCommands:\n");
    printf("  start         Start profiling\n");
    printf("  stop          Stop profiling\n");
    printf("  report        Generate performance report\n");
    printf("  clear         Clear all profile data\n");
    printf("  functions     List function statistics\n");
    printf("  hotspots      Show performance hotspots\n");
    printf("  timeline      Show execution timeline\n");
    printf("  memory        Show memory usage analysis\n");
    printf("  interactive   Enter interactive mode\n");
    printf("\nExamples:\n");
    printf("  %s start                    # Start profiling\n", program);
    printf("  %s -t 30 report            # Profile for 30 seconds and report\n", program);
    printf("  %s -f profile.dat report   # Load and analyze saved data\n", program);
}

uint64_t get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void add_function_sample(const char* name, uint64_t address, uint64_t execution_time) {
    // Find existing function or create new one
    int func_idx = -1;
    for (int i = 0; i < function_count; i++) {
        if (functions[i].address == address || strcmp(functions[i].name, name) == 0) {
            func_idx = i;
            break;
        }
    }
    
    if (func_idx == -1 && function_count < MAX_FUNCTIONS) {
        func_idx = function_count++;
        strncpy(functions[func_idx].name, name, MAX_NAME_LEN - 1);
        functions[func_idx].name[MAX_NAME_LEN - 1] = '\0';
        functions[func_idx].address = address;
        functions[func_idx].total_time = 0;
        functions[func_idx].call_count = 0;
        functions[func_idx].min_time = UINT64_MAX;
        functions[func_idx].max_time = 0;
    }
    
    if (func_idx >= 0) {
        functions[func_idx].total_time += execution_time;
        functions[func_idx].call_count++;
        if (execution_time < functions[func_idx].min_time) {
            functions[func_idx].min_time = execution_time;
        }
        if (execution_time > functions[func_idx].max_time) {
            functions[func_idx].max_time = execution_time;
        }
        functions[func_idx].avg_time = (double)functions[func_idx].total_time / functions[func_idx].call_count;
    }
}

void add_sample(uint64_t address, const char* function, uint32_t cpu_usage, uint32_t memory_usage) {
    if (sample_count < MAX_SAMPLES) {
        samples[sample_count].timestamp = get_timestamp();
        samples[sample_count].address = address;
        strncpy(samples[sample_count].function, function, MAX_NAME_LEN - 1);
        samples[sample_count].function[MAX_NAME_LEN - 1] = '\0';
        samples[sample_count].cpu_usage = cpu_usage;
        samples[sample_count].memory_usage = memory_usage;
        sample_count++;
    }
}

void simulate_profiling(int duration_seconds) {
    printf("Starting profiling simulation for %d seconds...\n", duration_seconds);
    
    start_time = get_timestamp();
    profiling_active = 1;
    
    // Simulate some function calls and performance data
    const char* function_names[] = {
        "kernel_main", "scheduler_tick", "memory_alloc", "interrupt_handler",
        "filesystem_read", "network_send", "graphics_render", "process_switch",
        "timer_handler", "keyboard_handler", "disk_io", "cache_lookup"
    };
    
    int num_functions = sizeof(function_names) / sizeof(function_names[0]);
    
    for (int sec = 0; sec < duration_seconds; sec++) {
        printf("Profiling... %d/%d seconds\r", sec + 1, duration_seconds);
        fflush(stdout);
        
        // Simulate samples for this second
        for (int sample = 0; sample < 100; sample++) {
            int func_idx = rand() % num_functions;
            uint64_t address = 0x400000 + func_idx * 0x1000 + (rand() % 0x100);
            uint32_t cpu_usage = 20 + (rand() % 60); // 20-80% CPU
            uint32_t memory_usage = 10 + (rand() % 40); // 10-50% memory
            uint64_t execution_time = 1000 + (rand() % 10000); // 1-11 microseconds
            
            add_sample(address, function_names[func_idx], cpu_usage, memory_usage);
            add_function_sample(function_names[func_idx], address, execution_time);
        }
        
        // Sleep for 1 second (simulated)
        // In real implementation, would actually sleep
    }
    
    profiling_active = 0;
    printf("\nProfiling completed. Collected %d samples from %d functions.\n", 
           sample_count, function_count);
}

void generate_report(void) {
    if (function_count == 0) {
        printf("No profiling data available. Run profiling first.\n");
        return;
    }
    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    Performance Report                       ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    uint64_t total_time = 0;
    for (int i = 0; i < function_count; i++) {
        total_time += functions[i].total_time;
    }
    
    printf("\nOverall Statistics:\n");
    printf("  Total functions profiled: %d\n", function_count);
    printf("  Total samples collected: %d\n", sample_count);
    printf("  Total execution time: %.2f ms\n", total_time / 1000000.0);
    printf("  Average sample rate: %.1f Hz\n", 
           sample_count > 0 ? (double)sample_count / 10.0 : 0.0);
    
    // Calculate CPU and memory averages
    uint64_t total_cpu = 0, total_memory = 0;
    for (int i = 0; i < sample_count; i++) {
        total_cpu += samples[i].cpu_usage;
        total_memory += samples[i].memory_usage;
    }
    
    if (sample_count > 0) {
        printf("  Average CPU usage: %.1f%%\n", (double)total_cpu / sample_count);
        printf("  Average memory usage: %.1f%%\n", (double)total_memory / sample_count);
    }
}

void show_function_statistics(void) {
    if (function_count == 0) {
        printf("No function data available.\n");
        return;
    }
    
    printf("\nFunction Statistics:\n");
    printf("%-20s %8s %12s %10s %10s %10s\n", 
           "Function", "Calls", "Total(μs)", "Avg(μs)", "Min(μs)", "Max(μs)");
    printf("%-20s %8s %12s %10s %10s %10s\n", 
           "--------------------", "--------", "------------", "----------", "----------", "----------");
    
    // Sort functions by total time (simple bubble sort for demo)
    for (int i = 0; i < function_count - 1; i++) {
        for (int j = 0; j < function_count - i - 1; j++) {
            if (functions[j].total_time < functions[j + 1].total_time) {
                function_profile_t temp = functions[j];
                functions[j] = functions[j + 1];
                functions[j + 1] = temp;
            }
        }
    }
    
    for (int i = 0; i < function_count; i++) {
        printf("%-20s %8llu %12.2f %10.2f %10.2f %10.2f\n",
               functions[i].name,
               (unsigned long long)functions[i].call_count,
               functions[i].total_time / 1000.0,
               functions[i].avg_time / 1000.0,
               functions[i].min_time / 1000.0,
               functions[i].max_time / 1000.0);
    }
}

void show_hotspots(void) {
    if (function_count == 0) {
        printf("No hotspot data available.\n");
        return;
    }
    
    printf("\nPerformance Hotspots (Top 10):\n");
    printf("%-20s %10s %12s %8s\n", "Function", "% Time", "Total(μs)", "Calls");
    printf("%-20s %10s %12s %8s\n", "--------------------", "----------", "------------", "--------");
    
    uint64_t total_time = 0;
    for (int i = 0; i < function_count; i++) {
        total_time += functions[i].total_time;
    }
    
    int count = function_count < 10 ? function_count : 10;
    for (int i = 0; i < count; i++) {
        double percentage = total_time > 0 ? (double)functions[i].total_time * 100.0 / total_time : 0.0;
        printf("%-20s %9.2f%% %12.2f %8llu\n",
               functions[i].name,
               percentage,
               functions[i].total_time / 1000.0,
               (unsigned long long)functions[i].call_count);
    }
}

void show_timeline(void) {
    if (sample_count == 0) {
        printf("No timeline data available.\n");
        return;
    }
    
    printf("\nExecution Timeline (last 20 samples):\n");
    printf("%-12s %-20s %8s %8s\n", "Time(ms)", "Function", "CPU%", "Mem%");
    printf("%-12s %-20s %8s %8s\n", "------------", "--------------------", "--------", "--------");
    
    int start_idx = sample_count > 20 ? sample_count - 20 : 0;
    uint64_t base_time = start_idx > 0 ? samples[start_idx].timestamp : start_time;
    
    for (int i = start_idx; i < sample_count; i++) {
        double time_ms = (samples[i].timestamp - base_time) / 1000000.0;
        printf("%11.2f %-20s %7u%% %7u%%\n",
               time_ms,
               samples[i].function,
               samples[i].cpu_usage,
               samples[i].memory_usage);
    }
}

void show_memory_analysis(void) {
    if (sample_count == 0) {
        printf("No memory data available.\n");
        return;
    }
    
    printf("\nMemory Usage Analysis:\n");
    
    uint32_t min_memory = UINT32_MAX, max_memory = 0;
    uint64_t total_memory = 0;
    
    for (int i = 0; i < sample_count; i++) {
        if (samples[i].memory_usage < min_memory) min_memory = samples[i].memory_usage;
        if (samples[i].memory_usage > max_memory) max_memory = samples[i].memory_usage;
        total_memory += samples[i].memory_usage;
    }
    
    double avg_memory = (double)total_memory / sample_count;
    
    printf("  Minimum memory usage: %u%%\n", min_memory);
    printf("  Maximum memory usage: %u%%\n", max_memory);
    printf("  Average memory usage: %.1f%%\n", avg_memory);
    
    // Memory usage histogram
    printf("\nMemory Usage Distribution:\n");
    int buckets[10] = {0};
    for (int i = 0; i < sample_count; i++) {
        int bucket = samples[i].memory_usage / 10;
        if (bucket > 9) bucket = 9;
        buckets[bucket]++;
    }
    
    for (int i = 0; i < 10; i++) {
        printf("  %2d-%2d%%: ", i * 10, (i + 1) * 10 - 1);
        int bar_length = buckets[i] * 50 / sample_count;
        for (int j = 0; j < bar_length; j++) printf("█");
        printf(" (%d samples)\n", buckets[i]);
    }
}

void clear_data(void) {
    function_count = 0;
    sample_count = 0;
    memset(functions, 0, sizeof(functions));
    memset(samples, 0, sizeof(samples));
    printf("All profiling data cleared.\n");
}

void interactive_mode(void) {
    char command[256];
    
    printf("\nEntering interactive mode. Type 'help' for commands, 'exit' to quit.\n");
    
    while (1) {
        printf("profiler> ");
        fflush(stdout);
        
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }
        
        // Remove newline
        command[strcspn(command, "\n")] = '\0';
        
        if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
            break;
        } else if (strcmp(command, "help") == 0) {
            printf("Interactive commands:\n");
            printf("  start [seconds]  - Start profiling\n");
            printf("  stop             - Stop profiling\n");
            printf("  report           - Show performance report\n");
            printf("  functions        - Show function statistics\n");
            printf("  hotspots         - Show performance hotspots\n");
            printf("  timeline         - Show execution timeline\n");
            printf("  memory           - Show memory analysis\n");
            printf("  clear            - Clear all data\n");
            printf("  exit, quit       - Exit interactive mode\n");
        } else if (strncmp(command, "start", 5) == 0) {
            int duration = 10;
            sscanf(command, "start %d", &duration);
            simulate_profiling(duration);
        } else if (strcmp(command, "stop") == 0) {
            profiling_active = 0;
            printf("Profiling stopped.\n");
        } else if (strcmp(command, "report") == 0) {
            generate_report();
        } else if (strcmp(command, "functions") == 0) {
            show_function_statistics();
        } else if (strcmp(command, "hotspots") == 0) {
            show_hotspots();
        } else if (strcmp(command, "timeline") == 0) {
            show_timeline();
        } else if (strcmp(command, "memory") == 0) {
            show_memory_analysis();
        } else if (strcmp(command, "clear") == 0) {
            clear_data();
        } else if (strlen(command) > 0) {
            printf("Unknown command: %s\n", command);
        }
    }
    
    printf("Exiting interactive mode.\n");
}

int main(int argc, char* argv[]) {
    int duration = 10;
    int verbose = 0;
    const char* input_file = NULL;
    const char* output_file = NULL;
    const char* command = NULL;
    
    print_banner();
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            duration = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (!command) {
            command = argv[i];
        }
    }
    
    if (input_file) {
        printf("Loading profile data from %s...\n", input_file);
        // In real implementation, would load data from file
        simulate_profiling(5); // Simulate some data for demo
    }
    
    if (command) {
        if (strcmp(command, "start") == 0) {
            simulate_profiling(duration);
        } else if (strcmp(command, "report") == 0) {
            if (function_count == 0) simulate_profiling(duration);
            generate_report();
        } else if (strcmp(command, "functions") == 0) {
            if (function_count == 0) simulate_profiling(duration);
            show_function_statistics();
        } else if (strcmp(command, "hotspots") == 0) {
            if (function_count == 0) simulate_profiling(duration);
            show_hotspots();
        } else if (strcmp(command, "timeline") == 0) {
            if (function_count == 0) simulate_profiling(duration);
            show_timeline();
        } else if (strcmp(command, "memory") == 0) {
            if (function_count == 0) simulate_profiling(duration);
            show_memory_analysis();
        } else if (strcmp(command, "clear") == 0) {
            clear_data();
        } else if (strcmp(command, "interactive") == 0) {
            interactive_mode();
        } else {
            printf("Unknown command: %s\n", command);
            print_usage(argv[0]);
            return 1;
        }
    } else {
        // Default: run profiling and show report
        simulate_profiling(duration);
        generate_report();
    }
    
    if (output_file) {
        printf("Saving profile data to %s...\n", output_file);
        // In real implementation, would save data to file
    }
    
    return 0;
}