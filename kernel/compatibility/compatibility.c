#include "compatibility.h"
#include "windows/win32_api.h"
#include "macos/cocoa_api.h"
#include "memory/memory.h"
#include "process/process.h"
#include "security/security.h"
#include <string.h>
#include <stdio.h>

// Global compatibility state
static bool compatibility_initialized = false;
static compatibility_config_t global_config;
static compatibility_error_handler_t error_handler = NULL;

// Process tracking
#define MAX_COMPAT_PROCESSES 256
static compatibility_process_t compat_processes[MAX_COMPAT_PROCESSES];
static u32 next_process_id = 1;

// Statistics
static compatibility_stats_t current_stats = {0};

// Forward declarations
static error_t detect_binary_type(const char* path, compatibility_type_t* type);
static error_t create_sandbox_context(compatibility_type_t type, void** context);
static void destroy_sandbox_context(void* context);

error_t compatibility_init(void) {
    if (compatibility_initialized) {
        return SUCCESS;
    }
    
    // Initialize default configuration
    memset(&global_config, 0, sizeof(global_config));
    global_config.enable_graphics_acceleration = true;
    global_config.enable_audio_support = true;
    global_config.enable_network_support = true;
    global_config.enable_file_system_access = true;
    global_config.memory_limit_mb = 2048;  // 2GB default
    global_config.cpu_limit_percent = 100;
    
    // Initialize process tracking
    memset(compat_processes, 0, sizeof(compat_processes));
    next_process_id = 1;
    
    // Initialize statistics
    memset(&current_stats, 0, sizeof(current_stats));
    
    compatibility_initialized = true;
    compatibility_log("Compatibility layer initialized");
    
    return SUCCESS;
}

void compatibility_shutdown(void) {
    if (!compatibility_initialized) {
        return;
    }
    
    // Terminate all running processes
    for (int i = 0; i < MAX_COMPAT_PROCESSES; i++) {
        if (compat_processes[i].process_id != 0) {
            compatibility_process_terminate(compat_processes[i].process_id);
        }
    }
    
    // Shutdown APIs
    windows_compat_shutdown();
    macos_compat_shutdown();
    
    compatibility_initialized = false;
    compatibility_log("Compatibility layer shutdown complete");
}

error_t compatibility_process_create(const char* binary_path, compatibility_process_t* process) {
    if (!compatibility_initialized || !binary_path || !process) {
        return E_INVAL;
    }
    
    // Detect binary type
    compatibility_type_t binary_type;
    error_t result = detect_binary_type(binary_path, &binary_type);
    if (result != SUCCESS) {
        return result;
    }
    
    // Find free process slot
    int slot = -1;
    for (int i = 0; i < MAX_COMPAT_PROCESSES; i++) {
        if (compat_processes[i].process_id == 0) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        return E_NOMEM;  // No free slots
    }
    
    // Create sandbox context
    void* sandbox_context;
    result = create_sandbox_context(binary_type, &sandbox_context);
    if (result != SUCCESS) {
        return result;
    }
    
    // Initialize process structure
    process->process_id = next_process_id++;
    process->type = binary_type;
    process->sandbox_context = sandbox_context;
    process->memory_usage = 0;
    process->cpu_usage = 0;
    
    // Store in global array
    compat_processes[slot] = *process;
    
    // Update statistics
    current_stats.total_processes++;
    current_stats.active_processes++;
    
    compatibility_log("Created compatibility process %u for %s", 
                     process->process_id, binary_path);
    
    return SUCCESS;
}

error_t compatibility_process_terminate(u32 process_id) {
    if (!compatibility_initialized) {
        return E_INVAL;
    }
    
    // Find process
    int slot = -1;
    for (int i = 0; i < MAX_COMPAT_PROCESSES; i++) {
        if (compat_processes[i].process_id == process_id) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        return E_NOENT;  // Process not found
    }
    
    // Destroy sandbox context
    destroy_sandbox_context(compat_processes[slot].sandbox_context);
    
    // Update statistics
    current_stats.active_processes--;
    
    // Clear slot
    memset(&compat_processes[slot], 0, sizeof(compatibility_process_t));
    
    compatibility_log("Terminated compatibility process %u", process_id);
    
    return SUCCESS;
}

