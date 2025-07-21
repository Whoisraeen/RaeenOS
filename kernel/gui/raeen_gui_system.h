#ifndef RAEEN_GUI_SYSTEM_H
#define RAEEN_GUI_SYSTEM_H

#include "rendering/glass_compositor.h"
#include "desktop/hybrid_dock.h"
#include "window/glass_window_manager.h"
#include "search/spotlight_plus.h"
#include "customizer/raeen_customizer.h"
#include <stdint.h>
#include <stdbool.h>

// RaeenOS GUI System - Complete desktop environment
// Orchestrates all GUI components into a cohesive system

// GUI system states
typedef enum {
    GUI_STATE_INITIALIZING = 0,  // System is initializing
    GUI_STATE_LOADING,           // Loading components
    GUI_STATE_READY,             // Ready for user interaction
    GUI_STATE_RUNNING,           // Normal operation
    GUI_STATE_SLEEPING,          // System is sleeping
    GUI_STATE_SHUTDOWN           // System is shutting down
} gui_system_state_t;

// Display modes
typedef enum {
    DISPLAY_MODE_NORMAL = 0,     // Normal desktop mode
    DISPLAY_MODE_GAMING,         // Gaming mode (performance optimized)
    DISPLAY_MODE_PRESENTATION,   // Presentation mode
    DISPLAY_MODE_ACCESSIBILITY,  // Accessibility mode
    DISPLAY_MODE_SAFE            // Safe mode (minimal features)
} display_mode_t;

// Performance profiles
typedef enum {
    PERFORMANCE_PROFILE_POWER_SAVER = 0, // Power saving mode
    PERFORMANCE_PROFILE_BALANCED,        // Balanced performance
    PERFORMANCE_PROFILE_HIGH_PERFORMANCE, // High performance
    PERFORMANCE_PROFILE_GAMING,          // Gaming optimized
    PERFORMANCE_PROFILE_CUSTOM           // Custom profile
} performance_profile_t;

// GUI system configuration
typedef struct {
    uint32_t display_width, display_height;
    uint32_t refresh_rate;
    display_mode_t display_mode;
    performance_profile_t performance_profile;
    bool enable_hardware_acceleration;
    bool enable_vsync;
    bool enable_glass_effects;
    bool enable_animations;
    bool enable_ai_features;
    bool enable_voice_control;
    bool enable_gesture_control;
    bool enable_accessibility;
    uint32_t max_windows;
    uint32_t max_processes;
    float target_fps;
    uint32_t memory_limit_mb;
} gui_system_config_t;

// System metrics
typedef struct {
    float cpu_usage;
    float memory_usage;
    float gpu_usage;
    float disk_usage;
    float network_usage;
    float temperature;
    uint32_t fps;
    uint32_t frame_time_ms;
    uint32_t active_windows;
    uint32_t active_processes;
    uint64_t uptime_seconds;
} system_metrics_t;

// GUI system context
typedef struct {
    gui_system_config_t config;
    gui_system_state_t state;
    display_mode_t current_display_mode;
    performance_profile_t current_performance_profile;
    
    // Core components
    glass_compositor_t* compositor;
    hybrid_dock_t* dock;
    glass_window_manager_t* window_manager;
    spotlight_plus_t* search;
    raeen_customizer_t* customizer;
    
    // System metrics
    system_metrics_t metrics;
    uint64_t last_metrics_update;
    
    // Performance monitoring
    uint64_t frame_count;
    uint64_t last_frame_time;
    float average_fps;
    uint32_t frame_time_history[60];
    uint32_t frame_time_index;
    
    // Event handling
    bool mouse_grabbed;
    bool keyboard_grabbed;
    uint32_t last_input_time;
    
    // Initialization flags
    bool compositor_initialized;
    bool dock_initialized;
    bool window_manager_initialized;
    bool search_initialized;
    bool customizer_initialized;
    
    // Error handling
    uint32_t error_count;
    char last_error[256];
    
    // User preferences
    bool auto_hide_dock;
    bool show_desktop_icons;
    bool enable_notifications;
    bool enable_sound_effects;
    bool enable_visual_feedback;
    
    // Callback data
    void* user_data;
} raeen_gui_system_t;

// Function prototypes

// Initialization and shutdown
raeen_gui_system_t* raeen_gui_system_init(gui_system_config_t* config);
void raeen_gui_system_shutdown(raeen_gui_system_t* gui);
bool raeen_gui_system_is_initialized(raeen_gui_system_t* gui);

