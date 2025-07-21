#ifndef SYSTEM_RELIABILITY_H
#define SYSTEM_RELIABILITY_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// System reliability constants
#define MAX_RESTORE_POINTS 50
#define MAX_SAFE_MODES 10
#define MAX_BACKUP_JOBS 100
#define MAX_DIAGNOSTIC_TOOLS 30
#define MAX_CRASH_REPORTS 1000
#define MAX_MEMORY_DUMPS 100
#define MAX_TELEMETRY_METRICS 50
#define MAX_HARDWARE_DEVICES 1000
#define MAX_DRIVER_SIGNATURES 500
#define MAX_POWER_PROFILES 20
#define MAX_HIBERNATION_STATES 10
#define MAX_THERMAL_ZONES 20

// Restore point types
typedef enum {
    RESTORE_POINT_SYSTEM = 0,
    RESTORE_POINT_APPLICATION = 1,
    RESTORE_POINT_DRIVER = 2,
    RESTORE_POINT_UPDATE = 3,
    RESTORE_POINT_MANUAL = 4,
    RESTORE_POINT_SCHEDULED = 5
} restore_point_type_t;

// Safe mode types
typedef enum {
    SAFE_MODE_MINIMAL = 0,
    SAFE_MODE_NETWORK = 1,
    SAFE_MODE_GRAPHICS = 2,
    SAFE_MODE_DEBUG = 3,
    SAFE_MODE_RECOVERY = 4,
    SAFE_MODE_DIAGNOSTIC = 5
} safe_mode_type_t;

// Backup job types
typedef enum {
    BACKUP_JOB_FULL = 0,
    BACKUP_JOB_INCREMENTAL = 1,
    BACKUP_JOB_DIFFERENTIAL = 2,
    BACKUP_JOB_SYSTEM = 3,
    BACKUP_JOB_USER_DATA = 4,
    BACKUP_JOB_APPLICATIONS = 5
} backup_job_type_t;

// Diagnostic tool types
typedef enum {
    DIAGNOSTIC_TOOL_MEMORY = 0,
    DIAGNOSTIC_TOOL_DISK = 1,
    DIAGNOSTIC_TOOL_NETWORK = 2,
    DIAGNOSTIC_TOOL_CPU = 3,
    DIAGNOSTIC_TOOL_GPU = 4,
    DIAGNOSTIC_TOOL_SYSTEM = 5,
    DIAGNOSTIC_TOOL_APPLICATION = 6,
    DIAGNOSTIC_TOOL_SECURITY = 7
} diagnostic_tool_type_t;

// Crash report types
typedef enum {
    CRASH_REPORT_KERNEL = 0,
    CRASH_REPORT_APPLICATION = 1,
    CRASH_REPORT_DRIVER = 2,
    CRASH_REPORT_SYSTEM = 3,
    CRASH_REPORT_HARDWARE = 4,
    CRASH_REPORT_NETWORK = 5
} crash_report_type_t;

// Memory dump types
typedef enum {
    MEMORY_DUMP_COMPLETE = 0,
    MEMORY_DUMP_KERNEL = 1,
    MEMORY_DUMP_SMALL = 2,
    MEMORY_DUMP_ACTIVE = 3,
    MEMORY_DUMP_SELECTIVE = 4
} memory_dump_type_t;

// Telemetry metric types
typedef enum {
    TELEMETRY_SYSTEM_HEALTH = 0,
    TELEMETRY_PERFORMANCE = 1,
    TELEMETRY_STABILITY = 2,
    TELEMETRY_SECURITY = 3,
    TELEMETRY_USAGE = 4,
    TELEMETRY_ERRORS = 5,
    TELEMETRY_UPDATES = 6,
    TELEMETRY_HARDWARE = 7
} telemetry_metric_type_t;

// Hardware device types
typedef enum {
    HARDWARE_DEVICE_CPU = 0,
    HARDWARE_DEVICE_GPU = 1,
    HARDWARE_DEVICE_MEMORY = 2,
    HARDWARE_DEVICE_STORAGE = 3,
    HARDWARE_DEVICE_NETWORK = 4,
    HARDWARE_DEVICE_AUDIO = 5,
    HARDWARE_DEVICE_VIDEO = 6,
    HARDWARE_DEVICE_INPUT = 7,
    HARDWARE_DEVICE_OUTPUT = 8,
    HARDWARE_DEVICE_PERIPHERAL = 9
} hardware_device_type_t;

