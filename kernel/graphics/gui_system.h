#ifndef RAEEEN_GUI_SYSTEM_H
#define RAEEEN_GUI_SYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"

// Graphics modes
#define GRAPHICS_MODE_TEXT         0
#define GRAPHICS_MODE_VGA          1
#define GRAPHICS_MODE_VESA         2
#define GRAPHICS_MODE_UEFI         3
#define GRAPHICS_MODE_DIRECTX      4
#define GRAPHICS_MODE_OPENGL       5
#define GRAPHICS_MODE_VULKAN       6

// Color formats
#define COLOR_FORMAT_RGB565        0
#define COLOR_FORMAT_RGB888        1
#define COLOR_FORMAT_ARGB8888      2
#define COLOR_FORMAT_RGBA8888      3
#define COLOR_FORMAT_BGR888        4
#define COLOR_FORMAT_BGRA8888      5

// Window states
#define WINDOW_STATE_NORMAL        0
#define WINDOW_STATE_MINIMIZED     1
#define WINDOW_STATE_MAXIMIZED     2
#define WINDOW_STATE_FULLSCREEN    3
#define WINDOW_STATE_HIDDEN        4

// Window flags
#define WINDOW_FLAG_RESIZABLE      0x0001
#define WINDOW_FLAG_MINIMIZABLE    0x0002
#define WINDOW_FLAG_MAXIMIZABLE    0x0004
#define WINDOW_FLAG_CLOSABLE       0x0008
#define WINDOW_FLAG_ALWAYS_ON_TOP  0x0010
#define WINDOW_FLAG_BORDERLESS     0x0020
#define WINDOW_FLAG_TRANSPARENT    0x0040
#define WINDOW_FLAG_MODAL          0x0080
#define WINDOW_FLAG_TOOLTIP        0x0100
#define WINDOW_FLAG_POPUP          0x0200
#define WINDOW_FLAG_UTILITY        0x0400
#define WINDOW_FLAG_DESKTOP        0x0800

// Event types
#define EVENT_TYPE_NONE            0
#define EVENT_TYPE_KEY_PRESS       1
#define EVENT_TYPE_KEY_RELEASE     2
#define EVENT_TYPE_MOUSE_MOVE      3
#define EVENT_TYPE_MOUSE_BUTTON    4
#define EVENT_TYPE_MOUSE_WHEEL     5
#define EVENT_TYPE_WINDOW_CREATE   6
#define EVENT_TYPE_WINDOW_DESTROY  7
#define EVENT_TYPE_WINDOW_RESIZE   8
#define EVENT_TYPE_WINDOW_MOVE     9
#define EVENT_TYPE_WINDOW_FOCUS    10
#define EVENT_TYPE_WINDOW_BLUR     11
#define EVENT_TYPE_WINDOW_CLOSE    12
#define EVENT_TYPE_WINDOW_MINIMIZE 13
#define EVENT_TYPE_WINDOW_MAXIMIZE 14
#define EVENT_TYPE_WINDOW_RESTORE  15
#define EVENT_TYPE_PAINT           16
#define EVENT_TYPE_TIMER           17
#define EVENT_TYPE_CUSTOM          18

// Key codes
#define KEY_UNKNOWN                0
#define KEY_A                      65
#define KEY_B                      66
#define KEY_C                      67
#define KEY_D                      68
#define KEY_E                      69
#define KEY_F                      70
#define KEY_G                      71
#define KEY_H                      72
#define KEY_I                      73
#define KEY_J                      74
#define KEY_K                      75
#define KEY_L                      76
#define KEY_M                      77
#define KEY_N                      78
#define KEY_O                      79
#define KEY_P                      80
#define KEY_Q                      81
#define KEY_R                      82
#define KEY_S                      83
#define KEY_T                      84
#define KEY_U                      85
#define KEY_V                      86
#define KEY_W                      87
#define KEY_X                      88
#define KEY_Y                      89
#define KEY_Z                      90
#define KEY_0                      48
#define KEY_1                      49
#define KEY_2                      50
#define KEY_3                      51
#define KEY_4                      52
#define KEY_5                      53
#define KEY_6                      54
#define KEY_7                      55
#define KEY_8                      56
#define KEY_9                      57
#define KEY_ENTER                  13
#define KEY_ESCAPE                 27
#define KEY_BACKSPACE              8
#define KEY_TAB                    9
#define KEY_SPACE                  32
#define KEY_SHIFT                  16
#define KEY_CTRL                   17
#define KEY_ALT                    18
#define KEY_F1                     112
#define KEY_F2                     113
#define KEY_F3                     114
#define KEY_F4                     115
#define KEY_F5                     116
#define KEY_F6                     117
#define KEY_F7                     118
#define KEY_F8                     119
#define KEY_F9                     120
#define KEY_F10                    121
#define KEY_F11                    122
#define KEY_F12                    123
#define KEY_UP                     38
#define KEY_DOWN                   40
#define KEY_LEFT                   37
#define KEY_RIGHT                  39
#define KEY_HOME                   36
#define KEY_END                    35
#define KEY_PAGE_UP                33
#define KEY_PAGE_DOWN              34
#define KEY_INSERT                 45
#define KEY_DELETE                 46

