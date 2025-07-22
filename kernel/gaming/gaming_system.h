#ifndef GAMING_SYSTEM_H
#define GAMING_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations for kernel types
typedef uint32_t spinlock_t;

// Gaming system constants
#define MAX_GAMES 100
#define MAX_GAMING_PROFILES 50
#define MAX_GAMING_OVERLAYS 10
#define MAX_GAMING_MONITORS 4
#define MAX_GAMING_CONTROLLERS 8
#define MAX_GAMING_PERIPHERALS 32

// Gaming modes
typedef enum {
    GAMING_MODE_NORMAL = 0,
    GAMING_MODE_PERFORMANCE = 1,
    GAMING_MODE_ULTRA = 2,
    GAMING_MODE_CUSTOM = 3
} gaming_mode_t;

// Graphics APIs
typedef enum {
    GRAPHICS_API_DIRECTX_11 = 0,
    GRAPHICS_API_DIRECTX_12 = 1,
    GRAPHICS_API_VULKAN = 2,
    GRAPHICS_API_OPENGL = 3,
    GRAPHICS_API_OPENGL_ES = 4,
    GRAPHICS_API_METAL = 5,
    GRAPHICS_API_RAEENDX = 6
} graphics_api_t;

// Game platforms
typedef enum {
    GAME_PLATFORM_STEAM = 0,
    GAME_PLATFORM_EPIC = 1,
    GAME_PLATFORM_GOG = 2,
    GAME_PLATFORM_ORIGIN = 3,
    GAME_PLATFORM_UPLAY = 4,
    GAME_PLATFORM_BATTLENET = 5,
    GAME_PLATFORM_WINDOWS_STORE = 6,
    GAME_PLATFORM_NATIVE = 7
} game_platform_t;

// Ray tracing features
typedef enum {
    RAY_TRACING_NONE = 0,
    RAY_TRACING_SHADOWS = 1,
    RAY_TRACING_REFLECTIONS = 2,
    RAY_TRACING_GLOBAL_ILLUMINATION = 4,
    RAY_TRACING_AMBIENT_OCCLUSION = 8,
    RAY_TRACING_FULL = 15
} ray_tracing_features_t;

// VRR technologies
typedef enum {
    VRR_NONE = 0,
    VRR_G_SYNC = 1,
    VRR_FREE_SYNC = 2,
    VRR_ADAPTIVE_SYNC = 4,
    VRR_HDMI_VRR = 8
} vrr_technology_t;

// Game information
typedef struct game_info {
    uint32_t game_id;                            // Unique game identifier
    char name[128];                              // Game name
    char executable[256];                        // Executable path
    char install_path[512];                      // Installation path
    game_platform_t platform;                    // Game platform
    graphics_api_t preferred_api;                // Preferred graphics API
    uint32_t version;                            // Game version
    uint64_t install_date;                       // Installation date
    uint64_t last_played;                        // Last played date
    uint64_t play_time;                          // Total play time
    bool is_running;                             // Currently running
    bool is_installed;                           // Installed on system
    uint32_t process_id;                         // Process ID when running
    void* game_data;                             // Game-specific data
} game_info_t;

// Gaming profile
typedef struct gaming_profile {
    uint32_t profile_id;                         // Profile identifier
    char name[64];                               // Profile name
    char description[256];                       // Profile description
    gaming_mode_t mode;                          // Gaming mode
    bool cpu_boost_enabled;                      // CPU boost enabled
    bool gpu_boost_enabled;                      // GPU boost enabled
    uint32_t cpu_priority;                       // CPU priority (0-31)
    uint32_t gpu_priority;                       // GPU priority (0-31)
    bool memory_optimization;                    // Memory optimization
    bool network_optimization;                   // Network optimization
    bool audio_optimization;                     // Audio optimization
    bool overlay_enabled;                        // Game overlay enabled
    bool background_processes_disabled;          // Disable background processes
    bool notifications_disabled;                 // Disable notifications
    uint32_t target_fps;                         // Target FPS
    uint32_t target_resolution_width;            // Target resolution width
    uint32_t target_resolution_height;           // Target resolution height
    uint32_t target_refresh_rate;                // Target refresh rate
    ray_tracing_features_t ray_tracing;          // Ray tracing features
    vrr_technology_t vrr_technology;             // VRR technology
    bool hdr_enabled;                            // HDR enabled
    bool direct_storage_enabled;                 // DirectStorage enabled
    void* profile_data;                          // Profile-specific data
} gaming_profile_t;

// Game overlay
typedef struct game_overlay {
    uint32_t overlay_id;                         // Overlay identifier
    char name[64];                               // Overlay name
    bool enabled;                                // Overlay enabled
    bool visible;                                // Overlay visible
    uint32_t x;                                  // X position
    uint32_t y;                                  // Y position
    uint32_t width;                              // Width
    uint32_t height;                             // Height
    bool show_fps;                               // Show FPS counter
    bool show_cpu_usage;                         // Show CPU usage
    bool show_gpu_usage;                         // Show GPU usage
    bool show_memory_usage;                      // Show memory usage
    bool show_network_usage;                     // Show network usage
    bool show_temperature;                       // Show temperature
    bool show_clock_speeds;                      // Show clock speeds
    bool show_frame_times;                       // Show frame times
    bool show_input_lag;                         // Show input lag
    void* overlay_data;                          // Overlay-specific data
} game_overlay_t;

