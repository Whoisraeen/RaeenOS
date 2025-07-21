#include "window_manager.h"
#include "../framework/ui_core.c"
#include "kernel.h"
#include <string.h>

// Window manager global state
static bool window_manager_initialized = false;
static window_manager_config_t window_manager_config = {0};
static window_manager_window_t* window_list_head = NULL;
static window_manager_window_t* window_list_tail = NULL;
static window_manager_window_t* focused_window = NULL;
static window_manager_window_t* desktop_window = NULL;
static window_manager_window_t* taskbar_window = NULL;
static u32 next_window_id = 1;

// Default configuration
static const window_manager_config_t default_config = {
    .compositing_enabled = true,
    .window_animations = true,
    .desktop_effects = true,
    .animation_duration_ms = 200,
    .default_decoration = WINDOW_DECORATION_FULL,
    .desktop_background = {45, 45, 45, 255},
    .taskbar_color = {60, 60, 60, 255}
};

error_t window_manager_init(const window_manager_config_t* config) {
    if (window_manager_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing Window Manager");
    
    // Set configuration
    if (config) {
        window_manager_config = *config;
    } else {
        window_manager_config = default_config;
    }
    
    // Create desktop window
    desktop_window = window_manager_create_window("Desktop", ui_rect(0, 0, 1920, 1080));
    if (!desktop_window) {
        KERROR("Failed to create desktop window");
        return E_NOMEM;
    }
    
    desktop_window->is_desktop = true;
    desktop_window->is_system_window = true;
    desktop_window->ui_window->base.background_color = window_manager_config.desktop_background;
    
    // Create taskbar
    taskbar_window = window_manager_create_taskbar();
    if (!taskbar_window) {
        KERROR("Failed to create taskbar");
        return E_NOMEM;
    }
    
    window_manager_initialized = true;
    KINFO("Window Manager initialized successfully");
    
    return SUCCESS;
}

void window_manager_shutdown(void) {
    if (!window_manager_initialized) {
        return;
    }
    
    KINFO("Shutting down Window Manager");
    
    // Destroy all windows
    window_manager_window_t* window = window_list_head;
    while (window) {
        window_manager_window_t* next = window->next;
        window_manager_destroy_window(window);
        window = next;
    }
    
    window_list_head = NULL;
    window_list_tail = NULL;
    focused_window = NULL;
    desktop_window = NULL;
    taskbar_window = NULL;
    
    window_manager_initialized = false;
}

window_manager_window_t* window_manager_create_window(const char* title, ui_rect_t bounds) {
    if (!window_manager_initialized) {
        return NULL;
    }
    
    // Create UI window
    ui_window_t* ui_window = ui_create_window(title, bounds);
    if (!ui_window) {
        return NULL;
    }
    
    // Create window manager window
    window_manager_window_t* window = memory_alloc(sizeof(window_manager_window_t));
    if (!window) {
        ui_destroy_window(ui_window);
        return NULL;
    }
    
    // Initialize window
    memset(window, 0, sizeof(window_manager_window_t));
    window->ui_window = ui_window;
    window->state = WINDOW_STATE_NORMAL;
    window->decoration = window_manager_config.default_decoration;
    window->window_id = next_window_id++;
    
    strncpy(window->title, title, sizeof(window->title) - 1);
    window->original_bounds = bounds;
    window->current_bounds = bounds;
    window->resizable = true;
    window->movable = true;
    window->closeable = true;
    window->minimizable = true;
    window->maximizable = true;
    
    // Add to window list
    if (!window_list_head) {
        window_list_head = window;
        window_list_tail = window;
    } else {
        window->prev = window_list_tail;
        window_list_tail->next = window;
        window_list_tail = window;
    }
    
    // Set event handlers
    ui_set_widget_event_handler(&ui_window->base, window_manager_window_event_handler);
    
    KINFO("Created window: %s (ID: %u)", title, window->window_id);
    
    return window;
}

void window_manager_destroy_window(window_manager_window_t* window) {
    if (!window || !window_manager_initialized) {
        return;
    }
    
    // Remove from window list
    if (window->prev) {
        window->prev->next = window->next;
    } else {
        window_list_head = window->next;
    }
    
    if (window->next) {
        window->next->prev = window->prev;
    } else {
        window_list_tail = window->prev;
    }
    
    // Clear focus if this window was focused
    if (focused_window == window) {
        focused_window = NULL;
    }
    
    // Destroy UI window
    if (window->ui_window) {
        ui_destroy_window(window->ui_window);
    }
    
    KINFO("Destroyed window: %s (ID: %u)", window->title, window->window_id);
    
    // Free window
    memory_free(window);
}

void window_manager_show_window(window_manager_window_t* window) {
    if (!window) {
        return;
    }
    
    ui_show_window(window->ui_window);
    window_manager_raise_window(window);
    window_manager_set_focus(window);
}

void window_manager_hide_window(window_manager_window_t* window) {
    if (!window) {
        return;
    }
    
    ui_hide_window(window->ui_window);
    
    if (focused_window == window) {
        focused_window = NULL;
    }
}

void window_manager_minimize_window(window_manager_window_t* window) {
    if (!window || !window->minimizable) {
        return;
    }
    
    window->state = WINDOW_STATE_MINIMIZED;
    window->original_bounds = window->current_bounds;
    
    // Hide window (in a real implementation, this would minimize to taskbar)
    ui_hide_window(window->ui_window);
    
    if (focused_window == window) {
        focused_window = NULL;
    }
    
    if (window->on_minimize) {
        window->on_minimize(window);
    }
}

void window_manager_maximize_window(window_manager_window_t* window) {
    if (!window || !window->maximizable) {
        return;
    }
    
    window->state = WINDOW_STATE_MAXIMIZED;
    window->original_bounds = window->current_bounds;
    
    // Maximize to full screen (excluding taskbar)
    ui_rect_t max_bounds = ui_rect(0, 0, 1920, 1040); // Assuming 80px taskbar
    window->current_bounds = max_bounds;
    ui_set_widget_bounds(&window->ui_window->base, max_bounds);
    
    if (window->on_maximize) {
        window->on_maximize(window);
    }
}

void window_manager_restore_window(window_manager_window_t* window) {
    if (!window) {
        return;
    }
    
    window->state = WINDOW_STATE_NORMAL;
    window->current_bounds = window->original_bounds;
    ui_set_widget_bounds(&window->ui_window->base, window->original_bounds);
    
    if (window->on_restore) {
        window->on_restore(window);
    }
}

void window_manager_move_window(window_manager_window_t* window, ui_point_t position) {
    if (!window || !window->movable) {
        return;
    }
    
    window->current_bounds.position = position;
    ui_set_widget_bounds(&window->ui_window->base, window->current_bounds);
    
    if (window->on_move) {
        window->on_move(window, position);
    }
}

void window_manager_resize_window(window_manager_window_t* window, ui_size_t size) {
    if (!window || !window->resizable) {
        return;
    }
    
    window->current_bounds.size = size;
    ui_set_widget_bounds(&window->ui_window->base, window->current_bounds);
    
    if (window->on_resize) {
        window->on_resize(window, size);
    }
}

void window_manager_center_window(window_manager_window_t* window) {
    if (!window) {
        return;
    }
    
    ui_point_t center = {
        (1920 - window->current_bounds.size.width) / 2,
        (1080 - window->current_bounds.size.height) / 2
    };
    
    window_manager_move_window(window, center);
}

void window_manager_raise_window(window_manager_window_t* window) {
    if (!window || window == window_list_tail) {
        return;
    }
    
    // Remove from current position
    if (window->prev) {
        window->prev->next = window->next;
    } else {
        window_list_head = window->next;
    }
    
    if (window->next) {
        window->next->prev = window->prev;
    } else {
        window_list_tail = window->prev;
    }
    
    // Add to top
    window->prev = window_list_tail;
    window->next = NULL;
    window_list_tail->next = window;
    window_list_tail = window;
}

void window_manager_set_focus(window_manager_window_t* window) {
    if (!window) {
        return;
    }
    
    focused_window = window;
    ui_set_focus(&window->ui_window->base);
    window_manager_raise_window(window);
}

window_manager_window_t* window_manager_get_focused_window(void) {
    return focused_window;
}

window_manager_window_t* window_manager_get_window_at_point(ui_point_t point) {
    // Search from top to bottom (reverse of window list)
    window_manager_window_t* window = window_list_tail;
    while (window) {
        if (ui_point_in_rect(point, window->current_bounds)) {
            return window;
        }
        window = window->prev;
    }
    
    return NULL;
}

window_manager_window_t* window_manager_create_taskbar(void) {
    ui_rect_t taskbar_bounds = ui_rect(0, 1040, 1920, 40); // Bottom of screen
    window_manager_window_t* taskbar = window_manager_create_window("Taskbar", taskbar_bounds);
    
    if (taskbar) {
        taskbar->is_taskbar = true;
        taskbar->is_system_window = true;
        taskbar->resizable = false;
        taskbar->movable = false;
        taskbar->closeable = false;
        taskbar->minimizable = false;
        taskbar->maximizable = false;
        taskbar->ui_window->base.background_color = window_manager_config.taskbar_color;
    }
    
    return taskbar;
}

bool window_manager_handle_mouse_event(ui_event_t* event) {
    if (!event) {
        return false;
    }
    
    switch (event->type) {
        case UI_EVENT_MOUSE_DOWN: {
            window_manager_window_t* window = window_manager_get_window_at_point(event->mouse_button.position);
            if (window) {
                window_manager_set_focus(window);
                return true;
            }
            break;
        }
        
        case UI_EVENT_MOUSE_MOVE: {
            // Handle window dragging if mouse is down on title bar
            // This would be implemented with drag state tracking
            break;
        }
        
        default:
            break;
    }
    
    return false;
}

void window_manager_render_frame(void) {
    if (!window_manager_initialized) {
        return;
    }
    
    // Render desktop first
    if (desktop_window) {
        ui_invalidate_widget(&desktop_window->ui_window->base);
    }
    
    // Render all windows from bottom to top
    window_manager_window_t* window = window_list_head;
    while (window) {
        if (window != desktop_window && window != taskbar_window) {
            ui_invalidate_widget(&window->ui_window->base);
        }
        window = window->next;
    }
    
    // Render taskbar last
    if (taskbar_window) {
        ui_invalidate_widget(&taskbar_window->ui_window->base);
    }
    
    // Render the frame
    ui_render_frame();
}

// Window event handler (called by UI framework)
static bool window_manager_window_event_handler(ui_widget_t* widget, ui_event_t* event) {
    // Find the window manager window for this UI window
    window_manager_window_t* window = window_manager_find_window_by_ui_window((ui_window_t*)widget);
    if (!window) {
        return false;
    }
    
    // Handle window-specific events
    switch (event->type) {
        case UI_EVENT_WINDOW_CLOSE:
            if (window->closeable) {
                window_manager_destroy_window(window);
                return true;
            }
            break;
            
        case UI_EVENT_WINDOW_RESIZE:
            if (window->resizable) {
                window->current_bounds.size = event->resize.new_size;
                if (window->on_resize) {
                    window->on_resize(window, event->resize.new_size);
                }
                return true;
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

// Helper function to find window by UI window
static window_manager_window_t* window_manager_find_window_by_ui_window(ui_window_t* ui_window) {
    window_manager_window_t* window = window_list_head;
    while (window) {
        if (window->ui_window == ui_window) {
            return window;
        }
        window = window->next;
    }
    return NULL;
}

window_manager_window_t* window_manager_find_window_by_id(u32 window_id) {
    window_manager_window_t* window = window_list_head;
    while (window) {
        if (window->window_id == window_id) {
            return window;
        }
        window = window->next;
    }
    return NULL;
}

u32 window_manager_get_window_count(void) {
    u32 count = 0;
    window_manager_window_t* window = window_list_head;
    while (window) {
        count++;
        window = window->next;
    }
    return count;
} 