// Power profile types
typedef enum {
    POWER_PROFILE_POWER_SAVER = 0,
    POWER_PROFILE_BALANCED = 1,
    POWER_PROFILE_HIGH_PERFORMANCE = 2,
    POWER_PROFILE_GAMING = 3,
    POWER_PROFILE_CREATIVE = 4,
    POWER_PROFILE_ENTERPRISE = 5,
    POWER_PROFILE_CUSTOM = 6
} power_profile_type_t;

// Hibernation state types
typedef enum {
    HIBERNATION_STATE_SLEEP = 0,
    HIBERNATION_STATE_HIBERNATE = 1,
    HIBERNATION_STATE_HYBRID = 2,
    HIBERNATION_STATE_FAST_STARTUP = 3
} hibernation_state_type_t;

// Thermal zone types
typedef enum {
    THERMAL_ZONE_CPU = 0,
    THERMAL_ZONE_GPU = 1,
    THERMAL_ZONE_MEMORY = 2,
    THERMAL_ZONE_STORAGE = 3,
    THERMAL_ZONE_MOTHERBOARD = 4,
    THERMAL_ZONE_CASE = 5
} thermal_zone_type_t;

// Restore point
typedef struct restore_point {
    uint32_t restore_point_id;                   // Restore point identifier
    char name[128];                              // Restore point name
    char description[256];                       // Restore point description
    restore_point_type_t type;                   // Restore point type
    bool active;                                 // Restore point active
    uint64_t created_time;                       // Creation time
    uint64_t size;                               // Restore point size (bytes)
    uint32_t file_count;                         // Number of files
    char backup_location[512];                   // Backup location
    bool verified;                               // Restore point verified
    void* restore_point_data;                    // Restore point-specific data
} restore_point_t;

// Safe mode
typedef struct safe_mode {
    uint32_t safe_mode_id;                       // Safe mode identifier
    char name[64];                               // Safe mode name
    char description[256];                       // Safe mode description
    safe_mode_type_t type;                       // Safe mode type
    bool enabled;                                // Safe mode enabled
    bool active;                                 // Safe mode active
    uint32_t boot_time;                          // Boot time (ms)
    uint32_t services_disabled;                  // Number of services disabled
    uint32_t drivers_disabled;                   // Number of drivers disabled
    uint64_t last_used;                          // Last used time
    void* safe_mode_data;                        // Safe mode-specific data
} safe_mode_t;

// Backup job
typedef struct backup_job {
    uint32_t job_id;                             // Job identifier
    char name[128];                              // Job name
    char description[256];                       // Job description
    backup_job_type_t type;                      // Job type
    bool enabled;                                // Job enabled
    bool running;                                // Job running
    bool scheduled;                              // Job scheduled
    uint64_t last_run;                           // Last run time
    uint64_t next_run;                           // Next run time
    uint64_t backup_size;                        // Backup size (bytes)
    uint32_t file_count;                         // Number of files
    char destination[512];                       // Backup destination
    bool encrypted;                              // Backup encrypted
    void* job_data;                              // Job-specific data
} backup_job_t;

// Diagnostic tool
typedef struct diagnostic_tool {
    uint32_t tool_id;                            // Tool identifier
    char name[64];                               // Tool name
    char description[256];                       // Tool description
    diagnostic_tool_type_t type;                 // Tool type
    bool enabled;                                // Tool enabled
    bool running;                                // Tool running
    uint32_t duration;                           // Duration (ms)
    float success_rate;                          // Success rate (0.0-1.0)
    uint64_t last_run;                           // Last run time
    uint32_t run_count;                          // Run count
    void* tool_function;                         // Tool function pointer
    void* tool_data;                             // Tool-specific data
} diagnostic_tool_t;