// System lifecycle
void raeen_gui_system_start(raeen_gui_system_t* gui);
void raeen_gui_system_stop(raeen_gui_system_t* gui);
void raeen_gui_system_pause(raeen_gui_system_t* gui);
void raeen_gui_system_resume(raeen_gui_system_t* gui);
void raeen_gui_system_sleep(raeen_gui_system_t* gui);
void raeen_gui_system_wake(raeen_gui_system_t* gui);

// Main loop
void raeen_gui_system_main_loop(raeen_gui_system_t* gui);
void raeen_gui_system_update(raeen_gui_system_t* gui, float delta_time);
void raeen_gui_system_render(raeen_gui_system_t* gui);
void raeen_gui_system_present(raeen_gui_system_t* gui);

// Display management
void raeen_gui_system_set_display_mode(raeen_gui_system_t* gui, display_mode_t mode);
void raeen_gui_system_set_resolution(raeen_gui_system_t* gui, uint32_t width, uint32_t height);
void raeen_gui_system_set_refresh_rate(raeen_gui_system_t* gui, uint32_t refresh_rate);
void raeen_gui_system_set_fullscreen(raeen_gui_system_t* gui, bool fullscreen);
void raeen_gui_system_set_vsync(raeen_gui_system_t* gui, bool enable);

// Performance management
void raeen_gui_system_set_performance_profile(raeen_gui_system_t* gui, performance_profile_t profile);
void raeen_gui_system_optimize_for_gaming(raeen_gui_system_t* gui);
void raeen_gui_system_optimize_for_power(raeen_gui_system_t* gui);
void raeen_gui_system_optimize_for_accessibility(raeen_gui_system_t* gui);
void raeen_gui_system_set_memory_limit(raeen_gui_system_t* gui, uint32_t limit_mb);

// Component access
glass_compositor_t* raeen_gui_system_get_compositor(raeen_gui_system_t* gui);
hybrid_dock_t* raeen_gui_system_get_dock(raeen_gui_system_t* gui);
glass_window_manager_t* raeen_gui_system_get_window_manager(raeen_gui_system_t* gui);
spotlight_plus_t* raeen_gui_system_get_search(raeen_gui_system_t* gui);
raeen_customizer_t* raeen_gui_system_get_customizer(raeen_gui_system_t* gui);

// Input handling
bool raeen_gui_system_handle_mouse_move(raeen_gui_system_t* gui, float x, float y);
bool raeen_gui_system_handle_mouse_click(raeen_gui_system_t* gui, float x, float y, bool left_click);
bool raeen_gui_system_handle_mouse_wheel(raeen_gui_system_t* gui, float x, float y, float delta);
bool raeen_gui_system_handle_key_press(raeen_gui_system_t* gui, uint32_t key_code);
bool raeen_gui_system_handle_text_input(raeen_gui_system_t* gui, const char* text);
bool raeen_gui_system_handle_gesture(raeen_gui_system_t* gui, uint32_t gesture_type, float x, float y);
bool raeen_gui_system_handle_voice_command(raeen_gui_system_t* gui, const char* command);

// System operations
void raeen_gui_system_show_desktop(raeen_gui_system_t* gui);
void raeen_gui_system_minimize_all_windows(raeen_gui_system_t* gui);
void raeen_gui_system_restore_all_windows(raeen_gui_system_t* gui);
void raeen_gui_system_switch_desktop(raeen_gui_system_t* gui, uint32_t desktop_id);
void raeen_gui_system_create_desktop(raeen_gui_system_t* gui);
void raeen_gui_system_remove_desktop(raeen_gui_system_t* gui, uint32_t desktop_id);

// Search and navigation
void raeen_gui_system_show_search(raeen_gui_system_t* gui);
void raeen_gui_system_hide_search(raeen_gui_system_t* gui);
void raeen_gui_system_toggle_search(raeen_gui_system_t* gui);
void raeen_gui_system_show_task_view(raeen_gui_system_t* gui);
void raeen_gui_system_hide_task_view(raeen_gui_system_t* gui);
void raeen_gui_system_show_notification_center(raeen_gui_system_t* gui);
void raeen_gui_system_hide_notification_center(raeen_gui_system_t* gui);

// Customization
void raeen_gui_system_show_customizer(raeen_gui_system_t* gui);
void raeen_gui_system_hide_customizer(raeen_gui_system_t* gui);
void raeen_gui_system_toggle_customizer(raeen_gui_system_t* gui);
void raeen_gui_system_apply_theme(raeen_gui_system_t* gui, uint32_t theme_id);
void raeen_gui_system_reset_theme(raeen_gui_system_t* gui);

