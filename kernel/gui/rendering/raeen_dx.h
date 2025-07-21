#ifndef RAEEN_DX_H
#define RAEEN_DX_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// RaeenDX Graphics API - Custom graphics interface for RaeenOS
// Combines the best of DirectX, Vulkan, and OpenGL for optimal performance

// Color formats
typedef enum {
    RAEEN_FORMAT_RGBA8 = 0,      // 32-bit RGBA
    RAEEN_FORMAT_BGRA8,          // 32-bit BGRA (Windows compatibility)
    RAEEN_FORMAT_RGB8,           // 24-bit RGB
    RAEEN_FORMAT_RGBA16,         // 64-bit RGBA (HDR support)
    RAEEN_FORMAT_RGBA32F,        // 128-bit float RGBA
} raeen_format_t;

// Blend modes for glass effects
typedef enum {
    RAEEN_BLEND_OPAQUE = 0,      // No blending
    RAEEN_BLEND_ALPHA,           // Standard alpha blending
    RAEEN_BLEND_ADDITIVE,        // Additive blending
    RAEEN_BLEND_MULTIPLY,        // Multiply blending
    RAEEN_BLEND_SCREEN,          // Screen blending
    RAEEN_BLEND_OVERLAY,         // Overlay blending
    RAEEN_BLEND_GLASS,           // Custom glass effect
} raeen_blend_mode_t;

// Shader types for glass effects
typedef enum {
    RAEEN_SHADER_VERTEX = 0,     // Vertex shader
    RAEEN_SHADER_FRAGMENT,       // Fragment shader
    RAEEN_SHADER_COMPUTE,        // Compute shader
    RAEEN_SHADER_GLASS,          // Glass effect shader
    RAEEN_SHADER_BLUR,           // Gaussian blur shader
} raeen_shader_type_t;

// Vertex structure for 2D rendering
typedef struct {
    float x, y;                  // Position
    float u, v;                  // Texture coordinates
    uint32_t color;              // RGBA color
    float alpha;                 // Alpha value for glass effects
} raeen_vertex_t;

// Rectangle structure
typedef struct {
    float x, y, width, height;
} raeen_rect_t;

// Matrix for transformations
typedef struct {
    float m[4][4];
} raeen_matrix_t;

// Texture handle
typedef struct {
    uint32_t id;
    uint32_t width, height;
    raeen_format_t format;
    void* data;
} raeen_texture_t;

// Render target (framebuffer)
typedef struct {
    uint32_t id;
    uint32_t width, height;
    raeen_texture_t* color_attachment;
    raeen_texture_t* depth_attachment;
} raeen_render_target_t;

// Shader program
typedef struct {
    uint32_t id;
    raeen_shader_type_t vertex_type;
    raeen_shader_type_t fragment_type;
    void* vertex_shader;
    void* fragment_shader;
} raeen_shader_t;

// Glass effect parameters
typedef struct {
    float blur_radius;           // Blur amount (0.0 - 10.0)
    float transparency;          // Transparency (0.0 - 1.0)
    float border_width;          // Border width
    uint32_t border_color;       // Border color
    float corner_radius;         // Corner radius for rounded corners
    bool enable_shadow;          // Enable drop shadow
    float shadow_offset_x;       // Shadow X offset
    float shadow_offset_y;       // Shadow Y offset
    float shadow_blur;           // Shadow blur radius
    uint32_t shadow_color;       // Shadow color
} raeen_glass_params_t;

// Animation parameters
typedef struct {
    float duration;              // Animation duration in seconds
    float easing;                // Easing function (0.0 = linear, 1.0 = ease-out)
    bool loop;                   // Loop animation
    bool reverse;                // Reverse animation
} raeen_animation_params_t;

// RaeenDX Context
typedef struct {
    uint32_t width, height;      // Display resolution
    raeen_format_t format;       // Display format
    raeen_render_target_t* backbuffer;
    raeen_shader_t* default_shader;
    raeen_shader_t* glass_shader;
    raeen_shader_t* blur_shader;
    raeen_matrix_t projection_matrix;
    raeen_matrix_t view_matrix;
    bool vsync_enabled;
    bool gpu_acceleration;
    void* gpu_context;           // GPU-specific context
} raeen_dx_context_t;

// Function prototypes

