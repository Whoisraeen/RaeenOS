#ifndef BUILD_SYSTEM_H
#define BUILD_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

// Build System - Comprehensive build orchestration for RaeenOS
// Coordinates all components and features into a complete OS

// Build targets
typedef enum {
    BUILD_TARGET_KERNEL = 0,     // Kernel only
    BUILD_TARGET_BOOTLOADER,     // Bootloader only
    BUILD_TARGET_DRIVERS,        // Drivers only
    BUILD_TARGET_GUI,            // GUI system only
    BUILD_TARGET_APPS,           // Applications only
    BUILD_TARGET_AUDIO,          // Audio system only
    BUILD_TARGET_NETWORK,        // Network stack only
    BUILD_TARGET_GAMING,         // Gaming features only
    BUILD_TARGET_AI,             // AI system only
    BUILD_TARGET_QUANTUM,        // Quantum computing only
    BUILD_TARGET_BLOCKCHAIN,     // Blockchain system only
    BUILD_TARGET_XR,             // XR system only
    BUILD_TARGET_ENTERPRISE,     // Enterprise features only
    BUILD_TARGET_FULL,           // Complete OS
    BUILD_TARGET_ISO,            // Bootable ISO
    BUILD_TARGET_USB,            // USB bootable
    BUILD_TARGET_VM,             // Virtual machine image
} build_target_t;

// Build configurations
typedef enum {
    BUILD_CONFIG_DEBUG = 0,      // Debug build
    BUILD_CONFIG_RELEASE,        // Release build
    BUILD_CONFIG_PROFILE,        // Profiling build
    BUILD_CONFIG_MINIMAL,        // Minimal build
    BUILD_CONFIG_FULL,           // Full feature build
    BUILD_CONFIG_GAMING,         // Gaming optimized
    BUILD_CONFIG_ENTERPRISE,     // Enterprise optimized
    BUILD_CONFIG_DEVELOPMENT,    // Development build
} build_config_t;

// Build architectures
typedef enum {
    BUILD_ARCH_X86_64 = 0,       // x86-64 architecture
    BUILD_ARCH_X86,              // x86 architecture
    BUILD_ARCH_ARM64,            // ARM64 architecture
    BUILD_ARCH_ARM,              // ARM architecture
    BUILD_ARCH_RISCV64,          // RISC-V 64-bit
    BUILD_ARCH_RISCV32,          // RISC-V 32-bit
} build_arch_t;

// Build components
typedef struct {
    bool kernel;                 // Kernel component
    bool bootloader;             // Bootloader component
    bool drivers;                // Driver components
    bool gui;                    // GUI system
    bool desktop;                // Desktop environment
    bool file_manager;           // File manager
    bool web_browser;            // Web browser
    bool email_client;           // Email client
    bool audio_system;           // Audio system
    bool wifi_system;            // WiFi system
    bool power_management;       // Power management
    bool directx_compatibility;  // DirectX compatibility
    bool ai_system;              // AI system
    bool quantum_framework;      // Quantum computing
    bool blockchain_framework;   // Blockchain system
    bool xr_framework;           // XR system
    bool enterprise_features;    // Enterprise features
    bool security_system;        // Security system
    bool networking;             // Networking stack
    bool storage_system;         // Storage system
    bool virtualization;         // Virtualization
    bool gaming_system;          // Gaming system
    bool creative_suite;         // Creative suite
    bool accessibility;          // Accessibility features
    bool cloud_integration;      // Cloud integration
    bool device_integration;     // Device integration
} build_components_t;

// Build dependencies
typedef struct {
    char name[64];               // Dependency name
    char version[32];            // Version requirement
    char path[256];              // Path to dependency
    bool required;               // Is required
    bool found;                  // Is found
    bool built;                  // Is built
} build_dependency_t;