// Mouse buttons
#define MOUSE_BUTTON_LEFT          1
#define MOUSE_BUTTON_RIGHT         2
#define MOUSE_BUTTON_MIDDLE        3
#define MOUSE_BUTTON_X1            4
#define MOUSE_BUTTON_X2            5

// Drawing primitives
#define DRAW_MODE_SOLID            0
#define DRAW_MODE_WIREFRAME        1
#define DRAW_MODE_POINTS           2
#define DRAW_MODE_LINES            3
#define DRAW_MODE_TRIANGLES        4
#define DRAW_MODE_QUADS            5

// Font styles
#define FONT_STYLE_NORMAL          0
#define FONT_STYLE_BOLD            1
#define FONT_STYLE_ITALIC          2
#define FONT_STYLE_UNDERLINE       4
#define FONT_STYLE_STRIKETHROUGH   8

// Graphics context structure
typedef struct graphics_context {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t pitch;
    void *framebuffer;
    uint32_t color_format;
    uint32_t graphics_mode;
    void *driver_data;
    struct graphics_ops *ops;
} graphics_context_t;

// Graphics operations structure
typedef struct graphics_ops {
    int (*init)(graphics_context_t *ctx);
    void (*cleanup)(graphics_context_t *ctx);
    int (*set_mode)(graphics_context_t *ctx, uint32_t width, uint32_t height, uint32_t bpp);
    void (*clear)(graphics_context_t *ctx, uint32_t color);
    void (*put_pixel)(graphics_context_t *ctx, int x, int y, uint32_t color);
    uint32_t (*get_pixel)(graphics_context_t *ctx, int x, int y);
    void (*draw_line)(graphics_context_t *ctx, int x1, int y1, int x2, int y2, uint32_t color);
    void (*draw_rectangle)(graphics_context_t *ctx, int x, int y, int width, int height, uint32_t color);
    void (*fill_rectangle)(graphics_context_t *ctx, int x, int y, int width, int height, uint32_t color);
    void (*draw_circle)(graphics_context_t *ctx, int x, int y, int radius, uint32_t color);
    void (*fill_circle)(graphics_context_t *ctx, int x, int y, int radius, uint32_t color);
    void (*draw_triangle)(graphics_context_t *ctx, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
    void (*fill_triangle)(graphics_context_t *ctx, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
    void (*draw_text)(graphics_context_t *ctx, int x, int y, const char *text, uint32_t color);
    void (*draw_image)(graphics_context_t *ctx, int x, int y, const void *image_data, int width, int height);
    void (*blit)(graphics_context_t *ctx, int dest_x, int dest_y, graphics_context_t *src, int src_x, int src_y, int width, int height);
    void (*flip)(graphics_context_t *ctx);
    void (*vsync)(graphics_context_t *ctx);
} graphics_ops_t;

// Window structure
typedef struct window {
    uint32_t id;
    char title[256];
    int x;
    int y;
    int width;
    int height;
    int min_width;
    int min_height;
    int max_width;
    int max_height;
    uint32_t state;
    uint32_t flags;
    uint32_t background_color;
    uint32_t border_color;
    uint32_t border_width;
    graphics_context_t *graphics;
    struct window *parent;
    struct window *children;
    struct window *siblings;
    struct window *next;
    struct window *prev;
    void *user_data;
    bool visible;
    bool focused;
    bool dirty;
    spinlock_t lock;
} window_t;

// Event structure
typedef struct event {
    uint32_t type;
    uint32_t timestamp;
    window_t *window;
    union {
        struct {
            uint32_t key_code;
            uint32_t scan_code;
            bool ctrl;
            bool shift;
            bool alt;
            bool super;
        } key;
        struct {
            int x;
            int y;
            int delta_x;
            int delta_y;
            uint32_t buttons;
            uint32_t button;
            bool pressed;
        } mouse;
        struct {
            int x;
            int y;
            int width;
            int height;
        } window;
        struct {
            void *data;
            uint32_t size;
        } custom;
    } data;
} event_t;

// Event handler function type
typedef void (*event_handler_t)(event_t *event, void *user_data);

// Window manager structure
typedef struct window_manager {
    window_t *windows;
    window_t *focused_window;
    window_t *desktop_window;
    int window_count;
    graphics_context_t *screen;
    event_handler_t event_handler;
    void *event_handler_data;
    bool running;
    spinlock_t lock;
} window_manager_t;

// Compositor structure
typedef struct compositor {
    graphics_context_t *screen;
    window_t **window_stack;
    int window_count;
    int max_windows;
    bool vsync_enabled;
    bool hardware_acceleration;
    void *hw_accel_data;
    spinlock_t lock;
} compositor_t;

// Font structure
typedef struct font {
    char name[64];
    int size;
    uint32_t style;
    void *font_data;
    int (*get_char_width)(struct font *font, char c);
    int (*get_char_height)(struct font *font, char c);
    void (*draw_char)(graphics_context_t *ctx, struct font *font, int x, int y, char c, uint32_t color);
    void (*draw_string)(graphics_context_t *ctx, struct font *font, int x, int y, const char *text, uint32_t color);
} font_t;

// Widget structure
typedef struct widget {
    uint32_t type;
    char text[256];
    int x;
    int y;
    int width;
    int height;
    uint32_t background_color;
    uint32_t foreground_color;
    uint32_t border_color;
    uint32_t border_width;
    font_t *font;
    bool enabled;
    bool visible;
    bool focused;
    event_handler_t event_handler;
    void *user_data;
    struct widget *parent;
    struct widget *children;
    struct widget *siblings;
} widget_t;

// Desktop environment structure
typedef struct desktop_environment {
    window_t *desktop_window;
    widget_t *taskbar;
    widget_t *start_menu;
    widget_t *system_tray;
    widget_t *clock;
    widget_t *volume_control;
    widget_t *network_indicator;
    widget_t *battery_indicator;
    bool show_desktop_icons;
    bool show_taskbar;
    bool show_start_menu;
    bool show_system_tray;
} desktop_environment_t;

// Function prototypes

// Graphics system
void graphics_system_init(void);
graphics_context_t *graphics_context_create(uint32_t width, uint32_t height, uint32_t bpp);
void graphics_context_destroy(graphics_context_t *ctx);
int graphics_set_mode(uint32_t width, uint32_t height, uint32_t bpp);
graphics_context_t *graphics_get_screen(void);

// Graphics operations
void graphics_clear(graphics_context_t *ctx, uint32_t color);
void graphics_put_pixel(graphics_context_t *ctx, int x, int y, uint32_t color);
uint32_t graphics_get_pixel(graphics_context_t *ctx, int x, int y);
void graphics_draw_line(graphics_context_t *ctx, int x1, int y1, int x2, int y2, uint32_t color);
void graphics_draw_rectangle(graphics_context_t *ctx, int x, int y, int width, int height, uint32_t color);
void graphics_fill_rectangle(graphics_context_t *ctx, int x, int y, int width, int height, uint32_t color);
void graphics_draw_circle(graphics_context_t *ctx, int x, int y, int radius, uint32_t color);
void graphics_fill_circle(graphics_context_t *ctx, int x, int y, int radius, uint32_t color);
void graphics_draw_triangle(graphics_context_t *ctx, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
void graphics_fill_triangle(graphics_context_t *ctx, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
void graphics_draw_text(graphics_context_t *ctx, int x, int y, const char *text, uint32_t color);
void graphics_draw_image(graphics_context_t *ctx, int x, int y, const void *image_data, int width, int height);
void graphics_blit(graphics_context_t *ctx, int dest_x, int dest_y, graphics_context_t *src, int src_x, int src_y, int width, int height);
void graphics_flip(graphics_context_t *ctx);
void graphics_vsync(graphics_context_t *ctx);

// Color utilities
uint32_t graphics_color_rgb(uint8_t r, uint8_t g, uint8_t b);
uint32_t graphics_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint8_t graphics_color_get_red(uint32_t color);
uint8_t graphics_color_get_green(uint32_t color);
uint8_t graphics_color_get_blue(uint32_t color);
uint8_t graphics_color_get_alpha(uint32_t color);
uint32_t graphics_color_blend(uint32_t color1, uint32_t color2, float alpha);

// Window management
void window_manager_init(void);
window_t *window_create(const char *title, int x, int y, int width, int height, uint32_t flags);
void window_destroy(window_t *window);
int window_show(window_t *window);
int window_hide(window_t *window);
int window_focus(window_t *window);
int window_move(window_t *window, int x, int y);
int window_resize(window_t *window, int width, int height);
int window_minimize(window_t *window);
int window_maximize(window_t *window);
int window_restore(window_t *window);
int window_close(window_t *window);
window_t *window_find_at_position(int x, int y);
window_t *window_get_focused(void);
void window_set_focused(window_t *window);
void window_bring_to_front(window_t *window);
void window_send_to_back(window_t *window);

// Event system
void event_system_init(void);
int event_register_handler(event_handler_t handler, void *user_data);
int event_unregister_handler(event_handler_t handler);
void event_post(event_t *event);
void event_process(void);
void event_poll(void);
event_t *event_create(uint32_t type);
void event_destroy(event_t *event);

// Compositor
void compositor_init(void);
int compositor_add_window(window_t *window);
int compositor_remove_window(window_t *window);
int compositor_raise_window(window_t *window);
int compositor_lower_window(window_t *window);
void compositor_render(void);
void compositor_set_vsync(bool enabled);
void compositor_set_hardware_acceleration(bool enabled);

// Font system
void font_system_init(void);
font_t *font_load(const char *name, int size, uint32_t style);
void font_destroy(font_t *font);
int font_get_text_width(font_t *font, const char *text);
int font_get_text_height(font_t *font, const char *text);
void font_draw_text(graphics_context_t *ctx, font_t *font, int x, int y, const char *text, uint32_t color);

// Widget system
void widget_system_init(void);
widget_t *widget_create(uint32_t type, const char *text, int x, int y, int width, int height);
void widget_destroy(widget_t *widget);
int widget_add_child(widget_t *parent, widget_t *child);
int widget_remove_child(widget_t *parent, widget_t *child);
void widget_set_text(widget_t *widget, const char *text);
void widget_set_position(widget_t *widget, int x, int y);
void widget_set_size(widget_t *widget, int width, int height);
void widget_set_colors(widget_t *widget, uint32_t background, uint32_t foreground, uint32_t border);
void widget_set_font(widget_t *widget, font_t *font);
void widget_set_enabled(widget_t *widget, bool enabled);
void widget_set_visible(widget_t *widget, bool visible);
void widget_set_focused(widget_t *widget, bool focused);
void widget_set_event_handler(widget_t *widget, event_handler_t handler, void *user_data);
void widget_draw(widget_t *widget, graphics_context_t *ctx);
void widget_handle_event(widget_t *widget, event_t *event);

// Desktop environment
void desktop_environment_init(void);
desktop_environment_t *desktop_environment_create(void);
void desktop_environment_destroy(desktop_environment_t *de);
int desktop_environment_show_taskbar(desktop_environment_t *de, bool show);
int desktop_environment_show_start_menu(desktop_environment_t *de, bool show);
int desktop_environment_show_system_tray(desktop_environment_t *de, bool show);
int desktop_environment_show_desktop_icons(desktop_environment_t *de, bool show);
void desktop_environment_update_clock(desktop_environment_t *de);
void desktop_environment_update_volume(desktop_environment_t *de, int volume);
void desktop_environment_update_network(desktop_environment_t *de, bool connected);
void desktop_environment_update_battery(desktop_environment_t *de, int percentage);
void desktop_environment_render(desktop_environment_t *de);

// GUI utilities
int gui_show_message_box(const char *title, const char *message, uint32_t flags);
int gui_show_input_dialog(const char *title, const char *prompt, char *buffer, size_t buffer_size);
int gui_show_file_dialog(const char *title, char *filename, size_t filename_size, bool save);
int gui_show_color_dialog(uint32_t *color);
int gui_show_font_dialog(font_t **font);

// GUI themes
typedef struct {
    uint32_t window_background;
    uint32_t window_border;
    uint32_t window_title_background;
    uint32_t window_title_text;
    uint32_t button_background;
    uint32_t button_text;
    uint32_t button_border;
    uint32_t text_background;
    uint32_t text_foreground;
    uint32_t menu_background;
    uint32_t menu_text;
    uint32_t menu_selection;
    uint32_t scrollbar_background;
    uint32_t scrollbar_thumb;
    font_t *default_font;
} gui_theme_t;

void gui_theme_init(void);
gui_theme_t *gui_theme_create(void);
void gui_theme_destroy(gui_theme_t *theme);
int gui_theme_load(gui_theme_t *theme, const char *filename);
int gui_theme_save(gui_theme_t *theme, const char *filename);
void gui_theme_apply(gui_theme_t *theme);

// GUI debugging
void gui_dump_window_tree(window_t *window, int depth);
void gui_dump_widget_tree(widget_t *widget, int depth);
void gui_dump_event(event_t *event);
void gui_dump_graphics_context(graphics_context_t *ctx);

// GUI statistics
typedef struct {
    uint64_t windows_created;
    uint64_t windows_destroyed;
    uint64_t events_processed;
    uint64_t frames_rendered;
    uint64_t draw_calls;
    uint64_t pixels_drawn;
} gui_stats_t;

void gui_get_stats(gui_stats_t *stats);
void gui_reset_stats(void);

#endif // RAEEEN_GUI_SYSTEM_H 