// Crash report
typedef struct crash_report {
    uint32_t report_id;                          // Report identifier
    char name[128];                              // Report name
    crash_report_type_t type;                    // Report type
    uint64_t crash_time;                         // Crash time
    uint32_t process_id;                         // Process ID
    char process_name[64];                       // Process name
    uint32_t thread_id;                          // Thread ID
    uint64_t exception_address;                  // Exception address
    uint32_t exception_code;                     // Exception code
    char stack_trace[2048];                      // Stack trace
    char minidump_path[512];                     // Minidump path
    bool analyzed;                               // Report analyzed
    bool resolved;                               // Issue resolved
    void* report_data;                           // Report-specific data
} crash_report_t;

// Memory dump
typedef struct memory_dump {
    uint32_t dump_id;                            // Dump identifier
    char name[128];                              // Dump name
    memory_dump_type_t type;                     // Dump type
    uint64_t created_time;                       // Creation time
    uint64_t size;                               // Dump size (bytes)
    char file_path[512];                         // File path
    bool compressed;                             // Dump compressed
    bool encrypted;                              // Dump encrypted
    bool analyzed;                               // Dump analyzed
    char analysis_result[1024];                  // Analysis result
    void* dump_data;                             // Dump-specific data
} memory_dump_t;

// Telemetry metric
typedef struct telemetry_metric {
    uint32_t metric_id;                          // Metric identifier
    char name[64];                               // Metric name
    char description[256];                       // Metric description
    telemetry_metric_type_t type;                // Metric type
    bool enabled;                                // Metric enabled
    bool real_time;                              // Real-time collection
    float current_value;                         // Current value
    float min_value;                             // Minimum value
    float max_value;                             // Maximum value
    float average_value;                         // Average value
    uint64_t last_update;                        // Last update time
    uint64_t collection_interval;                // Collection interval (ms)
    uint32_t history_size;                       // History size
    float* history_data;                         // History data
    void* metric_data;                           // Metric-specific data
} telemetry_metric_t;

// Hardware device
typedef struct hardware_device {
    uint32_t device_id;                          // Device identifier
    char name[128];                              // Device name
    char manufacturer[64];                       // Manufacturer
    char model[64];                              // Device model
    hardware_device_type_t type;                 // Device type
    bool connected;                              // Device connected
    bool working;                                // Device working
    bool hot_swappable;                          // Hot swappable
    uint32_t driver_id;                          // Associated driver
    bool driver_signed;                          // Driver signed
    uint32_t health_score;                       // Health score (0-100)
    uint64_t last_maintenance;                   // Last maintenance time
    void* device_data;                           // Device-specific data
} hardware_device_t;

// Driver signature
typedef struct driver_signature {
    uint32_t signature_id;                       // Signature identifier
    char name[64];                               // Signature name
    char description[256];                       // Signature description
    char certificate_path[512];                  // Certificate path
    uint64_t expiry_time;                        // Expiry time
    bool valid;                                  // Signature valid
    bool revoked;                                // Signature revoked
    uint32_t driver_count;                       // Number of signed drivers
    uint32_t* signed_drivers;                    // Signed driver list
    void* signature_data;                        // Signature-specific data
} driver_signature_t;

// Power profile
typedef struct power_profile {
    uint32_t profile_id;                         // Profile identifier
    char name[64];                               // Profile name
    char description[256];                       // Profile description
    power_profile_type_t type;                   // Profile type
    bool active;                                 // Profile active
    bool custom;                                 // Custom profile
    uint32_t cpu_power_limit;                    // CPU power limit (%)
    uint32_t gpu_power_limit;                    // GPU power limit (%)
    uint32_t display_brightness;                 // Display brightness (%)
    uint32_t sleep_timeout;                      // Sleep timeout (minutes)
    uint32_t hibernate_timeout;                  // Hibernate timeout (minutes)
    bool fast_startup;                           // Fast startup enabled
    bool thermal_management;                     // Thermal management enabled
    void* profile_data;                          // Profile-specific data
} power_profile_t;

// Hibernation state
typedef struct hibernation_state {
    uint32_t state_id;                           // State identifier
    char name[64];                               // State name
    char description[256];                       // State description
    hibernation_state_type_t type;               // State type
    bool enabled;                                // State enabled
    bool active;                                 // State active
    uint32_t transition_time;                    // Transition time (ms)
    uint32_t resume_time;                        // Resume time (ms)
    uint64_t state_size;                         // State size (bytes)
    char state_file_path[512];                   // State file path
    bool encrypted;                              // State encrypted
    void* state_data;                            // State-specific data
} hibernation_state_t;

