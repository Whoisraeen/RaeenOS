#include "ui_framework.h"
#include "kernel.h"

// Global UI state
static bool ui_initialized = false;
static ui_context_t ui_context = {0};
static ui_theme_t* current_theme = NULL;
static ui_widget_t* root_widget = NULL;
static ui_widget_t* focused_widget = NULL;

// Built-in themes
ui_theme_t ui_theme_light = {
    .name = "Light",
    .window_background = {240, 240, 240, 255},
    .widget_background = {255, 255, 255, 255},
    .text_color = {0, 0, 0, 255},
    .accent_color = {0, 120, 215, 255},
    .border_color = {200, 200, 200, 255},
    .shadow_color = {0, 0, 0, 50},
    .default_font = {"System", 12, 400, false},
    .title_font = {"System", 14, 600, false},
    .monospace_font = {"Monospace", 11, 400, false},
    .border_width = 1,
    .padding = 8,
    .margin = 4,
    .corner_radius = 4,
    .animation_duration_ms = 200,
    .animations_enabled = true
};

ui_theme_t ui_theme_dark = {
    .name = "Dark",
    .window_background = {30, 30, 30, 255},
    .widget_background = {45, 45, 45, 255},
    .text_color = {255, 255, 255, 255},
    .accent_color = {100, 150, 255, 255},
    .border_color = {70, 70, 70, 255},
    .shadow_color = {0, 0, 0, 100},
    .default_font = {"System", 12, 400, false},
    .title_font = {"System", 14, 600, false},
    .monospace_font = {"Monospace", 11, 400, false},
    .border_width = 1,
    .padding = 8,
    .margin = 4,
    .corner_radius = 4,
    .animation_duration_ms = 200,
    .animations_enabled = true
};

ui_theme_t ui_theme_macos_like = {
    .name = "macOS Style",
    .window_background = {245, 245, 245, 255},
    .widget_background = {255, 255, 255, 255},
    .text_color = {0, 0, 0, 255},
    .accent_color = {0, 122, 255, 255},
    .border_color = {200, 200, 200, 255},
    .shadow_color = {0, 0, 0, 30},
    .default_font = {"-apple-system", 13, 400, false},
    .title_font = {"-apple-system", 16, 600, false},
    .monospace_font = {"SF Mono", 12, 400, false},
    .border_width = 1,
    .padding = 12,
    .margin = 8,
    .corner_radius = 8,
    .animation_duration_ms = 250,
    .animations_enabled = true
};

ui_theme_t ui_theme_windows_like = {
    .name = "Windows Style",
    .window_background = {240, 240, 240, 255},
    .widget_background = {255, 255, 255, 255},
    .text_color = {0, 0, 0, 255},
    .accent_color = {0, 120, 215, 255},
    .border_color = {173, 173, 173, 255},
    .shadow_color = {0, 0, 0, 40},
    .default_font = {"Segoe UI", 12, 400, false},
    .title_font = {"Segoe UI", 16, 600, false},
    .monospace_font = {"Consolas", 11, 400, false},
    .border_width = 1,
    .padding = 6,
    .margin = 3,
    .corner_radius = 2,
    .animation_duration_ms = 150,
    .animations_enabled = true
};

