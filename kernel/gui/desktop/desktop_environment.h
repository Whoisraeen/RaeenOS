#ifndef DESKTOP_ENVIRONMENT_H
#define DESKTOP_ENVIRONMENT_H

#include "../rendering/glass_compositor.h"
#include "../window/glass_window_manager.h"
#include <stdint.h>
#include <stdbool.h>

// Desktop Environment - Complete desktop experience for RaeenOS
// Provides window management, desktop icons, and system integration

// Desktop icon types
typedef enum {
    DESKTOP_ICON_APPLICATION = 0, // Application shortcut
    DESKTOP_ICON_FOLDER,          // Folder
    DESKTOP_ICON_FILE,            // File
    DESKTOP_ICON_DRIVE,           // Drive/volume
    DESKTOP_ICON_NETWORK,         // Network location
    DESKTOP_ICON_TRASH,           // Trash/recycle bin
    DESKTOP_ICON_CUSTOM           // Custom icon
} desktop_icon_type_t;

// Desktop icon states
typedef enum {
    ICON_STATE_NORMAL = 0,        // Normal state
    ICON_STATE_HOVER,             // Mouse hover
    ICON_STATE_SELECTED,          // Selected
    ICON_STATE_DRAGGING,          // Being dragged
    ICON_STATE_OPENING,           // Opening animation
    ICON_STATE_ERROR              // Error state
} desktop_icon_state_t;

// Desktop icon
typedef struct {
    uint32_t id;
    char name[64];
    char path[256];
    char icon_path[256];
    desktop_icon_type_t type;
    desktop_icon_state_t state;
    raeen_rect_t bounds;
    raeen_texture_t* icon_texture;
    raeen_texture_t* hover_texture;
    raeen_texture_t* selected_texture;
    bool visible;
    bool enabled;
    uint64_t last_accessed;
    uint64_t file_size;
    void* user_data;
} desktop_icon_t;

// Desktop wallpaper types
typedef enum {
    WALLPAPER_TYPE_STATIC = 0,    // Static image
    WALLPAPER_TYPE_DYNAMIC,       // Dynamic/changing
    WALLPAPER_TYPE_VIDEO,         // Video wallpaper
    WALLPAPER_TYPE_LIVE,          // Live wallpaper (particles, etc.)
    WALLPAPER_TYPE_SLIDESHOW      // Slideshow
} wallpaper_type_t;

// Desktop wallpaper
typedef struct {
    uint32_t id;
    char name[64];
    char path[256];
    wallpaper_type_t type;
    raeen_texture_t* texture;
    raeen_texture_t* video_frames[60]; // For video wallpapers
    uint32_t frame_count;
    uint32_t current_frame;
    float playback_speed;
    bool is_playing;
    bool is_looping;
    uint64_t last_frame_time;
    void* live_data;              // For live wallpapers
} desktop_wallpaper_t;

// Desktop workspace
typedef struct {
    uint32_t id;
    char name[64];
    raeen_rect_t bounds;
    desktop_icon_t* icons;
    uint32_t icon_count;
    uint32_t max_icons;
    desktop_wallpaper_t* wallpaper;
    bool is_active;
    bool is_visible;
} desktop_workspace_t;

// Desktop environment modes
typedef enum {
    DESKTOP_MODE_NORMAL = 0,      // Normal desktop mode
    DESKTOP_MODE_GAMING,          // Gaming mode (minimal UI)
    DESKTOP_MODE_PRESENTATION,    // Presentation mode
    DESKTOP_MODE_ACCESSIBILITY,   // Accessibility mode
    DESKTOP_MODE_SAFE             // Safe mode
} desktop_mode_t;

// Desktop environment configuration
typedef struct {
    uint32_t width, height;
    desktop_mode_t mode;
    bool enable_desktop_icons;
    bool enable_wallpapers;
    bool enable_animations;
    bool enable_glass_effects;
    bool enable_auto_arrange;
    bool enable_grid_snap;
    uint32_t icon_size;
    uint32_t icon_spacing;
    uint32_t max_workspaces;
    float wallpaper_fade_duration;
    bool enable_wallpaper_slideshow;
    uint32_t slideshow_interval;
    uint32_t background_color;
    uint32_t accent_color;
} desktop_environment_config_t;