// Build configuration
typedef struct {
    build_target_t target;       // Build target
    build_config_t config;       // Build configuration
    build_arch_t architecture;   // Target architecture
    build_components_t components; // Components to build
    char output_path[256];       // Output path
    char source_path[256];       // Source path
    char toolchain_path[256];    // Toolchain path
    char kernel_path[256];       // Kernel path
    char bootloader_path[256];   // Bootloader path
    char drivers_path[256];      // Drivers path
    char gui_path[256];          // GUI path
    char apps_path[256];         // Applications path
    char audio_path[256];        // Audio system path
    char network_path[256];      // Network path
    char gaming_path[256];       // Gaming path
    char ai_path[256];           // AI system path
    char quantum_path[256];      // Quantum framework path
    char blockchain_path[256];   // Blockchain path
    char xr_path[256];           // XR framework path
    char enterprise_path[256];   // Enterprise features path
    bool enable_optimization;    // Enable optimization
    bool enable_debugging;       // Enable debugging
    bool enable_profiling;       // Enable profiling
    bool enable_testing;         // Enable testing
    bool enable_documentation;   // Enable documentation
    bool enable_clean_build;     // Clean build
    bool enable_parallel_build;  // Parallel build
    uint32_t max_jobs;           // Maximum parallel jobs
    uint32_t verbosity_level;    // Verbosity level
} build_configuration_t;

// Build progress
typedef struct {
    uint32_t total_components;   // Total components
    uint32_t completed_components; // Completed components
    uint32_t current_component;  // Current component
    char current_component_name[64]; // Current component name
    float progress_percentage;   // Progress percentage
    uint64_t start_time;         // Build start time
    uint64_t estimated_time;     // Estimated completion time
    bool is_building;            // Is currently building
    bool has_errors;             // Has build errors
    bool has_warnings;           // Has build warnings
    uint32_t error_count;        // Error count
    uint32_t warning_count;      // Warning count
} build_progress_t;

// Build result
typedef struct {
    bool success;                // Build success
    uint32_t error_count;        // Error count
    uint32_t warning_count;      // Warning count
    char* errors;                // Error messages
    char* warnings;              // Warning messages
    char output_file[256];       // Output file path
    uint64_t build_time;         // Build time in seconds
    uint64_t output_size;        // Output file size
    build_progress_t progress;   // Build progress
} build_result_t;

// Build system context
typedef struct {
    build_configuration_t config;
    build_progress_t progress;
    build_dependency_t* dependencies;
    uint32_t dependency_count;
    uint32_t max_dependencies;
    build_result_t last_result;
    bool initialized;
    char build_log_path[256];
    char error_log_path[256];
    char warning_log_path[256];
} build_system_t;

// Function prototypes

// Initialization and shutdown
build_system_t* build_system_init(build_configuration_t* config);
void build_system_shutdown(build_system_t* bs);
bool build_system_is_initialized(build_system_t* bs);

// Build execution
build_result_t build_system_build(build_system_t* bs);
build_result_t build_system_build_target(build_system_t* bs, build_target_t target);
build_result_t build_system_build_component(build_system_t* bs, const char* component_name);
build_result_t build_system_clean(build_system_t* bs);
build_result_t build_system_clean_target(build_system_t* bs, build_target_t target);
build_result_t build_system_clean_component(build_system_t* bs, const char* component_name);

// Build management
void build_system_set_target(build_system_t* bs, build_target_t target);
build_target_t build_system_get_target(build_system_t* bs);
void build_system_set_config(build_system_t* bs, build_config_t config);
build_config_t build_system_get_config(build_system_t* bs);
void build_system_set_architecture(build_system_t* bs, build_arch_t architecture);
build_arch_t build_system_get_architecture(build_system_t* bs);

// Component management
void build_system_enable_component(build_system_t* bs, const char* component_name, bool enable);
bool build_system_is_component_enabled(build_system_t* bs, const char* component_name);
void build_system_set_components(build_system_t* bs, build_components_t* components);
build_components_t* build_system_get_components(build_system_t* bs);
uint32_t build_system_get_component_count(build_system_t* bs);
char** build_system_get_enabled_components(build_system_t* bs, uint32_t* count);

// Dependency management
uint32_t build_system_add_dependency(build_system_t* bs, build_dependency_t* dependency);
void build_system_remove_dependency(build_system_t* bs, uint32_t dependency_id);
build_dependency_t* build_system_get_dependency(build_system_t* bs, uint32_t dependency_id);
build_dependency_t* build_system_get_dependencies(build_system_t* bs, uint32_t* count);
bool build_system_check_dependencies(build_system_t* bs);
bool build_system_resolve_dependencies(build_system_t* bs);
bool build_system_build_dependencies(build_system_t* bs);

