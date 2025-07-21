#include "raeen_dx.h"
#include <kernel/memory/memory.h>
#include <kernel/libc/string.h>
#include <kernel/libc/stdio.h>
#include <kernel/hal/hal.h>

// Internal structures
typedef struct {
    uint32_t id;
    uint32_t width, height;
    uint32_t* pixels;
    bool dirty;
} internal_texture_t;

typedef struct {
    uint32_t id;
    uint32_t width, height;
    internal_texture_t* color_attachment;
    internal_texture_t* depth_attachment;
} internal_render_target_t;

typedef struct {
    uint32_t id;
    void* vertex_data;
    void* fragment_data;
    size_t vertex_size;
    size_t fragment_size;
} internal_shader_t;

// Global state
static raeen_dx_context_t* g_context = NULL;
static raeen_dx_error_t g_last_error = RAEEN_DX_SUCCESS;
static uint32_t g_frame_count = 0;
static uint64_t g_last_frame_time = 0;
static uint32_t g_fps = 0;

// Performance measurement
static char g_perf_name[64] = {0};
static uint64_t g_perf_start_time = 0;

// Internal functions
static void set_error(raeen_dx_error_t error) {
    g_last_error = error;
}

static uint32_t* get_framebuffer(void) {
    if (!g_context || !g_context->backbuffer) {
        return NULL;
    }
    return g_context->backbuffer->color_attachment->pixels;
}

static void clear_framebuffer(uint32_t color) {
    uint32_t* fb = get_framebuffer();
    if (!fb) return;
    
    uint32_t pixel_count = g_context->width * g_context->height;
    for (uint32_t i = 0; i < pixel_count; i++) {
        fb[i] = color;
    }
}

// Glass effect shader (software implementation)
static uint32_t apply_glass_effect(uint32_t pixel, raeen_glass_params_t* params) {
    uint8_t r, g, b, a;
    raeen_dx_color_to_rgba(pixel, &r, &g, &b, &a);
    
    // Apply transparency
    a = (uint8_t)(a * params->transparency);
    
    // Apply blur effect (simplified - in real implementation would use convolution)
    if (params->blur_radius > 0.0f) {
        // Simple blur by averaging with neighbors
        uint8_t blur_factor = (uint8_t)(params->blur_radius * 25.5f);
        r = (r + blur_factor) / 2;
        g = (g + blur_factor) / 2;
        b = (b + blur_factor) / 2;
    }
    
    return raeen_dx_color_rgba(r, g, b, a);
}