// Desktop environment context
typedef struct {
    desktop_environment_config_t config;
    glass_compositor_t* compositor;
    glass_window_manager_t* window_manager;
    desktop_workspace_t* workspaces;
    uint32_t workspace_count;
    uint32_t max_workspaces;
    uint32_t active_workspace;
    desktop_icon_t* desktop_icons;
    uint32_t icon_count;
    uint32_t max_icons;
    desktop_wallpaper_t* current_wallpaper;
    bool initialized;
    bool icons_visible;
    bool wallpaper_visible;
    uint32_t next_icon_id;
    uint32_t next_workspace_id;
    uint64_t last_update_time;
} desktop_environment_t;

// Function prototypes

// Initialization and shutdown
desktop_environment_t* desktop_environment_init(glass_compositor_t* compositor, 
                                               glass_window_manager_t* window_manager,
                                               desktop_environment_config_t* config);
void desktop_environment_shutdown(desktop_environment_t* desktop);
bool desktop_environment_is_initialized(desktop_environment_t* desktop);

// Workspace management
uint32_t desktop_environment_create_workspace(desktop_environment_t* desktop, const char* name);
void desktop_environment_destroy_workspace(desktop_environment_t* desktop, uint32_t workspace_id);
void desktop_environment_switch_workspace(desktop_environment_t* desktop, uint32_t workspace_id);
uint32_t desktop_environment_get_active_workspace(desktop_environment_t* desktop);
desktop_workspace_t* desktop_environment_get_workspace(desktop_environment_t* desktop, uint32_t workspace_id);
uint32_t desktop_environment_get_workspace_count(desktop_environment_t* desktop);

// Desktop icon management
uint32_t desktop_environment_add_icon(desktop_environment_t* desktop, const char* name, 
                                     const char* path, const char* icon_path, desktop_icon_type_t type);
void desktop_environment_remove_icon(desktop_environment_t* desktop, uint32_t icon_id);
void desktop_environment_move_icon(desktop_environment_t* desktop, uint32_t icon_id, float x, float y);
void desktop_environment_rename_icon(desktop_environment_t* desktop, uint32_t icon_id, const char* new_name);
desktop_icon_t* desktop_environment_get_icon(desktop_environment_t* desktop, uint32_t icon_id);
uint32_t desktop_environment_get_icon_count(desktop_environment_t* desktop);

// Icon operations
void desktop_environment_open_icon(desktop_environment_t* desktop, uint32_t icon_id);
void desktop_environment_show_icon_context_menu(desktop_environment_t* desktop, uint32_t icon_id, float x, float y);
void desktop_environment_select_icon(desktop_environment_t* desktop, uint32_t icon_id, bool select);
void desktop_environment_select_icons_in_rect(desktop_environment_t* desktop, raeen_rect_t rect);
void desktop_environment_clear_selection(desktop_environment_t* desktop);
void desktop_environment_arrange_icons(desktop_environment_t* desktop);
void desktop_environment_auto_arrange_icons(desktop_environment_t* desktop);

// Wallpaper management
uint32_t desktop_environment_set_wallpaper(desktop_environment_t* desktop, const char* path, wallpaper_type_t type);
void desktop_environment_remove_wallpaper(desktop_environment_t* desktop);
desktop_wallpaper_t* desktop_environment_get_wallpaper(desktop_environment_t* desktop);
void desktop_environment_play_wallpaper(desktop_environment_t* desktop);
void desktop_environment_pause_wallpaper(desktop_environment_t* desktop);
void desktop_environment_set_wallpaper_speed(desktop_environment_t* desktop, float speed);

