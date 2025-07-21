#include "desktop_shell.h"
#include "kernel.h"
#include <string.h>

// Desktop shell global state
static bool desktop_shell_initialized = false;
static desktop_shell_config_t desktop_shell_config = {0};
static desktop_state_t desktop_state = DESKTOP_STATE_NORMAL;
static desktop_icon_t* desktop_icons = NULL;
static system_tray_item_t* system_tray_items = NULL;
static window_manager_window_t* start_menu_window = NULL;
static bool start_menu_visible = false;
static bool screen_locked = false;
static u8 system_volume = 50;
static bool audio_muted = false;
static bool network_connected = false;
static bool power_save_mode = false;
static u8 screen_brightness = 100;

// Default configuration
static const desktop_shell_config_t default_config = {
    .show_desktop_icons = true,
    .show_clock = true,
    .show_system_tray = true,
    .enable_animations = true,
    .enable_effects = true,
    .desktop_color = {45, 45, 45, 255},
    .wallpaper_path = "",
    .icon_size = 48,
    .taskbar_height = 40
};

error_t desktop_shell_init(const desktop_shell_config_t* config) {
    if (desktop_shell_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing Desktop Shell");
    
    // Set configuration
    if (config) {
        desktop_shell_config = *config;
    } else {
        desktop_shell_config = default_config;
    }
    
    // Initialize window manager if not already done
    if (!window_manager_is_initialized()) {
        window_manager_config_t wm_config = {
            .compositing_enabled = true,
            .window_animations = desktop_shell_config.enable_animations,
            .desktop_effects = desktop_shell_config.enable_effects,
            .animation_duration_ms = 200,
            .default_decoration = WINDOW_DECORATION_FULL,
            .desktop_background = desktop_shell_config.desktop_color,
            .taskbar_color = {60, 60, 60, 255}
        };
        
        error_t result = window_manager_init(&wm_config);
        if (result != SUCCESS) {
            KERROR("Failed to initialize window manager");
            return result;
        }
    }
    
    // Create default desktop icons
    if (desktop_shell_config.show_desktop_icons) {
        desktop_shell_create_icon("Computer", "/system/computer", ui_point(50, 50));
        desktop_shell_create_icon("Documents", "/home/documents", ui_point(50, 120));
        desktop_shell_create_icon("Downloads", "/home/downloads", ui_point(50, 190));
        desktop_shell_create_icon("Trash", "/system/trash", ui_point(50, 260));
    }
    
    // Create system tray items
    if (desktop_shell_config.show_system_tray) {
        desktop_shell_add_tray_item("Volume", "/system/icons/volume.png");
        desktop_shell_add_tray_item("Network", "/system/icons/network.png");
        desktop_shell_add_tray_item("Power", "/system/icons/power.png");
        desktop_shell_add_tray_item("Clock", "/system/icons/clock.png");
    }
    
    desktop_shell_initialized = true;
    KINFO("Desktop Shell initialized successfully");
    
    return SUCCESS;
}

void desktop_shell_shutdown(void) {
    if (!desktop_shell_initialized) {
        return;
    }
    
    KINFO("Shutting down Desktop Shell");
    
    // Destroy desktop icons
    desktop_icon_t* icon = desktop_icons;
    while (icon) {
        desktop_icon_t* next = icon->next;
        desktop_shell_destroy_icon(icon);
        icon = next;
    }
    
    // Destroy system tray items
    system_tray_item_t* tray_item = system_tray_items;
    while (tray_item) {
        system_tray_item_t* next = tray_item->next;
        memory_free(tray_item);
        tray_item = next;
    }
    
    // Hide start menu
    if (start_menu_window) {
        window_manager_destroy_window(start_menu_window);
        start_menu_window = NULL;
    }
    
    desktop_shell_initialized = false;
}

void desktop_shell_set_wallpaper(const char* image_path) {
    if (!image_path) {
        return;
    }
    
    strncpy(desktop_shell_config.wallpaper_path, image_path, sizeof(desktop_shell_config.wallpaper_path) - 1);
    
    // TODO: Load and display wallpaper image
    KINFO("Set wallpaper: %s", image_path);
}

void desktop_shell_set_desktop_color(ui_color_t color) {
    desktop_shell_config.desktop_color = color;
    
    // Update window manager desktop background
    window_manager_config_t wm_config = *window_manager_get_config();
    wm_config.desktop_background = color;
    window_manager_set_config(&wm_config);
    
    desktop_shell_redraw_desktop();
}

desktop_icon_t* desktop_shell_create_icon(const char* name, const char* path, ui_point_t position) {
    if (!name || !path) {
        return NULL;
    }
    
    desktop_icon_t* icon = memory_alloc(sizeof(desktop_icon_t));
    if (!icon) {
        return NULL;
    }
    
    memset(icon, 0, sizeof(desktop_icon_t));
    strncpy(icon->name, name, sizeof(icon->name) - 1);
    strncpy(icon->path, path, sizeof(icon->path) - 1);
    icon->position = position;
    icon->visible = true;
    icon->selected = false;
    
    // Add to icon list
    icon->next = desktop_icons;
    desktop_icons = icon;
    
    KINFO("Created desktop icon: %s at (%d, %d)", name, position.x, position.y);
    
    return icon;
}

void desktop_shell_destroy_icon(desktop_icon_t* icon) {
    if (!icon) {
        return;
    }
    
    // Remove from icon list
    if (desktop_icons == icon) {
        desktop_icons = icon->next;
    } else {
        desktop_icon_t* current = desktop_icons;
        while (current && current->next != icon) {
            current = current->next;
        }
        if (current) {
            current->next = icon->next;
        }
    }
    
    memory_free(icon);
}

void desktop_shell_select_icon(desktop_icon_t* icon) {
    if (!icon) {
        return;
    }
    
    // Deselect all other icons
    desktop_shell_deselect_all_icons();
    
    icon->selected = true;
    desktop_shell_invalidate_desktop();
}

void desktop_shell_deselect_all_icons(void) {
    desktop_icon_t* icon = desktop_icons;
    while (icon) {
        icon->selected = false;
        icon = icon->next;
    }
    desktop_shell_invalidate_desktop();
}

system_tray_item_t* desktop_shell_add_tray_item(const char* name, const char* icon_path) {
    if (!name) {
        return NULL;
    }
    
    system_tray_item_t* item = memory_alloc(sizeof(system_tray_item_t));
    if (!item) {
        return NULL;
    }
    
    memset(item, 0, sizeof(system_tray_item_t));
    strncpy(item->name, name, sizeof(item->name) - 1);
    if (icon_path) {
        strncpy(item->icon_path, icon_path, sizeof(item->icon_path) - 1);
    }
    item->visible = true;
    
    // Add to tray item list
    item->next = system_tray_items;
    system_tray_items = item;
    
    KINFO("Added system tray item: %s", name);
    
    return item;
}

void desktop_shell_show_start_menu(void) {
    if (start_menu_visible) {
        return;
    }
    
    // Create start menu window if it doesn't exist
    if (!start_menu_window) {
        ui_rect_t menu_bounds = ui_rect(0, 1000, 300, 400); // Bottom left
        start_menu_window = window_manager_create_window("Start Menu", menu_bounds);
        if (!start_menu_window) {
            return;
        }
        
        start_menu_window->is_system_window = true;
        start_menu_window->resizable = false;
        start_menu_window->movable = false;
        start_menu_window->closeable = false;
        start_menu_window->minimizable = false;
        start_menu_window->maximizable = false;
    }
    
    window_manager_show_window(start_menu_window);
    start_menu_visible = true;
}

void desktop_shell_hide_start_menu(void) {
    if (!start_menu_visible || !start_menu_window) {
        return;
    }
    
    window_manager_hide_window(start_menu_window);
    start_menu_visible = false;
}

bool desktop_shell_is_start_menu_visible(void) {
    return start_menu_visible;
}

void desktop_shell_lock_screen(void) {
    if (screen_locked) {
        return;
    }
    
    screen_locked = true;
    desktop_state = DESKTOP_STATE_LOCKED;
    
    // Create lock screen window
    ui_rect_t lock_bounds = ui_rect(0, 0, 1920, 1080);
    window_manager_window_t* lock_window = window_manager_create_window("Lock Screen", lock_bounds);
    if (lock_window) {
        lock_window->is_system_window = true;
        lock_window->resizable = false;
        lock_window->movable = false;
        lock_window->closeable = false;
        lock_window->minimizable = false;
        lock_window->maximizable = false;
        window_manager_show_window(lock_window);
    }
    
    KINFO("Screen locked");
}

void desktop_shell_unlock_screen(const char* password) {
    if (!screen_locked) {
        return;
    }
    
    // TODO: Verify password
    // For now, just unlock
    screen_locked = false;
    desktop_state = DESKTOP_STATE_NORMAL;
    
    // Hide lock screen
    window_manager_window_t* lock_window = window_manager_find_window_by_title("Lock Screen");
    if (lock_window) {
        window_manager_destroy_window(lock_window);
    }
    
    KINFO("Screen unlocked");
}

bool desktop_shell_handle_mouse_event(ui_event_t* event) {
    if (!event) {
        return false;
    }
    
    switch (event->type) {
        case UI_EVENT_MOUSE_DOWN: {
            // Check if click is on desktop icon
            desktop_icon_t* icon = desktop_icons;
            while (icon) {
                ui_rect_t icon_bounds = ui_rect(
                    icon->position.x,
                    icon->position.y,
                    desktop_shell_config.icon_size,
                    desktop_shell_config.icon_size
                );
                
                if (ui_point_in_rect(event->mouse_button.position, icon_bounds)) {
                    desktop_shell_select_icon(icon);
                    
                    // Double-click to launch
                    if (event->mouse_button.button == UI_MOUSE_LEFT) {
                        desktop_shell_launch_application(icon->path);
                    }
                    return true;
                }
                icon = icon->next;
            }
            
            // Deselect icons if clicking on empty space
            desktop_shell_deselect_all_icons();
            break;
        }
        
        case UI_EVENT_MOUSE_DOUBLE_CLICK: {
            // Handle double-click on desktop
            break;
        }
        
        default:
            break;
    }
    
    return false;
}

void desktop_shell_render_frame(void) {
    if (!desktop_shell_initialized) {
        return;
    }
    
    // Render desktop icons
    if (desktop_shell_config.show_desktop_icons) {
        desktop_icon_t* icon = desktop_icons;
        while (icon) {
            if (icon->visible) {
                // TODO: Render icon with selection state
                // This would draw the icon image and text
            }
            icon = icon->next;
        }
    }
    
    // Render system tray
    if (desktop_shell_config.show_system_tray) {
        system_tray_item_t* item = system_tray_items;
        while (item) {
            if (item->visible) {
                // TODO: Render tray item icon
            }
            item = item->next;
        }
    }
    
    // Render clock if enabled
    if (desktop_shell_config.show_clock) {
        desktop_shell_update_clock();
    }
    
    // Let window manager render the frame
    window_manager_render_frame();
}

void desktop_shell_launch_application(const char* path) {
    if (!path) {
        return;
    }
    
    KINFO("Launching application: %s", path);
    
    // TODO: Implement application launching
    // This would involve:
    // 1. Loading the executable
    // 2. Creating a new process
    // 3. Creating a window for the application
    // 4. Adding to taskbar
}

void desktop_shell_set_volume(u8 volume) {
    system_volume = volume > 100 ? 100 : volume;
    
    // TODO: Set actual system volume
    KINFO("Set volume to %u%%", system_volume);
}

u8 desktop_shell_get_volume(void) {
    return system_volume;
}

void desktop_shell_mute_audio(bool mute) {
    audio_muted = mute;
    
    // TODO: Mute/unmute actual audio
    KINFO("Audio %s", mute ? "muted" : "unmuted");
}

bool desktop_shell_is_audio_muted(void) {
    return audio_muted;
}

void desktop_shell_set_network_status(bool connected) {
    network_connected = connected;
    
    // TODO: Update network tray icon
    KINFO("Network %s", connected ? "connected" : "disconnected");
}

bool desktop_shell_is_network_connected(void) {
    return network_connected;
}

void desktop_shell_update_clock(void) {
    // TODO: Update clock display in taskbar
    // This would get current time and format it
}

timestamp_t desktop_shell_get_current_time(void) {
    // TODO: Get current system time
    return 0; // Placeholder
}

desktop_state_t desktop_shell_get_state(void) {
    return desktop_state;
}

bool desktop_shell_is_locked(void) {
    return screen_locked;
}

void desktop_shell_invalidate_desktop(void) {
    // TODO: Mark desktop area for redraw
    // This would invalidate the desktop background and icons
} 