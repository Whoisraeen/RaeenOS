#ifndef GLASS_COMPOSITOR_H
#define GLASS_COMPOSITOR_H

#include "raeen_dx.h"
#include <stdint.h>
#include <stdbool.h>

// Glass Compositor - Advanced rendering system for RaeenOS
// Provides glass-like effects, blur, transparency, and GPU acceleration

// Glass effect types
typedef enum {
    GLASS_EFFECT_NONE = 0,
    GLASS_EFFECT_FROSTED,        // Frosted glass effect
    GLASS_EFFECT_TRANSLUCENT,    // Translucent glass
    GLASS_EFFECT_MIRROR,         // Mirror-like reflection
    GLASS_EFFECT_CRYSTAL,        // Crystal clear glass
    GLASS_EFFECT_SMOKE,          // Smoked glass
    GLASS_EFFECT_CUSTOM          // Custom effect
} glass_effect_type_t;

// Blur algorithms
typedef enum {
    BLUR_ALGORITHM_GAUSSIAN = 0, // Gaussian blur (best quality)
    BLUR_ALGORITHM_BOX,          // Box blur (fast)
    BLUR_ALGORITHM_TENT,         // Tent blur (balanced)
    BLUR_ALGORITHM_KAWASE,       // Kawase blur (anime-style)
    BLUR_ALGORITHM_DUAL,         // Dual blur (high quality)
} blur_algorithm_t;

// Glass layer structure
typedef struct {
    uint32_t id;
    raeen_rect_t bounds;
    glass_effect_type_t effect_type;
    raeen_glass_params_t params;
    raeen_texture_t* texture;
    bool visible;
    bool dirty;
    float z_order;               // Depth ordering
} glass_layer_t;

// Compositor layer types
typedef enum {
    LAYER_TYPE_BACKGROUND = 0,   // Wallpaper/background
    LAYER_TYPE_DESKTOP,          // Desktop icons
    LAYER_TYPE_WINDOW,           // Application windows
    LAYER_TYPE_DOCK,             // Dock/taskbar
    LAYER_TYPE_MENU,             // Menus and dropdowns
    LAYER_TYPE_NOTIFICATION,     // Notifications
    LAYER_TYPE_OVERLAY,          // Overlays and modals
    LAYER_TYPE_CURSOR            // Mouse cursor
} layer_type_t;

// Compositor layer
typedef struct {
    uint32_t id;
    layer_type_t type;
    raeen_rect_t bounds;
    raeen_texture_t* texture;
    glass_layer_t* glass_effect;
    bool visible;
    bool dirty;
    float opacity;
    float z_order;
    void* user_data;             // User-specific data
} compositor_layer_t;

// Animation curve types
typedef enum {
    ANIMATION_CURVE_LINEAR = 0,
    ANIMATION_CURVE_EASE_IN,
    ANIMATION_CURVE_EASE_OUT,
    ANIMATION_CURVE_EASE_IN_OUT,
    ANIMATION_CURVE_BOUNCE,
    ANIMATION_CURVE_ELASTIC,
    ANIMATION_CURVE_CUSTOM
} animation_curve_t;

// Animation state
typedef struct {
    uint32_t id;
    bool active;
    float start_time;
    float duration;
    animation_curve_t curve;
    void* start_value;
    void* end_value;
    void* current_value;
    size_t value_size;
    void (*update_callback)(void* value, float t);
} animation_t;

// Compositor configuration
typedef struct {
    uint32_t width, height;
    bool enable_vsync;
    bool enable_gpu_acceleration;
    bool enable_glass_effects;
    bool enable_animations;
    uint32_t max_layers;
    uint32_t max_animations;
    float target_fps;
} compositor_config_t;

// Glass Compositor context
typedef struct {
    compositor_config_t config;
    raeen_dx_context_t* renderer;
    compositor_layer_t* layers;
    uint32_t layer_count;
    animation_t* animations;
    uint32_t animation_count;
    uint32_t next_layer_id;
    uint32_t next_animation_id;
    bool initialized;
    uint64_t frame_count;
    float last_frame_time;
    float current_fps;
} glass_compositor_t;

// Function prototypes

// Initialization and shutdown
glass_compositor_t* glass_compositor_init(compositor_config_t* config);
void glass_compositor_shutdown(glass_compositor_t* compositor);
void glass_compositor_resize(glass_compositor_t* compositor, uint32_t width, uint32_t height);