// Thermal zone
typedef struct thermal_zone {
    uint32_t zone_id;                            // Zone identifier
    char name[64];                               // Zone name
    char description[256];                       // Zone description
    thermal_zone_type_t type;                    // Zone type
    bool active;                                 // Zone active
    uint32_t current_temperature;                // Current temperature (°C)
    uint32_t critical_temperature;               // Critical temperature (°C)
    uint32_t max_temperature;                    // Maximum temperature (°C)
    uint32_t min_temperature;                    // Minimum temperature (°C)
    uint32_t target_temperature;                 // Target temperature (°C)
    bool thermal_throttling;                     // Thermal throttling active
    uint32_t fan_speed;                          // Fan speed (%)
    void* zone_data;                             // Zone-specific data
} thermal_zone_t;

// System reliability system
typedef struct system_reliability_system {
    spinlock_t lock;                             // System lock
    bool initialized;                            // Initialization flag
    
    // Recovery tools management
    restore_point_t restore_points[MAX_RESTORE_POINTS]; // Restore points
    uint32_t restore_point_count;                // Number of restore points
    
    // Safe mode management
    safe_mode_t safe_modes[MAX_SAFE_MODES];      // Safe modes
    uint32_t safe_mode_count;                    // Number of safe modes
    safe_mode_t* active_safe_mode;               // Active safe mode
    
    // Backup management
    backup_job_t backup_jobs[MAX_BACKUP_JOBS];   // Backup jobs
    uint32_t backup_job_count;                   // Number of backup jobs
    
    // Diagnostic tools management
    diagnostic_tool_t diagnostic_tools[MAX_DIAGNOSTIC_TOOLS]; // Diagnostic tools
    uint32_t diagnostic_tool_count;              // Number of diagnostic tools
    
    // Crash reporting management
    crash_report_t crash_reports[MAX_CRASH_REPORTS]; // Crash reports
    uint32_t crash_report_count;                 // Number of crash reports
    
    // Memory dump management
    memory_dump_t memory_dumps[MAX_MEMORY_DUMPS]; // Memory dumps
    uint32_t memory_dump_count;                  // Number of memory dumps
    
    // Telemetry management
    telemetry_metric_t telemetry_metrics[MAX_TELEMETRY_METRICS]; // Telemetry metrics
    uint32_t telemetry_metric_count;             // Number of telemetry metrics
    
    // Hardware management
    hardware_device_t hardware_devices[MAX_HARDWARE_DEVICES]; // Hardware devices
    uint32_t hardware_device_count;              // Number of hardware devices
    
    // Driver signature management
    driver_signature_t driver_signatures[MAX_DRIVER_SIGNATURES]; // Driver signatures
    uint32_t driver_signature_count;             // Number of driver signatures
    
    // Power management
    power_profile_t power_profiles[MAX_POWER_PROFILES]; // Power profiles
    uint32_t power_profile_count;                // Number of power profiles
    power_profile_t* active_power_profile;       // Active power profile
    
    // Hibernation management
    hibernation_state_t hibernation_states[MAX_HIBERNATION_STATES]; // Hibernation states
    uint32_t hibernation_state_count;            // Number of hibernation states
    
    // Thermal management
    thermal_zone_t thermal_zones[MAX_THERMAL_ZONES]; // Thermal zones
    uint32_t thermal_zone_count;                 // Number of thermal zones
    
    // System configuration
    bool recovery_tools_enabled;                 // Recovery tools enabled
    bool diagnostics_enabled;                    // Diagnostics enabled
    bool hardware_management_enabled;            // Hardware management enabled
    bool power_management_enabled;               // Power management enabled
    bool automatic_backup_enabled;               // Automatic backup enabled
    bool crash_reporting_enabled;                // Crash reporting enabled
    bool telemetry_enabled;                      // Telemetry enabled
    bool thermal_management_enabled;             // Thermal management enabled
    
    // Recovery configuration
    bool system_restore_enabled;                 // System restore enabled
    bool safe_mode_enabled;                      // Safe mode enabled
    bool automatic_backup_enabled;               // Automatic backup enabled
    uint32_t restore_point_retention;            // Restore point retention (days)
    
    // Diagnostic configuration
    bool memory_diagnostics_enabled;             // Memory diagnostics enabled
    bool disk_diagnostics_enabled;               // Disk diagnostics enabled
    bool network_diagnostics_enabled;            // Network diagnostics enabled
    bool system_diagnostics_enabled;             // System diagnostics enabled
    
    // Hardware configuration
    bool hot_swappable_devices_enabled;          // Hot swappable devices enabled
    bool driver_signing_enabled;                 // Driver signing enabled
    bool hardware_monitoring_enabled;            // Hardware monitoring enabled
    bool automatic_maintenance_enabled;          // Automatic maintenance enabled
    
    // Power configuration
    bool advanced_power_profiles_enabled;        // Advanced power profiles enabled
    bool hibernation_enabled;                    // Hibernation enabled
    bool thermal_management_enabled;             // Thermal management enabled
    bool power_efficiency_enabled;               // Power efficiency enabled
    
    // Statistics
    uint64_t restore_points_created;             // Restore points created
    uint64_t backups_performed;                  // Backups performed
    uint64_t diagnostics_run;                    // Diagnostics run
    uint64_t crash_reports_generated;            // Crash reports generated
    uint64_t memory_dumps_created;               // Memory dumps created
    uint64_t hardware_issues_resolved;           // Hardware issues resolved
    uint64_t power_optimizations_applied;        // Power optimizations applied
    uint64_t last_update;                        // Last update time
} system_reliability_system_t;

