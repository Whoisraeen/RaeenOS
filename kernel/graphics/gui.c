#include "gui.h"
#include "memory.h"
#include <string.h>
#include <stddef.h>

// Global GUI system
static gui_system_t gui_system = {0};
static theme_t current_theme = {0};
static uint64_t last_render_time = 0;
static bool gui_initialized = false;

// Default theme
static theme_t default_theme = {
    .name = "default",
    .background = {32, 32, 32, 255},
    .foreground = {255, 255, 255, 255},
    .accent = {0, 120, 215, 255},
    .border = {64, 64, 64, 255},
    .highlight = {0, 120, 215, 128},
    .shadow = {0, 0, 0, 128},
    .border_width = 1,
    .padding = 4,
    .font_size = 12,
    .font_name = "default"
};

// Initialize GUI system
int gui_init(void) {
    memset(&gui_system, 0, sizeof(gui_system_t));
    gui_system.lock = SPINLOCK_INIT;
    gui_system.running = true;
    
    // Initialize default theme
    current_theme = default_theme;
    
    // Create screen graphics context
    gui_system.screen_gc = graphics_context_create(GUI_DEFAULT_WIDTH, GUI_DEFAULT_HEIGHT, GUI_DEFAULT_BPP);
    if (!gui_system.screen_gc) {
        KERROR("Failed to create screen graphics context");
        return -1;
    }
    
    // Initialize window manager
    if (window_manager_init() != 0) {
        KERROR("Failed to initialize window manager");
        return -1;
    }
    
    // Initialize desktop
    if (desktop_init() != 0) {
        KERROR("Failed to initialize desktop");
        return -1;
    }
    
    gui_initialized = true;
    KINFO("GUI system initialized");
    return 0;
}

void gui_shutdown(void) {
    if (!gui_initialized) {
        return;
    }
    
    // Shutdown desktop
    desktop_shutdown();
    
    // Shutdown window manager
    window_manager_shutdown();
    
    // Clean up windows
    window_t* window = gui_system.windows;
    while (window) {
        window_t* next = window->next;
        window_destroy(window);
        window = next;
    }
    
    // Clean up events
    event_t* event = gui_system.event_queue;
    while (event) {
        event_t* next = event->next;
        event_destroy(event);
        event = next;
    }
    
    // Clean up graphics context
    if (gui_system.screen_gc) {
        graphics_context_destroy(gui_system.screen_gc);
    }
    
    gui_initialized = false;
    KINFO("GUI system shutdown complete");
}

gui_system_t* gui_get_system(void) {
    return &gui_system;
}

// Graphics operations
graphics_context_t* graphics_context_create(uint32_t width, uint32_t height, uint32_t bpp) {
    graphics_context_t* gc = kmalloc(sizeof(graphics_context_t));
    if (!gc) {
        return NULL;
    }
    
    // Allocate framebuffer
    size_t fb_size = width * height * (bpp / 8);
    gc->framebuffer = kmalloc(fb_size);
    if (!gc->framebuffer) {
        kfree(gc);
        return NULL;
    }
    
    gc->width = width;
    gc->height = height;
    gc->pitch = width * (bpp / 8);
    gc->bpp = bpp;
    gc->foreground = (color_t){255, 255, 255, 255};
    gc->background = (color_t){0, 0, 0, 255};
    gc->font_size = 12;
    gc->clipping_enabled = false;
    gc->clip_rect = (rect_t){0, 0, width, height};
    
    // Clear framebuffer
    graphics_clear(gc, gc->background);
    
    return gc;
}

void graphics_context_destroy(graphics_context_t* gc) {
    if (!gc) {
        return;
    }
    
    if (gc->framebuffer) {
        kfree(gc->framebuffer);
    }
    
    kfree(gc);
}

void graphics_clear(graphics_context_t* gc, color_t color) {
    if (!gc || !gc->framebuffer) {
        return;
    }
    
    uint32_t* fb = (uint32_t*)gc->framebuffer;
    uint32_t rgba = color_to_rgba(color);
    
    for (uint32_t i = 0; i < gc->width * gc->height; i++) {
        fb[i] = rgba;
    }
}

