#include "ui/components/window_manager.h"
#include "drivers/vga.h"
#include "kernel.h"
#include <string.h>
#include <stdlib.h>

// Window manager constants
#define MAX_WINDOWS 32
#define MAX_WIDGETS_PER_WINDOW 64
#define WINDOW_TITLE_HEIGHT 2
#define WINDOW_BORDER_WIDTH 1

// Window states
#define WINDOW_STATE_NORMAL 0
#define WINDOW_STATE_MINIMIZED 1
#define WINDOW_STATE_MAXIMIZED 2
#define WINDOW_STATE_CLOSED 3

// Widget types
#define WIDGET_TYPE_BUTTON 1
#define WIDGET_TYPE_TEXTBOX 2
#define WIDGET_TYPE_LABEL 3
#define WIDGET_TYPE_LISTBOX 4
#define WIDGET_TYPE_CHECKBOX 5
#define WIDGET_TYPE_RADIOBUTTON 6

// Global window manager state
static window_manager_state_t window_manager_state = {0};
static bool window_manager_initialized = false;
static window_t windows[MAX_WINDOWS];
static int window_count = 0;
static int active_window = -1;
static int focused_widget = -1;

// Mouse state
static int mouse_x = 0;
static int mouse_y = 0;
static bool mouse_left_pressed = false;
static bool mouse_right_pressed = false;

// Initialize window manager
error_t window_manager_init(void) {
    if (window_manager_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing window manager...");
    
    // Initialize window manager state
    memset(&window_manager_state, 0, sizeof(window_manager_state));
    window_manager_state.screen_width = 80;  // VGA width
    window_manager_state.screen_height = 25; // VGA height
    window_manager_state.running = true;
    
    // Initialize windows array
    memset(windows, 0, sizeof(windows));
    window_count = 0;
    active_window = -1;
    focused_widget = -1;
    
    // Initialize mouse state
    mouse_x = window_manager_state.screen_width / 2;
    mouse_y = window_manager_state.screen_height / 2;
    mouse_left_pressed = false;
    mouse_right_pressed = false;
    
    window_manager_initialized = true;
    KINFO("Window manager initialized successfully");
    
    return SUCCESS;
}

// Create a new window
int window_manager_create_window(const char* title, int x, int y, int width, int height) {
    if (!window_manager_initialized || window_count >= MAX_WINDOWS) {
        return -1;
    }
    
    if (!title || width <= 0 || height <= 0) {
        return -1;
    }
    
    // Find free window slot
    int window_id = -1;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].state == WINDOW_STATE_CLOSED) {
            window_id = i;
            break;
        }
    }
    
    if (window_id == -1) {
        return -1;
    }
    
    // Initialize window
    window_t* window = &windows[window_id];
    memset(window, 0, sizeof(window_t));
    
    window->id = window_id;
    strncpy(window->title, title, sizeof(window->title) - 1);
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    window->state = WINDOW_STATE_NORMAL;
    window->visible = true;
    window->widget_count = 0;
    
    // Set as active window
    active_window = window_id;
    
    window_count++;
    
    KDEBUG("Created window %d: %s at (%d,%d) size %dx%d", 
           window_id, title, x, y, width, height);
    
    return window_id;
}

// Destroy a window
void window_manager_destroy_window(int window_id) {
    if (!window_manager_initialized || window_id < 0 || window_id >= MAX_WINDOWS) {
        return;
    }
    
    window_t* window = &windows[window_id];
    if (window->state == WINDOW_STATE_CLOSED) {
        return;
    }
    
    // Clear window from screen
    window_manager_clear_window(window_id);
    
    // Mark window as closed
    window->state = WINDOW_STATE_CLOSED;
    window_count--;
    
    // Update active window if needed
    if (active_window == window_id) {
        active_window = -1;
        // Find next available window
        for (int i = 0; i < MAX_WINDOWS; i++) {
            if (windows[i].state != WINDOW_STATE_CLOSED) {
                active_window = i;
                break;
            }
        }
    }
    
    KDEBUG("Destroyed window %d", window_id);
}

