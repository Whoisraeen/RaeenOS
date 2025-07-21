#include "graphics/include/graphics.h"
#include "kernel.h"
#include "memory/memory.h"
#include <string.h>

// Window manager configuration
#define MAX_WINDOWS 256
#define MAX_DISPLAYS 4
#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600
#define TITLE_BAR_HEIGHT 30
#define BORDER_WIDTH 2
#define MIN_WINDOW_WIDTH 100
#define MIN_WINDOW_HEIGHT 50

// Window states
typedef enum {
    WINDOW_STATE_NORMAL,
    WINDOW_STATE_MINIMIZED,
    WINDOW_STATE_MAXIMIZED,
    WINDOW_STATE_FULLSCREEN
} window_state_t;

// Window structure
typedef struct {
    u32 id;
    char title[256];
    int x, y;
    int width, height;
    int original_x, original_y;
    int original_width, original_height;
    window_state_t state;
    bool visible;
    bool focused;
    bool resizable;
    bool movable;
    bool closeable;
    bool minimized;
    bool maximized;
    u32 background_color;
    u32 border_color;
    u32 title_color;
    void* framebuffer;
    u32 framebuffer_size;
    void* backing_store;
    u32 backing_store_size;
    window_event_handler_t event_handler;
    void* user_data;
    struct window* parent;
    struct window* children;
    struct window* next_sibling;
    struct window* prev_sibling;
    bool active;
} window_t;

// Display structure
typedef struct {
    u32 id;
    char name[64];
    int x, y;
    int width, height;
    int depth;
    u32 refresh_rate;
    void* framebuffer;
    u32 framebuffer_size;
    u32 pitch;
    bool active;
} display_t;

// Mouse state
typedef struct {
    int x, y;
    int last_x, last_y;
    u32 buttons;
    u32 last_buttons;
    bool moved;
    bool clicked;
    bool released;
} mouse_state_t;

// Keyboard state
typedef struct {
    u8 key_states[256];
    u32 modifiers;
    bool key_pressed;
    bool key_released;
    u32 last_key;
} keyboard_state_t;

// Window manager state
static window_t windows[MAX_WINDOWS];
static display_t displays[MAX_DISPLAYS];
static u32 num_windows = 0;
static u32 num_displays = 0;
static u32 next_window_id = 1;
static u32 next_display_id = 1;
static window_t* focused_window = NULL;
static window_t* root_window = NULL;
static mouse_state_t mouse_state = {0};
static keyboard_state_t keyboard_state = {0};
static bool window_manager_initialized = false;

// Graphics context
static graphics_context_t* graphics_ctx = NULL;

// Forward declarations
static error_t window_create_framebuffer(window_t* window);
static error_t window_destroy_framebuffer(window_t* window);
static error_t window_redraw(window_t* window);
static error_t window_handle_mouse_event(window_t* window, mouse_event_t* event);
static error_t window_handle_keyboard_event(window_t* window, keyboard_event_t* event);
static window_t* window_find_at_position(int x, int y);
static void window_bring_to_front(window_t* window);
static void window_send_to_back(window_t* window);
static error_t window_resize(window_t* window, int width, int height);
static error_t window_move(window_t* window, int x, int y);
static void window_draw_title_bar(window_t* window);
static void window_draw_border(window_t* window);

// Initialize window manager
error_t window_manager_init(void) {
    if (window_manager_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing window manager");
    
    // Initialize graphics system
    graphics_ctx = graphics_init();
    if (!graphics_ctx) {
        KERROR("Failed to initialize graphics system");
        return E_INIT;
    }
    
    // Initialize window table
    memset(windows, 0, sizeof(windows));
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        windows[i].active = false;
    }
    
    // Initialize display table
    memset(displays, 0, sizeof(displays));
    
    // Set up primary display
    displays[0].id = next_display_id++;
    strcpy(displays[0].name, "Primary Display");
    displays[0].x = 0;
    displays[0].y = 0;
    displays[0].width = 1920;
    displays[0].height = 1080;
    displays[0].depth = 32;
    displays[0].refresh_rate = 60;
    displays[0].active = true;
    num_displays = 1;
    
    // Create root window
    error_t result = window_create("Desktop", 0, 0, displays[0].width, displays[0].height, &root_window);
    if (result != SUCCESS) {
        KERROR("Failed to create root window");
        return result;
    }
    
    root_window->background_color = 0x2D2D30; // Dark gray
    root_window->resizable = false;
    root_window->movable = false;
    root_window->closeable = false;
    
    window_manager_initialized = true;
    
    KINFO("Window manager initialized with %u displays", num_displays);
    return SUCCESS;
}

