#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#include "types.h"
#include <stddef.h>

// Compatibility layer types
typedef enum {
    COMPAT_WINDOWS = 0,
    COMPAT_MACOS = 1,
    COMPAT_LINUX = 2
} compatibility_type_t;

typedef enum {
    COMPAT_STATUS_SUCCESS = 0,
    COMPAT_STATUS_NOT_SUPPORTED = -1,
    COMPAT_STATUS_INVALID_BINARY = -2,
    COMPAT_STATUS_MISSING_DEPENDENCY = -3,
    COMPAT_STATUS_PERMISSION_DENIED = -4,
    COMPAT_STATUS_RESOURCE_UNAVAILABLE = -5
} compatibility_status_t;

// Windows compatibility structures
typedef struct {
    u32 major_version;
    u32 minor_version;
    u32 build_number;
    u32 platform_id;
    char service_pack[128];
} windows_version_info_t;

typedef struct {
    void* pe_header;
    void* import_table;
    void* export_table;
    void* resource_table;
    u32 entry_point;
    u32 image_base;
    u32 image_size;
} pe_binary_info_t;

// macOS compatibility structures
typedef struct {
    u32 major_version;
    u32 minor_version;
    u32 patch_version;
    char build_string[64];
} macos_version_info_t;

typedef struct {
    void* mach_header;
    void* load_commands;
    u32 ncmds;
    u32 sizeofcmds;
    u32 flags;
} mach_binary_info_t;

// Compatibility layer configuration
typedef struct {
    compatibility_type_t type;
    bool enable_graphics_acceleration;
    bool enable_audio_support;
    bool enable_network_support;
    bool enable_file_system_access;
    u32 memory_limit_mb;
    u32 cpu_limit_percent;
} compatibility_config_t;

// Core compatibility functions
error_t compatibility_init(void);
void compatibility_shutdown(void);

// Windows compatibility functions
error_t windows_compat_init(const compatibility_config_t* config);
error_t windows_compat_load_binary(const char* path, pe_binary_info_t* info);
error_t windows_compat_execute_binary(pe_binary_info_t* info, int argc, char** argv);
error_t windows_compat_register_syscall(u32 syscall_num, void* handler);
error_t windows_compat_get_version(windows_version_info_t* version);

// macOS compatibility functions
error_t macos_compat_init(const compatibility_config_t* config);
error_t macos_compat_load_binary(const char* path, mach_binary_info_t* info);
error_t macos_compat_execute_binary(mach_binary_info_t* info, int argc, char** argv);
error_t macos_compat_register_syscall(u32 syscall_num, void* handler);
error_t macos_compat_get_version(macos_version_info_t* version);

// Graphics compatibility
error_t compatibility_graphics_init(void);
error_t compatibility_graphics_create_window(const char* title, u32 width, u32 height);
error_t compatibility_graphics_swap_buffers(void);
error_t compatibility_graphics_shutdown(void);

// Audio compatibility
error_t compatibility_audio_init(void);
error_t compatibility_audio_play_buffer(const void* buffer, size_t size);
error_t compatibility_audio_shutdown(void);

// File system compatibility
error_t compatibility_fs_mount_windows_drive(const char* drive_letter, const char* path);
error_t compatibility_fs_mount_macos_volume(const char* volume_name, const char* path);
error_t compatibility_fs_translate_path(const char* foreign_path, char* native_path, size_t size);

// Registry/Preferences compatibility
error_t compatibility_registry_init(void);
error_t compatibility_registry_set_value(const char* key, const char* value, const void* data, size_t size);
error_t compatibility_registry_get_value(const char* key, const char* value, void* data, size_t* size);

// Process isolation and security
typedef struct {
    u32 process_id;
    compatibility_type_t type;
    void* sandbox_context;
    u32 memory_usage;
    u32 cpu_usage;
} compatibility_process_t;

error_t compatibility_process_create(const char* binary_path, compatibility_process_t* process);
error_t compatibility_process_terminate(u32 process_id);
error_t compatibility_process_suspend(u32 process_id);
error_t compatibility_process_resume(u32 process_id);

// Performance monitoring
typedef struct {
    u32 total_processes;
    u32 active_processes;
    u64 total_memory_usage;
    u32 cpu_utilization;
    u32 gpu_utilization;
} compatibility_stats_t;

error_t compatibility_get_stats(compatibility_stats_t* stats);

// Configuration management
error_t compatibility_load_config(const char* config_file);
error_t compatibility_save_config(const char* config_file);
error_t compatibility_set_default_config(compatibility_type_t type);

// Error handling and debugging
typedef void (*compatibility_error_handler_t)(compatibility_status_t status, const char* message);
error_t compatibility_set_error_handler(compatibility_error_handler_t handler);

// Logging and diagnostics
void compatibility_log(const char* format, ...);
void compatibility_debug_dump_process(u32 process_id);
void compatibility_debug_dump_memory_usage(void);

#endif // COMPATIBILITY_H 