// Configuration management
void build_system_set_output_path(build_system_t* bs, const char* path);
const char* build_system_get_output_path(build_system_t* bs);
void build_system_set_source_path(build_system_t* bs, const char* path);
const char* build_system_get_source_path(build_system_t* bs);
void build_system_set_toolchain_path(build_system_t* bs, const char* path);
const char* build_system_get_toolchain_path(build_system_t* bs);
void build_system_enable_optimization(build_system_t* bs, bool enable);
bool build_system_is_optimization_enabled(build_system_t* bs);
void build_system_enable_debugging(build_system_t* bs, bool enable);
bool build_system_is_debugging_enabled(build_system_t* bs);
void build_system_enable_profiling(build_system_t* bs, bool enable);
bool build_system_is_profiling_enabled(build_system_t* bs);
void build_system_enable_testing(build_system_t* bs, bool enable);
bool build_system_is_testing_enabled(build_system_t* bs);
void build_system_enable_parallel_build(build_system_t* bs, bool enable);
bool build_system_is_parallel_build_enabled(build_system_t* bs);
void build_system_set_max_jobs(build_system_t* bs, uint32_t max_jobs);
uint32_t build_system_get_max_jobs(build_system_t* bs);

// Progress monitoring
build_progress_t* build_system_get_progress(build_system_t* bs);
bool build_system_is_building(build_system_t* bs);
float build_system_get_progress_percentage(build_system_t* bs);
uint64_t build_system_get_estimated_time(build_system_t* bs);
const char* build_system_get_current_component(build_system_t* bs);
bool build_system_has_errors(build_system_t* bs);
bool build_system_has_warnings(build_system_t* bs);
uint32_t build_system_get_error_count(build_system_t* bs);
uint32_t build_system_get_warning_count(build_system_t* bs);

// Result management
build_result_t* build_system_get_last_result(build_system_t* bs);
bool build_system_was_successful(build_system_t* bs);
const char* build_system_get_output_file(build_system_t* bs);
uint64_t build_system_get_build_time(build_system_t* bs);
uint64_t build_system_get_output_size(build_system_t* bs);
const char* build_system_get_errors(build_system_t* bs);
const char* build_system_get_warnings(build_system_t* bs);

// Logging
void build_system_set_build_log_path(build_system_t* bs, const char* path);
const char* build_system_get_build_log_path(build_system_t* bs);
void build_system_set_error_log_path(build_system_t* bs, const char* path);
const char* build_system_get_error_log_path(build_system_t* bs);
void build_system_set_warning_log_path(build_system_t* bs, const char* path);
const char* build_system_get_warning_log_path(build_system_t* bs);
void build_system_log_message(build_system_t* bs, const char* message);
void build_system_log_error(build_system_t* bs, const char* error);
void build_system_log_warning(build_system_t* bs, const char* warning);

// Specific build targets
build_result_t build_system_build_kernel(build_system_t* bs);
build_result_t build_system_build_bootloader(build_system_t* bs);
build_result_t build_system_build_drivers(build_system_t* bs);
build_result_t build_system_build_gui(build_system_t* bs);
build_result_t build_system_build_desktop(build_system_t* bs);
build_result_t build_system_build_file_manager(build_system_t* bs);
build_result_t build_system_build_web_browser(build_system_t* bs);
build_result_t build_system_build_email_client(build_system_t* bs);
build_result_t build_system_build_audio_system(build_system_t* bs);
build_result_t build_system_build_wifi_system(build_system_t* bs);
build_result_t build_system_build_power_management(build_system_t* bs);
build_result_t build_system_build_directx_compatibility(build_system_t* bs);
build_result_t build_system_build_ai_system(build_system_t* bs);
build_result_t build_system_build_quantum_framework(build_system_t* bs);
build_result_t build_system_build_blockchain_framework(build_system_t* bs);
build_result_t build_system_build_xr_framework(build_system_t* bs);
build_result_t build_system_build_enterprise_features(build_system_t* bs);
build_result_t build_system_build_security_system(build_system_t* bs);
build_result_t build_system_build_networking(build_system_t* bs);
build_result_t build_system_build_storage_system(build_system_t* bs);
build_result_t build_system_build_virtualization(build_system_t* bs);
build_result_t build_system_build_gaming_system(build_system_t* bs);
build_result_t build_system_build_creative_suite(build_system_t* bs);
build_result_t build_system_build_accessibility(build_system_t* bs);
build_result_t build_system_build_cloud_integration(build_system_t* bs);
build_result_t build_system_build_device_integration(build_system_t* bs);