// Create a window
error_t window_create(const char* title, int x, int y, int width, int height, window_t** window) {
    if (!window_manager_initialized || !title || !window) {
        return E_INVAL;
    }
    
    if (num_windows >= MAX_WINDOWS) {
        return E_NOMEM;
    }
    
    KDEBUG("Creating window: %s (%dx%d at %d,%d)", title, width, height, x, y);
    
    // Find free window slot
    window_t* win = NULL;
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (!windows[i].active) {
            win = &windows[i];
            break;
        }
    }
    
    if (!win) {
        return E_NOMEM;
    }
    
    // Initialize window
    memset(win, 0, sizeof(window_t));
    win->id = next_window_id++;
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->original_x = x;
    win->original_y = y;
    win->original_width = width;
    win->original_height = height;
    win->state = WINDOW_STATE_NORMAL;
    win->visible = true;
    win->focused = false;
    win->resizable = true;
    win->movable = true;
    win->closeable = true;
    win->background_color = 0xFFFFFF; // White
    win->border_color = 0x000000;     // Black
    win->title_color = 0xFFFFFF;      // White
    win->active = true;
    
    // Create framebuffer
    error_t result = window_create_framebuffer(win);
    if (result != SUCCESS) {
        KERROR("Failed to create window framebuffer");
        win->active = false;
        return result;
    }
    
    // Add to window list
    win->next_sibling = root_window->children;
    if (root_window->children) {
        root_window->children->prev_sibling = win;
    }
    root_window->children = win;
    win->parent = root_window;
    
    num_windows++;
    *window = win;
    
    // Redraw window
    window_redraw(win);
    
    KDEBUG("Created window: ID=%u", win->id);
    return SUCCESS;
}

// Destroy a window
error_t window_destroy(window_t* window) {
    if (!window || !window->active) {
        return E_INVAL;
    }
    
    KDEBUG("Destroying window: ID=%u", window->id);
    
    // Remove from parent's child list
    if (window->parent) {
        if (window->prev_sibling) {
            window->prev_sibling->next_sibling = window->next_sibling;
        } else {
            window->parent->children = window->next_sibling;
        }
        
        if (window->next_sibling) {
            window->next_sibling->prev_sibling = window->prev_sibling;
        }
    }
    
    // Destroy child windows
    window_t* child = window->children;
    while (child) {
        window_t* next = child->next_sibling;
        window_destroy(child);
        child = next;
    }
    
    // Destroy framebuffer
    window_destroy_framebuffer(window);
    
    // Remove focus if this window was focused
    if (focused_window == window) {
        focused_window = NULL;
    }
    
    window->active = false;
    num_windows--;
    
    // Redraw desktop
    if (root_window) {
        window_redraw(root_window);
    }
    
    return SUCCESS;
}

// Show a window
error_t window_show(window_t* window) {
    if (!window || !window->active) {
        return E_INVAL;
    }
    
    window->visible = true;
    window_redraw(window);
    
    return SUCCESS;
}

// Hide a window
error_t window_hide(window_t* window) {
    if (!window || !window->active) {
        return E_INVAL;
    }
    
    window->visible = false;
    
    // Redraw desktop to cover the hidden window
    if (root_window) {
        window_redraw(root_window);
    }
    
    return SUCCESS;
}

// Focus a window
error_t window_focus(window_t* window) {
    if (!window || !window->active || !window->visible) {
        return E_INVAL;
    }
    
    // Remove focus from current window
    if (focused_window) {
        focused_window->focused = false;
        window_redraw(focused_window);
    }
    
    // Set new focus
    focused_window = window;
    window->focused = true;
    window_bring_to_front(window);
    window_redraw(window);
    
    return SUCCESS;
}

// Minimize a window
error_t window_minimize(window_t* window) {
    if (!window || !window->active) {
        return E_INVAL;
    }
    
    window->state = WINDOW_STATE_MINIMIZED;
    window->minimized = true;
    window->visible = false;
    
    // Redraw desktop
    if (root_window) {
        window_redraw(root_window);
    }
    
    return SUCCESS;
}

