#ifndef DIRECTX_COMPATIBILITY_H
#define DIRECTX_COMPATIBILITY_H

#include "../gui/rendering/raeen_dx.h"
#include <stdint.h>
#include <stdbool.h>

// DirectX Compatibility Layer - DirectX 11/12 support for RaeenOS
// Provides seamless DirectX compatibility for Windows games

// DirectX versions
typedef enum {
    DIRECTX_VERSION_11 = 0,      // DirectX 11
    DIRECTX_VERSION_12,          // DirectX 12
    DIRECTX_VERSION_12_1,        // DirectX 12.1
    DIRECTX_VERSION_12_2,        // DirectX 12.2
} directx_version_t;

// DirectX feature levels
typedef enum {
    DIRECTX_FEATURE_LEVEL_9_1 = 0,   // DirectX 9.1
    DIRECTX_FEATURE_LEVEL_9_2,       // DirectX 9.2
    DIRECTX_FEATURE_LEVEL_9_3,       // DirectX 9.3
    DIRECTX_FEATURE_LEVEL_10_0,      // DirectX 10.0
    DIRECTX_FEATURE_LEVEL_10_1,      // DirectX 10.1
    DIRECTX_FEATURE_LEVEL_11_0,      // DirectX 11.0
    DIRECTX_FEATURE_LEVEL_11_1,      // DirectX 11.1
    DIRECTX_FEATURE_LEVEL_12_0,      // DirectX 12.0
    DIRECTX_FEATURE_LEVEL_12_1,      // DirectX 12.1
    DIRECTX_FEATURE_LEVEL_12_2,      // DirectX 12.2
} directx_feature_level_t;

// DirectX adapter information
typedef struct {
    uint32_t id;
    char name[256];              // Adapter name
    char description[512];       // Adapter description
    uint64_t dedicated_video_memory;
    uint64_t dedicated_system_memory;
    uint64_t shared_system_memory;
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t sub_sys_id;
    uint32_t revision;
    directx_feature_level_t max_feature_level;
    bool is_software_adapter;
    bool is_hardware_adapter;
    bool supports_ray_tracing;
    bool supports_mesh_shaders;
    bool supports_variable_rate_shading;
    void* adapter_data;
} directx_adapter_t;

// DirectX device information
typedef struct {
    uint32_t id;
    directx_adapter_t* adapter;
    directx_version_t version;
    directx_feature_level_t feature_level;
    bool is_debug_enabled;
    bool is_validation_enabled;
    uint32_t max_texture_dimension;
    uint32_t max_anisotropy;
    uint32_t max_multisample_count;
    bool supports_compute_shaders;
    bool supports_tessellation;
    bool supports_geometry_shaders;
    bool supports_ray_tracing;
    bool supports_mesh_shaders;
    bool supports_variable_rate_shading;
    void* device_data;
} directx_device_t;

// DirectX swap chain information
typedef struct {
    uint32_t id;
    directx_device_t* device;
    uint32_t width, height;
    uint32_t buffer_count;
    raeen_color_format_t format;
    bool is_fullscreen;
    bool is_stereo;
    bool enable_vsync;
    uint32_t vsync_interval;
    void* swap_chain_data;
} directx_swap_chain_t;

// DirectX buffer types
typedef enum {
    DIRECTX_BUFFER_TYPE_VERTEX = 0,   // Vertex buffer
    DIRECTX_BUFFER_TYPE_INDEX,        // Index buffer
    DIRECTX_BUFFER_TYPE_CONSTANT,     // Constant buffer
    DIRECTX_BUFFER_TYPE_STRUCTURED,   // Structured buffer
    DIRECTX_BUFFER_TYPE_RAW,          // Raw buffer
    DIRECTX_BUFFER_TYPE_INDIRECT,     // Indirect buffer
} directx_buffer_type_t;

// DirectX buffer
typedef struct {
    uint32_t id;
    directx_device_t* device;
    directx_buffer_type_t type;
    uint32_t size;
    uint32_t stride;
    void* data;
    bool is_dynamic;
    bool is_staging;
    void* buffer_data;
} directx_buffer_t;