// Show a window
void window_manager_show_window(int window_id) {
    if (!window_manager_initialized || window_id < 0 || window_id >= MAX_WINDOWS) {
        return;
    }
    
    window_t* window = &windows[window_id];
    if (window->state == WINDOW_STATE_CLOSED) {
        return;
    }
    
    window->visible = true;
    window_manager_redraw_window(window_id);
    
    KDEBUG("Showed window %d", window_id);
}

// Hide a window
void window_manager_hide_window(int window_id) {
    if (!window_manager_initialized || window_id < 0 || window_id >= MAX_WINDOWS) {
        return;
    }
    
    window_t* window = &windows[window_id];
    if (window->state == WINDOW_STATE_CLOSED) {
        return;
    }
    
    window->visible = false;
    window_manager_clear_window(window_id);
    
    KDEBUG("Hid window %d", window_id);
}

// Move a window
void window_manager_move_window(int window_id, int x, int y) {
    if (!window_manager_initialized || window_id < 0 || window_id >= MAX_WINDOWS) {
        return;
    }
    
    window_t* window = &windows[window_id];
    if (window->state == WINDOW_STATE_CLOSED) {
        return;
    }
    
    // Clear old position
    window_manager_clear_window(window_id);
    
    // Update position
    window->x = x;
    window->y = y;
    
    // Redraw at new position
    if (window->visible) {
        window_manager_redraw_window(window_id);
    }
    
    KDEBUG("Moved window %d to (%d,%d)", window_id, x, y);
}

// Resize a window
void window_manager_resize_window(int window_id, int width, int height) {
    if (!window_manager_initialized || window_id < 0 || window_id >= MAX_WINDOWS) {
        return;
    }
    
    window_t* window = &windows[window_id];
    if (window->state == WINDOW_STATE_CLOSED || width <= 0 || height <= 0) {
        return;
    }
    
    // Clear old window
    window_manager_clear_window(window_id);
    
    // Update size
    window->width = width;
    window->height = height;
    
    // Redraw with new size
    if (window->visible) {
        window_manager_redraw_window(window_id);
    }
    
    KDEBUG("Resized window %d to %dx%d", window_id, width, height);
}

// Draw a window
void window_manager_redraw_window(int window_id) {
    if (!window_manager_initialized || window_id < 0 || window_id >= MAX_WINDOWS) {
        return;
    }
    
    window_t* window = &windows[window_id];
    if (window->state == WINDOW_STATE_CLOSED || !window->visible) {
        return;
    }
    
    // Draw window border and title
    window_manager_draw_window_border(window);
    
    // Draw window content
    window_manager_draw_window_content(window);
    
    // Draw widgets
    window_manager_draw_window_widgets(window);
}

// Draw window border
void window_manager_draw_window_border(window_t* window) {
    if (!window) {
        return;
    }
    
    // Draw top border with title
    for (int x = 0; x < window->width; x++) {
        int screen_x = window->x + x;
        int screen_y = window->y;
        
        if (screen_x >= 0 && screen_x < window_manager_state.screen_width &&
            screen_y >= 0 && screen_y < window_manager_state.screen_height) {
            
            char border_char = '=';
            if (x == 0) border_char = '+';
            else if (x == window->width - 1) border_char = '+';
            else if (x < strlen(window->title) + 2) {
                if (x == 1) {
                    vga_set_cursor(screen_x, screen_y);
                    vga_printf("%s", window->title);
                    x += strlen(window->title) - 1;
                    continue;
                }
            }
            
            vga_set_cursor(screen_x, screen_y);
            vga_putchar(border_char);
        }
    }
    
    // Draw side borders
    for (int y = 1; y < window->height - 1; y++) {
        int screen_x = window->x;
        int screen_y = window->y + y;
        
        if (screen_x >= 0 && screen_x < window_manager_state.screen_width &&
            screen_y >= 0 && screen_y < window_manager_state.screen_height) {
            vga_set_cursor(screen_x, screen_y);
            vga_putchar('|');
        }
        
        screen_x = window->x + window->width - 1;
        if (screen_x >= 0 && screen_x < window_manager_state.screen_width &&
            screen_y >= 0 && screen_y < window_manager_state.screen_height) {
            vga_set_cursor(screen_x, screen_y);
            vga_putchar('|');
        }
    }
    
    // Draw bottom border
    for (int x = 0; x < window->width; x++) {
        int screen_x = window->x + x;
        int screen_y = window->y + window->height - 1;
        
        if (screen_x >= 0 && screen_x < window_manager_state.screen_width &&
            screen_y >= 0 && screen_y < window_manager_state.screen_height) {
            
            char border_char = '=';
            if (x == 0 || x == window->width - 1) border_char = '+';
            
            vga_set_cursor(screen_x, screen_y);
            vga_putchar(border_char);
        }
    }
}

