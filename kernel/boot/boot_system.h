#ifndef BOOT_SYSTEM_H
#define BOOT_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memory.h>

// Boot System - Advanced boot management for RaeenOS
// Provides multi-boot support, boot configuration, and recovery options

// Boot modes
typedef enum {
    BOOT_MODE_NORMAL = 0,            // Normal boot
    BOOT_MODE_SAFE,                  // Safe mode
    BOOT_MODE_RECOVERY,              // Recovery mode
    BOOT_MODE_DEBUG,                 // Debug mode
    BOOT_MODE_SINGLE_USER,           // Single user mode
    BOOT_MODE_MAINTENANCE,           // Maintenance mode
    BOOT_MODE_EMERGENCY,             // Emergency mode
    BOOT_MODE_NETWORK,               // Network boot
    BOOT_MODE_LIVE,                  // Live boot
    BOOT_MODE_INSTALL,               // Install mode
} boot_mode_t;

// Boot loaders
typedef enum {
    BOOT_LOADER_GRUB = 0,            // GRUB bootloader
    BOOT_LOADER_SYSTEMD_BOOT,        // systemd-boot
    BOOT_LOADER_RUFUS,               // Rufus bootloader
    BOOT_LOADER_CUSTOM,              // Custom bootloader
    BOOT_LOADER_BIOS,                // Legacy BIOS boot
    BOOT_LOADER_UEFI,                // UEFI boot
} boot_loader_t;

// Boot devices
typedef enum {
    BOOT_DEVICE_HDD = 0,             // Hard disk drive
    BOOT_DEVICE_SSD,                 // Solid state drive
    BOOT_DEVICE_USB,                 // USB device
    BOOT_DEVICE_CD_DVD,              // CD/DVD drive
    BOOT_DEVICE_NETWORK,             // Network boot
    BOOT_DEVICE_SD_CARD,             // SD card
    BOOT_DEVICE_NVME,                // NVMe drive
    BOOT_DEVICE_VIRTUAL,             // Virtual disk
} boot_device_t;

// Boot configuration
typedef struct {
    uint32_t id;
    char name[64];                   // Configuration name
    char description[256];           // Configuration description
    boot_mode_t mode;                // Boot mode
    boot_loader_t loader;            // Boot loader
    boot_device_t device;            // Boot device
    char kernel_path[256];           // Kernel path
    char initrd_path[256];           // Initrd path
    char cmdline[512];               // Kernel command line
    uint32_t timeout;                // Boot timeout in seconds
    bool is_default;                 // Is default configuration
    bool is_active;                  // Is active configuration
    bool is_visible;                 // Is visible in boot menu
    uint32_t priority;               // Boot priority
    void* config_data;               // Configuration-specific data
} boot_config_t;

// Boot entry
typedef struct {
    uint32_t id;
    char title[64];                  // Boot entry title
    char description[256];           // Boot entry description
    boot_config_t* config;           // Associated configuration
    uint32_t boot_count;             // Number of boots
    uint64_t last_boot_time;         // Last boot time
    bool is_successful;              // Was last boot successful
    uint32_t failure_count;          // Number of failures
    uint64_t created_time;           // Creation time
    bool is_editable;                // Is entry editable
    void* entry_data;                // Entry-specific data
} boot_entry_t;

// Boot environment
typedef struct {
    char hostname[64];               // System hostname
    char domain[64];                 // Domain name
    char locale[16];                 // System locale
    char timezone[64];               // Timezone
    char keyboard_layout[32];        // Keyboard layout
    char display_resolution[16];     // Display resolution
    bool enable_networking;          // Enable networking
    bool enable_graphics;            // Enable graphics
    bool enable_sound;               // Enable sound
    bool enable_debugging;           // Enable debugging
    uint32_t memory_limit;           // Memory limit in MB
    uint32_t cpu_count;              // CPU count limit
    char* environment_vars;          // Environment variables
    uint32_t var_count;              // Number of variables
} boot_environment_t;

// Boot system configuration
typedef struct {
    bool enable_multi_boot;          // Enable multi-boot
    bool enable_boot_menu;           // Enable boot menu
    bool enable_boot_splash;         // Enable boot splash
    bool enable_boot_animation;      // Enable boot animation
    bool enable_boot_sound;          // Enable boot sound
    bool enable_secure_boot;         // Enable secure boot
    bool enable_fast_boot;           // Enable fast boot
    bool enable_quiet_boot;          // Enable quiet boot
    bool enable_verbose_boot;        // Enable verbose boot
    uint32_t default_timeout;        // Default timeout
    uint32_t max_boot_entries;       // Maximum boot entries
    uint32_t max_boot_configs;       // Maximum boot configurations
    char default_config[64];         // Default configuration
    char fallback_config[64];        // Fallback configuration
    char recovery_config[64];        // Recovery configuration
} boot_system_config_t;