// DirectX texture types
typedef enum {
    DIRECTX_TEXTURE_TYPE_1D = 0,      // 1D texture
    DIRECTX_TEXTURE_TYPE_2D,          // 2D texture
    DIRECTX_TEXTURE_TYPE_3D,          // 3D texture
    DIRECTX_TEXTURE_TYPE_CUBE,        // Cube texture
    DIRECTX_TEXTURE_TYPE_ARRAY,       // Texture array
} directx_texture_type_t;

// DirectX texture
typedef struct {
    uint32_t id;
    directx_device_t* device;
    directx_texture_type_t type;
    uint32_t width, height, depth;
    uint32_t mip_levels;
    uint32_t array_size;
    raeen_color_format_t format;
    uint32_t sample_count;
    uint32_t sample_quality;
    bool is_render_target;
    bool is_depth_stencil;
    bool is_unordered_access;
    void* texture_data;
} directx_texture_t;

// DirectX shader types
typedef enum {
    DIRECTX_SHADER_TYPE_VERTEX = 0,   // Vertex shader
    DIRECTX_SHADER_TYPE_PIXEL,        // Pixel shader
    DIRECTX_SHADER_TYPE_GEOMETRY,     // Geometry shader
    DIRECTX_SHADER_TYPE_HULL,         // Hull shader
    DIRECTX_SHADER_TYPE_DOMAIN,       // Domain shader
    DIRECTX_SHADER_TYPE_COMPUTE,      // Compute shader
    DIRECTX_SHADER_TYPE_MESH,         // Mesh shader
    DIRECTX_SHADER_TYPE_AMPLIFICATION, // Amplification shader
} directx_shader_type_t;

// DirectX shader
typedef struct {
    uint32_t id;
    directx_device_t* device;
    directx_shader_type_t type;
    char* bytecode;
    uint32_t bytecode_size;
    char* entry_point;
    char* target;
    void* shader_data;
} directx_shader_t;

// DirectX pipeline state
typedef struct {
    uint32_t id;
    directx_device_t* device;
    directx_shader_t* vertex_shader;
    directx_shader_t* pixel_shader;
    directx_shader_t* geometry_shader;
    directx_shader_t* hull_shader;
    directx_shader_t* domain_shader;
    raeen_blend_mode_t blend_mode;
    raeen_depth_test_t depth_test;
    raeen_cull_mode_t cull_mode;
    bool enable_depth_write;
    bool enable_stencil;
    uint32_t stencil_ref;
    void* pipeline_data;
} directx_pipeline_state_t;

// DirectX command list
typedef struct {
    uint32_t id;
    directx_device_t* device;
    bool is_recording;
    uint32_t command_count;
    void* command_data;
} directx_command_list_t;

// DirectX fence
typedef struct {
    uint32_t id;
    directx_device_t* device;
    uint64_t current_value;
    uint64_t completed_value;
    void* fence_data;
} directx_fence_t;

// DirectX resource barrier
typedef enum {
    DIRECTX_RESOURCE_STATE_COMMON = 0,
    DIRECTX_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
    DIRECTX_RESOURCE_STATE_INDEX_BUFFER,
    DIRECTX_RESOURCE_STATE_RENDER_TARGET,
    DIRECTX_RESOURCE_STATE_UNORDERED_ACCESS,
    DIRECTX_RESOURCE_STATE_DEPTH_WRITE,
    DIRECTX_RESOURCE_STATE_DEPTH_READ,
    DIRECTX_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
    DIRECTX_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
    DIRECTX_RESOURCE_STATE_STREAM_OUT,
    DIRECTX_RESOURCE_STATE_INDIRECT_ARGUMENT,
    DIRECTX_RESOURCE_STATE_COPY_DEST,
    DIRECTX_RESOURCE_STATE_COPY_SOURCE,
    DIRECTX_RESOURCE_STATE_RESOLVE_DEST,
    DIRECTX_RESOURCE_STATE_RESOLVE_SOURCE,
    DIRECTX_RESOURCE_STATE_PRESENT,
    DIRECTX_RESOURCE_STATE_PREDICATION,
} directx_resource_state_t;