// Maximize a window
error_t window_maximize(window_t* window) {
    if (!window || !window->active) {
        return E_INVAL;
    }
    
    if (window->state == WINDOW_STATE_MAXIMIZED) {
        // Restore
        window->state = WINDOW_STATE_NORMAL;
        window->maximized = false;
        window->x = window->original_x;
        window->y = window->original_y;
        window->width = window->original_width;
        window->height = window->original_height;
    } else {
        // Maximize
        window->state = WINDOW_STATE_MAXIMIZED;
        window->maximized = true;
        window->original_x = window->x;
        window->original_y = window->y;
        window->original_width = window->width;
        window->original_height = window->height;
        
        // Maximize to display size
        if (num_displays > 0) {
            window->x = 0;
            window->y = 0;
            window->width = displays[0].width;
            window->height = displays[0].height;
        }
    }
    
    // Recreate framebuffer with new size
    window_destroy_framebuffer(window);
    window_create_framebuffer(window);
    window_redraw(window);
    
    return SUCCESS;
}

// Move a window
error_t window_move(window_t* window, int x, int y) {
    if (!window || !window->active || !window->movable) {
        return E_INVAL;
    }
    
    if (window->state == WINDOW_STATE_MAXIMIZED) {
        return E_INVAL; // Cannot move maximized window
    }
    
    window->x = x;
    window->y = y;
    
    // Redraw window and desktop
    window_redraw(window);
    if (root_window) {
        window_redraw(root_window);
    }
    
    return SUCCESS;
}

// Resize a window
error_t window_resize(window_t* window, int width, int height) {
    if (!window || !window->active || !window->resizable) {
        return E_INVAL;
    }
    
    if (window->state == WINDOW_STATE_MAXIMIZED) {
        return E_INVAL; // Cannot resize maximized window
    }
    
    // Check minimum size
    if (width < MIN_WINDOW_WIDTH) width = MIN_WINDOW_WIDTH;
    if (height < MIN_WINDOW_HEIGHT) height = MIN_WINDOW_HEIGHT;
    
    window->width = width;
    window->height = height;
    
    // Recreate framebuffer with new size
    window_destroy_framebuffer(window);
    window_create_framebuffer(window);
    window_redraw(window);
    
    return SUCCESS;
}

// Set window event handler
error_t window_set_event_handler(window_t* window, window_event_handler_t handler, void* user_data) {
    if (!window || !window->active) {
        return E_INVAL;
    }
    
    window->event_handler = handler;
    window->user_data = user_data;
    
    return SUCCESS;
}

// Handle mouse events
error_t window_manager_handle_mouse_event(mouse_event_t* event) {
    if (!window_manager_initialized || !event) {
        return E_INVAL;
    }
    
    // Update mouse state
    mouse_state.last_x = mouse_state.x;
    mouse_state.last_y = mouse_state.y;
    mouse_state.last_buttons = mouse_state.buttons;
    
    mouse_state.x = event->x;
    mouse_state.y = event->y;
    mouse_state.buttons = event->buttons;
    mouse_state.moved = (mouse_state.x != mouse_state.last_x || mouse_state.y != mouse_state.last_y);
    mouse_state.clicked = (mouse_state.buttons & ~mouse_state.last_buttons) != 0;
    mouse_state.released = (~mouse_state.buttons & mouse_state.last_buttons) != 0;
    
    // Find window under mouse
    window_t* window = window_find_at_position(mouse_state.x, mouse_state.y);
    
    // Handle window focus
    if (mouse_state.clicked && window && window != focused_window) {
        window_focus(window);
    }
    
    // Send event to focused window
    if (focused_window && focused_window->event_handler) {
        window_handle_mouse_event(focused_window, event);
    }
    
    return SUCCESS;
}

// Handle keyboard events
error_t window_manager_handle_keyboard_event(keyboard_event_t* event) {
    if (!window_manager_initialized || !event) {
        return E_INVAL;
    }
    
    // Update keyboard state
    keyboard_state.key_pressed = event->pressed;
    keyboard_state.key_released = !event->pressed;
    keyboard_state.last_key = event->keycode;
    
    if (event->pressed) {
        keyboard_state.key_states[event->keycode] = 1;
    } else {
        keyboard_state.key_states[event->keycode] = 0;
    }
    
    // Update modifiers
    keyboard_state.modifiers = event->modifiers;
    
    // Send event to focused window
    if (focused_window && focused_window->event_handler) {
        window_handle_keyboard_event(focused_window, event);
    }
    
    return SUCCESS;
}

