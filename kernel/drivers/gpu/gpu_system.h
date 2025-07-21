#ifndef GPU_SYSTEM_H
#define GPU_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memory.h>

// GPU System - Advanced graphics processing for RaeenOS
// Provides GPU acceleration, shaders, and modern graphics features

// GPU vendors
typedef enum {
    GPU_VENDOR_NVIDIA = 0,           // NVIDIA Corporation
    GPU_VENDOR_AMD,                  // Advanced Micro Devices
    GPU_VENDOR_INTEL,                // Intel Corporation
    GPU_VENDOR_APPLE,                // Apple Inc.
    GPU_VENDOR_ARM,                  // ARM Limited
    GPU_VENDOR_QUALCOMM,             // Qualcomm
    GPU_VENDOR_MALI,                 // ARM Mali
    GPU_VENDOR_POWERVR,              // Imagination Technologies
    GPU_VENDOR_ADRENO,               // Qualcomm Adreno
    GPU_VENDOR_CUSTOM,               // Custom GPU
} gpu_vendor_t;

// GPU architectures
typedef enum {
    GPU_ARCH_UNKNOWN = 0,            // Unknown architecture
    GPU_ARCH_TURING,                 // NVIDIA Turing
    GPU_ARCH_AMPERE,                 // NVIDIA Ampere
    GPU_ARCH_HOPPER,                 // NVIDIA Hopper
    GPU_ARCH_ADA,                    // NVIDIA Ada Lovelace
    GPU_ARCH_RDNA,                   // AMD RDNA
    GPU_ARCH_RDNA2,                  // AMD RDNA 2
    GPU_ARCH_RDNA3,                  // AMD RDNA 3
    GPU_ARCH_GCN,                    // AMD GCN
    GPU_ARCH_CDNA,                   // AMD CDNA
    GPU_ARCH_XE,                     // Intel Xe
    GPU_ARCH_ARC,                    // Intel Arc
    GPU_ARCH_APPLE_SILICON,          // Apple Silicon
    GPU_ARCH_ARM_MALI,               // ARM Mali
    GPU_ARCH_QUALCOMM_ADRENO,        // Qualcomm Adreno
} gpu_architecture_t;

// GPU memory types
typedef enum {
    GPU_MEMORY_GDDR6 = 0,            // GDDR6 memory
    GPU_MEMORY_GDDR6X,               // GDDR6X memory
    GPU_MEMORY_GDDR5,                // GDDR5 memory
    GPU_MEMORY_HBM2,                 // HBM2 memory
    GPU_MEMORY_HBM3,                 // HBM3 memory
    GPU_MEMORY_LPDDR4,               // LPDDR4 memory
    GPU_MEMORY_LPDDR5,               // LPDDR5 memory
    GPU_MEMORY_SHARED,               // Shared system memory
    GPU_MEMORY_UNIFIED,              // Unified memory
} gpu_memory_type_t;

// GPU features
typedef enum {
    GPU_FEATURE_RAY_TRACING = 0,     // Hardware ray tracing
    GPU_FEATURE_DLSS,                // Deep Learning Super Sampling
    GPU_FEATURE_FSR,                 // FidelityFX Super Resolution
    GPU_FEATURE_VRS,                 // Variable Rate Shading
    GPU_FEATURE_MESH_SHADERS,        // Mesh shaders
    GPU_FEATURE_RAY_QUERY,           // Ray query
    GPU_FEATURE_ACCELERATION_STRUCTURE, // Acceleration structures
    GPU_FEATURE_AI_ACCELERATION,     // AI acceleration
    GPU_FEATURE_TENSOR_CORES,        // Tensor cores
    GPU_FEATURE_RT_CORES,            // RT cores
    GPU_FEATURE_CUDA,                // CUDA support
    GPU_FEATURE_OPENCL,              // OpenCL support
    GPU_FEATURE_VULKAN,              // Vulkan support
    GPU_FEATURE_OPENGL,              // OpenGL support
    GPU_FEATURE_DIRECTX,             // DirectX support
    GPU_FEATURE_METAL,               // Metal support
} gpu_feature_t;