// DirectX compatibility configuration
typedef struct {
    directx_version_t target_version;
    directx_feature_level_t min_feature_level;
    bool enable_debug_layer;
    bool enable_validation;
    bool enable_gpu_validation;
    bool enable_dred;
    bool enable_pix;
    bool enable_break_on_error;
    bool enable_break_on_warning;
    uint32_t max_adapters;
    uint32_t max_devices;
    uint32_t max_swap_chains;
    uint32_t max_buffers;
    uint32_t max_textures;
    uint32_t max_shaders;
    uint32_t max_pipeline_states;
    uint32_t max_command_lists;
    uint32_t max_fences;
} directx_compatibility_config_t;

// DirectX compatibility context
typedef struct {
    directx_compatibility_config_t config;
    raeen_dx_context_t* raeen_dx;
    directx_adapter_t* adapters;
    uint32_t adapter_count;
    uint32_t max_adapters;
    directx_device_t* devices;
    uint32_t device_count;
    uint32_t max_devices;
    directx_device_t* active_device;
    directx_swap_chain_t* swap_chains;
    uint32_t swap_chain_count;
    uint32_t max_swap_chains;
    directx_buffer_t* buffers;
    uint32_t buffer_count;
    uint32_t max_buffers;
    directx_texture_t* textures;
    uint32_t texture_count;
    uint32_t max_textures;
    directx_shader_t* shaders;
    uint32_t shader_count;
    uint32_t max_shaders;
    directx_pipeline_state_t* pipeline_states;
    uint32_t pipeline_state_count;
    uint32_t max_pipeline_states;
    directx_command_list_t* command_lists;
    uint32_t command_list_count;
    uint32_t max_command_lists;
    directx_fence_t* fences;
    uint32_t fence_count;
    uint32_t max_fences;
    bool initialized;
    uint32_t next_adapter_id;
    uint32_t next_device_id;
    uint32_t next_swap_chain_id;
    uint32_t next_buffer_id;
    uint32_t next_texture_id;
    uint32_t next_shader_id;
    uint32_t next_pipeline_state_id;
    uint32_t next_command_list_id;
    uint32_t next_fence_id;
} directx_compatibility_t;

// Function prototypes

// Initialization and shutdown
directx_compatibility_t* directx_compatibility_init(raeen_dx_context_t* raeen_dx,
                                                  directx_compatibility_config_t* config);
void directx_compatibility_shutdown(directx_compatibility_t* dx);
bool directx_compatibility_is_initialized(directx_compatibility_t* dx);

// Adapter management
uint32_t directx_compatibility_add_adapter(directx_compatibility_t* dx, directx_adapter_t* adapter);
void directx_compatibility_remove_adapter(directx_compatibility_t* dx, uint32_t adapter_id);
directx_adapter_t* directx_compatibility_get_adapter(directx_compatibility_t* dx, uint32_t adapter_id);
directx_adapter_t* directx_compatibility_get_adapters(directx_compatibility_t* dx, uint32_t* count);
directx_adapter_t* directx_compatibility_get_best_adapter(directx_compatibility_t* dx);
uint32_t directx_compatibility_get_adapter_count(directx_compatibility_t* dx);

// Device management
uint32_t directx_compatibility_create_device(directx_compatibility_t* dx, uint32_t adapter_id, directx_version_t version);
void directx_compatibility_destroy_device(directx_compatibility_t* dx, uint32_t device_id);
directx_device_t* directx_compatibility_get_device(directx_compatibility_t* dx, uint32_t device_id);
directx_device_t* directx_compatibility_get_active_device(directx_compatibility_t* dx);
void directx_compatibility_set_active_device(directx_compatibility_t* dx, uint32_t device_id);
uint32_t directx_compatibility_get_device_count(directx_compatibility_t* dx);
bool directx_compatibility_test_device(directx_compatibility_t* dx, uint32_t device_id);

// Swap chain management
uint32_t directx_compatibility_create_swap_chain(directx_compatibility_t* dx, uint32_t device_id, 
                                                uint32_t width, uint32_t height, raeen_color_format_t format);
void directx_compatibility_destroy_swap_chain(directx_compatibility_t* dx, uint32_t swap_chain_id);
directx_swap_chain_t* directx_compatibility_get_swap_chain(directx_compatibility_t* dx, uint32_t swap_chain_id);
void directx_compatibility_resize_swap_chain(directx_compatibility_t* dx, uint32_t swap_chain_id, 
                                            uint32_t width, uint32_t height);
