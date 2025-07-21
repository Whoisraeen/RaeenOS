#ifndef ADVANCED_GUI_H
#define ADVANCED_GUI_H

#include "gui.h"
#include <stdint.h>
#include <stdbool.h>

// Advanced GUI constants
#define MAX_3D_OBJECTS 1000
#define MAX_TEXTURES 500
#define MAX_SHADERS 100
#define MAX_ANIMATIONS 200
#define MAX_MULTIMEDIA_STREAMS 50
#define MAX_DISPLAYS 4
#define MAX_GPU_DEVICES 4

// 3D graphics types
typedef enum {
    RENDERER_OPENGL = 0,
    RENDERER_VULKAN = 1,
    RENDERER_DIRECTX = 2,
    RENDERER_METAL = 3,
    RENDERER_SOFTWARE = 4
} renderer_type_t;

// 3D object types
typedef enum {
    OBJECT_TYPE_MESH = 0,
    OBJECT_TYPE_SPRITE = 1,
    OBJECT_TYPE_PARTICLE = 2,
    OBJECT_TYPE_LIGHT = 3,
    OBJECT_TYPE_CAMERA = 4
} object_type_t;

// 3D vector and matrix types
typedef struct vec3 {
    float x, y, z;
} vec3_t;

typedef struct vec4 {
    float x, y, z, w;
} vec4_t;

typedef struct mat4 {
    float m[16];
} mat4_t;

// 3D object information
typedef struct object_3d {
    uint32_t id;                            // Object ID
    object_type_t type;                     // Object type
    char name[64];                          // Object name
    
    // Transform
    vec3_t position;                        // Position
    vec3_t rotation;                        // Rotation (Euler angles)
    vec3_t scale;                           // Scale
    
    // Geometry
    vec3_t* vertices;                       // Vertex data
    uint32_t* indices;                      // Index data
    uint32_t vertex_count;                  // Number of vertices
    uint32_t index_count;                   // Number of indices
    
    // Material
    uint32_t texture_id;                    // Texture ID
    vec4_t color;                           // Color
    float shininess;                        // Shininess
    float transparency;                     // Transparency
    
    // Animation
    bool animated;                          // Is animated
    uint32_t animation_id;                  // Animation ID
    float animation_time;                   // Current animation time
    
    // Rendering
    bool visible;                           // Is visible
    bool wireframe;                         // Wireframe mode
    uint32_t shader_id;                     // Shader ID
    
    // Physics
    bool has_physics;                       // Has physics
    vec3_t velocity;                        // Velocity
    vec3_t acceleration;                    // Acceleration
    float mass;                             // Mass
    
    void* user_data;                        // User-specific data
} object_3d_t;

// Texture information
typedef struct texture {
    uint32_t id;                            // Texture ID
    char name[64];                          // Texture name
    uint32_t width;                         // Width
    uint32_t height;                        // Height
    uint32_t format;                        // Format (RGBA, RGB, etc.)
    uint8_t* data;                          // Texture data
    uint32_t size;                          // Data size
    bool mipmap;                            // Has mipmaps
    bool compressed;                        // Is compressed
} texture_t;

// Shader information
typedef struct shader {
    uint32_t id;                            // Shader ID
    char name[64];                          // Shader name
    char vertex_source[4096];               // Vertex shader source
    char fragment_source[4096];             // Fragment shader source
    bool compiled;                          // Is compiled
    uint32_t program_id;                    // Shader program ID
    void* uniforms;                         // Uniform variables
} shader_t;

// Animation information
typedef struct animation {
    uint32_t id;                            // Animation ID
    char name[64];                          // Animation name
    uint32_t object_id;                     // Target object ID
    float duration;                         // Animation duration
    float start_time;                       // Start time
    float end_time;                         // End time
    bool loop;                              // Loop animation
    bool playing;                           // Is playing
    float current_time;                     // Current time
    void* keyframes;                        // Keyframe data
} animation_t;