// GPU information
typedef struct {
    uint32_t id;
    char name[64];                   // GPU name
    char driver_version[32];         // Driver version
    gpu_vendor_t vendor;             // GPU vendor
    gpu_architecture_t architecture; // GPU architecture
    uint32_t device_id;              // Device ID
    uint32_t vendor_id;              // Vendor ID
    uint32_t revision_id;            // Revision ID
    uint32_t memory_size;            // Memory size in MB
    gpu_memory_type_t memory_type;   // Memory type
    uint32_t memory_bandwidth;       // Memory bandwidth in GB/s
    uint32_t memory_clock;           // Memory clock in MHz
    uint32_t core_clock;             // Core clock in MHz
    uint32_t boost_clock;            // Boost clock in MHz
    uint32_t shader_units;           // Number of shader units
    uint32_t rt_cores;               // Number of RT cores
    uint32_t tensor_cores;           // Number of tensor cores
    uint32_t compute_units;          // Number of compute units
    uint32_t stream_processors;      // Number of stream processors
    uint32_t texture_units;          // Number of texture units
    uint32_t render_output_units;    // Number of ROPs
    uint32_t max_resolution_width;   // Maximum resolution width
    uint32_t max_resolution_height;  // Maximum resolution height
    uint32_t max_refresh_rate;       // Maximum refresh rate
    bool is_integrated;              // Is integrated GPU
    bool is_discrete;                // Is discrete GPU
    bool is_mobile;                  // Is mobile GPU
    bool is_connected;               // Is connected
    bool is_enabled;                 // Is enabled
    bool is_primary;                 // Is primary GPU
    uint64_t driver_load_time;       // Driver load time
    void* gpu_data;                  // GPU-specific data
} gpu_info_t;

// GPU performance metrics
typedef struct {
    float gpu_usage;                 // GPU usage percentage
    float memory_usage;              // Memory usage percentage
    float temperature;               // Temperature in Celsius
    float power_consumption;         // Power consumption in watts
    uint32_t core_clock_current;     // Current core clock
    uint32_t memory_clock_current;   // Current memory clock
    uint32_t fan_speed;              // Fan speed in RPM
    uint32_t frames_rendered;        // Frames rendered
    float fps;                       // Frames per second
    uint32_t draw_calls;             // Draw calls per frame
    uint32_t triangles_rendered;     // Triangles rendered
    uint32_t pixels_filled;          // Pixels filled
    uint64_t memory_allocated;       // Memory allocated
    uint64_t memory_used;            // Memory used
    uint64_t memory_free;            // Memory free
} gpu_performance_t;

// GPU configuration
typedef struct {
    uint32_t gpu_id;                 // GPU ID
    uint32_t resolution_width;       // Resolution width
    uint32_t resolution_height;      // Resolution height
    uint32_t refresh_rate;           // Refresh rate
    uint32_t color_depth;            // Color depth
    bool enable_vsync;               // Enable V-sync
    bool enable_adaptive_sync;       // Enable adaptive sync
    bool enable_hdr;                 // Enable HDR
    bool enable_ray_tracing;         // Enable ray tracing
    bool enable_dlss;                // Enable DLSS
    bool enable_fsr;                 // Enable FSR
    bool enable_vrs;                 // Enable VRS
    uint32_t anisotropic_filtering;  // Anisotropic filtering level
    uint32_t anti_aliasing;          // Anti-aliasing level
    uint32_t texture_quality;        // Texture quality
    uint32_t shadow_quality;         // Shadow quality
    uint32_t lighting_quality;       // Lighting quality
    uint32_t post_processing;        // Post-processing quality
    bool enable_overclocking;        // Enable overclocking
    uint32_t power_limit;            // Power limit percentage
    uint32_t temperature_limit;      // Temperature limit
    uint32_t fan_curve;              // Fan curve
} gpu_config_t;