// Draw window content
void window_manager_draw_window_content(window_t* window) {
    if (!window) {
        return;
    }
    
    // Clear window content area
    for (int y = 1; y < window->height - 1; y++) {
        for (int x = 1; x < window->width - 1; x++) {
            int screen_x = window->x + x;
            int screen_y = window->y + y;
            
            if (screen_x >= 0 && screen_x < window_manager_state.screen_width &&
                screen_y >= 0 && screen_y < window_manager_state.screen_height) {
                vga_set_cursor(screen_x, screen_y);
                vga_putchar(' ');
            }
        }
    }
}

// Draw window widgets
void window_manager_draw_window_widgets(window_t* window) {
    if (!window) {
        return;
    }
    
    for (int i = 0; i < window->widget_count; i++) {
        widget_t* widget = &window->widgets[i];
        if (widget->visible) {
            window_manager_draw_widget(window, widget);
        }
    }
}

// Draw a widget
void window_manager_draw_widget(window_t* window, widget_t* widget) {
    if (!window || !widget) {
        return;
    }
    
    int screen_x = window->x + widget->x + 1; // +1 for border
    int screen_y = window->y + widget->y + 1; // +1 for border
    
    vga_set_cursor(screen_x, screen_y);
    
    switch (widget->type) {
        case WIDGET_TYPE_BUTTON:
            vga_printf("[%s]", widget->text);
            break;
        case WIDGET_TYPE_LABEL:
            vga_printf("%s", widget->text);
            break;
        case WIDGET_TYPE_TEXTBOX:
            vga_printf("[%s]", widget->text);
            break;
        case WIDGET_TYPE_CHECKBOX:
            vga_printf("[%c] %s", widget->checked ? 'X' : ' ', widget->text);
            break;
        default:
            vga_printf("%s", widget->text);
            break;
    }
}

// Clear a window from screen
void window_manager_clear_window(int window_id) {
    if (!window_manager_initialized || window_id < 0 || window_id >= MAX_WINDOWS) {
        return;
    }
    
    window_t* window = &windows[window_id];
    if (window->state == WINDOW_STATE_CLOSED) {
        return;
    }
    
    // Clear window area
    for (int y = 0; y < window->height; y++) {
        for (int x = 0; x < window->width; x++) {
            int screen_x = window->x + x;
            int screen_y = window->y + y;
            
            if (screen_x >= 0 && screen_x < window_manager_state.screen_width &&
                screen_y >= 0 && screen_y < window_manager_state.screen_height) {
                vga_set_cursor(screen_x, screen_y);
                vga_putchar(' ');
            }
        }
    }
}

// Add a widget to a window
int window_manager_add_widget(int window_id, int type, int x, int y, int width, int height, const char* text) {
    if (!window_manager_initialized || window_id < 0 || window_id >= MAX_WINDOWS) {
        return -1;
    }
    
    window_t* window = &windows[window_id];
    if (window->state == WINDOW_STATE_CLOSED || window->widget_count >= MAX_WIDGETS_PER_WINDOW) {
        return -1;
    }
    
    widget_t* widget = &window->widgets[window->widget_count];
    widget->id = window->widget_count;
    widget->type = type;
    widget->x = x;
    widget->y = y;
    widget->width = width;
    widget->height = height;
    widget->visible = true;
    widget->enabled = true;
    widget->checked = false;
    
    if (text) {
        strncpy(widget->text, text, sizeof(widget->text) - 1);
    } else {
        widget->text[0] = '\0';
    }
    
    window->widget_count++;
    
    // Redraw window to show new widget
    if (window->visible) {
        window_manager_redraw_window(window_id);
    }
    
    KDEBUG("Added widget %d to window %d: type=%d, text='%s'", 
           widget->id, window_id, type, text ? text : "");
    
    return widget->id;
}

