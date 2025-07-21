#ifndef SYSTEM_INTEGRATION_H
#define SYSTEM_INTEGRATION_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memory.h>
#include <kernel/process/process.h>
#include <kernel/filesystem/vfs.h>
#include <kernel/network/network.h>
#include <kernel/security/security.h>
#include <kernel/drivers/display/display_system.h>
#include <kernel/drivers/input/input_system.h>
#include <kernel/drivers/audio/audio_system.h>
#include <kernel/network/wifi/wifi_system.h>
#include <kernel/security/authentication_system.h>
#include <kernel/gui/desktop/desktop_environment.h>
#include <kernel/gui/apps/file_manager.h>
#include <kernel/gui/apps/web_browser.h>
#include <kernel/gui/apps/email_client.h>
#include <kernel/gaming/directx_compatibility.h>
#include <kernel/power/power_management.h>
#include <kernel/apps/terminal/terminal_emulator.h>
#include <kernel/boot/boot_system.h>

// System Integration - Complete RaeenOS orchestration
// Integrates all subsystems into a unified, bootable operating system

// System states
typedef enum {
    SYSTEM_STATE_INITIALIZING = 0,   // System initializing
    SYSTEM_STATE_BOOTING,            // System booting
    SYSTEM_STATE_RUNNING,            // System running normally
    SYSTEM_STATE_SLEEPING,           // System sleeping
    SYSTEM_STATE_HIBERNATING,        // System hibernating
    SYSTEM_STATE_SHUTTING_DOWN,      // System shutting down
    SYSTEM_STATE_MAINTENANCE,        // System in maintenance mode
    SYSTEM_STATE_RECOVERY,           // System in recovery mode
    SYSTEM_STATE_EMERGENCY,          // System in emergency mode
    SYSTEM_STATE_OFFLINE,            // System offline
} system_state_t;

// System profiles
typedef enum {
    SYSTEM_PROFILE_DESKTOP = 0,      // Desktop computer profile
    SYSTEM_PROFILE_LAPTOP,           // Laptop computer profile
    SYSTEM_PROFILE_TABLET,           // Tablet device profile
    SYSTEM_PROFILE_SERVER,           // Server profile
    SYSTEM_PROFILE_WORKSTATION,      // Workstation profile
    SYSTEM_PROFILE_GAMING,           // Gaming system profile
    SYSTEM_PROFILE_CREATIVE,         // Creative workstation profile
    SYSTEM_PROFILE_ENTERPRISE,       // Enterprise system profile
    SYSTEM_PROFILE_EMBEDDED,         // Embedded system profile
    SYSTEM_PROFILE_IOT,              // IoT device profile
} system_profile_t;

// System capabilities
typedef struct {
    bool has_gpu;                    // Has dedicated GPU
    bool has_ssd;                    // Has SSD storage
    bool has_nvme;                   // Has NVMe storage
    bool has_wifi;                   // Has WiFi capability
    bool has_bluetooth;              // Has Bluetooth capability
    bool has_touchscreen;            // Has touchscreen
    bool has_biometrics;             // Has biometric sensors
    bool has_multiple_displays;      // Has multiple displays
    bool has_spatial_audio;          // Has spatial audio
    bool has_ai_acceleration;        // Has AI acceleration
    bool has_quantum_computing;      // Has quantum computing
    bool has_blockchain;             // Has blockchain support
    bool has_xr_support;             // Has XR support
    bool has_enterprise_features;    // Has enterprise features
    uint32_t cpu_cores;              // Number of CPU cores
    uint64_t total_memory;           // Total system memory
    uint64_t total_storage;          // Total storage capacity
    uint32_t display_count;          // Number of displays
    uint32_t usb_ports;              // Number of USB ports
    uint32_t network_adapters;       // Number of network adapters
} system_capabilities_t;

// System performance metrics
typedef struct {
    float cpu_usage;                 // CPU usage percentage
    float memory_usage;              // Memory usage percentage
    float disk_usage;                // Disk usage percentage
    float network_usage;             // Network usage percentage
    float gpu_usage;                 // GPU usage percentage
    float temperature;               // System temperature
    float power_consumption;         // Power consumption in watts
    uint32_t active_processes;       // Number of active processes
    uint32_t active_users;           // Number of active users
    uint32_t network_connections;    // Number of network connections
    uint64_t disk_io_read;           // Disk read operations
    uint64_t disk_io_write;          // Disk write operations
    uint64_t network_io_rx;          // Network received bytes
    uint64_t network_io_tx;          // Network transmitted bytes
    uint32_t uptime_seconds;         // System uptime in seconds
    uint32_t boot_time_seconds;      // Boot time in seconds
} system_performance_t;