// GPU system configuration
typedef struct {
    bool enable_gpu_acceleration;    // Enable GPU acceleration
    bool enable_multi_gpu;           // Enable multi-GPU support
    bool enable_gpu_compute;         // Enable GPU compute
    bool enable_ray_tracing;         // Enable ray tracing
    bool enable_ai_acceleration;     // Enable AI acceleration
    bool enable_vulkan;              // Enable Vulkan support
    bool enable_opengl;              // Enable OpenGL support
    bool enable_directx;             // Enable DirectX support
    bool enable_metal;               // Enable Metal support
    bool enable_cuda;                // Enable CUDA support
    bool enable_opencl;              // Enable OpenCL support
    bool enable_gpu_monitoring;      // Enable GPU monitoring
    bool enable_gpu_overclocking;    // Enable GPU overclocking
    bool enable_gpu_power_management; // Enable GPU power management
    uint32_t max_gpus;               // Maximum GPUs
    uint32_t max_displays;           // Maximum displays
    uint32_t max_resolution_width;   // Maximum resolution width
    uint32_t max_resolution_height;  // Maximum resolution height
    uint32_t max_refresh_rate;       // Maximum refresh rate
    uint32_t max_color_depth;        // Maximum color depth
    uint32_t gpu_memory_limit;       // GPU memory limit in MB
    uint32_t performance_update_interval; // Performance update interval
} gpu_system_config_t;

// GPU system context
typedef struct {
    gpu_system_config_t config;
    gpu_info_t* gpus;
    uint32_t gpu_count;
    uint32_t max_gpus;
    gpu_info_t* primary_gpu;
    gpu_performance_t* performance;
    gpu_config_t* configurations;
    bool initialized;
    bool multi_gpu_enabled;
    bool gpu_acceleration_enabled;
    uint32_t next_gpu_id;
    uint64_t last_update_time;
} gpu_system_t;

// Function prototypes

// Initialization and shutdown
gpu_system_t* gpu_system_init(gpu_system_config_t* config);
void gpu_system_shutdown(gpu_system_t* gpu);
bool gpu_system_is_initialized(gpu_system_t* gpu);

// GPU detection and management
uint32_t gpu_system_add_gpu(gpu_system_t* gpu, gpu_info_t* gpu_info);
void gpu_system_remove_gpu(gpu_system_t* gpu, uint32_t gpu_id);
gpu_info_t* gpu_system_get_gpu(gpu_system_t* gpu, uint32_t gpu_id);
gpu_info_t* gpu_system_get_gpus(gpu_system_t* gpu, uint32_t* count);
uint32_t gpu_system_get_gpu_count(gpu_system_t* gpu);
gpu_info_t* gpu_system_get_primary_gpu(gpu_system_t* gpu);
void gpu_system_set_primary_gpu(gpu_system_t* gpu, uint32_t gpu_id);
gpu_info_t* gpu_system_get_gpus_by_vendor(gpu_system_t* gpu, gpu_vendor_t vendor, uint32_t* count);
gpu_info_t* gpu_system_get_discrete_gpus(gpu_system_t* gpu, uint32_t* count);
gpu_info_t* gpu_system_get_integrated_gpus(gpu_system_t* gpu, uint32_t* count);

// GPU configuration
void gpu_system_set_gpu_config(gpu_system_t* gpu, uint32_t gpu_id, gpu_config_t* config);
gpu_config_t* gpu_system_get_gpu_config(gpu_system_t* gpu, uint32_t gpu_id);
void gpu_system_set_resolution(gpu_system_t* gpu, uint32_t gpu_id, uint32_t width, uint32_t height);
void gpu_system_get_resolution(gpu_system_t* gpu, uint32_t gpu_id, uint32_t* width, uint32_t* height);
void gpu_system_set_refresh_rate(gpu_system_t* gpu, uint32_t gpu_id, uint32_t refresh_rate);
uint32_t gpu_system_get_refresh_rate(gpu_system_t* gpu, uint32_t gpu_id);
void gpu_system_set_color_depth(gpu_system_t* gpu, uint32_t gpu_id, uint32_t color_depth);
uint32_t gpu_system_get_color_depth(gpu_system_t* gpu, uint32_t gpu_id);