// Light information
typedef struct light_3d {
    uint32_t id;                            // Light ID
    char name[64];                          // Light name
    vec3_t position;                        // Position
    vec3_t direction;                       // Direction
    vec4_t color;                           // Color
    float intensity;                        // Intensity
    float range;                            // Range
    float spot_angle;                       // Spot angle
    bool enabled;                           // Is enabled
    bool cast_shadows;                      // Cast shadows
} light_3d_t;

// Camera information
typedef struct camera_3d {
    uint32_t id;                            // Camera ID
    char name[64];                          // Camera name
    vec3_t position;                        // Position
    vec3_t target;                          // Target
    vec3_t up;                              // Up vector
    float fov;                              // Field of view
    float near_plane;                       // Near plane
    float far_plane;                        // Far plane
    float aspect_ratio;                     // Aspect ratio
    mat4_t view_matrix;                     // View matrix
    mat4_t projection_matrix;               // Projection matrix
} camera_3d_t;

// Multimedia stream types
typedef enum {
    STREAM_TYPE_AUDIO = 0,
    STREAM_TYPE_VIDEO = 1,
    STREAM_TYPE_AUDIO_VIDEO = 2
} stream_type_t;

// Multimedia stream information
typedef struct multimedia_stream {
    uint32_t id;                            // Stream ID
    char name[64];                          // Stream name
    stream_type_t type;                     // Stream type
    bool playing;                           // Is playing
    bool paused;                            // Is paused
    bool looping;                           // Is looping
    float volume;                           // Volume (0.0 - 1.0)
    float speed;                            // Playback speed
    uint64_t duration;                      // Duration (ms)
    uint64_t current_time;                  // Current time (ms)
    char file_path[256];                    // File path
    void* decoder;                          // Decoder data
    void* renderer;                         // Renderer data
} multimedia_stream_t;

// Display information
typedef struct display_info {
    uint32_t id;                            // Display ID
    char name[64];                          // Display name
    uint32_t width;                         // Width
    uint32_t height;                        // Height
    uint32_t refresh_rate;                  // Refresh rate
    uint32_t color_depth;                   // Color depth
    bool primary;                           // Is primary display
    bool connected;                         // Is connected
    bool active;                            // Is active
} display_info_t;

// GPU device information
typedef struct gpu_device {
    uint32_t id;                            // GPU ID
    char name[64];                          // GPU name
    char vendor[64];                        // Vendor name
    uint32_t memory_size;                   // Memory size (MB)
    uint32_t compute_units;                 // Compute units
    uint32_t max_clock;                     // Max clock speed (MHz)
    bool supported;                         // Is supported
    bool active;                            // Is active
} gpu_device_t;

// Advanced GUI system
typedef struct advanced_gui_system {
    spinlock_t lock;                        // System lock
    bool initialized;                       // Initialization flag
    
    // 3D rendering
    renderer_type_t renderer_type;          // Current renderer
    bool renderer_initialized;              // Renderer initialized
    object_3d_t* objects[MAX_3D_OBJECTS];   // 3D objects
    uint32_t object_count;                  // Number of objects
    texture_t* textures[MAX_TEXTURES];      // Textures
    uint32_t texture_count;                 // Number of textures
    shader_t* shaders[MAX_SHADERS];         // Shaders
    uint32_t shader_count;                  // Number of shaders
    animation_t* animations[MAX_ANIMATIONS]; // Animations
    uint32_t animation_count;               // Number of animations
    light_3d_t* lights[MAX_3D_OBJECTS];     // Lights
    uint32_t light_count;                   // Number of lights
    camera_3d_t* cameras[MAX_3D_OBJECTS];   // Cameras
    uint32_t camera_count;                  // Number of cameras
    camera_3d_t* active_camera;             // Active camera
    
    // Multimedia
    multimedia_stream_t* streams[MAX_MULTIMEDIA_STREAMS]; // Multimedia streams
    uint32_t stream_count;                  // Number of streams
    bool audio_enabled;                     // Audio enabled
    bool video_enabled;                     // Video enabled
    
    // Display management
    display_info_t displays[MAX_DISPLAYS];  // Displays
    uint32_t display_count;                 // Number of displays
    display_info_t* primary_display;        // Primary display
    
    // GPU management
    gpu_device_t gpus[MAX_GPU_DEVICES];     // GPU devices
    uint32_t gpu_count;                     // Number of GPUs
    gpu_device_t* active_gpu;               // Active GPU
    
    // Advanced features
    bool vsync_enabled;                     // V-Sync enabled
    bool antialiasing_enabled;              // Anti-aliasing enabled
    uint32_t antialiasing_level;            // Anti-aliasing level
    bool shadows_enabled;                   // Shadows enabled
    bool reflections_enabled;               // Reflections enabled
    bool post_processing_enabled;           // Post-processing enabled
} advanced_gui_system_t;