// System configuration
typedef struct {
    system_profile_t profile;        // System profile
    bool enable_gui;                 // Enable graphical user interface
    bool enable_networking;          // Enable networking
    bool enable_audio;               // Enable audio system
    bool enable_gaming;              // Enable gaming features
    bool enable_ai;                  // Enable AI features
    bool enable_quantum;             // Enable quantum computing
    bool enable_blockchain;          // Enable blockchain features
    bool enable_xr;                  // Enable XR features
    bool enable_enterprise;          // Enable enterprise features
    bool enable_security;            // Enable security features
    bool enable_power_management;    // Enable power management
    bool enable_remote_access;       // Enable remote access
    bool enable_backup;              // Enable backup services
    bool enable_monitoring;          // Enable system monitoring
    uint32_t max_users;              // Maximum number of users
    uint32_t max_processes;          // Maximum number of processes
    uint32_t max_connections;        // Maximum network connections
    uint32_t session_timeout;        // Session timeout in minutes
    uint32_t auto_save_interval;     // Auto-save interval in minutes
} system_config_t;

// System integration context
typedef struct {
    system_config_t config;
    system_state_t state;
    system_capabilities_t capabilities;
    system_performance_t performance;
    
    // Core subsystems
    memory_system_t* memory_system;
    process_system_t* process_system;
    vfs_system_t* vfs_system;
    network_system_t* network_system;
    security_system_t* security_system;
    
    // Hardware subsystems
    display_system_t* display_system;
    input_system_t* input_system;
    audio_system_t* audio_system;
    wifi_system_t* wifi_system;
    
    // User subsystems
    auth_system_t* auth_system;
    desktop_environment_t* desktop_environment;
    file_manager_t* file_manager;
    web_browser_t* web_browser;
    email_client_t* email_client;
    terminal_emulator_t* terminal_emulator;
    
    // Advanced subsystems
    directx_compatibility_t* directx_system;
    power_management_t* power_system;
    boot_system_t* boot_system;
    
    // System management
    bool initialized;
    bool running;
    uint64_t start_time;
    uint64_t last_update_time;
    uint32_t update_interval;
    
    // Error handling
    uint32_t error_count;
    char last_error[256];
    bool critical_error;
    
    // Performance monitoring
    uint64_t performance_update_time;
    uint32_t performance_update_interval;
    
    // System events
    uint32_t event_count;
    uint32_t max_events;
    void** event_handlers;
    
} system_integration_t;

// Function prototypes

// Initialization and shutdown
system_integration_t* system_integration_init(system_config_t* config);
void system_integration_shutdown(system_integration_t* system);
bool system_integration_is_initialized(system_integration_t* system);

// System lifecycle
bool system_integration_start(system_integration_t* system);
void system_integration_stop(system_integration_t* system);
bool system_integration_restart(system_integration_t* system);
bool system_integration_sleep(system_integration_t* system);
bool system_integration_hibernate(system_integration_t* system);
bool system_integration_wake(system_integration_t* system);
void system_integration_shutdown_system(system_integration_t* system);
bool system_integration_is_running(system_integration_t* system);

// System state management
system_state_t system_integration_get_state(system_integration_t* system);
void system_integration_set_state(system_integration_t* system, system_state_t state);
bool system_integration_transition_state(system_integration_t* system, system_state_t new_state);
const char* system_integration_get_state_name(system_state_t state);

// System profile management
system_profile_t system_integration_get_profile(system_integration_t* system);
void system_integration_set_profile(system_integration_t* system, system_profile_t profile);
bool system_integration_optimize_for_profile(system_integration_t* system, system_profile_t profile);
const char* system_integration_get_profile_name(system_profile_t profile);

// System capabilities
system_capabilities_t* system_integration_get_capabilities(system_integration_t* system);
bool system_integration_detect_capabilities(system_integration_t* system);
bool system_integration_has_capability(system_integration_t* system, uint32_t capability);
void system_integration_update_capabilities(system_integration_t* system);

// System performance
system_performance_t* system_integration_get_performance(system_integration_t* system);
void system_integration_update_performance(system_integration_t* system);
bool system_integration_monitor_performance(system_integration_t* system, bool enable);
void system_integration_set_performance_update_interval(system_integration_t* system, uint32_t interval);
uint32_t system_integration_get_performance_update_interval(system_integration_t* system);