// Context management
raeen_dx_context_t* raeen_dx_init(uint32_t width, uint32_t height, raeen_format_t format) {
    if (g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    
    g_context = kmalloc(sizeof(raeen_dx_context_t));
    if (!g_context) {
        set_error(RAEEN_DX_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    memset(g_context, 0, sizeof(raeen_dx_context_t));
    g_context->width = width;
    g_context->height = height;
    g_context->format = format;
    g_context->vsync_enabled = true;
    g_context->gpu_acceleration = false; // Start with software rendering
    
    // Create backbuffer
    g_context->backbuffer = kmalloc(sizeof(raeen_render_target_t));
    if (!g_context->backbuffer) {
        set_error(RAEEN_DX_ERROR_OUT_OF_MEMORY);
        kfree(g_context);
        g_context = NULL;
        return NULL;
    }
    
    memset(g_context->backbuffer, 0, sizeof(raeen_render_target_t));
    g_context->backbuffer->id = 1;
    g_context->backbuffer->width = width;
    g_context->backbuffer->height = height;
    
    // Create color attachment
    g_context->backbuffer->color_attachment = kmalloc(sizeof(raeen_texture_t));
    if (!g_context->backbuffer->color_attachment) {
        set_error(RAEEN_DX_ERROR_OUT_OF_MEMORY);
        kfree(g_context->backbuffer);
        kfree(g_context);
        g_context = NULL;
        return NULL;
    }
    
    memset(g_context->backbuffer->color_attachment, 0, sizeof(raeen_texture_t));
    g_context->backbuffer->color_attachment->id = 1;
    g_context->backbuffer->color_attachment->width = width;
    g_context->backbuffer->color_attachment->height = height;
    g_context->backbuffer->color_attachment->format = format;
    
    // Allocate pixel buffer
    size_t buffer_size = width * height * 4; // 4 bytes per pixel (RGBA)
    g_context->backbuffer->color_attachment->data = kmalloc(buffer_size);
    if (!g_context->backbuffer->color_attachment->data) {
        set_error(RAEEN_DX_ERROR_OUT_OF_MEMORY);
        kfree(g_context->backbuffer->color_attachment);
        kfree(g_context->backbuffer);
        kfree(g_context);
        g_context = NULL;
        return NULL;
    }
    
    // Set up projection matrix (orthographic for 2D)
    g_context->projection_matrix = raeen_dx_matrix_ortho(0, width, height, 0);
    g_context->view_matrix = raeen_dx_matrix_identity();
    
    // Initialize frame timing
    g_last_frame_time = hal_get_tick_count();
    g_frame_count = 0;
    g_fps = 0;
    
    printf("RaeenDX: Initialized %dx%d display with format %d\n", width, height, format);
    return g_context;
}

void raeen_dx_shutdown(raeen_dx_context_t* context) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Free backbuffer
    if (context->backbuffer) {
        if (context->backbuffer->color_attachment) {
            if (context->backbuffer->color_attachment->data) {
                kfree(context->backbuffer->color_attachment->data);
            }
            kfree(context->backbuffer->color_attachment);
        }
        kfree(context->backbuffer);
    }
    
    // Free shaders
    if (context->default_shader) {
        raeen_dx_destroy_shader(context->default_shader);
    }
    if (context->glass_shader) {
        raeen_dx_destroy_shader(context->glass_shader);
    }
    if (context->blur_shader) {
        raeen_dx_destroy_shader(context->blur_shader);
    }
    
    kfree(context);
    g_context = NULL;
    
    printf("RaeenDX: Shutdown complete\n");
}

void raeen_dx_begin_frame(raeen_dx_context_t* context) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Update FPS calculation
    uint64_t current_time = hal_get_tick_count();
    if (current_time - g_last_frame_time >= 1000) { // Every second
        g_fps = g_frame_count;
        g_frame_count = 0;
        g_last_frame_time = current_time;
    }
    g_frame_count++;
}

void raeen_dx_end_frame(raeen_dx_context_t* context) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // In a real implementation, this would swap buffers
    // For now, we just mark the frame as complete
    if (context->backbuffer && context->backbuffer->color_attachment) {
        context->backbuffer->color_attachment->dirty = true;
    }
}

void raeen_dx_clear(raeen_dx_context_t* context, uint32_t color) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    clear_framebuffer(color);
}

// Shader management
raeen_shader_t* raeen_dx_create_shader(raeen_dx_context_t* context,
                                      raeen_shader_type_t vertex_type,
                                      raeen_shader_type_t fragment_type) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    
    raeen_shader_t* shader = kmalloc(sizeof(raeen_shader_t));
    if (!shader) {
        set_error(RAEEN_DX_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    memset(shader, 0, sizeof(raeen_shader_t));
    shader->id = 1; // Simple ID generation
    shader->vertex_type = vertex_type;
    shader->fragment_type = fragment_type;
    
    // In a real implementation, this would compile shaders
    // For now, we just store the types
    shader->vertex_shader = NULL;
    shader->fragment_shader = NULL;
    
    return shader;
}

void raeen_dx_destroy_shader(raeen_shader_t* shader) {
    if (!shader) return;
    
    if (shader->vertex_shader) {
        kfree(shader->vertex_shader);
    }
    if (shader->fragment_shader) {
        kfree(shader->fragment_shader);
    }
    
    kfree(shader);
}

void raeen_dx_use_shader(raeen_dx_context_t* context, raeen_shader_t* shader) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // In a real implementation, this would bind the shader
    // For now, we just store the reference
    context->default_shader = shader;
}