error_t ui_init(u32 screen_width, u32 screen_height, u32 bpp) {
    if (ui_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing UI Framework (%ux%u, %u bpp)", screen_width, screen_height, bpp);
    
    // Initialize rendering context
    ui_context.width = screen_width;
    ui_context.height = screen_height;
    ui_context.bpp = bpp;
    ui_context.pitch = screen_width * (bpp / 8);
    ui_context.clip_rect = ui_rect(0, 0, screen_width, screen_height);
    
    // TODO: Initialize framebuffer through graphics subsystem
    // ui_context.framebuffer = graphics_get_framebuffer();
    
    // Set default theme
    current_theme = &ui_theme_light;
    ui_context.theme = current_theme;
    
    // Create root widget (desktop)
    root_widget = ui_create_widget(UI_WIDGET_PANEL, NULL);
    if (!root_widget) {
        KERROR("Failed to create root widget");
        return E_NOMEM;
    }
    
    ui_set_widget_bounds(root_widget, ui_rect(0, 0, screen_width, screen_height));
    root_widget->background_color = current_theme->window_background;
    
    ui_initialized = true;
    KINFO("UI Framework initialized successfully");
    
    return SUCCESS;
}

void ui_shutdown(void) {
    if (!ui_initialized) {
        return;
    }
    
    KINFO("Shutting down UI Framework");
    
    // Destroy all widgets
    if (root_widget) {
        ui_destroy_widget(root_widget);
        root_widget = NULL;
    }
    
    focused_widget = NULL;
    current_theme = NULL;
    
    ui_initialized = false;
}

bool ui_is_initialized(void) {
    return ui_initialized;
}

void ui_poll_events(void) {
    // TODO: Poll input devices and generate UI events
    // This will be implemented when input subsystem is ready
}

bool ui_handle_event(ui_event_t* event) {
    if (!ui_initialized || !event) {
        return false;
    }
    
    // Find target widget if not specified
    if (!event->target && root_widget) {
        // TODO: Implement hit testing to find widget under mouse cursor
        event->target = root_widget;
    }
    
    // Route event to target widget
    if (event->target && event->target->event_handler) {
        return event->target->event_handler(event->target, event);
    }
    
    return false;
}

void ui_post_event(ui_event_t* event) {
    if (!ui_initialized || !event) {
        return;
    }
    
    // TODO: Add event to queue for later processing
    // For now, handle immediately
    ui_handle_event(event);
}

ui_widget_t* ui_create_widget(ui_widget_type_t type, ui_widget_t* parent) {
    if (!ui_initialized) {
        return NULL;
    }
    
    ui_widget_t* widget = NULL;
    
    // Allocate memory based on widget type
    switch (type) {
        case UI_WIDGET_WINDOW:
            widget = (ui_widget_t*)memory_alloc(sizeof(ui_window_t));
            break;
        default:
            widget = (ui_widget_t*)memory_alloc(sizeof(ui_widget_t));
            break;
    }
    
    if (!widget) {
        return NULL;
    }
    
    // Initialize base widget
    widget->type = type;
    widget->parent = parent;
    widget->first_child = NULL;
    widget->last_child = NULL;
    widget->next_sibling = NULL;
    widget->prev_sibling = NULL;
    widget->bounds = ui_rect(0, 0, 100, 50);
    widget->visible = true;
    widget->enabled = true;
    widget->focused = false;
    widget->background_color = current_theme->widget_background;
    widget->foreground_color = current_theme->text_color;
    widget->font = current_theme->default_font;
    widget->text = NULL;
    widget->user_data = NULL;
    widget->event_handler = NULL;
    
    // Initialize virtual function table
    widget->vtable.paint = NULL;
    widget->vtable.handle_event = NULL;
    widget->vtable.destroy = NULL;
    widget->vtable.get_preferred_size = NULL;
    
    // Add to parent if specified
    if (parent) {
        ui_add_child(parent, widget);
    }
    
    return widget;
}

void ui_destroy_widget(ui_widget_t* widget) {
    if (!widget) {
        return;
    }
    
    // Remove focus if this widget has it
    if (focused_widget == widget) {
        focused_widget = NULL;
    }
    
    // Destroy all children first
    ui_widget_t* child = widget->first_child;
    while (child) {
        ui_widget_t* next = child->next_sibling;
        ui_destroy_widget(child);
        child = next;
    }
    
    // Remove from parent
    if (widget->parent) {
        ui_remove_child(widget->parent, widget);
    }
    
    // Call custom destructor if provided
    if (widget->vtable.destroy) {
        widget->vtable.destroy(widget);
    }
    
    // Free text if allocated
    if (widget->text) {
        memory_free(widget->text);
    }
    
    // Free widget memory
    memory_free(widget);
}

void ui_add_child(ui_widget_t* parent, ui_widget_t* child) {
    if (!parent || !child || child->parent == parent) {
        return;
    }
    
    // Remove from current parent first
    if (child->parent) {
        ui_remove_child(child->parent, child);
    }
    
    // Add to new parent
    child->parent = parent;
    child->next_sibling = NULL;
    
    if (parent->last_child) {
        parent->last_child->next_sibling = child;
        child->prev_sibling = parent->last_child;
        parent->last_child = child;
    } else {
        parent->first_child = child;
        parent->last_child = child;
        child->prev_sibling = NULL;
    }
}

void ui_remove_child(ui_widget_t* parent, ui_widget_t* child) {
    if (!parent || !child || child->parent != parent) {
        return;
    }
    
    // Update sibling links
    if (child->prev_sibling) {
        child->prev_sibling->next_sibling = child->next_sibling;
    } else {
        parent->first_child = child->next_sibling;
    }
    
    if (child->next_sibling) {
        child->next_sibling->prev_sibling = child->prev_sibling;
    } else {
        parent->last_child = child->prev_sibling;
    }
    
    // Clear parent and sibling pointers
    child->parent = NULL;
    child->prev_sibling = NULL;
    child->next_sibling = NULL;
}

void ui_set_focus(ui_widget_t* widget) {
    if (focused_widget == widget) {
        return;
    }
    
    // Remove focus from current widget
    if (focused_widget) {
        focused_widget->focused = false;
        // TODO: Send blur event
    }
    
    // Set focus to new widget
    focused_widget = widget;
    if (widget) {
        widget->focused = true;
        // TODO: Send focus event
    }
}

ui_widget_t* ui_get_focused_widget(void) {
    return focused_widget;
}

void ui_set_widget_bounds(ui_widget_t* widget, ui_rect_t bounds) {
    if (!widget) {
        return;
    }
    
    widget->bounds = bounds;
    ui_invalidate_widget(widget);
}

void ui_set_widget_text(ui_widget_t* widget, const char* text) {
    if (!widget) {
        return;
    }
    
    // Free existing text
    if (widget->text) {
        memory_free(widget->text);
        widget->text = NULL;
    }
    
    // Copy new text
    if (text) {
        size_t len = strlen(text);
        widget->text = (char*)memory_alloc(len + 1);
        if (widget->text) {
            strcpy(widget->text, text);
        }
    }
    
    ui_invalidate_widget(widget);
}

void ui_invalidate_widget(ui_widget_t* widget) {
    if (!widget) {
        return;
    }
    
    ui_invalidate_rect(widget->bounds);
}

void ui_invalidate_rect(ui_rect_t rect) {
    // TODO: Add rect to dirty region for next frame
    // For now, just mark entire screen as dirty
    (void)rect;
}

void ui_render_frame(void) {
    if (!ui_initialized || !root_widget) {
        return;
    }
    
    // TODO: Render all widgets to framebuffer
    // This is a placeholder implementation
}

ui_theme_t* ui_get_current_theme(void) {
    return current_theme;
}

void ui_set_theme(ui_theme_t* theme) {
    if (!theme) {
        return;
    }
    
    current_theme = theme;
    ui_context.theme = theme;
    
    // TODO: Invalidate all widgets to force redraw with new theme
}

// Utility functions
ui_color_t ui_rgb(u8 r, u8 g, u8 b) {
    return (ui_color_t){r, g, b, 255};
}

ui_color_t ui_rgba(u8 r, u8 g, u8 b, u8 a) {
    return (ui_color_t){r, g, b, a};
}

ui_point_t ui_point(s32 x, s32 y) {
    return (ui_point_t){x, y};
}

ui_size_t ui_size(u32 width, u32 height) {
    return (ui_size_t){width, height};
}

ui_rect_t ui_rect(s32 x, s32 y, u32 width, u32 height) {
    return (ui_rect_t){{x, y}, {width, height}};
}

bool ui_point_in_rect(ui_point_t point, ui_rect_t rect) {
    return point.x >= rect.position.x && 
           point.x < rect.position.x + (s32)rect.size.width &&
           point.y >= rect.position.y && 
           point.y < rect.position.y + (s32)rect.size.height;
}