// Function declarations

// System initialization
int system_reliability_init(void);
void system_reliability_shutdown(void);
system_reliability_system_t* system_reliability_get_system(void);

// Recovery tools management
restore_point_t* restore_point_create(const char* name, restore_point_type_t type);
int restore_point_destroy(uint32_t restore_point_id);
int restore_point_apply(uint32_t restore_point_id);
int restore_point_verify(uint32_t restore_point_id);
restore_point_t* restore_point_find(uint32_t restore_point_id);
restore_point_t* restore_point_find_by_name(const char* name);

// Safe mode management
safe_mode_t* safe_mode_create(const char* name, safe_mode_type_t type);
int safe_mode_destroy(uint32_t safe_mode_id);
int safe_mode_enter(uint32_t safe_mode_id);
int safe_mode_exit(uint32_t safe_mode_id);
safe_mode_t* safe_mode_find(uint32_t safe_mode_id);
safe_mode_t* safe_mode_find_by_name(const char* name);
safe_mode_t* safe_mode_get_active(void);

// Backup management
backup_job_t* backup_job_create(const char* name, backup_job_type_t type);
int backup_job_destroy(uint32_t job_id);
int backup_job_start(uint32_t job_id);
int backup_job_stop(uint32_t job_id);
int backup_job_schedule(uint32_t job_id, uint64_t next_run);
backup_job_t* backup_job_find(uint32_t job_id);
backup_job_t* backup_job_find_by_name(const char* name);

// Diagnostic tools management
diagnostic_tool_t* diagnostic_tool_create(const char* name, diagnostic_tool_type_t type);
int diagnostic_tool_destroy(uint32_t tool_id);
int diagnostic_tool_run(uint32_t tool_id);
int diagnostic_tool_run_all(void);
diagnostic_tool_t* diagnostic_tool_find(uint32_t tool_id);
diagnostic_tool_t* diagnostic_tool_find_by_name(const char* name);

// Crash reporting management
crash_report_t* crash_report_create(const char* name, crash_report_type_t type);
int crash_report_destroy(uint32_t report_id);
int crash_report_analyze(uint32_t report_id);
int crash_report_resolve(uint32_t report_id);
crash_report_t* crash_report_find(uint32_t report_id);
crash_report_t* crash_report_find_by_name(const char* name);

// Memory dump management
memory_dump_t* memory_dump_create(const char* name, memory_dump_type_t type);
int memory_dump_destroy(uint32_t dump_id);
int memory_dump_analyze(uint32_t dump_id);
int memory_dump_compress(uint32_t dump_id);
memory_dump_t* memory_dump_find(uint32_t dump_id);
memory_dump_t* memory_dump_find_by_name(const char* name);