// Advanced GUI statistics
typedef struct advanced_gui_stats {
    // 3D rendering statistics
    uint32_t objects_rendered;              // Objects rendered
    uint32_t triangles_rendered;            // Triangles rendered
    uint32_t draw_calls;                    // Draw calls
    uint32_t texture_switches;              // Texture switches
    uint32_t shader_switches;               // Shader switches
    
    // Performance statistics
    float frame_time;                       // Frame time (ms)
    float fps;                              // Frames per second
    float gpu_usage;                        // GPU usage (%)
    float memory_usage;                     // Memory usage (%)
    
    // Multimedia statistics
    uint32_t audio_streams;                 // Active audio streams
    uint32_t video_streams;                 // Active video streams
    uint64_t audio_bytes_processed;         // Audio bytes processed
    uint64_t video_frames_processed;        // Video frames processed
    
    // System statistics
    uint64_t last_update;                   // Last update time
} advanced_gui_stats_t;

// Function declarations

// System initialization
int advanced_gui_init(void);
void advanced_gui_shutdown(void);
advanced_gui_system_t* advanced_gui_get_system(void);

// 3D object management
object_3d_t* object_3d_create(const char* name, object_type_t type);
int object_3d_destroy(object_3d_t* object);
object_3d_t* object_3d_find(const char* name);
object_3d_t* object_3d_find_by_id(uint32_t id);
int object_3d_set_transform(object_3d_t* object, vec3_t position, vec3_t rotation, vec3_t scale);
int object_3d_set_geometry(object_3d_t* object, vec3_t* vertices, uint32_t* indices, uint32_t vertex_count, uint32_t index_count);
int object_3d_set_material(object_3d_t* object, uint32_t texture_id, vec4_t color, float shininess, float transparency);

// Texture management
texture_t* texture_create(const char* name, uint32_t width, uint32_t height, uint32_t format);
int texture_destroy(texture_t* texture);
int texture_load_from_file(texture_t* texture, const char* file_path);
int texture_load_from_memory(texture_t* texture, uint8_t* data, uint32_t size);
texture_t* texture_find(const char* name);
texture_t* texture_find_by_id(uint32_t id);

// Shader management
shader_t* shader_create(const char* name);
int shader_destroy(shader_t* shader);
int shader_set_source(shader_t* shader, const char* vertex_source, const char* fragment_source);
int shader_compile(shader_t* shader);
int shader_use(shader_t* shader);
shader_t* shader_find(const char* name);
shader_t* shader_find_by_id(uint32_t id);

// Animation management
animation_t* animation_create(const char* name, uint32_t object_id);
int animation_destroy(animation_t* animation);
int animation_play(animation_t* animation);
int animation_pause(animation_t* animation);
int animation_stop(animation_t* animation);
int animation_set_duration(animation_t* animation, float duration);
animation_t* animation_find(const char* name);
animation_t* animation_find_by_id(uint32_t id);