// Boot system context
typedef struct {
    boot_system_config_t config;
    boot_config_t* configurations;
    uint32_t config_count;
    uint32_t max_configs;
    boot_entry_t* entries;
    uint32_t entry_count;
    uint32_t max_entries;
    boot_config_t* active_config;
    boot_entry_t* active_entry;
    boot_environment_t environment;
    bool initialized;
    bool booting;
    boot_mode_t current_mode;
    uint32_t next_config_id;
    uint32_t next_entry_id;
    uint64_t boot_start_time;
} boot_system_t;

// Function prototypes

// Initialization and shutdown
boot_system_t* boot_system_init(boot_system_config_t* config);
void boot_system_shutdown(boot_system_t* boot);
bool boot_system_is_initialized(boot_system_t* boot);

// Boot configuration management
uint32_t boot_system_add_config(boot_system_t* boot, boot_config_t* config);
void boot_system_remove_config(boot_system_t* boot, uint32_t config_id);
boot_config_t* boot_system_get_config(boot_system_t* boot, uint32_t config_id);
boot_config_t* boot_system_get_configs(boot_system_t* boot, uint32_t* count);
uint32_t boot_system_get_config_count(boot_system_t* boot);
void boot_system_set_default_config(boot_system_t* boot, uint32_t config_id);
boot_config_t* boot_system_get_default_config(boot_system_t* boot);
void boot_system_set_active_config(boot_system_t* boot, uint32_t config_id);
boot_config_t* boot_system_get_active_config(boot_system_t* boot);

// Boot entry management
uint32_t boot_system_add_entry(boot_system_t* boot, boot_entry_t* entry);
void boot_system_remove_entry(boot_system_t* boot, uint32_t entry_id);
boot_entry_t* boot_system_get_entry(boot_system_t* boot, uint32_t entry_id);
boot_entry_t* boot_system_get_entries(boot_system_t* boot, uint32_t* count);
uint32_t boot_system_get_entry_count(boot_system_t* boot);
void boot_system_set_default_entry(boot_system_t* boot, uint32_t entry_id);
boot_entry_t* boot_system_get_default_entry(boot_system_t* boot);
void boot_system_set_active_entry(boot_system_t* boot, uint32_t entry_id);
boot_entry_t* boot_system_get_active_entry(boot_system_t* boot);

// Boot process
bool boot_system_start_boot(boot_system_t* boot, uint32_t entry_id);
bool boot_system_start_boot_with_config(boot_system_t* boot, uint32_t config_id);
bool boot_system_start_boot_with_mode(boot_system_t* boot, boot_mode_t mode);
bool boot_system_start_boot_with_device(boot_system_t* boot, boot_device_t device);
bool boot_system_start_boot_with_kernel(boot_system_t* boot, const char* kernel_path);
void boot_system_stop_boot(boot_system_t* boot);
bool boot_system_is_booting(boot_system_t* boot);
boot_mode_t boot_system_get_current_mode(boot_system_t* boot);

// Boot menu
void boot_system_show_boot_menu(boot_system_t* boot);
void boot_system_hide_boot_menu(boot_system_t* boot);
bool boot_system_is_boot_menu_visible(boot_system_t* boot);
void boot_system_set_boot_menu_timeout(boot_system_t* boot, uint32_t timeout);
uint32_t boot_system_get_boot_menu_timeout(boot_system_t* boot);
void boot_system_enable_boot_menu(boot_system_t* boot, bool enable);
bool boot_system_is_boot_menu_enabled(boot_system_t* boot);

// Boot environment
void boot_system_set_environment(boot_system_t* boot, boot_environment_t* environment);
boot_environment_t* boot_system_get_environment(boot_system_t* boot);
void boot_system_set_hostname(boot_system_t* boot, const char* hostname);
const char* boot_system_get_hostname(boot_system_t* boot);
void boot_system_set_domain(boot_system_t* boot, const char* domain);
const char* boot_system_get_domain(boot_system_t* boot);
void boot_system_set_locale(boot_system_t* boot, const char* locale);
const char* boot_system_get_locale(boot_system_t* boot);
void boot_system_set_timezone(boot_system_t* boot, const char* timezone);
const char* boot_system_get_timezone(boot_system_t* boot);
void boot_system_set_keyboard_layout(boot_system_t* boot, const char* layout);
const char* boot_system_get_keyboard_layout(boot_system_t* boot);
void boot_system_set_display_resolution(boot_system_t* boot, const char* resolution);
const char* boot_system_get_display_resolution(boot_system_t* boot);