// System information
gui_system_state_t raeen_gui_system_get_state(raeen_gui_system_t* gui);
display_mode_t raeen_gui_system_get_display_mode(raeen_gui_system_t* gui);
performance_profile_t raeen_gui_system_get_performance_profile(raeen_gui_system_t* gui);
system_metrics_t* raeen_gui_system_get_metrics(raeen_gui_system_t* gui);
float raeen_gui_system_get_fps(raeen_gui_system_t* gui);
uint64_t raeen_gui_system_get_uptime(raeen_gui_system_t* gui);

// Error handling
uint32_t raeen_gui_system_get_error_count(raeen_gui_system_t* gui);
const char* raeen_gui_system_get_last_error(raeen_gui_system_t* gui);
void raeen_gui_system_clear_errors(raeen_gui_system_t* gui);

// Utility functions
void raeen_gui_system_take_screenshot(raeen_gui_system_t* gui, const char* file_path);
void raeen_gui_system_record_screen(raeen_gui_system_t* gui, const char* file_path, uint32_t duration_seconds);
void raeen_gui_system_export_debug_info(raeen_gui_system_t* gui, const char* file_path);
void raeen_gui_system_optimize_memory(raeen_gui_system_t* gui);
void raeen_gui_system_cleanup_resources(raeen_gui_system_t* gui);

// Hotkeys and shortcuts
void raeen_gui_system_register_hotkey(raeen_gui_system_t* gui, uint32_t key_code, uint32_t modifiers, void (*callback)(void*), void* user_data);
void raeen_gui_system_unregister_hotkey(raeen_gui_system_t* gui, uint32_t key_code, uint32_t modifiers);
void raeen_gui_system_show_hotkey_help(raeen_gui_system_t* gui);

// Accessibility
void raeen_gui_system_enable_high_contrast(raeen_gui_system_t* gui, bool enable);
void raeen_gui_system_enable_large_text(raeen_gui_system_t* gui, bool enable);
void raeen_gui_system_enable_screen_reader(raeen_gui_system_t* gui, bool enable);
void raeen_gui_system_enable_magnifier(raeen_gui_system_t* gui, bool enable);
void raeen_gui_system_enable_color_blind_support(raeen_gui_system_t* gui, bool enable);

// Callbacks
typedef void (*gui_system_state_change_callback_t)(raeen_gui_system_t* gui, gui_system_state_t old_state, gui_system_state_t new_state, void* user_data);
typedef void (*gui_system_error_callback_t)(raeen_gui_system_t* gui, const char* error_message, void* user_data);
typedef void (*gui_system_performance_callback_t)(raeen_gui_system_t* gui, system_metrics_t* metrics, void* user_data);

void raeen_gui_system_set_state_change_callback(raeen_gui_system_t* gui, gui_system_state_change_callback_t callback, void* user_data);
void raeen_gui_system_set_error_callback(raeen_gui_system_t* gui, gui_system_error_callback_t callback, void* user_data);
void raeen_gui_system_set_performance_callback(raeen_gui_system_t* gui, gui_system_performance_callback_t callback, void* user_data);

// Preset configurations
gui_system_config_t raeen_gui_system_preset_desktop_style(void);
gui_system_config_t raeen_gui_system_preset_gaming_style(void);
gui_system_config_t raeen_gui_system_preset_minimal_style(void);
gui_system_config_t raeen_gui_system_preset_accessibility_style(void);

// Error handling
typedef enum {
    RAEEN_GUI_SYSTEM_SUCCESS = 0,
    RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT,
    RAEEN_GUI_SYSTEM_ERROR_INITIALIZATION_FAILED,
    RAEEN_GUI_SYSTEM_ERROR_OUT_OF_MEMORY,
    RAEEN_GUI_SYSTEM_ERROR_RENDER_FAILED,
    RAEEN_GUI_SYSTEM_ERROR_INPUT_FAILED,
    RAEEN_GUI_SYSTEM_ERROR_COMPONENT_FAILED,
} raeen_gui_system_error_t;

raeen_gui_system_error_t raeen_gui_system_get_last_error_code(raeen_gui_system_t* gui);
const char* raeen_gui_system_get_error_string(raeen_gui_system_error_t error);

#endif // RAEEN_GUI_SYSTEM_H 