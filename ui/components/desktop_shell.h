#ifndef DESKTOP_SHELL_H
#define DESKTOP_SHELL_H

#include "window_manager.h"
#include "../framework/include/ui_framework.h"
#include "types.h"

// Desktop shell states
typedef enum {
    DESKTOP_STATE_NORMAL,
    DESKTOP_STATE_LOCKED,
    DESKTOP_STATE_SLEEP,
    DESKTOP_STATE_SHUTDOWN
} desktop_state_t;

// Desktop shell configuration
typedef struct {
    bool show_desktop_icons;
    bool show_clock;
    bool show_system_tray;
    bool enable_animations;
    bool enable_effects;
    ui_color_t desktop_color;
    char wallpaper_path[256];
    u32 icon_size;
    u32 taskbar_height;
} desktop_shell_config_t;

// Desktop icon structure
typedef struct desktop_icon {
    char name[64];
    char path[256];
    char icon_path[256];
    ui_point_t position;
    bool selected;
    bool visible;
    struct desktop_icon* next;
} desktop_icon_t;

// System tray item structure
typedef struct system_tray_item {
    char name[64];
    char icon_path[256];
    bool visible;
    void (*on_click)(struct system_tray_item* item);
    void (*on_right_click)(struct system_tray_item* item);
    void* user_data;
    struct system_tray_item* next;
} system_tray_item_t;

// Desktop shell functions
error_t desktop_shell_init(const desktop_shell_config_t* config);
void desktop_shell_shutdown(void);

// Desktop management
void desktop_shell_set_wallpaper(const char* image_path);
void desktop_shell_set_desktop_color(ui_color_t color);
void desktop_shell_redraw_desktop(void);

// Desktop icons
desktop_icon_t* desktop_shell_create_icon(const char* name, const char* path, ui_point_t position);
void desktop_shell_destroy_icon(desktop_icon_t* icon);
void desktop_shell_move_icon(desktop_icon_t* icon, ui_point_t position);
void desktop_shell_select_icon(desktop_icon_t* icon);
void desktop_shell_deselect_all_icons(void);

// System tray
system_tray_item_t* desktop_shell_add_tray_item(const char* name, const char* icon_path);
void desktop_shell_remove_tray_item(system_tray_item_t* item);
void desktop_shell_show_tray_item(system_tray_item_t* item);
void desktop_shell_hide_tray_item(system_tray_item_t* item);

// Taskbar management
void desktop_shell_add_taskbar_button(const char* title, void (*on_click)(void));
void desktop_shell_remove_taskbar_button(const char* title);
void desktop_shell_update_taskbar_button(const char* title, bool active);

// Start menu
void desktop_shell_show_start_menu(void);
void desktop_shell_hide_start_menu(void);
bool desktop_shell_is_start_menu_visible(void);

// System operations
void desktop_shell_lock_screen(void);
void desktop_shell_unlock_screen(const char* password);
void desktop_shell_sleep(void);
void desktop_shell_shutdown(void);
void desktop_shell_restart(void);

// Event handling
bool desktop_shell_handle_mouse_event(ui_event_t* event);
bool desktop_shell_handle_keyboard_event(ui_event_t* event);
void desktop_shell_process_events(void);

// Rendering
void desktop_shell_render_frame(void);
void desktop_shell_invalidate_desktop(void);

// Configuration
void desktop_shell_set_config(const desktop_shell_config_t* config);
desktop_shell_config_t* desktop_shell_get_config(void);

// Utility functions
desktop_state_t desktop_shell_get_state(void);
bool desktop_shell_is_locked(void);
void desktop_shell_show_context_menu(ui_point_t position, const char** items, u32 count);

// Application launcher
void desktop_shell_launch_application(const char* path);
void desktop_shell_launch_file(const char* path);
void desktop_shell_open_folder(const char* path);

// Notification system
void desktop_shell_show_notification(const char* title, const char* message, u32 duration_ms);
void desktop_shell_hide_notification(u32 notification_id);

// Power management
void desktop_shell_set_power_save_mode(bool enabled);
bool desktop_shell_is_power_save_mode(void);
void desktop_shell_set_brightness(u8 brightness);
u8 desktop_shell_get_brightness(void);

// Volume control
void desktop_shell_set_volume(u8 volume);
u8 desktop_shell_get_volume(void);
void desktop_shell_mute_audio(bool mute);
bool desktop_shell_is_audio_muted(void);

// Network status
void desktop_shell_set_network_status(bool connected);
bool desktop_shell_is_network_connected(void);
void desktop_shell_show_network_menu(void);

// Clock and time
void desktop_shell_update_clock(void);
timestamp_t desktop_shell_get_current_time(void);
void desktop_shell_set_time_format(const char* format);

// File operations
void desktop_shell_copy_file(const char* source, const char* destination);
void desktop_shell_move_file(const char* source, const char* destination);
void desktop_shell_delete_file(const char* path);
void desktop_shell_create_folder(const char* path);

// Search functionality
void desktop_shell_show_search_dialog(void);
void desktop_shell_search_files(const char* query);
void desktop_shell_search_applications(const char* query);

// Accessibility
void desktop_shell_set_high_contrast_mode(bool enabled);
bool desktop_shell_is_high_contrast_mode(void);
void desktop_shell_set_large_text_mode(bool enabled);
bool desktop_shell_is_large_text_mode(void);

#endif // DESKTOP_SHELL_H 