// Telemetry management
telemetry_metric_t* telemetry_metric_create(const char* name, telemetry_metric_type_t type);
int telemetry_metric_destroy(uint32_t metric_id);
int telemetry_metric_enable(uint32_t metric_id, bool enabled);
int telemetry_metric_update(uint32_t metric_id, float value);
telemetry_metric_t* telemetry_metric_find(uint32_t metric_id);
telemetry_metric_t* telemetry_metric_find_by_name(const char* name);

// Hardware management
int hardware_device_enumerate(void);
hardware_device_t* hardware_device_get_info(uint32_t device_id);
int hardware_device_connect(uint32_t device_id);
int hardware_device_disconnect(uint32_t device_id);
int hardware_device_maintain(uint32_t device_id);
hardware_device_t* hardware_device_find(uint32_t device_id);
hardware_device_t* hardware_device_find_by_name(const char* name);

// Driver signature management
driver_signature_t* driver_signature_create(const char* name, const char* certificate_path);
int driver_signature_destroy(uint32_t signature_id);
int driver_signature_verify(uint32_t signature_id);
int driver_signature_revoke(uint32_t signature_id);
driver_signature_t* driver_signature_find(uint32_t signature_id);
driver_signature_t* driver_signature_find_by_name(const char* name);

// Power management
power_profile_t* power_profile_create(const char* name, power_profile_type_t type);
int power_profile_destroy(uint32_t profile_id);
int power_profile_apply(uint32_t profile_id);
int power_profile_customize(uint32_t profile_id, void* custom_settings);
power_profile_t* power_profile_find(uint32_t profile_id);
power_profile_t* power_profile_find_by_name(const char* name);
power_profile_t* power_profile_get_active(void);

// Hibernation management
hibernation_state_t* hibernation_state_create(const char* name, hibernation_state_type_t type);
int hibernation_state_destroy(uint32_t state_id);
int hibernation_state_enter(uint32_t state_id);
int hibernation_state_exit(uint32_t state_id);
hibernation_state_t* hibernation_state_find(uint32_t state_id);
hibernation_state_t* hibernation_state_find_by_name(const char* name);

// Thermal management
thermal_zone_t* thermal_zone_create(const char* name, thermal_zone_type_t type);
int thermal_zone_destroy(uint32_t zone_id);
int thermal_zone_monitor(uint32_t zone_id);
int thermal_zone_cool(uint32_t zone_id);
thermal_zone_t* thermal_zone_find(uint32_t zone_id);
thermal_zone_t* thermal_zone_find_by_name(const char* name);

// System configuration
int recovery_tools_enable(bool enabled);
int diagnostics_enable(bool enabled);
int hardware_management_enable(bool enabled);
int power_management_enable(bool enabled);
int automatic_backup_enable(bool enabled);
int crash_reporting_enable(bool enabled);
int telemetry_enable(bool enabled);
int thermal_management_enable(bool enabled);

// Recovery configuration
int system_restore_enable(bool enabled);
int safe_mode_enable(bool enabled);
int automatic_backup_enable(bool enabled);
int restore_point_retention_set(uint32_t days);

// Diagnostic configuration
int memory_diagnostics_enable(bool enabled);
int disk_diagnostics_enable(bool enabled);
int network_diagnostics_enable(bool enabled);
int system_diagnostics_enable(bool enabled);

// Hardware configuration
int hot_swappable_devices_enable(bool enabled);
int driver_signing_enable(bool enabled);
int hardware_monitoring_enable(bool enabled);
int automatic_maintenance_enable(bool enabled);

// Power configuration
int advanced_power_profiles_enable(bool enabled);
int hibernation_enable(bool enabled);
int thermal_management_enable(bool enabled);
int power_efficiency_enable(bool enabled);

// High-level reliability functions
int system_reliability_check(void);
int system_reliability_optimize(void);
int system_reliability_maintain(void);
int system_reliability_backup(void);

// Statistics
void system_reliability_get_stats(system_reliability_system_t* stats);
void system_reliability_reset_stats(void);

#endif // SYSTEM_RELIABILITY_H 