void directx_compatibility_present_swap_chain(directx_compatibility_t* dx, uint32_t swap_chain_id, uint32_t sync_interval);
raeen_texture_t* directx_compatibility_get_back_buffer(directx_compatibility_t* dx, uint32_t swap_chain_id);

// Buffer management
uint32_t directx_compatibility_create_buffer(directx_compatibility_t* dx, uint32_t device_id, 
                                           directx_buffer_type_t type, uint32_t size, uint32_t stride, void* data);
void directx_compatibility_destroy_buffer(directx_compatibility_t* dx, uint32_t buffer_id);
directx_buffer_t* directx_compatibility_get_buffer(directx_compatibility_t* dx, uint32_t buffer_id);
void directx_compatibility_update_buffer(directx_compatibility_t* dx, uint32_t buffer_id, void* data, uint32_t size);
void directx_compatibility_map_buffer(directx_compatibility_t* dx, uint32_t buffer_id, void** data);
void directx_compatibility_unmap_buffer(directx_compatibility_t* dx, uint32_t buffer_id);

// Texture management
uint32_t directx_compatibility_create_texture(directx_compatibility_t* dx, uint32_t device_id,
                                            directx_texture_type_t type, uint32_t width, uint32_t height, uint32_t depth,
                                            uint32_t mip_levels, uint32_t array_size, raeen_color_format_t format);
void directx_compatibility_destroy_texture(directx_compatibility_t* dx, uint32_t texture_id);
directx_texture_t* directx_compatibility_get_texture(directx_compatibility_t* dx, uint32_t texture_id);
void directx_compatibility_update_texture(directx_compatibility_t* dx, uint32_t texture_id, void* data, uint32_t pitch);
raeen_texture_t* directx_compatibility_get_texture_resource(directx_compatibility_t* dx, uint32_t texture_id);

// Shader management
uint32_t directx_compatibility_create_shader(directx_compatibility_t* dx, uint32_t device_id,
                                           directx_shader_type_t type, char* bytecode, uint32_t bytecode_size,
                                           const char* entry_point, const char* target);
void directx_compatibility_destroy_shader(directx_compatibility_t* dx, uint32_t shader_id);
directx_shader_t* directx_compatibility_get_shader(directx_compatibility_t* dx, uint32_t shader_id);
uint32_t directx_compatibility_compile_shader(directx_compatibility_t* dx, const char* source, const char* entry_point,
                                            const char* target, char** bytecode, uint32_t* bytecode_size);

// Pipeline state management
uint32_t directx_compatibility_create_pipeline_state(directx_compatibility_t* dx, uint32_t device_id,
                                                   directx_pipeline_state_t* pipeline_state);
void directx_compatibility_destroy_pipeline_state(directx_compatibility_t* dx, uint32_t pipeline_state_id);
directx_pipeline_state_t* directx_compatibility_get_pipeline_state(directx_compatibility_t* dx, uint32_t pipeline_state_id);

// Command list management
uint32_t directx_compatibility_create_command_list(directx_compatibility_t* dx, uint32_t device_id);
void directx_compatibility_destroy_command_list(directx_compatibility_t* dx, uint32_t command_list_id);
directx_command_list_t* directx_compatibility_get_command_list(directx_compatibility_t* dx, uint32_t command_list_id);
void directx_compatibility_reset_command_list(directx_compatibility_t* dx, uint32_t command_list_id);
void directx_compatibility_close_command_list(directx_compatibility_t* dx, uint32_t command_list_id);
void directx_compatibility_execute_command_list(directx_compatibility_t* dx, uint32_t command_list_id);

// Fence management
uint32_t directx_compatibility_create_fence(directx_compatibility_t* dx, uint32_t device_id);
void directx_compatibility_destroy_fence(directx_compatibility_t* dx, uint32_t fence_id);
directx_fence_t* directx_compatibility_get_fence(directx_compatibility_t* dx, uint32_t fence_id);
void directx_compatibility_signal_fence(directx_compatibility_t* dx, uint32_t fence_id, uint64_t value);
bool directx_compatibility_is_fence_complete(directx_compatibility_t* dx, uint32_t fence_id, uint64_t value);
void directx_compatibility_wait_for_fence(directx_compatibility_t* dx, uint32_t fence_id, uint64_t value);