// Wallpaper slideshow
void desktop_environment_start_slideshow(desktop_environment_t* desktop, const char** paths, uint32_t count, uint32_t interval);
void desktop_environment_stop_slideshow(desktop_environment_t* desktop);
bool desktop_environment_is_slideshow_active(desktop_environment_t* desktop);
void desktop_environment_set_slideshow_interval(desktop_environment_t* desktop, uint32_t interval);

// Live wallpapers
uint32_t desktop_environment_create_live_wallpaper(desktop_environment_t* desktop, const char* name, void* live_data);
void desktop_environment_destroy_live_wallpaper(desktop_environment_t* desktop, uint32_t wallpaper_id);
void desktop_environment_update_live_wallpaper(desktop_environment_t* desktop, uint32_t wallpaper_id, void* data);

// Desktop operations
void desktop_environment_show_desktop(desktop_environment_t* desktop);
void desktop_environment_hide_desktop(desktop_environment_t* desktop);
void desktop_environment_toggle_desktop(desktop_environment_t* desktop);
void desktop_environment_refresh_desktop(desktop_environment_t* desktop);
void desktop_environment_clear_desktop(desktop_environment_t* desktop);

// Desktop mode management
void desktop_environment_set_mode(desktop_environment_t* desktop, desktop_mode_t mode);
desktop_mode_t desktop_environment_get_mode(desktop_environment_t* desktop);
void desktop_environment_optimize_for_gaming(desktop_environment_t* desktop);
void desktop_environment_optimize_for_presentation(desktop_environment_t* desktop);
void desktop_environment_optimize_for_accessibility(desktop_environment_t* desktop);

// Rendering
void desktop_environment_render(desktop_environment_t* desktop);
void desktop_environment_render_wallpaper(desktop_environment_t* desktop);
void desktop_environment_render_icons(desktop_environment_t* desktop);
void desktop_environment_render_selection(desktop_environment_t* desktop);
void desktop_environment_update(desktop_environment_t* desktop, float delta_time);

// Input handling
bool desktop_environment_handle_mouse_move(desktop_environment_t* desktop, float x, float y);
bool desktop_environment_handle_mouse_click(desktop_environment_t* desktop, float x, float y, bool left_click);
bool desktop_environment_handle_mouse_double_click(desktop_environment_t* desktop, float x, float y);
bool desktop_environment_handle_mouse_drag(desktop_environment_t* desktop, float x, float y, bool start_drag);
bool desktop_environment_handle_mouse_wheel(desktop_environment_t* desktop, float x, float y, float delta);
bool desktop_environment_handle_key_press(desktop_environment_t* desktop, uint32_t key_code);

// Icon interaction
uint32_t desktop_environment_get_icon_at_point(desktop_environment_t* desktop, float x, float y);
void desktop_environment_highlight_icon(desktop_environment_t* desktop, uint32_t icon_id, bool highlight);
void desktop_environment_animate_icon(desktop_environment_t* desktop, uint32_t icon_id, desktop_icon_state_t state);
void desktop_environment_show_icon_tooltip(desktop_environment_t* desktop, uint32_t icon_id, float x, float y);

// File operations
void desktop_environment_create_folder(desktop_environment_t* desktop, const char* name);
void desktop_environment_create_shortcut(desktop_environment_t* desktop, const char* target_path, const char* name);
void desktop_environment_copy_files(desktop_environment_t* desktop, const char** source_paths, uint32_t count, const char* dest_path);
void desktop_environment_move_files(desktop_environment_t* desktop, const char** source_paths, uint32_t count, const char* dest_path);
void desktop_environment_delete_files(desktop_environment_t* desktop, const char** paths, uint32_t count);

// System integration
void desktop_environment_show_context_menu(desktop_environment_t* desktop, float x, float y);
void desktop_environment_show_properties(desktop_environment_t* desktop, uint32_t icon_id);
void desktop_environment_show_desktop_settings(desktop_environment_t* desktop);
void desktop_environment_show_display_settings(desktop_environment_t* desktop);
void desktop_environment_show_personalization(desktop_environment_t* desktop);