// Subsystem management
memory_system_t* system_integration_get_memory_system(system_integration_t* system);
process_system_t* system_integration_get_process_system(system_integration_t* system);
vfs_system_t* system_integration_get_vfs_system(system_integration_t* system);
network_system_t* system_integration_get_network_system(system_integration_t* system);
security_system_t* system_integration_get_security_system(system_integration_t* system);
display_system_t* system_integration_get_display_system(system_integration_t* system);
input_system_t* system_integration_get_input_system(system_integration_t* system);
audio_system_t* system_integration_get_audio_system(system_integration_t* system);
wifi_system_t* system_integration_get_wifi_system(system_integration_t* system);
auth_system_t* system_integration_get_auth_system(system_integration_t* system);
desktop_environment_t* system_integration_get_desktop_environment(system_integration_t* system);
file_manager_t* system_integration_get_file_manager(system_integration_t* system);
web_browser_t* system_integration_get_web_browser(system_integration_t* system);
email_client_t* system_integration_get_email_client(system_integration_t* system);
terminal_emulator_t* system_integration_get_terminal_emulator(system_integration_t* system);
directx_compatibility_t* system_integration_get_directx_system(system_integration_t* system);
power_management_t* system_integration_get_power_system(system_integration_t* system);
boot_system_t* system_integration_get_boot_system(system_integration_t* system);

// System configuration
void system_integration_set_config(system_integration_t* system, system_config_t* config);
system_config_t* system_integration_get_config(system_integration_t* system);
bool system_integration_load_config(system_integration_t* system, const char* config_path);
bool system_integration_save_config(system_integration_t* system, const char* config_path);
bool system_integration_validate_config(system_integration_t* system, system_config_t* config);

// System monitoring
void system_integration_start_monitoring(system_integration_t* system);
void system_integration_stop_monitoring(system_integration_t* system);
bool system_integration_is_monitoring(system_integration_t* system);
void system_integration_set_monitoring_interval(system_integration_t* system, uint32_t interval);
uint32_t system_integration_get_monitoring_interval(system_integration_t* system);

// System diagnostics
bool system_integration_run_diagnostics(system_integration_t* system);
bool system_integration_check_system_health(system_integration_t* system);
bool system_integration_repair_system(system_integration_t* system);
bool system_integration_optimize_system(system_integration_t* system);
char* system_integration_get_diagnostic_report(system_integration_t* system);

// System backup and recovery
bool system_integration_create_backup(system_integration_t* system, const char* backup_path);
bool system_integration_restore_backup(system_integration_t* system, const char* backup_path);
bool system_integration_create_restore_point(system_integration_t* system, const char* description);
bool system_integration_restore_to_point(system_integration_t* system, uint32_t restore_point_id);
uint32_t system_integration_get_restore_points(system_integration_t* system, uint32_t* count);

// System updates
bool system_integration_check_for_updates(system_integration_t* system);
bool system_integration_install_updates(system_integration_t* system);
bool system_integration_rollback_updates(system_integration_t* system);
uint32_t system_integration_get_available_updates(system_integration_t* system, uint32_t* count);
bool system_integration_is_update_available(system_integration_t* system);

// System security
bool system_integration_enable_security(system_integration_t* system, bool enable);
bool system_integration_is_security_enabled(system_integration_t* system);
bool system_integration_run_security_scan(system_integration_t* system);
bool system_integration_update_security_definitions(system_integration_t* system);
bool system_integration_quarantine_threat(system_integration_t* system, const char* threat_path);

// System networking
bool system_integration_enable_networking(system_integration_t* system, bool enable);
bool system_integration_is_networking_enabled(system_integration_t* system);
bool system_integration_connect_to_network(system_integration_t* system, const char* network_name, const char* password);
bool system_integration_disconnect_from_network(system_integration_t* system);
bool system_integration_is_network_connected(system_integration_t* system);

// System power management
bool system_integration_enable_power_management(system_integration_t* system, bool enable);
bool system_integration_is_power_management_enabled(system_integration_t* system);
bool system_integration_set_power_plan(system_integration_t* system, const char* plan_name);
const char* system_integration_get_current_power_plan(system_integration_t* system);
bool system_integration_optimize_power_usage(system_integration_t* system);