void graphics_set_pixel(graphics_context_t* gc, int x, int y, color_t color) {
    if (!gc || !gc->framebuffer || x < 0 || y < 0 || x >= gc->width || y >= gc->height) {
        return;
    }
    
    // Check clipping
    if (gc->clipping_enabled) {
        if (x < gc->clip_rect.x || y < gc->clip_rect.y ||
            x >= gc->clip_rect.x + gc->clip_rect.width ||
            y >= gc->clip_rect.y + gc->clip_rect.height) {
            return;
        }
    }
    
    uint32_t* fb = (uint32_t*)gc->framebuffer;
    uint32_t offset = y * gc->width + x;
    fb[offset] = color_to_rgba(color);
}

color_t graphics_get_pixel(graphics_context_t* gc, int x, int y) {
    if (!gc || !gc->framebuffer || x < 0 || y < 0 || x >= gc->width || y >= gc->height) {
        return (color_t){0, 0, 0, 0};
    }
    
    uint32_t* fb = (uint32_t*)gc->framebuffer;
    uint32_t offset = y * gc->width + x;
    return color_from_rgba(fb[offset]);
}

void graphics_draw_line(graphics_context_t* gc, int x1, int y1, int x2, int y2, color_t color) {
    if (!gc) {
        return;
    }
    
    // Bresenham's line algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        graphics_set_pixel(gc, x1, y1, color);
        
        if (x1 == x2 && y1 == y2) {
            break;
        }
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void graphics_draw_rect(graphics_context_t* gc, rect_t rect, color_t color, bool filled) {
    if (!gc) {
        return;
    }
    
    if (filled) {
        for (int y = rect.y; y < rect.y + rect.height; y++) {
            for (int x = rect.x; x < rect.x + rect.width; x++) {
                graphics_set_pixel(gc, x, y, color);
            }
        }
    } else {
        // Draw outline
        graphics_draw_line(gc, rect.x, rect.y, rect.x + rect.width - 1, rect.y, color);
        graphics_draw_line(gc, rect.x + rect.width - 1, rect.y, rect.x + rect.width - 1, rect.y + rect.height - 1, color);
        graphics_draw_line(gc, rect.x + rect.width - 1, rect.y + rect.height - 1, rect.x, rect.y + rect.height - 1, color);
        graphics_draw_line(gc, rect.x, rect.y + rect.height - 1, rect.x, rect.y, color);
    }
}

void graphics_draw_circle(graphics_context_t* gc, int x, int y, int radius, color_t color, bool filled) {
    if (!gc) {
        return;
    }
    
    // Midpoint circle algorithm
    int x0 = radius;
    int y0 = 0;
    int err = 0;
    
    while (x0 >= y0) {
        if (filled) {
            graphics_draw_line(gc, x - x0, y + y0, x + x0, y + y0, color);
            graphics_draw_line(gc, x - y0, y + x0, x + y0, y + x0, color);
            graphics_draw_line(gc, x - x0, y - y0, x + x0, y - y0, color);
            graphics_draw_line(gc, x - y0, y - x0, x + y0, y - x0, color);
        } else {
            graphics_set_pixel(gc, x + x0, y + y0, color);
            graphics_set_pixel(gc, x + y0, y + x0, color);
            graphics_set_pixel(gc, x - y0, y + x0, color);
            graphics_set_pixel(gc, x - x0, y + y0, color);
            graphics_set_pixel(gc, x - x0, y - y0, color);
            graphics_set_pixel(gc, x - y0, y - x0, color);
            graphics_set_pixel(gc, x + y0, y - x0, color);
            graphics_set_pixel(gc, x + x0, y - y0, color);
        }
        
        if (err <= 0) {
            y0 += 1;
            err += 2 * y0 + 1;
        }
        if (err > 0) {
            x0 -= 1;
            err -= 2 * x0 + 1;
        }
    }
}

void graphics_draw_text(graphics_context_t* gc, int x, int y, const char* text, color_t color) {
    if (!gc || !text) {
        return;
    }
    
    // Simple bitmap font rendering
    // In a real implementation, this would use a proper font system
    int char_width = 8;
    int char_height = 12;
    
    for (int i = 0; text[i]; i++) {
        char c = text[i];
        if (c >= 32 && c <= 126) {
            // Draw character (simplified)
            for (int cy = 0; cy < char_height; cy++) {
                for (int cx = 0; cx < char_width; cx++) {
                    // Simple character pattern
                    if ((c - 32) % 2 == 0) {
                        graphics_set_pixel(gc, x + i * char_width + cx, y + cy, color);
                    }
                }
            }
        }
    }
}

void graphics_copy_rect(graphics_context_t* gc, rect_t src_rect, graphics_context_t* dst_gc, rect_t dst_rect) {
    if (!gc || !dst_gc) {
        return;
    }
    
    for (int y = 0; y < src_rect.height && y < dst_rect.height; y++) {
        for (int x = 0; x < src_rect.width && x < dst_rect.width; x++) {
            color_t color = graphics_get_pixel(gc, src_rect.x + x, src_rect.y + y);
            graphics_set_pixel(dst_gc, dst_rect.x + x, dst_rect.y + y, color);
        }
    }
}

void graphics_set_clipping(graphics_context_t* gc, rect_t rect) {
    if (!gc) {
        return;
    }
    
    gc->clipping_enabled = true;
    gc->clip_rect = rect;
}

void graphics_clear_clipping(graphics_context_t* gc) {
    if (!gc) {
        return;
    }
    
    gc->clipping_enabled = false;
}

// Color operations
color_t color_make(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (color_t){r, g, b, a};
}

color_t color_blend(color_t color1, color_t color2, float alpha) {
    color_t result;
    result.r = (uint8_t)(color1.r * (1 - alpha) + color2.r * alpha);
    result.g = (uint8_t)(color1.g * (1 - alpha) + color2.g * alpha);
    result.b = (uint8_t)(color1.b * (1 - alpha) + color2.b * alpha);
    result.a = (uint8_t)(color1.a * (1 - alpha) + color2.a * alpha);
    return result;
}

uint32_t color_to_rgba(color_t color) {
    return (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
}

color_t color_from_rgba(uint32_t rgba) {
    color_t color;
    color.r = (rgba >> 16) & 0xFF;
    color.g = (rgba >> 8) & 0xFF;
    color.b = rgba & 0xFF;
    color.a = (rgba >> 24) & 0xFF;
    return color;
}

// Window management
window_t* window_create(const char* title, rect_t bounds, window_type_t type, uint32_t flags) {
    if (!title) {
        return NULL;
    }
    
    window_t* window = kmalloc(sizeof(window_t));
    if (!window) {
        return NULL;
    }
    
    memset(window, 0, sizeof(window_t));
    strncpy(window->title, title, sizeof(window->title) - 1);
    window->type = type;
    window->state = WINDOW_STATE_NORMAL;
    window->flags = flags;
    window->bounds = bounds;
    window->client_bounds = (rect_t){
        bounds.x + 2, bounds.y + 20,
        bounds.width - 4, bounds.height - 22
    };
    window->background = current_theme.background;
    
    // Create graphics context for window
    window->gc = graphics_context_create(bounds.width, bounds.height, GUI_DEFAULT_BPP);
    if (!window->gc) {
        kfree(window);
        return NULL;
    }
    
    // Add to window list
    spinlock_acquire(&gui_system.lock);
    window->next = gui_system.windows;
    gui_system.windows = window;
    gui_system.window_count++;
    spinlock_release(&gui_system.lock);
    
    return window;
}

void window_destroy(window_t* window) {
    if (!window) {
        return;
    }
    
    // Remove from window list
    spinlock_acquire(&gui_system.lock);
    window_t* current = gui_system.windows;
    window_t* prev = NULL;
    
    while (current) {
        if (current == window) {
            if (prev) {
                prev->next = current->next;
            } else {
                gui_system.windows = current->next;
            }
            gui_system.window_count--;
            break;
        }
        prev = current;
        current = current->next;
    }
    spinlock_release(&gui_system.lock);
    
    // Clean up graphics context
    if (window->gc) {
        graphics_context_destroy(window->gc);
    }
    
    // Clean up user data
    if (window->user_data) {
        kfree(window->user_data);
    }
    
    kfree(window);
}

int window_show(window_t* window) {
    if (!window) {
        return -1;
    }
    
    window->state = WINDOW_STATE_NORMAL;
    return 0;
}

int window_hide(window_t* window) {
    if (!window) {
        return -1;
    }
    
    window->state = WINDOW_STATE_HIDDEN;
    return 0;
}

int window_close(window_t* window) {
    if (!window) {
        return -1;
    }
    
    window_destroy(window);
    return 0;
}

int window_move(window_t* window, int x, int y) {
    if (!window) {
        return -1;
    }
    
    window->bounds.x = x;
    window->bounds.y = y;
    window->client_bounds.x = x + 2;
    window->client_bounds.y = y + 20;
    
    return 0;
}

int window_resize(window_t* window, int width, int height) {
    if (!window) {
        return -1;
    }
    
    window->bounds.width = width;
    window->bounds.height = height;
    window->client_bounds.width = width - 4;
    window->client_bounds.height = height - 22;
    
    // Recreate graphics context
    if (window->gc) {
        graphics_context_destroy(window->gc);
    }
    
    window->gc = graphics_context_create(width, height, GUI_DEFAULT_BPP);
    
    return 0;
}

int window_set_title(window_t* window, const char* title) {
    if (!window || !title) {
        return -1;
    }
    
    strncpy(window->title, title, sizeof(window->title) - 1);
    return 0;
}

int window_set_state(window_t* window, window_state_t state) {
    if (!window) {
        return -1;
    }
    
    window->state = state;
    return 0;
}

int window_focus(window_t* window) {
    if (!window) {
        return -1;
    }
    
    gui_system.focused_window = window;
    return 0;
}

int window_raise(window_t* window) {
    if (!window) {
        return -1;
    }
    
    // Move window to front of list
    spinlock_acquire(&gui_system.lock);
    
    // Remove from current position
    window_t* current = gui_system.windows;
    window_t* prev = NULL;
    
    while (current) {
        if (current == window) {
            if (prev) {
                prev->next = current->next;
            } else {
                gui_system.windows = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }
    
    // Add to front
    window->next = gui_system.windows;
    gui_system.windows = window;
    
    spinlock_release(&gui_system.lock);
    
    return 0;
}

int window_lower(window_t* window) {
    if (!window) {
        return -1;
    }
    
    // Move window to back of list
    // This is a simplified implementation
    return 0;
}

window_t* window_find_at_point(int x, int y) {
    window_t* window = gui_system.windows;
    while (window) {
        if (window->state != WINDOW_STATE_HIDDEN &&
            x >= window->bounds.x && x < window->bounds.x + window->bounds.width &&
            y >= window->bounds.y && y < window->bounds.y + window->bounds.height) {
            return window;
        }
        window = window->next;
    }
    
    return NULL;
}

window_t* window_get_top_level(window_t* window) {
    if (!window) {
        return gui_system.windows;
    }
    
    return window;
}

// Widget management
widget_t* widget_create(widget_type_t type, rect_t bounds, const char* name) {
    if (!name) {
        return NULL;
    }
    
    widget_t* widget = kmalloc(sizeof(widget_t));
    if (!widget) {
        return NULL;
    }
    
    memset(widget, 0, sizeof(widget_t));
    strncpy(widget->name, name, sizeof(widget->name) - 1);
    widget->type = type;
    widget->state = WIDGET_STATE_NORMAL;
    widget->bounds = bounds;
    widget->background = current_theme.background;
    widget->foreground = current_theme.foreground;
    
    return widget;
}

void widget_destroy(widget_t* widget) {
    if (!widget) {
        return;
    }
    
    // Free text
    if (widget->text) {
        kfree(widget->text);
    }
    
    // Free data
    if (widget->data) {
        kfree(widget->data);
    }
    
    kfree(widget);
}

int widget_add_child(widget_t* parent, widget_t* child) {
    if (!parent || !child) {
        return -1;
    }
    
    child->parent = parent;
    child->next_sibling = parent->children;
    if (parent->children) {
        parent->children->prev_sibling = child;
    }
    parent->children = child;
    
    return 0;
}

int widget_remove_child(widget_t* parent, widget_t* child) {
    if (!parent || !child) {
        return -1;
    }
    
    if (child->prev_sibling) {
        child->prev_sibling->next_sibling = child->next_sibling;
    } else {
        parent->children = child->next_sibling;
    }
    
    if (child->next_sibling) {
        child->next_sibling->prev_sibling = child->prev_sibling;
    }
    
    child->parent = NULL;
    child->next_sibling = NULL;
    child->prev_sibling = NULL;
    
    return 0;
}

int widget_move(widget_t* widget, int x, int y) {
    if (!widget) {
        return -1;
    }
    
    widget->bounds.x = x;
    widget->bounds.y = y;
    
    return 0;
}

int widget_resize(widget_t* widget, int width, int height) {
    if (!widget) {
        return -1;
    }
    
    widget->bounds.width = width;
    widget->bounds.height = height;
    
    return 0;
}

int widget_set_text(widget_t* widget, const char* text) {
    if (!widget) {
        return -1;
    }
    
    if (widget->text) {
        kfree(widget->text);
    }
    
    if (text) {
        widget->text = kmalloc(strlen(text) + 1);
        if (widget->text) {
            strcpy(widget->text, text);
        }
    } else {
        widget->text = NULL;
    }
    
    return 0;
}

int widget_set_visible(widget_t* widget, bool visible) {
    if (!widget) {
        return -1;
    }
    
    // This would be implemented with a visible flag
    return 0;
}

int widget_set_enabled(widget_t* widget, bool enabled) {
    if (!widget) {
        return -1;
    }
    
    widget->state = enabled ? WIDGET_STATE_NORMAL : WIDGET_STATE_DISABLED;
    return 0;
}

int widget_focus(widget_t* widget) {
    if (!widget) {
        return -1;
    }
    
    gui_system.focused_widget = widget;
    widget->state = WIDGET_STATE_FOCUSED;
    
    if (widget->on_focus) {
        widget->on_focus(widget);
    }
    
    return 0;
}

widget_t* widget_find_at_point(widget_t* parent, int x, int y) {
    if (!parent) {
        return NULL;
    }
    
    if (x >= parent->bounds.x && x < parent->bounds.x + parent->bounds.width &&
        y >= parent->bounds.y && y < parent->bounds.y + parent->bounds.height) {
        
        // Check children first
        widget_t* child = parent->children;
        while (child) {
            widget_t* found = widget_find_at_point(child, x, y);
            if (found) {
                return found;
            }
            child = child->next_sibling;
        }
        
        return parent;
    }
    
    return NULL;
}

widget_t* widget_find_by_name(widget_t* parent, const char* name) {
    if (!parent || !name) {
        return NULL;
    }
    
    if (strcmp(parent->name, name) == 0) {
        return parent;
    }
    
    widget_t* child = parent->children;
    while (child) {
        widget_t* found = widget_find_by_name(child, name);
        if (found) {
            return found;
        }
        child = child->next_sibling;
    }
    
    return NULL;
}

// Widget-specific operations
widget_t* button_create(rect_t bounds, const char* text) {
    widget_t* button = widget_create(WIDGET_TYPE_BUTTON, bounds, "button");
    if (button && text) {
        widget_set_text(button, text);
    }
    return button;
}

widget_t* label_create(rect_t bounds, const char* text) {
    widget_t* label = widget_create(WIDGET_TYPE_LABEL, bounds, "label");
    if (label && text) {
        widget_set_text(label, text);
    }
    return label;
}

widget_t* textbox_create(rect_t bounds, const char* text) {
    widget_t* textbox = widget_create(WIDGET_TYPE_TEXTBOX, bounds, "textbox");
    if (textbox && text) {
        widget_set_text(textbox, text);
    }
    return textbox;
}

widget_t* listbox_create(rect_t bounds) {
    return widget_create(WIDGET_TYPE_LISTBOX, bounds, "listbox");
}

widget_t* checkbox_create(rect_t bounds, const char* text) {
    widget_t* checkbox = widget_create(WIDGET_TYPE_CHECKBOX, bounds, "checkbox");
    if (checkbox && text) {
        widget_set_text(checkbox, text);
    }
    return checkbox;
}

widget_t* radiobutton_create(rect_t bounds, const char* text) {
    widget_t* radio = widget_create(WIDGET_TYPE_RADIOBUTTON, bounds, "radiobutton");
    if (radio && text) {
        widget_set_text(radio, text);
    }
    return radio;
}

widget_t* slider_create(rect_t bounds, int min, int max, int value) {
    widget_t* slider = widget_create(WIDGET_TYPE_SLIDER, bounds, "slider");
    if (slider) {
        // Store min, max, value in widget data
        int* data = kmalloc(3 * sizeof(int));
        if (data) {
            data[0] = min;
            data[1] = max;
            data[2] = value;
            slider->data = data;
        }
    }
    return slider;
}

widget_t* progressbar_create(rect_t bounds, int min, int max, int value) {
    widget_t* progress = widget_create(WIDGET_TYPE_PROGRESSBAR, bounds, "progressbar");
    if (progress) {
        // Store min, max, value in widget data
        int* data = kmalloc(3 * sizeof(int));
        if (data) {
            data[0] = min;
            data[1] = max;
            data[2] = value;
            progress->data = data;
        }
    }
    return progress;
}

widget_t* menu_create(rect_t bounds) {
    return widget_create(WIDGET_TYPE_MENU, bounds, "menu");
}

widget_t* toolbar_create(rect_t bounds) {
    return widget_create(WIDGET_TYPE_TOOLBAR, bounds, "toolbar");
}

// Event handling
int gui_post_event(event_t* event) {
    if (!event) {
        return -1;
    }
    
    spinlock_acquire(&gui_system.lock);
    
    event->next = gui_system.event_queue;
    gui_system.event_queue = event;
    gui_system.event_count++;
    
    spinlock_release(&gui_system.lock);
    
    return 0;
}

event_t* gui_get_event(void) {
    spinlock_acquire(&gui_system.lock);
    
    event_t* event = gui_system.event_queue;
    if (event) {
        gui_system.event_queue = event->next;
        gui_system.event_count--;
        event->next = NULL;
    }
    
    spinlock_release(&gui_system.lock);
    
    return event;
}

void gui_process_events(void) {
    event_t* event;
    while ((event = gui_get_event()) != NULL) {
        // Handle event based on type
        switch (event->type) {
            case EVENT_TYPE_MOUSE_MOVE:
                gui_handle_mouse_move(event->x, event->y);
                break;
            case EVENT_TYPE_MOUSE_DOWN:
                gui_handle_mouse_button(event->x, event->y, event->button, true);
                break;
            case EVENT_TYPE_MOUSE_UP:
                gui_handle_mouse_button(event->x, event->y, event->button, false);
                break;
            case EVENT_TYPE_KEY_DOWN:
                gui_handle_key(event->key, true);
                break;
            case EVENT_TYPE_KEY_UP:
                gui_handle_key(event->key, false);
                break;
            case EVENT_TYPE_WINDOW_CLOSE:
                if (event->window) {
                    window_close(event->window);
                }
                break;
            default:
                break;
        }
        
        event_destroy(event);
    }
}

event_t* event_create(event_type_t type, window_t* window, widget_t* widget) {
    event_t* event = kmalloc(sizeof(event_t));
    if (!event) {
        return NULL;
    }
    
    memset(event, 0, sizeof(event_t));
    event->type = type;
    event->timestamp = hal_get_timestamp();
    event->window = window;
    event->widget = widget;
    
    return event;
}

void event_destroy(event_t* event) {
    if (event) {
        kfree(event);
    }
}

int event_set_mouse_data(event_t* event, int x, int y, int button) {
    if (!event) {
        return -1;
    }
    
    event->x = x;
    event->y = y;
    event->button = button;
    
    return 0;
}

int event_set_key_data(event_t* event, int key, int modifiers) {
    if (!event) {
        return -1;
    }
    
    event->key = key;
    event->modifiers = modifiers;
    
    return 0;
}

// Input handling
int gui_handle_mouse_move(int x, int y) {
    // Find window at point
    window_t* window = window_find_at_point(x, y);
    if (window) {
        // Convert to window coordinates
        int wx = x - window->bounds.x;
        int wy = y - window->bounds.y;
        
        // Find widget at point
        widget_t* widget = window->children;
        while (widget) {
            widget_t* found = widget_find_at_point(widget, wx, wy);
            if (found) {
                if (found->on_mouse_move) {
                    found->on_mouse_move(found, wx, wy);
                }
                break;
            }
            widget = widget->next_sibling;
        }
    }
    
    return 0;
}

int gui_handle_mouse_button(int x, int y, int button, bool pressed) {
    // Find window at point
    window_t* window = window_find_at_point(x, y);
    if (window) {
        // Convert to window coordinates
        int wx = x - window->bounds.x;
        int wy = y - window->bounds.y;
        
        // Find widget at point
        widget_t* widget = window->children;
        while (widget) {
            widget_t* found = widget_find_at_point(widget, wx, wy);
            if (found) {
                if (pressed) {
                    if (found->on_mouse_down) {
                        found->on_mouse_down(found, wx, wy, button);
                    }
                } else {
                    if (found->on_mouse_up) {
                        found->on_mouse_up(found, wx, wy, button);
                    }
                    if (found->on_click) {
                        found->on_click(found, wx, wy);
                    }
                }
                break;
            }
            widget = widget->next_sibling;
        }
    }
    
    return 0;
}

int gui_handle_mouse_wheel(int x, int y, int delta) {
    // Handle mouse wheel events
    return 0;
}

int gui_handle_key(int key, bool pressed) {
    if (gui_system.focused_widget) {
        if (pressed) {
            if (gui_system.focused_widget->on_key_down) {
                gui_system.focused_widget->on_key_down(gui_system.focused_widget, key);
            }
        } else {
            if (gui_system.focused_widget->on_key_up) {
                gui_system.focused_widget->on_key_up(gui_system.focused_widget, key);
            }
        }
    }
    
    return 0;
}

int gui_handle_text_input(const char* text) {
    // Handle text input events
    return 0;
}

// Rendering
void gui_render_all(void) {
    if (!gui_initialized) {
        return;
    }
    
    // Clear screen
    graphics_clear(gui_system.screen_gc, current_theme.background);
    
    // Render all windows
    window_t* window = gui_system.windows;
    while (window) {
        if (window->state != WINDOW_STATE_HIDDEN) {
            gui_render_window(window);
        }
        window = window->next;
    }
    
    // Composite to screen
    gui_composite_screen();
    
    last_render_time = hal_get_timestamp();
}

void gui_render_window(window_t* window) {
    if (!window || !window->gc) {
        return;
    }
    
    // Clear window
    graphics_clear(window->gc, window->background);
    
    // Draw window border
    graphics_draw_rect(window->gc, (rect_t){0, 0, window->bounds.width, window->bounds.height}, 
                      current_theme.border, false);
    
    // Draw title bar
    graphics_draw_rect(window->gc, (rect_t){1, 1, window->bounds.width - 2, 18}, 
                      current_theme.accent, true);
    
    // Draw title text
    graphics_draw_text(window->gc, 5, 3, window->title, current_theme.foreground);
    
    // Render widgets
    widget_t* widget = window->children;
    while (widget) {
        gui_render_widget(widget, window->gc);
        widget = widget->next_sibling;
    }
}

void gui_render_widget(widget_t* widget, graphics_context_t* gc) {
    if (!widget || !gc) {
        return;
    }
    
    // Draw widget background
    graphics_draw_rect(gc, widget->bounds, widget->background, true);
    
    // Draw widget border
    graphics_draw_rect(gc, widget->bounds, widget->foreground, false);
    
    // Draw widget text
    if (widget->text) {
        graphics_draw_text(gc, widget->bounds.x + 2, widget->bounds.y + 2, 
                          widget->text, widget->foreground);
    }
    
    // Render children
    widget_t* child = widget->children;
    while (child) {
        gui_render_widget(child, gc);
        child = child->next_sibling;
    }
}

void gui_composite_screen(void) {
    // Copy all windows to screen
    window_t* window = gui_system.windows;
    while (window) {
        if (window->state != WINDOW_STATE_HIDDEN && window->gc) {
            graphics_copy_rect(window->gc, (rect_t){0, 0, window->bounds.width, window->bounds.height},
                              gui_system.screen_gc, window->bounds);
        }
        window = window->next;
    }
}

// Window manager
int window_manager_init(void) {
    KINFO("Window manager initialized");
    return 0;
}

void window_manager_shutdown(void) {
    KINFO("Window manager shutdown");
}

int window_manager_add_window(window_t* window) {
    return 0;
}

int window_manager_remove_window(window_t* window) {
    return 0;
}

int window_manager_focus_window(window_t* window) {
    return window_focus(window);
}

int window_manager_raise_window(window_t* window) {
    return window_raise(window);
}

int window_manager_lower_window(window_t* window) {
    return window_lower(window);
}

window_t* window_manager_get_focused_window(void) {
    return gui_system.focused_window;
}

window_t* window_manager_get_window_at_point(int x, int y) {
    return window_find_at_point(x, y);
}

// Desktop environment
int desktop_init(void) {
    KINFO("Desktop environment initialized");
    return 0;
}

void desktop_shutdown(void) {
    KINFO("Desktop environment shutdown");
}

int desktop_show(void) {
    return 0;
}

int desktop_hide(void) {
    return 0;
}

int desktop_add_icon(const char* name, const char* icon_path, int x, int y) {
    return 0;
}

int desktop_remove_icon(const char* name) {
    return 0;
}

int desktop_show_context_menu(int x, int y) {
    return 0;
}

// Theme system
int theme_load(const char* name) {
    // Load theme from file
    return 0;
}

theme_t* theme_get_current(void) {
    return &current_theme;
}

int theme_set_current(const char* name) {
    // Set current theme
    return 0;
}

color_t theme_get_color(const char* name) {
    if (strcmp(name, "background") == 0) {
        return current_theme.background;
    } else if (strcmp(name, "foreground") == 0) {
        return current_theme.foreground;
    } else if (strcmp(name, "accent") == 0) {
        return current_theme.accent;
    } else if (strcmp(name, "border") == 0) {
        return current_theme.border;
    } else if (strcmp(name, "highlight") == 0) {
        return current_theme.highlight;
    } else if (strcmp(name, "shadow") == 0) {
        return current_theme.shadow;
    }
    
    return (color_t){0, 0, 0, 0};
}

// GUI utilities
rect_t rect_make(int x, int y, int width, int height) {
    return (rect_t){x, y, width, height};
}

bool rect_contains(rect_t rect, int x, int y) {
    return x >= rect.x && x < rect.x + rect.width &&
           y >= rect.y && y < rect.y + rect.height;
}

bool rect_intersects(rect_t rect1, rect_t rect2) {
    return rect1.x < rect2.x + rect2.width &&
           rect1.x + rect1.width > rect2.x &&
           rect1.y < rect2.y + rect2.height &&
           rect1.y + rect1.height > rect2.y;
}

rect_t rect_intersection(rect_t rect1, rect_t rect2) {
    int x1 = rect1.x > rect2.x ? rect1.x : rect2.x;
    int y1 = rect1.y > rect2.y ? rect1.y : rect2.y;
    int x2 = rect1.x + rect1.width < rect2.x + rect2.width ? 
             rect1.x + rect1.width : rect2.x + rect2.width;
    int y2 = rect1.y + rect1.height < rect2.y + rect2.height ? 
             rect1.y + rect1.height : rect2.y + rect2.height;
    
    if (x1 < x2 && y1 < y2) {
        return (rect_t){x1, y1, x2 - x1, y2 - y1};
    }
    
    return (rect_t){0, 0, 0, 0};
}

rect_t rect_union(rect_t rect1, rect_t rect2) {
    int x1 = rect1.x < rect2.x ? rect1.x : rect2.x;
    int y1 = rect1.y < rect2.y ? rect1.y : rect2.y;
    int x2 = rect1.x + rect1.width > rect2.x + rect2.width ? 
             rect1.x + rect1.width : rect2.x + rect2.width;
    int y2 = rect1.y + rect1.height > rect2.y + rect2.height ? 
             rect1.y + rect1.height : rect2.y + rect2.height;
    
    return (rect_t){x1, y1, x2 - x1, y2 - y1};
}

point_t point_make(int x, int y) {
    return (point_t){x, y};
}

bool point_in_rect(point_t point, rect_t rect) {
    return rect_contains(rect, point.x, point.y);
}

// GUI debugging
void gui_dump_windows(void) {
    KINFO("GUI Windows:");
    window_t* window = gui_system.windows;
    while (window) {
        KINFO("  %s: (%d,%d,%d,%d) state=%d", 
              window->title, window->bounds.x, window->bounds.y, 
              window->bounds.width, window->bounds.height, window->state);
        window = window->next;
    }
}

void gui_dump_widgets(widget_t* widget, int depth) {
    if (!widget) {
        return;
    }
    
    for (int i = 0; i < depth; i++) {
        KINFO("  ");
    }
    KINFO("%s: (%d,%d,%d,%d) type=%d", 
          widget->name, widget->bounds.x, widget->bounds.y, 
          widget->bounds.width, widget->bounds.height, widget->type);
    
    widget_t* child = widget->children;
    while (child) {
        gui_dump_widgets(child, depth + 1);
        child = child->next_sibling;
    }
}

void gui_dump_events(void) {
    KINFO("GUI Events: %d in queue", gui_system.event_count);
} 