// Handle mouse events
void window_manager_handle_mouse(int x, int y, bool left_pressed, bool right_pressed) {
    if (!window_manager_initialized) {
        return;
    }
    
    mouse_x = x;
    mouse_y = y;
    mouse_left_pressed = left_pressed;
    mouse_right_pressed = right_pressed;
    
    // Check for window clicks
    for (int i = 0; i < MAX_WINDOWS; i++) {
        window_t* window = &windows[i];
        if (window->state == WINDOW_STATE_CLOSED || !window->visible) {
            continue;
        }
        
        if (x >= window->x && x < window->x + window->width &&
            y >= window->y && y < window->y + window->height) {
            
            // Window clicked
            if (left_pressed && active_window != i) {
                // Bring window to front
                window_manager_bring_to_front(i);
            }
            
            // Check for widget clicks
            if (left_pressed) {
                window_manager_handle_widget_click(window, x - window->x, y - window->y);
            }
            
            break;
        }
    }
}

// Handle widget clicks
void window_manager_handle_widget_click(window_t* window, int rel_x, int rel_y) {
    if (!window) {
        return;
    }
    
    for (int i = 0; i < window->widget_count; i++) {
        widget_t* widget = &window->widgets[i];
        if (!widget->visible || !widget->enabled) {
            continue;
        }
        
        if (rel_x >= widget->x && rel_x < widget->x + widget->width &&
            rel_y >= widget->y && rel_y < widget->y + widget->height) {
            
            // Widget clicked
            window_manager_handle_widget_event(window, widget, WIDGET_EVENT_CLICK);
            break;
        }
    }
}

// Handle widget events
void window_manager_handle_widget_event(window_t* window, widget_t* widget, int event) {
    if (!window || !widget) {
        return;
    }
    
    KDEBUG("Widget event: window=%d, widget=%d, event=%d", window->id, widget->id, event);
    
    switch (widget->type) {
        case WIDGET_TYPE_BUTTON:
            if (event == WIDGET_EVENT_CLICK) {
                // Handle button click
                vga_printf("Button clicked: %s\n", widget->text);
            }
            break;
        case WIDGET_TYPE_CHECKBOX:
            if (event == WIDGET_EVENT_CLICK) {
                // Toggle checkbox
                widget->checked = !widget->checked;
                window_manager_redraw_window(window->id);
            }
            break;
        default:
            break;
    }
}

// Bring window to front
void window_manager_bring_to_front(int window_id) {
    if (!window_manager_initialized || window_id < 0 || window_id >= MAX_WINDOWS) {
        return;
    }
    
    window_t* window = &windows[window_id];
    if (window->state == WINDOW_STATE_CLOSED) {
        return;
    }
    
    active_window = window_id;
    
    // Redraw all windows to update layering
    window_manager_redraw_all();
    
    KDEBUG("Brought window %d to front", window_id);
}

// Redraw all windows
void window_manager_redraw_all(void) {
    if (!window_manager_initialized) {
        return;
    }
    
    // Clear screen
    vga_clear();
    
    // Redraw all visible windows
    for (int i = 0; i < MAX_WINDOWS; i++) {
        window_t* window = &windows[i];
        if (window->state != WINDOW_STATE_CLOSED && window->visible) {
            window_manager_redraw_window(i);
        }
    }
}

// Get window manager state
window_manager_state_t* window_manager_get_state(void) {
    return &window_manager_state;
}

// Check if window manager is initialized
bool window_manager_is_initialized(void) {
    return window_manager_initialized;
}

// Update window manager (called periodically)
void window_manager_update(void) {
    if (!window_manager_initialized) {
        return;
    }
    
    // Handle any pending events
    // TODO: Process event queue
    
    // Update mouse cursor
    vga_set_cursor(mouse_x, mouse_y);
    vga_putchar('X'); // Simple mouse cursor
}

// Shutdown window manager
void window_manager_shutdown(void) {
    if (!window_manager_initialized) {
        return;
    }
    
    KINFO("Shutting down window manager...");
    
    // Clear all windows
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].state != WINDOW_STATE_CLOSED) {
            window_manager_destroy_window(i);
        }
    }
    
    window_manager_state.running = false;
    window_manager_initialized = false;
    
    KINFO("Window manager shutdown complete");
} 