// Context management
raeen_dx_context_t* raeen_dx_init(uint32_t width, uint32_t height, raeen_format_t format);
void raeen_dx_shutdown(raeen_dx_context_t* context);
void raeen_dx_begin_frame(raeen_dx_context_t* context);
void raeen_dx_end_frame(raeen_dx_context_t* context);
void raeen_dx_clear(raeen_dx_context_t* context, uint32_t color);

// Shader management
raeen_shader_t* raeen_dx_create_shader(raeen_dx_context_t* context, 
                                      raeen_shader_type_t vertex_type,
                                      raeen_shader_type_t fragment_type);
void raeen_dx_destroy_shader(raeen_shader_t* shader);
void raeen_dx_use_shader(raeen_dx_context_t* context, raeen_shader_t* shader);

// Texture management
raeen_texture_t* raeen_dx_create_texture(raeen_dx_context_t* context,
                                        uint32_t width, uint32_t height,
                                        raeen_format_t format, void* data);
void raeen_dx_destroy_texture(raeen_texture_t* texture);
void raeen_dx_bind_texture(raeen_dx_context_t* context, raeen_texture_t* texture, uint32_t slot);

// Rendering primitives
void raeen_dx_draw_rect(raeen_dx_context_t* context, raeen_rect_t rect, uint32_t color);
void raeen_dx_draw_rect_glass(raeen_dx_context_t* context, raeen_rect_t rect, 
                             raeen_glass_params_t* params);
void raeen_dx_draw_texture(raeen_dx_context_t* context, raeen_rect_t rect, 
                          raeen_texture_t* texture);
void raeen_dx_draw_texture_glass(raeen_dx_context_t* context, raeen_rect_t rect,
                                raeen_texture_t* texture, raeen_glass_params_t* params);

// Matrix operations
raeen_matrix_t raeen_dx_matrix_identity(void);
raeen_matrix_t raeen_dx_matrix_ortho(float left, float right, float bottom, float top);
raeen_matrix_t raeen_dx_matrix_translate(float x, float y, float z);
raeen_matrix_t raeen_dx_matrix_scale(float x, float y, float z);
raeen_matrix_t raeen_dx_matrix_multiply(raeen_matrix_t a, raeen_matrix_t b);
void raeen_dx_set_projection(raeen_dx_context_t* context, raeen_matrix_t matrix);
void raeen_dx_set_view(raeen_dx_context_t* context, raeen_matrix_t matrix);

// Blending
void raeen_dx_set_blend_mode(raeen_dx_context_t* context, raeen_blend_mode_t mode);
void raeen_dx_set_alpha(raeen_dx_context_t* context, float alpha);

// Animation support
void raeen_dx_animate_rect(raeen_dx_context_t* context, raeen_rect_t* rect,
                          raeen_rect_t target, raeen_animation_params_t* params);
void raeen_dx_animate_color(raeen_dx_context_t* context, uint32_t* color,
                           uint32_t target, raeen_animation_params_t* params);
void raeen_dx_animate_glass(raeen_dx_context_t* context, raeen_glass_params_t* params,
                           raeen_glass_params_t target, raeen_animation_params_t* anim_params);

// Performance and optimization
void raeen_dx_enable_vsync(raeen_dx_context_t* context, bool enable);
void raeen_dx_set_gpu_acceleration(raeen_dx_context_t* context, bool enable);
uint32_t raeen_dx_get_fps(raeen_dx_context_t* context);
void raeen_dx_begin_performance_measurement(const char* name);
void raeen_dx_end_performance_measurement(void);

// Utility functions
uint32_t raeen_dx_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t raeen_dx_color_rgb(uint8_t r, uint8_t g, uint8_t b);
void raeen_dx_color_to_rgba(uint32_t color, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);
raeen_rect_t raeen_dx_rect(float x, float y, float width, float height);

// Error handling
typedef enum {
    RAEEN_DX_SUCCESS = 0,
    RAEEN_DX_ERROR_INVALID_CONTEXT,
    RAEEN_DX_ERROR_INVALID_SHADER,
    RAEEN_DX_ERROR_INVALID_TEXTURE,
    RAEEN_DX_ERROR_GPU_NOT_SUPPORTED,
    RAEEN_DX_ERROR_OUT_OF_MEMORY,
} raeen_dx_error_t;

raeen_dx_error_t raeen_dx_get_last_error(void);
const char* raeen_dx_get_error_string(raeen_dx_error_t error);

#endif // RAEEN_DX_H 