// Texture management
raeen_texture_t* raeen_dx_create_texture(raeen_dx_context_t* context,
                                        uint32_t width, uint32_t height,
                                        raeen_format_t format, void* data) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    
    raeen_texture_t* texture = kmalloc(sizeof(raeen_texture_t));
    if (!texture) {
        set_error(RAEEN_DX_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    memset(texture, 0, sizeof(raeen_texture_t));
    texture->id = 2; // Simple ID generation
    texture->width = width;
    texture->height = height;
    texture->format = format;
    
    // Allocate texture data
    size_t data_size = width * height * 4; // Assume RGBA8
    texture->data = kmalloc(data_size);
    if (!texture->data) {
        set_error(RAEEN_DX_ERROR_OUT_OF_MEMORY);
        kfree(texture);
        return NULL;
    }
    
    // Copy data if provided
    if (data) {
        memcpy(texture->data, data, data_size);
    } else {
        memset(texture->data, 0, data_size);
    }
    
    return texture;
}

void raeen_dx_destroy_texture(raeen_texture_t* texture) {
    if (!texture) return;
    
    if (texture->data) {
        kfree(texture->data);
    }
    kfree(texture);
}

void raeen_dx_bind_texture(raeen_dx_context_t* context, raeen_texture_t* texture, uint32_t slot) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // In a real implementation, this would bind the texture to a texture unit
    // For now, we just store the reference
}

// Rendering primitives
void raeen_dx_draw_rect(raeen_dx_context_t* context, raeen_rect_t rect, uint32_t color) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    uint32_t* fb = get_framebuffer();
    if (!fb) return;
    
    // Clamp coordinates
    int x1 = (int)rect.x;
    int y1 = (int)rect.y;
    int x2 = (int)(rect.x + rect.width);
    int y2 = (int)(rect.y + rect.height);
    
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > (int)context->width) x2 = context->width;
    if (y2 > (int)context->height) y2 = context->height;
    
    // Draw rectangle
    for (int y = y1; y < y2; y++) {
        for (int x = x1; x < x2; x++) {
            fb[y * context->width + x] = color;
        }
    }
}

void raeen_dx_draw_rect_glass(raeen_dx_context_t* context, raeen_rect_t rect,
                             raeen_glass_params_t* params) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    uint32_t* fb = get_framebuffer();
    if (!fb) return;
    
    // Clamp coordinates
    int x1 = (int)rect.x;
    int y1 = (int)rect.y;
    int x2 = (int)(rect.x + rect.width);
    int y2 = (int)(rect.y + rect.height);
    
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > (int)context->width) x2 = context->width;
    if (y2 > (int)context->height) y2 = context->height;
    
    // Draw glass rectangle
    for (int y = y1; y < y2; y++) {
        for (int x = x1; x < x2; x++) {
            uint32_t original_pixel = fb[y * context->width + x];
            uint32_t glass_pixel = apply_glass_effect(original_pixel, params);
            fb[y * context->width + x] = glass_pixel;
        }
    }
}

void raeen_dx_draw_texture(raeen_dx_context_t* context, raeen_rect_t rect,
                          raeen_texture_t* texture) {
    if (!context || context != g_context || !texture) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    uint32_t* fb = get_framebuffer();
    if (!fb) return;
    
    uint32_t* tex_data = (uint32_t*)texture->data;
    
    // Clamp coordinates
    int x1 = (int)rect.x;
    int y1 = (int)rect.y;
    int x2 = (int)(rect.x + rect.width);
    int y2 = (int)(rect.y + rect.height);
    
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > (int)context->width) x2 = context->width;
    if (y2 > (int)context->height) y2 = context->height;
    
    // Draw texture
    for (int y = y1; y < y2; y++) {
        for (int x = x1; x < x2; x++) {
            int tex_x = (x - x1) * texture->width / (int)rect.width;
            int tex_y = (y - y1) * texture->height / (int)rect.height;
            
            if (tex_x >= 0 && tex_x < (int)texture->width &&
                tex_y >= 0 && tex_y < (int)texture->height) {
                uint32_t tex_pixel = tex_data[tex_y * texture->width + tex_x];
                fb[y * context->width + x] = tex_pixel;
            }
        }
    }
}