// Light management
light_3d_t* light_create(const char* name, vec3_t position, vec4_t color);
int light_destroy(light_3d_t* light);
int light_set_position(light_3d_t* light, vec3_t position);
int light_set_color(light_3d_t* light, vec4_t color);
int light_set_intensity(light_3d_t* light, float intensity);
light_3d_t* light_find(const char* name);
light_3d_t* light_find_by_id(uint32_t id);

// Camera management
camera_3d_t* camera_create(const char* name, vec3_t position, vec3_t target);
int camera_destroy(camera_3d_t* camera);
int camera_set_position(camera_3d_t* camera, vec3_t position);
int camera_set_target(camera_3d_t* camera, vec3_t target);
int camera_set_fov(camera_3d_t* camera, float fov);
int camera_set_active(camera_3d_t* camera);
camera_3d_t* camera_find(const char* name);
camera_3d_t* camera_get_active(void);

// Multimedia management
multimedia_stream_t* multimedia_stream_create(const char* name, stream_type_t type);
int multimedia_stream_destroy(multimedia_stream_t* stream);
int multimedia_stream_load_file(multimedia_stream_t* stream, const char* file_path);
int multimedia_stream_play(multimedia_stream_t* stream);
int multimedia_stream_pause(multimedia_stream_t* stream);
int multimedia_stream_stop(multimedia_stream_t* stream);
int multimedia_stream_set_volume(multimedia_stream_t* stream, float volume);
multimedia_stream_t* multimedia_stream_find(const char* name);
multimedia_stream_t* multimedia_stream_find_by_id(uint32_t id);

// Display management
int display_enumerate(void);
display_info_t* display_get_info(uint32_t display_id);
display_info_t* display_get_primary(void);
int display_set_primary(uint32_t display_id);
int display_set_resolution(uint32_t display_id, uint32_t width, uint32_t height);
int display_set_refresh_rate(uint32_t display_id, uint32_t refresh_rate);

// GPU management
int gpu_enumerate(void);
gpu_device_t* gpu_get_info(uint32_t gpu_id);
gpu_device_t* gpu_get_active(void);
int gpu_set_active(uint32_t gpu_id);
int gpu_get_usage(uint32_t gpu_id, float* usage);
int gpu_get_memory_usage(uint32_t gpu_id, float* usage);

// Advanced rendering
int renderer_set_type(renderer_type_t type);
int renderer_initialize(void);
int renderer_shutdown(void);
int renderer_begin_frame(void);
int renderer_end_frame(void);
int renderer_clear(vec4_t color);
int renderer_render_scene(void);
int renderer_set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

// Advanced features
int vsync_enable(bool enabled);
int antialiasing_enable(bool enabled, uint32_t level);
int shadows_enable(bool enabled);
int reflections_enable(bool enabled);
int post_processing_enable(bool enabled);

// Math utilities
vec3_t vec3_make(float x, float y, float z);
vec4_t vec4_make(float x, float y, float z, float w);
mat4_t mat4_identity(void);
mat4_t mat4_translation(vec3_t translation);
mat4_t mat4_rotation(vec3_t rotation);
mat4_t mat4_scale(vec3_t scale);
mat4_t mat4_multiply(mat4_t a, mat4_t b);
mat4_t mat4_look_at(vec3_t eye, vec3_t target, vec3_t up);
mat4_t mat4_perspective(float fov, float aspect, float near, float far);

// Advanced GUI statistics
void advanced_gui_get_stats(advanced_gui_stats_t* stats);
void advanced_gui_reset_stats(void);

// Advanced GUI debugging
void advanced_gui_dump_objects(void);
void advanced_gui_dump_textures(void);
void advanced_gui_dump_shaders(void);
void advanced_gui_dump_animations(void);
void advanced_gui_dump_lights(void);
void advanced_gui_dump_cameras(void);
void advanced_gui_dump_streams(void);
void advanced_gui_dump_displays(void);
void advanced_gui_dump_gpus(void);
void advanced_gui_dump_stats(void);

#endif // ADVANCED_GUI_H 