// Gaming monitor
typedef struct gaming_monitor {
    uint32_t monitor_id;                         // Monitor identifier
    char name[64];                               // Monitor name
    uint32_t width;                              // Width
    uint32_t height;                             // Height
    uint32_t refresh_rate;                       // Refresh rate
    uint32_t color_depth;                        // Color depth
    bool hdr_supported;                          // HDR supported
    bool hdr_enabled;                            // HDR enabled
    vrr_technology_t vrr_supported;              // VRR technologies supported
    vrr_technology_t vrr_enabled;                // VRR technology enabled
    uint32_t response_time;                      // Response time (ms)
    bool g_sync_compatible;                      // G-Sync compatible
    bool free_sync_compatible;                   // FreeSync compatible
    bool adaptive_sync_compatible;               // Adaptive Sync compatible
    void* monitor_data;                          // Monitor-specific data
} gaming_monitor_t;

// Gaming controller
typedef struct gaming_controller {
    uint32_t controller_id;                      // Controller identifier
    char name[64];                               // Controller name
    char manufacturer[64];                       // Manufacturer
    bool connected;                              // Connected
    bool wireless;                               // Wireless
    uint32_t battery_level;                      // Battery level (0-100)
    bool vibration_enabled;                      // Vibration enabled
    bool force_feedback_enabled;                 // Force feedback enabled
    uint32_t button_count;                       // Number of buttons
    uint32_t axis_count;                         // Number of axes
    uint32_t trigger_count;                      // Number of triggers
    bool touchpad;                               // Has touchpad
    bool gyroscope;                              // Has gyroscope
    bool accelerometer;                          // Has accelerometer
    void* controller_data;                       // Controller-specific data
} gaming_controller_t;

// Gaming peripheral
typedef struct gaming_peripheral {
    uint32_t peripheral_id;                      // Peripheral identifier
    char name[64];                               // Peripheral name
    char type[32];                               // Peripheral type
    bool connected;                              // Connected
    bool wireless;                               // Wireless
    uint32_t battery_level;                      // Battery level (0-100)
    bool rgb_lighting;                           // RGB lighting
    bool macro_support;                          // Macro support
    bool profile_support;                        // Profile support
    void* peripheral_data;                       // Peripheral-specific data
} gaming_peripheral_t;

// Performance metrics
typedef struct gaming_performance {
    uint32_t fps;                                // Current FPS
    uint32_t fps_min;                            // Minimum FPS
    uint32_t fps_max;                            // Maximum FPS
    uint32_t fps_average;                        // Average FPS
    float cpu_usage;                             // CPU usage (%)
    float gpu_usage;                             // GPU usage (%)
    float memory_usage;                          // Memory usage (%)
    float network_usage;                         // Network usage (%)
    uint32_t cpu_temperature;                    // CPU temperature (°C)
    uint32_t gpu_temperature;                    // GPU temperature (°C)
    uint32_t cpu_clock_speed;                    // CPU clock speed (MHz)
    uint32_t gpu_clock_speed;                    // GPU clock speed (MHz)
    uint32_t memory_clock_speed;                 // Memory clock speed (MHz)
    uint32_t frame_time;                         // Frame time (ms)
    uint32_t input_lag;                          // Input lag (ms)
    uint64_t timestamp;                          // Timestamp
} gaming_performance_t;

// Gaming system
typedef struct gaming_system {
    spinlock_t lock;                             // System lock
    bool initialized;                            // Initialization flag
    
    // Game management
    game_info_t games[MAX_GAMES];                // Games
    uint32_t game_count;                         // Number of games
    game_info_t* current_game;                   // Currently running game
    
    // Profile management
    gaming_profile_t profiles[MAX_GAMING_PROFILES]; // Gaming profiles
    uint32_t profile_count;                      // Number of profiles
    gaming_profile_t* active_profile;            // Active profile
    
    // Overlay management
    game_overlay_t overlays[MAX_GAMING_OVERLAYS]; // Game overlays
    uint32_t overlay_count;                      // Number of overlays
    
    // Monitor management
    gaming_monitor_t monitors[MAX_GAMING_MONITORS]; // Gaming monitors
    uint32_t monitor_count;                      // Number of monitors
    gaming_monitor_t* primary_monitor;           // Primary monitor
    
    // Controller management
    gaming_controller_t controllers[MAX_GAMING_CONTROLLERS]; // Gaming controllers
    uint32_t controller_count;                   // Number of controllers
    
    // Peripheral management
    gaming_peripheral_t peripherals[MAX_GAMING_PERIPHERALS]; // Gaming peripherals
    uint32_t peripheral_count;                   // Number of peripherals
    
    // Performance monitoring
    gaming_performance_t performance;            // Current performance
    gaming_performance_t performance_history[100]; // Performance history
    uint32_t performance_index;                  // Performance history index
    
    // System configuration
    bool gaming_mode_enabled;                    // Gaming mode enabled
    bool auto_optimization_enabled;              // Auto optimization enabled
    bool performance_monitoring_enabled;         // Performance monitoring enabled
    bool overlay_enabled;                        // Overlay enabled
    bool ray_tracing_enabled;                    // Ray tracing enabled
    bool vrr_enabled;                            // VRR enabled
    bool hdr_enabled;                            // HDR enabled
    bool direct_storage_enabled;                 // DirectStorage enabled
    
    // Statistics
    uint64_t total_gaming_time;                  // Total gaming time
    uint64_t games_launched;                     // Games launched
    uint64_t performance_optimizations;          // Performance optimizations
    uint64_t last_update;                        // Last update time
} gaming_system_t;