// Layer management
uint32_t glass_compositor_create_layer(glass_compositor_t* compositor, layer_type_t type);
void glass_compositor_destroy_layer(glass_compositor_t* compositor, uint32_t layer_id);
void glass_compositor_set_layer_bounds(glass_compositor_t* compositor, uint32_t layer_id, raeen_rect_t bounds);
void glass_compositor_set_layer_texture(glass_compositor_t* compositor, uint32_t layer_id, raeen_texture_t* texture);
void glass_compositor_set_layer_visible(glass_compositor_t* compositor, uint32_t layer_id, bool visible);
void glass_compositor_set_layer_opacity(glass_compositor_t* compositor, uint32_t layer_id, float opacity);
void glass_compositor_set_layer_z_order(glass_compositor_t* compositor, uint32_t layer_id, float z_order);

// Glass effects
uint32_t glass_compositor_create_glass_effect(glass_compositor_t* compositor, uint32_t layer_id, glass_effect_type_t effect_type);
void glass_compositor_destroy_glass_effect(glass_compositor_t* compositor, uint32_t layer_id);
void glass_compositor_set_glass_params(glass_compositor_t* compositor, uint32_t layer_id, raeen_glass_params_t* params);
void glass_compositor_set_blur_algorithm(glass_compositor_t* compositor, uint32_t layer_id, blur_algorithm_t algorithm);

// Rendering
void glass_compositor_begin_frame(glass_compositor_t* compositor);
void glass_compositor_end_frame(glass_compositor_t* compositor);
void glass_compositor_render(glass_compositor_t* compositor);
void glass_compositor_present(glass_compositor_t* compositor);

// Animations
uint32_t glass_compositor_create_animation(glass_compositor_t* compositor, float duration, animation_curve_t curve);
void glass_compositor_destroy_animation(glass_compositor_t* compositor, uint32_t animation_id);
void glass_compositor_animate_rect(glass_compositor_t* compositor, uint32_t animation_id, raeen_rect_t* rect, raeen_rect_t target);
void glass_compositor_animate_color(glass_compositor_t* compositor, uint32_t animation_id, uint32_t* color, uint32_t target);
void glass_compositor_animate_glass(glass_compositor_t* compositor, uint32_t animation_id, raeen_glass_params_t* params, raeen_glass_params_t target);
void glass_compositor_animate_opacity(glass_compositor_t* compositor, uint32_t animation_id, uint32_t layer_id, float target_opacity);
void glass_compositor_start_animation(glass_compositor_t* compositor, uint32_t animation_id);
void glass_compositor_stop_animation(glass_compositor_t* compositor, uint32_t animation_id);
void glass_compositor_update_animations(glass_compositor_t* compositor, float delta_time);

// Performance and statistics
float glass_compositor_get_fps(glass_compositor_t* compositor);
uint64_t glass_compositor_get_frame_count(glass_compositor_t* compositor);
void glass_compositor_get_performance_stats(glass_compositor_t* compositor, uint32_t* layer_count, uint32_t* animation_count, float* render_time);

// Utility functions
raeen_rect_t glass_compositor_get_screen_bounds(glass_compositor_t* compositor);
bool glass_compositor_is_point_visible(glass_compositor_t* compositor, float x, float y);
uint32_t glass_compositor_get_layer_at_point(glass_compositor_t* compositor, float x, float y);
void glass_compositor_screenshot(glass_compositor_t* compositor, raeen_rect_t area, void* buffer);

// Glass effect presets
raeen_glass_params_t glass_compositor_preset_frosted_glass(void);
raeen_glass_params_t glass_compositor_preset_translucent_glass(void);
raeen_glass_params_t glass_compositor_preset_mirror_glass(void);
raeen_glass_params_t glass_compositor_preset_crystal_glass(void);
raeen_glass_params_t glass_compositor_preset_smoke_glass(void);

// Animation curve functions
float glass_compositor_ease_linear(float t);
float glass_compositor_ease_in(float t);
float glass_compositor_ease_out(float t);
float glass_compositor_ease_in_out(float t);
float glass_compositor_ease_bounce(float t);
float glass_compositor_ease_elastic(float t);

// Error handling
typedef enum {
    GLASS_COMPOSITOR_SUCCESS = 0,
    GLASS_COMPOSITOR_ERROR_INVALID_CONTEXT,
    GLASS_COMPOSITOR_ERROR_INVALID_LAYER,
    GLASS_COMPOSITOR_ERROR_INVALID_ANIMATION,
    GLASS_COMPOSITOR_ERROR_OUT_OF_MEMORY,
    GLASS_COMPOSITOR_ERROR_GPU_NOT_SUPPORTED,
    GLASS_COMPOSITOR_ERROR_RENDER_FAILED,
} glass_compositor_error_t;

glass_compositor_error_t glass_compositor_get_last_error(void);
const char* glass_compositor_get_error_string(glass_compositor_error_t error);

#endif // GLASS_COMPOSITOR_H 