// Create window framebuffer
static error_t window_create_framebuffer(window_t* window) {
    if (!window) {
        return E_INVAL;
    }
    
    // Calculate framebuffer size
    window->framebuffer_size = window->width * window->height * 4; // 32-bit color
    
    // Allocate framebuffer
    window->framebuffer = memory_alloc(window->framebuffer_size);
    if (!window->framebuffer) {
        return E_NOMEM;
    }
    
    // Allocate backing store for double buffering
    window->backing_store = memory_alloc(window->framebuffer_size);
    if (!window->backing_store) {
        memory_free(window->framebuffer);
        window->framebuffer = NULL;
        return E_NOMEM;
    }
    
    // Clear framebuffer
    memset(window->framebuffer, 0, window->framebuffer_size);
    memset(window->backing_store, 0, window->framebuffer_size);
    
    return SUCCESS;
}

// Destroy window framebuffer
static error_t window_destroy_framebuffer(window_t* window) {
    if (!window) {
        return E_INVAL;
    }
    
    if (window->framebuffer) {
        memory_free(window->framebuffer);
        window->framebuffer = NULL;
    }
    
    if (window->backing_store) {
        memory_free(window->backing_store);
        window->backing_store = NULL;
    }
    
    return SUCCESS;
}

// Redraw a window
static error_t window_redraw(window_t* window) {
    if (!window || !window->active || !window->framebuffer) {
        return E_INVAL;
    }
    
    // Clear background
    u32* fb = (u32*)window->framebuffer;
    for (int i = 0; i < window->width * window->height; i++) {
        fb[i] = window->background_color;
    }
    
    // Draw title bar if not root window
    if (window != root_window) {
        window_draw_title_bar(window);
        window_draw_border(window);
    }
    
    // Call window's redraw handler
    if (window->event_handler) {
        window_event_t event;
        event.type = WINDOW_EVENT_REDRAW;
        event.window = window;
        window->event_handler(&event, window->user_data);
    }
    
    // Update display
    if (graphics_ctx) {
        graphics_blit(graphics_ctx, window->framebuffer, window->x, window->y, 
                     window->width, window->height);
    }
    
    return SUCCESS;
}

// Draw window title bar
static void window_draw_title_bar(window_t* window) {
    if (!window || !window->framebuffer) {
        return;
    }
    
    u32* fb = (u32*)window->framebuffer;
    u32 title_color = window->focused ? 0x007ACC : 0xCCCCCC; // Blue when focused, gray when not
    
    // Draw title bar background
    for (int y = 0; y < TITLE_BAR_HEIGHT; y++) {
        for (int x = 0; x < window->width; x++) {
            fb[y * window->width + x] = title_color;
        }
    }
    
    // Draw title text (simplified)
    // In a real implementation, this would use a font renderer
    for (int i = 0; i < strlen(window->title) && i < window->width / 8; i++) {
        // Simple character rendering
        for (int y = 2; y < TITLE_BAR_HEIGHT - 2; y++) {
            for (int x = 2 + i * 8; x < 10 + i * 8; x++) {
                if (x < window->width) {
                    fb[y * window->width + x] = 0x000000; // Black text
                }
            }
        }
    }
}

// Draw window border
static void window_draw_border(window_t* window) {
    if (!window || !window->framebuffer) {
        return;
    }
    
    u32* fb = (u32*)window->framebuffer;
    u32 border_color = window->focused ? 0x007ACC : 0xCCCCCC;
    
    // Draw borders
    for (int i = 0; i < BORDER_WIDTH; i++) {
        // Top border
        for (int x = 0; x < window->width; x++) {
            fb[i * window->width + x] = border_color;
        }
        
        // Bottom border
        for (int x = 0; x < window->width; x++) {
            fb[(window->height - 1 - i) * window->width + x] = border_color;
        }
        
        // Left border
        for (int y = 0; y < window->height; y++) {
            fb[y * window->width + i] = border_color;
        }
        
        // Right border
        for (int y = 0; y < window->height; y++) {
            fb[y * window->width + (window->width - 1 - i)] = border_color;
        }
    }
}

// Find window at position
static window_t* window_find_at_position(int x, int y) {
    // Start from top-most window (last in list)
    window_t* window = root_window->children;
    window_t* top_window = NULL;
    
    while (window) {
        if (window->active && window->visible &&
            x >= window->x && x < window->x + window->width &&
            y >= window->y && y < window->y + window->height) {
            top_window = window;
        }
        window = window->next_sibling;
    }
    
    return top_window ? top_window : root_window;
}