// System user management
bool system_integration_create_user(system_integration_t* system, const char* username, const char* password);
bool system_integration_delete_user(system_integration_t* system, const char* username);
bool system_integration_login_user(system_integration_t* system, const char* username, const char* password);
bool system_integration_logout_user(system_integration_t* system);
const char* system_integration_get_current_user(system_integration_t* system);

// System application management
bool system_integration_install_application(system_integration_t* system, const char* app_path);
bool system_integration_uninstall_application(system_integration_t* system, const char* app_name);
bool system_integration_update_application(system_integration_t* system, const char* app_name);
bool system_integration_launch_application(system_integration_t* system, const char* app_name);
bool system_integration_is_application_installed(system_integration_t* system, const char* app_name);

// System information
uint64_t system_integration_get_uptime(system_integration_t* system);
uint64_t system_integration_get_boot_time(system_integration_t* system);
uint32_t system_integration_get_active_process_count(system_integration_t* system);
uint32_t system_integration_get_active_user_count(system_integration_t* system);
uint32_t system_integration_get_installed_application_count(system_integration_t* system);
const char* system_integration_get_system_version(system_integration_t* system);
const char* system_integration_get_kernel_version(system_integration_t* system);

// System events
bool system_integration_register_event_handler(system_integration_t* system, uint32_t event_type, void* handler);
bool system_integration_unregister_event_handler(system_integration_t* system, uint32_t event_type);
bool system_integration_trigger_event(system_integration_t* system, uint32_t event_type, void* event_data);
uint32_t system_integration_get_event_count(system_integration_t* system);

// System error handling
uint32_t system_integration_get_error_count(system_integration_t* system);
const char* system_integration_get_last_error(system_integration_t* system);
bool system_integration_has_critical_error(system_integration_t* system);
void system_integration_clear_errors(system_integration_t* system);
bool system_integration_log_error(system_integration_t* system, const char* error_message);

// System maintenance
bool system_integration_run_maintenance(system_integration_t* system);
bool system_integration_cleanup_temp_files(system_integration_t* system);
bool system_integration_defragment_disk(system_integration_t* system);
bool system_integration_optimize_memory(system_integration_t* system);
bool system_integration_clear_cache(system_integration_t* system);

// Utility functions
char* system_integration_get_system_info(system_integration_t* system);
char* system_integration_get_hardware_info(system_integration_t* system);
char* system_integration_get_software_info(system_integration_t* system);
bool system_integration_validate_system(system_integration_t* system);
bool system_integration_test_system(system_integration_t* system);

// Callbacks
typedef void (*system_state_change_callback_t)(system_integration_t* system, system_state_t old_state, system_state_t new_state, void* user_data);
typedef void (*system_performance_callback_t)(system_integration_t* system, system_performance_t* performance, void* user_data);
typedef void (*system_error_callback_t)(system_integration_t* system, const char* error, void* user_data);

void system_integration_set_state_change_callback(system_integration_t* system, system_state_change_callback_t callback, void* user_data);
void system_integration_set_performance_callback(system_integration_t* system, system_performance_callback_t callback, void* user_data);
void system_integration_set_error_callback(system_integration_t* system, system_error_callback_t callback, void* user_data);

// Preset configurations
system_config_t system_integration_preset_desktop_style(void);
system_config_t system_integration_preset_laptop_style(void);
system_config_t system_integration_preset_server_style(void);
system_config_t system_integration_preset_gaming_style(void);
system_config_t system_integration_preset_enterprise_style(void);

// Error handling
typedef enum {
    SYSTEM_INTEGRATION_SUCCESS = 0,
    SYSTEM_INTEGRATION_ERROR_INVALID_CONTEXT,
    SYSTEM_INTEGRATION_ERROR_INITIALIZATION_FAILED,
    SYSTEM_INTEGRATION_ERROR_SUBSYSTEM_FAILED,
    SYSTEM_INTEGRATION_ERROR_OUT_OF_MEMORY,
    SYSTEM_INTEGRATION_ERROR_INVALID_CONFIG,
    SYSTEM_INTEGRATION_ERROR_STATE_TRANSITION_FAILED,
    SYSTEM_INTEGRATION_ERROR_PERMISSION_DENIED,
    SYSTEM_INTEGRATION_ERROR_RESOURCE_UNAVAILABLE,
    SYSTEM_INTEGRATION_ERROR_TIMEOUT,
} system_integration_error_t;

system_integration_error_t system_integration_get_last_error(void);
const char* system_integration_get_error_string(system_integration_error_t error);

#endif // SYSTEM_INTEGRATION_H 