// Configuration
void desktop_environment_set_icon_size(desktop_environment_t* desktop, uint32_t size);
uint32_t desktop_environment_get_icon_size(desktop_environment_t* desktop);
void desktop_environment_set_icon_spacing(desktop_environment_t* desktop, uint32_t spacing);
uint32_t desktop_environment_get_icon_spacing(desktop_environment_t* desktop);
void desktop_environment_set_background_color(desktop_environment_t* desktop, uint32_t color);
uint32_t desktop_environment_get_background_color(desktop_environment_t* desktop);
void desktop_environment_set_accent_color(desktop_environment_t* desktop, uint32_t color);
uint32_t desktop_environment_get_accent_color(desktop_environment_t* desktop);

// Information
bool desktop_environment_are_icons_visible(desktop_environment_t* desktop);
bool desktop_environment_is_wallpaper_visible(desktop_environment_t* desktop);
raeen_rect_t desktop_environment_get_bounds(desktop_environment_t* desktop);
uint32_t desktop_environment_get_selected_icon_count(desktop_environment_t* desktop);
desktop_icon_t* desktop_environment_get_selected_icons(desktop_environment_t* desktop, uint32_t* count);

// Utility functions
void desktop_environment_center_icons(desktop_environment_t* desktop);
void desktop_environment_align_icons_to_grid(desktop_environment_t* desktop);
void desktop_environment_sort_icons_by_name(desktop_environment_t* desktop);
void desktop_environment_sort_icons_by_type(desktop_environment_t* desktop);
void desktop_environment_sort_icons_by_date(desktop_environment_t* desktop);
void desktop_environment_sort_icons_by_size(desktop_environment_t* desktop);

// Callbacks
typedef void (*desktop_icon_click_callback_t)(desktop_environment_t* desktop, uint32_t icon_id, void* user_data);
typedef void (*desktop_icon_double_click_callback_t)(desktop_environment_t* desktop, uint32_t icon_id, void* user_data);
typedef void (*desktop_icon_drag_callback_t)(desktop_environment_t* desktop, uint32_t icon_id, float x, float y, void* user_data);
typedef void (*desktop_workspace_change_callback_t)(desktop_environment_t* desktop, uint32_t old_workspace, uint32_t new_workspace, void* user_data);

void desktop_environment_set_icon_click_callback(desktop_environment_t* desktop, desktop_icon_click_callback_t callback, void* user_data);
void desktop_environment_set_icon_double_click_callback(desktop_environment_t* desktop, desktop_icon_double_click_callback_t callback, void* user_data);
void desktop_environment_set_icon_drag_callback(desktop_environment_t* desktop, desktop_icon_drag_callback_t callback, void* user_data);
void desktop_environment_set_workspace_change_callback(desktop_environment_t* desktop, desktop_workspace_change_callback_t callback, void* user_data);

// Preset configurations
desktop_environment_config_t desktop_environment_preset_normal_style(void);
desktop_environment_config_t desktop_environment_preset_gaming_style(void);
desktop_environment_config_t desktop_environment_preset_minimal_style(void);
desktop_environment_config_t desktop_environment_preset_accessibility_style(void);

// Error handling
typedef enum {
    DESKTOP_ENVIRONMENT_SUCCESS = 0,
    DESKTOP_ENVIRONMENT_ERROR_INVALID_CONTEXT,
    DESKTOP_ENVIRONMENT_ERROR_INVALID_ICON,
    DESKTOP_ENVIRONMENT_ERROR_INVALID_WORKSPACE,
    DESKTOP_ENVIRONMENT_ERROR_OUT_OF_MEMORY,
    DESKTOP_ENVIRONMENT_ERROR_INVALID_CONFIG,
    DESKTOP_ENVIRONMENT_ERROR_RENDER_FAILED,
} desktop_environment_error_t;

desktop_environment_error_t desktop_environment_get_last_error(void);
const char* desktop_environment_get_error_string(desktop_environment_error_t error);

#endif // DESKTOP_ENVIRONMENT_H 