// Function declarations

// System initialization
int gaming_system_init(void);
void gaming_system_shutdown(void);
gaming_system_t* gaming_system_get_system(void);

// Game management
game_info_t* game_add(const char* name, const char* executable, const char* install_path, game_platform_t platform);
int game_remove(uint32_t game_id);
int game_launch(uint32_t game_id);
int game_terminate(uint32_t game_id);
game_info_t* game_find(uint32_t game_id);
game_info_t* game_find_by_name(const char* name);
game_info_t* game_get_current(void);

// Profile management
gaming_profile_t* gaming_profile_create(const char* name, const char* description);
int gaming_profile_destroy(uint32_t profile_id);
int gaming_profile_apply(uint32_t profile_id);
int gaming_profile_save(uint32_t profile_id);
int gaming_profile_load(uint32_t profile_id);
gaming_profile_t* gaming_profile_find(uint32_t profile_id);
gaming_profile_t* gaming_profile_find_by_name(const char* name);
gaming_profile_t* gaming_profile_get_active(void);

// Overlay management
game_overlay_t* overlay_create(const char* name);
int overlay_destroy(uint32_t overlay_id);
int overlay_show(uint32_t overlay_id);
int overlay_hide(uint32_t overlay_id);
int overlay_set_position(uint32_t overlay_id, uint32_t x, uint32_t y);
int overlay_set_size(uint32_t overlay_id, uint32_t width, uint32_t height);
game_overlay_t* overlay_find(uint32_t overlay_id);

// Monitor management
int gaming_monitor_enumerate(void);
gaming_monitor_t* gaming_monitor_get_info(uint32_t monitor_id);
gaming_monitor_t* gaming_monitor_get_primary(void);
int gaming_monitor_set_primary(uint32_t monitor_id);
int gaming_monitor_set_resolution(uint32_t monitor_id, uint32_t width, uint32_t height);
int gaming_monitor_set_refresh_rate(uint32_t monitor_id, uint32_t refresh_rate);
int gaming_monitor_enable_hdr(uint32_t monitor_id, bool enabled);
int gaming_monitor_enable_vrr(uint32_t monitor_id, vrr_technology_t technology);

// Controller management
int gaming_controller_enumerate(void);
gaming_controller_t* gaming_controller_get_info(uint32_t controller_id);
int gaming_controller_connect(uint32_t controller_id);
int gaming_controller_disconnect(uint32_t controller_id);
int gaming_controller_set_vibration(uint32_t controller_id, bool enabled);
int gaming_controller_set_force_feedback(uint32_t controller_id, bool enabled);

// Peripheral management
int gaming_peripheral_enumerate(void);
gaming_peripheral_t* gaming_peripheral_get_info(uint32_t peripheral_id);
int gaming_peripheral_connect(uint32_t peripheral_id);
int gaming_peripheral_disconnect(uint32_t peripheral_id);
int gaming_peripheral_set_rgb(uint32_t peripheral_id, uint32_t color);

// Performance monitoring
int gaming_performance_start_monitoring(void);
int gaming_performance_stop_monitoring(void);
gaming_performance_t* gaming_performance_get_current(void);
gaming_performance_t* gaming_performance_get_history(uint32_t index);
int gaming_performance_get_average(gaming_performance_t* average);

// System configuration
int gaming_mode_enable(bool enabled);
int auto_optimization_enable(bool enabled);
int performance_monitoring_enable(bool enabled);
int overlay_enable(bool enabled);
int ray_tracing_enable(bool enabled);
int vrr_enable(bool enabled);
int hdr_enable(bool enabled);
int direct_storage_enable(bool enabled);

// Optimization
int gaming_optimize_cpu(void);
int gaming_optimize_gpu(void);
int gaming_optimize_memory(void);
int gaming_optimize_network(void);
int gaming_optimize_audio(void);
int gaming_optimize_system(void);

// Platform compatibility
int wine_compatibility_init(void);
int directx_support_init(void);
int steam_compatibility_init(void);
int epic_compatibility_init(void);
int gog_compatibility_init(void);

// Graphics API
int raeendx_api_init(void);
int directx_api_init(void);
int vulkan_api_init(void);
int opengl_api_init(void);

// Statistics
void gaming_system_get_stats(gaming_system_t* stats);
void gaming_system_reset_stats(void);

#endif // GAMING_SYSTEM_H 