void raeen_dx_draw_texture_glass(raeen_dx_context_t* context, raeen_rect_t rect,
                                raeen_texture_t* texture, raeen_glass_params_t* params) {
    if (!context || context != g_context || !texture) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    uint32_t* fb = get_framebuffer();
    if (!fb) return;
    
    uint32_t* tex_data = (uint32_t*)texture->data;
    
    // Clamp coordinates
    int x1 = (int)rect.x;
    int y1 = (int)rect.y;
    int x2 = (int)(rect.x + rect.width);
    int y2 = (int)(rect.y + rect.height);
    
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > (int)context->width) x2 = context->width;
    if (y2 > (int)context->height) y2 = context->height;
    
    // Draw glass texture
    for (int y = y1; y < y2; y++) {
        for (int x = x1; x < x2; x++) {
            int tex_x = (x - x1) * texture->width / (int)rect.width;
            int tex_y = (y - y1) * texture->height / (int)rect.height;
            
            if (tex_x >= 0 && tex_x < (int)texture->width &&
                tex_y >= 0 && tex_y < (int)texture->height) {
                uint32_t tex_pixel = tex_data[tex_y * texture->width + tex_x];
                uint32_t glass_pixel = apply_glass_effect(tex_pixel, params);
                fb[y * context->width + x] = glass_pixel;
            }
        }
    }
}

// Matrix operations
raeen_matrix_t raeen_dx_matrix_identity(void) {
    raeen_matrix_t matrix = {0};
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

raeen_matrix_t raeen_dx_matrix_ortho(float left, float right, float bottom, float top) {
    raeen_matrix_t matrix = {0};
    float width = right - left;
    float height = top - bottom;
    
    matrix.m[0][0] = 2.0f / width;
    matrix.m[1][1] = 2.0f / height;
    matrix.m[2][2] = -1.0f;
    matrix.m[3][0] = -(right + left) / width;
    matrix.m[3][1] = -(top + bottom) / height;
    matrix.m[3][3] = 1.0f;
    
    return matrix;
}

raeen_matrix_t raeen_dx_matrix_translate(float x, float y, float z) {
    raeen_matrix_t matrix = raeen_dx_matrix_identity();
    matrix.m[3][0] = x;
    matrix.m[3][1] = y;
    matrix.m[3][2] = z;
    return matrix;
}

raeen_matrix_t raeen_dx_matrix_scale(float x, float y, float z) {
    raeen_matrix_t matrix = {0};
    matrix.m[0][0] = x;
    matrix.m[1][1] = y;
    matrix.m[2][2] = z;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

raeen_matrix_t raeen_dx_matrix_multiply(raeen_matrix_t a, raeen_matrix_t b) {
    raeen_matrix_t result = {0};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                result.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    
    return result;
}

void raeen_dx_set_projection(raeen_dx_context_t* context, raeen_matrix_t matrix) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    context->projection_matrix = matrix;
}

void raeen_dx_set_view(raeen_dx_context_t* context, raeen_matrix_t matrix) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    context->view_matrix = matrix;
}

// Blending
void raeen_dx_set_blend_mode(raeen_dx_context_t* context, raeen_blend_mode_t mode) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    // In a real implementation, this would set the blend mode
}

void raeen_dx_set_alpha(raeen_dx_context_t* context, float alpha) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    // In a real implementation, this would set the global alpha
}

// Animation support (simplified)
void raeen_dx_animate_rect(raeen_dx_context_t* context, raeen_rect_t* rect,
                          raeen_rect_t target, raeen_animation_params_t* params) {
    if (!context || context != g_context || !rect || !params) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Simple linear interpolation
    float t = 0.0f; // This would be calculated based on time
    rect->x = rect->x + (target.x - rect->x) * t;
    rect->y = rect->y + (target.y - rect->y) * t;
    rect->width = rect->width + (target.width - rect->width) * t;
    rect->height = rect->height + (target.height - rect->height) * t;
}