// Bring window to front
static void window_bring_to_front(window_t* window) {
    if (!window || !window->parent) {
        return;
    }
    
    // Remove from current position
    if (window->prev_sibling) {
        window->prev_sibling->next_sibling = window->next_sibling;
    } else {
        window->parent->children = window->next_sibling;
    }
    
    if (window->next_sibling) {
        window->next_sibling->prev_sibling = window->prev_sibling;
    }
    
    // Add to front
    window->next_sibling = window->parent->children;
    window->prev_sibling = NULL;
    if (window->parent->children) {
        window->parent->children->prev_sibling = window;
    }
    window->parent->children = window;
}

// Send window to back
static void window_send_to_back(window_t* window) {
    if (!window || !window->parent) {
        return;
    }
    
    // Remove from current position
    if (window->prev_sibling) {
        window->prev_sibling->next_sibling = window->next_sibling;
    } else {
        window->parent->children = window->next_sibling;
    }
    
    if (window->next_sibling) {
        window->next_sibling->prev_sibling = window->prev_sibling;
    }
    
    // Find last child
    window_t* last = window->parent->children;
    while (last && last->next_sibling) {
        last = last->next_sibling;
    }
    
    // Add to back
    window->next_sibling = NULL;
    window->prev_sibling = last;
    if (last) {
        last->next_sibling = window;
    } else {
        window->parent->children = window;
    }
}

// Handle mouse event for window
static error_t window_handle_mouse_event(window_t* window, mouse_event_t* event) {
    if (!window || !event) {
        return E_INVAL;
    }
    
    if (window->event_handler) {
        window_event_t win_event;
        win_event.type = WINDOW_EVENT_MOUSE;
        win_event.window = window;
        win_event.mouse = *event;
        window->event_handler(&win_event, window->user_data);
    }
    
    return SUCCESS;
}

// Handle keyboard event for window
static error_t window_handle_keyboard_event(window_t* window, keyboard_event_t* event) {
    if (!window || !event) {
        return E_INVAL;
    }
    
    if (window->event_handler) {
        window_event_t win_event;
        win_event.type = WINDOW_EVENT_KEYBOARD;
        win_event.window = window;
        win_event.keyboard = *event;
        window->event_handler(&win_event, window->user_data);
    }
    
    return SUCCESS;
}

// Get window by ID
window_t* window_get_by_id(u32 id) {
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].active && windows[i].id == id) {
            return &windows[i];
        }
    }
    return NULL;
}

// Get focused window
window_t* window_get_focused(void) {
    return focused_window;
}

// Get root window
window_t* window_get_root(void) {
    return root_window;
}

// Dump window manager information
void window_manager_dump_info(void) {
    KINFO("=== Window Manager Information ===");
    KINFO("Initialized: %s", window_manager_initialized ? "Yes" : "No");
    KINFO("Windows: %u", num_windows);
    KINFO("Displays: %u", num_displays);
    KINFO("Focused window: %u", focused_window ? focused_window->id : 0);
    
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].active) {
            window_t* win = &windows[i];
            KINFO("  Window %u: '%s' (%dx%d at %d,%d) %s", 
                  win->id, win->title, win->width, win->height, win->x, win->y,
                  win->focused ? "[FOCUSED]" : "");
        }
    }
    
    for (u32 i = 0; i < num_displays; i++) {
        display_t* disp = &displays[i];
        KINFO("  Display %u: %s (%dx%d)", 
              disp->id, disp->name, disp->width, disp->height);
    }
} 

// AI-Enhanced Hybrid Compositor
void composite_frame() {
    // Neural Style Transfer-based Theme Adaptation
    apply_ai_theme(current_usage_pattern());
    
    // macOS-like Quartz Extreme + Windows DWM hybrid
    foreach(window, z-ordered_list) {
        if(window->ai_priority > current_threshold) {
            // Hardware-accelerated path
            gpu_blit(window->buffer);
        } else {
            // Software fallback with SIMD optimizations
            simd_blit(window->buffer);
        }
    }
    
    // AI-Predictive Pre-caching
    neural_predict_next_frames();
}

// Adaptive VSYNC Controller
void vsync_handler() {
    // ML model chooses between:
    // - macOS-style adaptive sync
    // - Windows-style full-rate
    // - Custom AI-predictive mode
    ai_select_vsync_mode();
}