// Boot options
void boot_system_enable_networking(boot_system_t* boot, bool enable);
bool boot_system_is_networking_enabled(boot_system_t* boot);
void boot_system_enable_graphics(boot_system_t* boot, bool enable);
bool boot_system_is_graphics_enabled(boot_system_t* boot);
void boot_system_enable_sound(boot_system_t* boot, bool enable);
bool boot_system_is_sound_enabled(boot_system_t* boot);
void boot_system_enable_debugging(boot_system_t* boot, bool enable);
bool boot_system_is_debugging_enabled(boot_system_t* boot);
void boot_system_set_memory_limit(boot_system_t* boot, uint32_t limit);
uint32_t boot_system_get_memory_limit(boot_system_t* boot);
void boot_system_set_cpu_count(boot_system_t* boot, uint32_t count);
uint32_t boot_system_get_cpu_count(boot_system_t* boot);

// Boot modes
bool boot_system_boot_normal(boot_system_t* boot);
bool boot_system_boot_safe(boot_system_t* boot);
bool boot_system_boot_recovery(boot_system_t* boot);
bool boot_system_boot_debug(boot_system_t* boot);
bool boot_system_boot_single_user(boot_system_t* boot);
bool boot_system_boot_maintenance(boot_system_t* boot);
bool boot_system_boot_emergency(boot_system_t* boot);
bool boot_system_boot_network(boot_system_t* boot);
bool boot_system_boot_live(boot_system_t* boot);
bool boot_system_boot_install(boot_system_t* boot);

// Boot devices
bool boot_system_boot_from_hdd(boot_system_t* boot);
bool boot_system_boot_from_ssd(boot_system_t* boot);
bool boot_system_boot_from_usb(boot_system_t* boot);
bool boot_system_boot_from_cd_dvd(boot_system_t* boot);
bool boot_system_boot_from_network(boot_system_t* boot);
bool boot_system_boot_from_sd_card(boot_system_t* boot);
bool boot_system_boot_from_nvme(boot_system_t* boot);
bool boot_system_boot_from_virtual(boot_system_t* boot);

// Boot loaders
bool boot_system_use_grub(boot_system_t* boot);
bool boot_system_use_systemd_boot(boot_system_t* boot);
bool boot_system_use_rufus(boot_system_t* boot);
bool boot_system_use_custom_loader(boot_system_t* boot, const char* loader_path);
bool boot_system_use_bios(boot_system_t* boot);
bool boot_system_use_uefi(boot_system_t* boot);

// Boot recovery
bool boot_system_boot_last_known_good(boot_system_t* boot);
bool boot_system_boot_safe_mode(boot_system_t* boot);
bool boot_system_boot_recovery_mode(boot_system_t* boot);
bool boot_system_boot_emergency_mode(boot_system_t* boot);
bool boot_system_repair_boot_config(boot_system_t* boot);
bool boot_system_reset_boot_config(boot_system_t* boot);
bool boot_system_backup_boot_config(boot_system_t* boot, const char* backup_path);
bool boot_system_restore_boot_config(boot_system_t* boot, const char* backup_path);

// Boot monitoring
uint64_t boot_system_get_boot_time(boot_system_t* boot);
uint64_t boot_system_get_boot_duration(boot_system_t* boot);
bool boot_system_is_boot_successful(boot_system_t* boot);
void boot_system_mark_boot_successful(boot_system_t* boot, bool successful);
uint32_t boot_system_get_boot_count(boot_system_t* boot, uint32_t entry_id);
uint32_t boot_system_get_failure_count(boot_system_t* boot, uint32_t entry_id);
void boot_system_reset_failure_count(boot_system_t* boot, uint32_t entry_id);

// Boot splash and animation
void boot_system_show_boot_splash(boot_system_t* boot);
void boot_system_hide_boot_splash(boot_system_t* boot);
bool boot_system_is_boot_splash_visible(boot_system_t* boot);
void boot_system_set_boot_splash_image(boot_system_t* boot, const char* image_path);
const char* boot_system_get_boot_splash_image(boot_system_t* boot);
void boot_system_start_boot_animation(boot_system_t* boot);
void boot_system_stop_boot_animation(boot_system_t* boot);
bool boot_system_is_boot_animation_running(boot_system_t* boot);