error_t windows_compat_init(const compatibility_config_t* config) {
    if (!compatibility_initialized) {
        return E_INVAL;
    }
    
    // Initialize Win32 API
    error_t result = win32_api_init();
    if (result != SUCCESS) {
        return result;
    }
    
    // Store configuration
    if (config) {
        global_config = *config;
    }
    
    compatibility_log("Windows compatibility layer initialized");
    
    return SUCCESS;
}

error_t macos_compat_init(const compatibility_config_t* config) {
    if (!compatibility_initialized) {
        return E_INVAL;
    }
    
    // Initialize Cocoa API
    error_t result = cocoa_api_init();
    if (result != SUCCESS) {
        return result;
    }
    
    // Store configuration
    if (config) {
        global_config = *config;
    }
    
    compatibility_log("macOS compatibility layer initialized");
    
    return SUCCESS;
}

error_t compatibility_get_stats(compatibility_stats_t* stats) {
    if (!compatibility_initialized || !stats) {
        return E_INVAL;
    }
    
    *stats = current_stats;
    
    // Calculate current memory usage
    stats->total_memory_usage = 0;
    for (int i = 0; i < MAX_COMPAT_PROCESSES; i++) {
        if (compat_processes[i].process_id != 0) {
            stats->total_memory_usage += compat_processes[i].memory_usage;
        }
    }
    
    return SUCCESS;
}

// Helper functions
static error_t detect_binary_type(const char* path, compatibility_type_t* type) {
    if (!path || !type) {
        return E_INVAL;
    }
    
    // Read file header to detect type
    FILE* file = fopen(path, "rb");
    if (!file) {
        return E_NOENT;
    }
    
    u8 header[16];
    size_t bytes_read = fread(header, 1, sizeof(header), file);
    fclose(file);
    
    if (bytes_read < 4) {
        return E_INVAL;
    }
    
    // Check for PE (Windows) signature
    if (header[0] == 'M' && header[1] == 'Z') {
        *type = COMPAT_WINDOWS;
        return SUCCESS;
    }
    
    // Check for Mach-O (macOS) signature
    if ((header[0] == 0xFE && header[1] == 0xED && header[2] == 0xFA && header[3] == 0xCE) ||
        (header[0] == 0xFE && header[1] == 0xED && header[2] == 0xFA && header[3] == 0xCF)) {
        *type = COMPAT_MACOS;
        return SUCCESS;
    }
    
    return COMPAT_STATUS_INVALID_BINARY;
}

static error_t create_sandbox_context(compatibility_type_t type, void** context) {
    if (!context) {
        return E_INVAL;
    }
    
    // Allocate sandbox context
    *context = memory_alloc(sizeof(sandbox_context_t));
    if (!*context) {
        return E_NOMEM;
    }
    
    // Initialize based on type
    sandbox_context_t* ctx = (sandbox_context_t*)*context;
    ctx->type = type;
    ctx->memory_limit = global_config.memory_limit_mb * 1024 * 1024;
    ctx->cpu_limit = global_config.cpu_limit_percent;
    ctx->file_access_enabled = global_config.enable_file_system_access;
    ctx->network_access_enabled = global_config.enable_network_support;
    
    return SUCCESS;
}

static void destroy_sandbox_context(void* context) {
    if (context) {
        memory_free(context);
    }
}

void compatibility_log(const char* format, ...) {
    if (!compatibility_initialized) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    // Use kernel logging system
    kernel_log("COMPAT", format, args);
    
    va_end(args);
}

// Stub implementations for missing functions
error_t windows_compat_shutdown(void) {
    return SUCCESS;
}

error_t macos_compat_shutdown(void) {
    return SUCCESS;
} 