// ISO and bootable image creation
build_result_t build_system_create_iso(build_system_t* bs, const char* iso_path);
build_result_t build_system_create_usb_image(build_system_t* bs, const char* usb_path);
build_result_t build_system_create_vm_image(build_system_t* bs, const char* vm_path);
build_result_t build_system_create_installer(build_system_t* bs, const char* installer_path);

// Testing
build_result_t build_system_run_tests(build_system_t* bs);
build_result_t build_system_run_unit_tests(build_system_t* bs);
build_result_t build_system_run_integration_tests(build_system_t* bs);
build_result_t build_system_run_system_tests(build_system_t* bs);
build_result_t build_system_run_performance_tests(build_system_t* bs);
build_result_t build_system_run_security_tests(build_system_t* bs);

// Documentation
build_result_t build_system_generate_documentation(build_system_t* bs);
build_result_t build_system_generate_api_docs(build_system_t* bs);
build_result_t build_system_generate_user_manual(build_system_t* bs);
build_result_t build_system_generate_developer_guide(build_system_t* bs);
build_result_t build_system_generate_architecture_docs(build_system_t* bs);

// Validation
bool build_system_validate_configuration(build_system_t* bs);
bool build_system_validate_dependencies(build_system_t* bs);
bool build_system_validate_components(build_system_t* bs);
bool build_system_validate_output(build_system_t* bs);
bool build_system_validate_iso(build_system_t* bs, const char* iso_path);

// Utility functions
char* build_system_get_architecture_name(build_arch_t architecture);
char* build_system_get_target_name(build_target_t target);
char* build_system_get_config_name(build_config_t config);
bool build_system_is_architecture_supported(build_arch_t architecture);
bool build_system_is_target_supported(build_target_t target);
bool build_system_is_config_supported(build_config_t config);
uint64_t build_system_estimate_build_time(build_system_t* bs);
uint64_t build_system_estimate_disk_space(build_system_t* bs);
uint64_t build_system_estimate_memory_usage(build_system_t* bs);

// Preset configurations
build_configuration_t build_system_preset_debug_config(void);
build_configuration_t build_system_preset_release_config(void);
build_configuration_t build_system_preset_minimal_config(void);
build_configuration_t build_system_preset_full_config(void);
build_configuration_t build_system_preset_gaming_config(void);
build_configuration_t build_system_preset_enterprise_config(void);
build_configuration_t build_system_preset_development_config(void);

// Callbacks
typedef void (*build_progress_callback_t)(build_system_t* bs, build_progress_t* progress, void* user_data);
typedef void (*build_complete_callback_t)(build_system_t* bs, build_result_t* result, void* user_data);
typedef void (*build_error_callback_t)(build_system_t* bs, const char* error, void* user_data);

void build_system_set_progress_callback(build_system_t* bs, build_progress_callback_t callback, void* user_data);
void build_system_set_complete_callback(build_system_t* bs, build_complete_callback_t callback, void* user_data);
void build_system_set_error_callback(build_system_t* bs, build_error_callback_t callback, void* user_data);

// Error handling
typedef enum {
    BUILD_SYSTEM_SUCCESS = 0,
    BUILD_SYSTEM_ERROR_INVALID_CONTEXT,
    BUILD_SYSTEM_ERROR_INVALID_CONFIGURATION,
    BUILD_SYSTEM_ERROR_MISSING_DEPENDENCIES,
    BUILD_SYSTEM_ERROR_BUILD_FAILED,
    BUILD_SYSTEM_ERROR_COMPILATION_FAILED,
    BUILD_SYSTEM_ERROR_LINKING_FAILED,
    BUILD_SYSTEM_ERROR_OUT_OF_MEMORY,
    BUILD_SYSTEM_ERROR_DISK_FULL,
    BUILD_SYSTEM_ERROR_PERMISSION_DENIED,
    BUILD_SYSTEM_ERROR_TIMEOUT,
    BUILD_SYSTEM_ERROR_INVALID_TARGET,
    BUILD_SYSTEM_ERROR_INVALID_ARCHITECTURE,
    BUILD_SYSTEM_ERROR_TOOLCHAIN_NOT_FOUND,
    BUILD_SYSTEM_ERROR_SOURCE_NOT_FOUND,
} build_system_error_t;

build_system_error_t build_system_get_last_error(void);
const char* build_system_get_error_string(build_system_error_t error);

#endif // BUILD_SYSTEM_H 