// Boot sound
void boot_system_play_boot_sound(boot_system_t* boot);
void boot_system_stop_boot_sound(boot_system_t* boot);
bool boot_system_is_boot_sound_playing(boot_system_t* boot);
void boot_system_set_boot_sound_file(boot_system_t* boot, const char* sound_path);
const char* boot_system_get_boot_sound_file(boot_system_t* boot);
void boot_system_enable_boot_sound(boot_system_t* boot, bool enable);
bool boot_system_is_boot_sound_enabled(boot_system_t* boot);

// Boot security
void boot_system_enable_secure_boot(boot_system_t* boot, bool enable);
bool boot_system_is_secure_boot_enabled(boot_system_t* boot);
bool boot_system_verify_boot_signature(boot_system_t* boot, const char* file_path);
bool boot_system_sign_boot_file(boot_system_t* boot, const char* file_path, const char* key_path);
bool boot_system_verify_boot_integrity(boot_system_t* boot);

// Boot performance
void boot_system_enable_fast_boot(boot_system_t* boot, bool enable);
bool boot_system_is_fast_boot_enabled(boot_system_t* boot);
void boot_system_enable_quiet_boot(boot_system_t* boot, bool enable);
bool boot_system_is_quiet_boot_enabled(boot_system_t* boot);
void boot_system_enable_verbose_boot(boot_system_t* boot, bool enable);
bool boot_system_is_verbose_boot_enabled(boot_system_t* boot);
void boot_system_optimize_boot_performance(boot_system_t* boot);

// Information
uint32_t boot_system_get_total_boot_time(boot_system_t* boot);
uint32_t boot_system_get_average_boot_time(boot_system_t* boot);
uint32_t boot_system_get_fastest_boot_time(boot_system_t* boot);
uint32_t boot_system_get_slowest_boot_time(boot_system_t* boot);
bool boot_system_is_multi_boot_enabled(boot_system_t* boot);
uint32_t boot_system_get_available_boot_entries(boot_system_t* boot);
bool boot_system_has_boot_errors(boot_system_t* boot);

// Utility functions
char* boot_system_get_mode_name(boot_mode_t mode);
char* boot_system_get_loader_name(boot_loader_t loader);
char* boot_system_get_device_name(boot_device_t device);
bool boot_system_is_mode_supported(boot_mode_t mode);
bool boot_system_is_loader_supported(boot_loader_t loader);
bool boot_system_is_device_supported(boot_device_t device);
uint32_t boot_system_calculate_boot_time(boot_system_t* boot);
bool boot_system_validate_boot_config(boot_system_t* boot, boot_config_t* config);

// Callbacks
typedef void (*boot_progress_callback_t)(boot_system_t* boot, uint32_t progress, const char* stage, void* user_data);
typedef void (*boot_complete_callback_t)(boot_system_t* boot, bool success, void* user_data);
typedef void (*boot_error_callback_t)(boot_system_t* boot, const char* error, void* user_data);

void boot_system_set_progress_callback(boot_system_t* boot, boot_progress_callback_t callback, void* user_data);
void boot_system_set_complete_callback(boot_system_t* boot, boot_complete_callback_t callback, void* user_data);
void boot_system_set_error_callback(boot_system_t* boot, boot_error_callback_t callback, void* user_data);

// Preset configurations
boot_system_config_t boot_system_preset_normal_style(void);
boot_system_config_t boot_system_preset_secure_style(void);
boot_system_config_t boot_system_preset_fast_style(void);
boot_system_config_t boot_system_preset_debug_style(void);

// Error handling
typedef enum {
    BOOT_SYSTEM_SUCCESS = 0,
    BOOT_SYSTEM_ERROR_INVALID_CONTEXT,
    BOOT_SYSTEM_ERROR_INVALID_CONFIG,
    BOOT_SYSTEM_ERROR_INVALID_ENTRY,
    BOOT_SYSTEM_ERROR_BOOT_FAILED,
    BOOT_SYSTEM_ERROR_DEVICE_NOT_FOUND,
    BOOT_SYSTEM_ERROR_KERNEL_NOT_FOUND,
    BOOT_SYSTEM_ERROR_OUT_OF_MEMORY,
    BOOT_SYSTEM_ERROR_TIMEOUT,
    BOOT_SYSTEM_ERROR_SECURITY_VIOLATION,
} boot_system_error_t;

boot_system_error_t boot_system_get_last_error(void);
const char* boot_system_get_error_string(boot_system_error_t error);

#endif // BOOT_SYSTEM_H 