void raeen_dx_animate_color(raeen_dx_context_t* context, uint32_t* color,
                           uint32_t target, raeen_animation_params_t* params) {
    if (!context || context != g_context || !color || !params) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Simple linear interpolation
    float t = 0.0f; // This would be calculated based on time
    uint8_t r1, g1, b1, a1, r2, g2, b2, a2;
    raeen_dx_color_to_rgba(*color, &r1, &g1, &b1, &a1);
    raeen_dx_color_to_rgba(target, &r2, &g2, &b2, &a2);
    
    uint8_t r = (uint8_t)(r1 + (r2 - r1) * t);
    uint8_t g = (uint8_t)(g1 + (g2 - g1) * t);
    uint8_t b = (uint8_t)(b1 + (b2 - b1) * t);
    uint8_t a = (uint8_t)(a1 + (a2 - a1) * t);
    
    *color = raeen_dx_color_rgba(r, g, b, a);
}

void raeen_dx_animate_glass(raeen_dx_context_t* context, raeen_glass_params_t* params,
                           raeen_glass_params_t target, raeen_animation_params_t* anim_params) {
    if (!context || context != g_context || !params || !anim_params) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Simple linear interpolation
    float t = 0.0f; // This would be calculated based on time
    params->blur_radius = params->blur_radius + (target.blur_radius - params->blur_radius) * t;
    params->transparency = params->transparency + (target.transparency - params->transparency) * t;
    params->border_width = params->border_width + (target.border_width - params->border_width) * t;
    params->corner_radius = params->corner_radius + (target.corner_radius - params->corner_radius) * t;
}

// Performance and optimization
void raeen_dx_enable_vsync(raeen_dx_context_t* context, bool enable) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    context->vsync_enabled = enable;
}

void raeen_dx_set_gpu_acceleration(raeen_dx_context_t* context, bool enable) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return;
    }
    context->gpu_acceleration = enable;
}

uint32_t raeen_dx_get_fps(raeen_dx_context_t* context) {
    if (!context || context != g_context) {
        set_error(RAEEN_DX_ERROR_INVALID_CONTEXT);
        return 0;
    }
    return g_fps;
}

void raeen_dx_begin_performance_measurement(const char* name) {
    strncpy(g_perf_name, name, sizeof(g_perf_name) - 1);
    g_perf_start_time = hal_get_tick_count();
}

void raeen_dx_end_performance_measurement(void) {
    uint64_t end_time = hal_get_tick_count();
    uint64_t duration = end_time - g_perf_start_time;
    printf("RaeenDX Performance: %s took %llu ms\n", g_perf_name, duration);
}

// Utility functions
uint32_t raeen_dx_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (uint32_t)a << 24 | (uint32_t)b << 16 | (uint32_t)g << 8 | (uint32_t)r;
}

uint32_t raeen_dx_color_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return raeen_dx_color_rgba(r, g, b, 255);
}

void raeen_dx_color_to_rgba(uint32_t color, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a) {
    *r = (color >> 0) & 0xFF;
    *g = (color >> 8) & 0xFF;
    *b = (color >> 16) & 0xFF;
    *a = (color >> 24) & 0xFF;
}

raeen_rect_t raeen_dx_rect(float x, float y, float width, float height) {
    raeen_rect_t rect = {x, y, width, height};
    return rect;
}

// Error handling
raeen_dx_error_t raeen_dx_get_last_error(void) {
    return g_last_error;
}

const char* raeen_dx_get_error_string(raeen_dx_error_t error) {
    switch (error) {
        case RAEEN_DX_SUCCESS:
            return "Success";
        case RAEEN_DX_ERROR_INVALID_CONTEXT:
            return "Invalid context";
        case RAEEN_DX_ERROR_INVALID_SHADER:
            return "Invalid shader";
        case RAEEN_DX_ERROR_INVALID_TEXTURE:
            return "Invalid texture";
        case RAEEN_DX_ERROR_GPU_NOT_SUPPORTED:
            return "GPU not supported";
        case RAEEN_DX_ERROR_OUT_OF_MEMORY:
            return "Out of memory";
        default:
            return "Unknown error";
    }
} 