// GPU performance
gpu_performance_t* gpu_system_get_performance(gpu_system_t* gpu, uint32_t gpu_id);
void gpu_system_update_performance(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_enable_monitoring(gpu_system_t* gpu, uint32_t gpu_id, bool enable);
bool gpu_system_is_monitoring_enabled(gpu_system_t* gpu, uint32_t gpu_id);
void gpu_system_set_performance_update_interval(gpu_system_t* gpu, uint32_t interval);
uint32_t gpu_system_get_performance_update_interval(gpu_system_t* gpu);

// GPU acceleration
void gpu_system_enable_acceleration(gpu_system_t* gpu, bool enable);
bool gpu_system_is_acceleration_enabled(gpu_system_t* gpu);
bool gpu_system_supports_acceleration(gpu_system_t* gpu, uint32_t gpu_id);
void gpu_system_enable_multi_gpu(gpu_system_t* gpu, bool enable);
bool gpu_system_is_multi_gpu_enabled(gpu_system_t* gpu);
bool gpu_system_supports_multi_gpu(gpu_system_t* gpu);

// GPU features
bool gpu_system_supports_feature(gpu_system_t* gpu, uint32_t gpu_id, gpu_feature_t feature);
bool gpu_system_enable_feature(gpu_system_t* gpu, uint32_t gpu_id, gpu_feature_t feature, bool enable);
bool gpu_system_is_feature_enabled(gpu_system_t* gpu, uint32_t gpu_id, gpu_feature_t feature);
uint32_t gpu_system_get_supported_features(gpu_system_t* gpu, uint32_t gpu_id, gpu_feature_t* features, uint32_t max_features);

// GPU compute
bool gpu_system_enable_compute(gpu_system_t* gpu, uint32_t gpu_id, bool enable);
bool gpu_system_is_compute_enabled(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_supports_cuda(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_supports_opencl(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_supports_vulkan(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_supports_opengl(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_supports_directx(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_supports_metal(gpu_system_t* gpu, uint32_t gpu_id);

// GPU overclocking
bool gpu_system_enable_overclocking(gpu_system_t* gpu, uint32_t gpu_id, bool enable);
bool gpu_system_is_overclocking_enabled(gpu_system_t* gpu, uint32_t gpu_id);
void gpu_system_set_core_clock(gpu_system_t* gpu, uint32_t gpu_id, uint32_t clock);
uint32_t gpu_system_get_core_clock(gpu_system_t* gpu, uint32_t gpu_id);
void gpu_system_set_memory_clock(gpu_system_t* gpu, uint32_t gpu_id, uint32_t clock);
uint32_t gpu_system_get_memory_clock(gpu_system_t* gpu, uint32_t gpu_id);
void gpu_system_set_power_limit(gpu_system_t* gpu, uint32_t gpu_id, uint32_t limit);
uint32_t gpu_system_get_power_limit(gpu_system_t* gpu, uint32_t gpu_id);
void gpu_system_set_temperature_limit(gpu_system_t* gpu, uint32_t gpu_id, uint32_t limit);
uint32_t gpu_system_get_temperature_limit(gpu_system_t* gpu, uint32_t gpu_id);

// GPU power management
bool gpu_system_enable_power_management(gpu_system_t* gpu, uint32_t gpu_id, bool enable);
bool gpu_system_is_power_management_enabled(gpu_system_t* gpu, uint32_t gpu_id);
void gpu_system_set_power_state(gpu_system_t* gpu, uint32_t gpu_id, uint32_t state);
uint32_t gpu_system_get_power_state(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_suspend_gpu(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_resume_gpu(gpu_system_t* gpu, uint32_t gpu_id);

// GPU drivers
bool gpu_system_load_driver(gpu_system_t* gpu, uint32_t gpu_id, const char* driver_path);
bool gpu_system_unload_driver(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_is_driver_loaded(gpu_system_t* gpu, uint32_t gpu_id);
const char* gpu_system_get_driver_version(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_update_driver(gpu_system_t* gpu, uint32_t gpu_id, const char* driver_path);

// GPU memory management
uint64_t gpu_system_allocate_memory(gpu_system_t* gpu, uint32_t gpu_id, uint64_t size);
bool gpu_system_free_memory(gpu_system_t* gpu, uint32_t gpu_id, uint64_t address);
uint64_t gpu_system_get_memory_info(gpu_system_t* gpu, uint32_t gpu_id, uint64_t* total, uint64_t* used, uint64_t* free);
bool gpu_system_map_memory(gpu_system_t* gpu, uint32_t gpu_id, uint64_t gpu_address, void* cpu_address, uint64_t size);
bool gpu_system_unmap_memory(gpu_system_t* gpu, uint32_t gpu_id, uint64_t gpu_address);

// GPU synchronization
bool gpu_system_synchronize(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_wait_for_idle(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_flush_cache(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_invalidate_cache(gpu_system_t* gpu, uint32_t gpu_id);

// Information
bool gpu_system_is_gpu_connected(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_is_gpu_enabled(gpu_system_t* gpu, uint32_t gpu_id);
bool gpu_system_is_gpu_primary(gpu_system_t* gpu, uint32_t gpu_id);
uint32_t gpu_system_get_total_memory(gpu_system_t* gpu);
uint32_t gpu_system_get_connected_gpu_count(gpu_system_t* gpu);
uint32_t gpu_system_get_enabled_gpu_count(gpu_system_t* gpu);
bool gpu_system_has_discrete_gpu(gpu_system_t* gpu);
bool gpu_system_has_integrated_gpu(gpu_system_t* gpu);

// Utility functions
char* gpu_system_get_vendor_name(gpu_vendor_t vendor);
char* gpu_system_get_architecture_name(gpu_architecture_t architecture);
char* gpu_system_get_memory_type_name(gpu_memory_type_t memory_type);
char* gpu_system_get_feature_name(gpu_feature_t feature);
bool gpu_system_is_vendor_supported(gpu_vendor_t vendor);
bool gpu_system_is_architecture_supported(gpu_architecture_t architecture);
uint32_t gpu_system_calculate_memory_bandwidth(gpu_memory_type_t memory_type, uint32_t memory_clock, uint32_t bus_width);
uint32_t gpu_system_calculate_performance_score(gpu_info_t* gpu_info);

// Callbacks
typedef void (*gpu_performance_callback_t)(gpu_system_t* gpu, uint32_t gpu_id, gpu_performance_t* performance, void* user_data);
typedef void (*gpu_connected_callback_t)(gpu_system_t* gpu, uint32_t gpu_id, void* user_data);
typedef void (*gpu_disconnected_callback_t)(gpu_system_t* gpu, uint32_t gpu_id, void* user_data);

void gpu_system_set_performance_callback(gpu_system_t* gpu, gpu_performance_callback_t callback, void* user_data);
void gpu_system_set_connected_callback(gpu_system_t* gpu, gpu_connected_callback_t callback, void* user_data);
void gpu_system_set_disconnected_callback(gpu_system_t* gpu, gpu_disconnected_callback_t callback, void* user_data);

// Preset configurations
gpu_system_config_t gpu_system_preset_normal_style(void);
gpu_system_config_t gpu_system_preset_gaming_style(void);
gpu_system_config_t gpu_system_preset_workstation_style(void);
gpu_system_config_t gpu_system_preset_power_saving_style(void);

// Error handling
typedef enum {
    GPU_SYSTEM_SUCCESS = 0,
    GPU_SYSTEM_ERROR_INVALID_CONTEXT,
    GPU_SYSTEM_ERROR_INVALID_GPU,
    GPU_SYSTEM_ERROR_GPU_NOT_FOUND,
    GPU_SYSTEM_ERROR_DRIVER_FAILED,
    GPU_SYSTEM_ERROR_OUT_OF_MEMORY,
    GPU_SYSTEM_ERROR_FEATURE_NOT_SUPPORTED,
    GPU_SYSTEM_ERROR_OVERCLOCK_FAILED,
    GPU_SYSTEM_ERROR_POWER_MANAGEMENT_FAILED,
} gpu_system_error_t;

gpu_system_error_t gpu_system_get_last_error(void);
const char* gpu_system_get_error_string(gpu_system_error_t error);

#endif // GPU_SYSTEM_H 