// Resource barrier management
void directx_compatibility_transition_resource(directx_compatibility_t* dx, uint32_t command_list_id,
                                             uint32_t resource_id, directx_resource_state_t old_state,
                                             directx_resource_state_t new_state);

// Rendering commands
void directx_compatibility_clear_render_target(directx_compatibility_t* dx, uint32_t command_list_id,
                                             uint32_t render_target_id, float* color);
void directx_compatibility_clear_depth_stencil(directx_compatibility_t* dx, uint32_t command_list_id,
                                             uint32_t depth_stencil_id, float depth, uint8_t stencil);
void directx_compatibility_set_render_targets(directx_compatibility_t* dx, uint32_t command_list_id,
                                            uint32_t* render_target_ids, uint32_t count, uint32_t depth_stencil_id);
void directx_compatibility_set_viewport(directx_compatibility_t* dx, uint32_t command_list_id,
                                      float x, float y, float width, float height, float min_depth, float max_depth);
void directx_compatibility_set_scissor_rect(directx_compatibility_t* dx, uint32_t command_list_id,
                                          uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
void directx_compatibility_set_pipeline_state(directx_compatibility_t* dx, uint32_t command_list_id,
                                            uint32_t pipeline_state_id);
void directx_compatibility_set_vertex_buffer(directx_compatibility_t* dx, uint32_t command_list_id,
                                           uint32_t slot, uint32_t buffer_id, uint32_t offset, uint32_t stride);
void directx_compatibility_set_index_buffer(directx_compatibility_t* dx, uint32_t command_list_id,
                                          uint32_t buffer_id, uint32_t offset, uint32_t format);
void directx_compatibility_draw(directx_compatibility_t* dx, uint32_t command_list_id,
                              uint32_t vertex_count, uint32_t start_vertex);
void directx_compatibility_draw_indexed(directx_compatibility_t* dx, uint32_t command_list_id,
                                      uint32_t index_count, uint32_t start_index, int32_t base_vertex);

// Compute shader commands
void directx_compatibility_dispatch(directx_compatibility_t* dx, uint32_t command_list_id,
                                  uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z);

// Resource copying
void directx_compatibility_copy_buffer(directx_compatibility_t* dx, uint32_t command_list_id,
                                     uint32_t dst_buffer_id, uint32_t dst_offset,
                                     uint32_t src_buffer_id, uint32_t src_offset, uint32_t size);
void directx_compatibility_copy_texture(directx_compatibility_t* dx, uint32_t command_list_id,
                                      uint32_t dst_texture_id, uint32_t src_texture_id);

// Information
directx_version_t directx_compatibility_get_supported_version(directx_compatibility_t* dx);
directx_feature_level_t directx_compatibility_get_max_feature_level(directx_compatibility_t* dx);
bool directx_compatibility_supports_ray_tracing(directx_compatibility_t* dx);
bool directx_compatibility_supports_mesh_shaders(directx_compatibility_t* dx);
bool directx_compatibility_supports_variable_rate_shading(directx_compatibility_t* dx);

// Error handling
typedef enum {
    DIRECTX_COMPATIBILITY_SUCCESS = 0,
    DIRECTX_COMPATIBILITY_ERROR_INVALID_CONTEXT,
    DIRECTX_COMPATIBILITY_ERROR_INVALID_DEVICE,
    DIRECTX_COMPATIBILITY_ERROR_INVALID_RESOURCE,
    DIRECTX_COMPATIBILITY_ERROR_OUT_OF_MEMORY,
    DIRECTX_COMPATIBILITY_ERROR_UNSUPPORTED_FEATURE,
    DIRECTX_COMPATIBILITY_ERROR_INVALID_PARAMETER,
    DIRECTX_COMPATIBILITY_ERROR_DEVICE_REMOVED,
    DIRECTX_COMPATIBILITY_ERROR_DRIVER_INTERNAL_ERROR,
} directx_compatibility_error_t;

directx_compatibility_error_t directx_compatibility_get_last_error(void);
const char* directx_compatibility_get_error_string(directx_compatibility_error_t error);

#endif // DIRECTX_COMPATIBILITY_H 