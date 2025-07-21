#ifndef EXTENDED_REALITY_H
#define EXTENDED_REALITY_H

#include "kernel.h"
#include <stdint.h>
#include <stdbool.h>

// Extended reality constants
#define MAX_XR_DEVICES 10
#define MAX_XR_APPLICATIONS 50
#define MAX_XR_SCENES 100
#define MAX_XR_OBJECTS 1000
#define MAX_XR_SENSORS 20
#define MAX_XR_CONTROLLERS 8
#define MAX_XR_TRACKERS 16
#define MAX_XR_DISPLAYS 4

// XR device types
typedef enum {
    XR_DEVICE_VR_HEADSET = 0,        // Virtual Reality headset
    XR_DEVICE_AR_GLASSES = 1,        // Augmented Reality glasses
    XR_DEVICE_MR_HEADSET = 2,        // Mixed Reality headset
    XR_DEVICE_HOLOLENS = 3,          // Microsoft HoloLens
    XR_DEVICE_QUEST = 4,             // Meta Quest
    XR_DEVICE_VIVE = 5,              // HTC Vive
    XR_DEVICE_CUSTOM = 6             // Custom XR device
} xr_device_type_t;

// XR application types
typedef enum {
    XR_APP_GAME = 0,                 // XR game
    XR_APP_EDUCATION = 1,            // Educational application
    XR_APP_TRAINING = 2,             // Training simulation
    XR_APP_DESIGN = 3,               // Design and modeling
    XR_APP_MEDICAL = 4,              // Medical application
    XR_APP_ARCHITECTURE = 5,         // Architecture visualization
    XR_APP_ENTERTAINMENT = 6,        // Entertainment application
    XR_APP_PRODUCTIVITY = 7,         // Productivity application
    XR_APP_SOCIAL = 8,               // Social VR/AR
    XR_APP_CUSTOM = 9                // Custom application
} xr_application_type_t;

// XR sensor types
typedef enum {
    XR_SENSOR_IMU = 0,               // Inertial Measurement Unit
    XR_SENSOR_CAMERA = 1,            // Camera sensor
    XR_SENSOR_DEPTH = 2,             // Depth sensor
    XR_SENSOR_LIDAR = 3,             // LiDAR sensor
    XR_SENSOR_ULTRASONIC = 4,        // Ultrasonic sensor
    XR_SENSOR_INFRARED = 5,          // Infrared sensor
    XR_SENSOR_MAGNETIC = 6,          // Magnetic sensor
    XR_SENSOR_OPTICAL = 7,           // Optical tracking
    XR_SENSOR_CUSTOM = 8             // Custom sensor
} xr_sensor_type_t;

// XR tracking types
typedef enum {
    XR_TRACKING_INSIDE_OUT = 0,      // Inside-out tracking
    XR_TRACKING_OUTSIDE_IN = 1,      // Outside-in tracking
    XR_TRACKING_HYBRID = 2,          // Hybrid tracking
    XR_TRACKING_MARKER = 3,          // Marker-based tracking
    XR_TRACKING_MARKERLESS = 4,      // Markerless tracking
    XR_TRACKING_SLAM = 5,            // SLAM tracking
    XR_TRACKING_CUSTOM = 6           // Custom tracking
} xr_tracking_type_t;

// XR display types
typedef enum {
    XR_DISPLAY_LCD = 0,              // LCD display
    XR_DISPLAY_OLED = 1,             // OLED display
    XR_DISPLAY_MICROLED = 2,         // MicroLED display
    XR_DISPLAY_LASER = 3,            // Laser projection
    XR_DISPLAY_WAVEGUIDE = 4,        // Waveguide display
    XR_DISPLAY_HOLOGRAM = 5,         // Holographic display
    XR_DISPLAY_CUSTOM = 6            // Custom display
} xr_display_type_t;

// XR device information
typedef struct xr_device {
    uint32_t device_id;                           // Unique device identifier
    char name[64];                                // Device name
    char description[256];                        // Device description
    xr_device_type_t type;                        // Device type
    bool connected;                               // Device connected
    bool active;                                  // Device active
    
    // Device capabilities
    uint32_t resolution_width;                    // Display resolution width
    uint32_t resolution_height;                   // Display resolution height
    float refresh_rate;                           // Refresh rate (Hz)
    float field_of_view_h;                        // Horizontal field of view (degrees)
    float field_of_view_v;                        // Vertical field of view (degrees)
    float interpupillary_distance;                // Interpupillary distance (mm)
    
    // Device performance
    uint32_t latency;                             // Motion-to-photon latency (ms)
    uint32_t tracking_accuracy;                   // Tracking accuracy (mm)
    uint32_t tracking_frequency;                  // Tracking frequency (Hz)
    float battery_life;                           // Battery life (hours)
    float weight;                                 // Device weight (grams)
    
    // Device sensors
    xr_sensor_type_t sensors[MAX_XR_SENSORS];     // Device sensors
    uint32_t sensor_count;                        // Number of sensors
    xr_tracking_type_t tracking_type;             // Tracking type
    bool hand_tracking;                           // Hand tracking supported
    bool eye_tracking;                            // Eye tracking supported
    bool facial_tracking;                         // Facial tracking supported
    
    // Device displays
    xr_display_type_t display_type;               // Display type
    uint32_t display_count;                       // Number of displays
    bool passthrough;                             // Passthrough supported
    bool see_through;                             // See-through supported
    
    // Device status
    uint64_t last_update;                         // Last update time
    uint32_t uptime;                              // Device uptime (seconds)
    float temperature;                            // Device temperature (°C)
    float battery_level;                          // Battery level (0-1)
    
    void* device_data;                            // Device-specific data
} xr_device_t;

// XR application information
typedef struct xr_application {
    uint32_t app_id;                              // Unique application identifier
    char name[64];                                // Application name
    char description[256];                        // Application description
    xr_application_type_t type;                   // Application type
    bool running;                                 // Application running
    
    // Application requirements
    uint32_t min_resolution_width;                // Minimum resolution width
    uint32_t min_resolution_height;               // Minimum resolution height
    float min_refresh_rate;                       // Minimum refresh rate (Hz)
    uint32_t min_tracking_frequency;              // Minimum tracking frequency (Hz)
    bool requires_hand_tracking;                  // Requires hand tracking
    bool requires_eye_tracking;                   // Requires eye tracking
    
    // Application performance
    uint32_t frame_rate;                          // Current frame rate (FPS)
    uint32_t render_time;                         // Render time per frame (ms)
    uint32_t cpu_usage;                           // CPU usage percentage
    uint32_t gpu_usage;                           // GPU usage percentage
    uint32_t memory_usage;                        // Memory usage (MB)
    
    // Application state
    uint64_t start_time;                          // Start time
    uint64_t total_runtime;                       // Total runtime (seconds)
    uint32_t session_count;                       // Number of sessions
    bool paused;                                  // Application paused
    
    // Application configuration
    char config_file[256];                        // Configuration file path
    char assets_path[256];                        // Assets directory path
    char save_path[256];                          // Save directory path
    bool auto_save;                               // Auto save enabled
    
    void* app_data;                               // Application-specific data
} xr_application_t;

// XR scene information
typedef struct xr_scene {
    uint32_t scene_id;                            // Unique scene identifier
    char name[64];                                // Scene name
    char description[256];                        // Scene description
    uint32_t app_id;                              // Associated application
    
    // Scene structure
    uint32_t object_count;                        // Number of objects
    uint32_t light_count;                         // Number of lights
    uint32_t camera_count;                        // Number of cameras
    uint32_t material_count;                      // Number of materials
    uint32_t texture_count;                       // Number of textures
    
    // Scene performance
    uint32_t polygon_count;                       // Total polygon count
    uint32_t vertex_count;                        // Total vertex count
    uint32_t draw_calls;                          // Number of draw calls
    float render_time;                            // Render time (ms)
    float load_time;                              // Load time (seconds)
    
    // Scene state
    bool loaded;                                  // Scene loaded
    bool visible;                                 // Scene visible
    bool interactive;                             // Scene interactive
    uint64_t last_update;                         // Last update time
    
    void* scene_data;                             // Scene-specific data
} xr_scene_t;

// XR object information
typedef struct xr_object {
    uint32_t object_id;                           // Unique object identifier
    char name[64];                                // Object name
    uint32_t scene_id;                            // Associated scene
    
    // Object transform
    float position_x;                             // X position
    float position_y;                             // Y position
    float position_z;                             // Z position
    float rotation_x;                             // X rotation (radians)
    float rotation_y;                             // Y rotation (radians)
    float rotation_z;                             // Z rotation (radians)
    float scale_x;                                // X scale
    float scale_y;                                // Y scale
    float scale_z;                                // Z scale
    
    // Object properties
    uint32_t mesh_id;                             // Associated mesh
    uint32_t material_id;                         // Associated material
    uint32_t texture_id;                          // Associated texture
    bool visible;                                 // Object visible
    bool interactive;                             // Object interactive
    bool collidable;                              // Object collidable
    
    // Object physics
    float mass;                                   // Object mass
    float friction;                               // Object friction
    float restitution;                            // Object restitution
    bool kinematic;                               // Kinematic object
    bool static_object;                           // Static object
    
    void* object_data;                            // Object-specific data
} xr_object_t;

// XR controller information
typedef struct xr_controller {
    uint32_t controller_id;                       // Unique controller identifier
    char name[64];                                // Controller name
    uint32_t device_id;                           // Associated device
    
    // Controller state
    bool connected;                               // Controller connected
    bool active;                                  // Controller active
    uint32_t battery_level;                       // Battery level (0-100)
    
    // Controller input
    bool trigger_pressed;                         // Trigger button pressed
    bool grip_pressed;                            // Grip button pressed
    bool menu_pressed;                            // Menu button pressed
    bool system_pressed;                          // System button pressed
    float trigger_value;                          // Trigger value (0-1)
    float grip_value;                             // Grip value (0-1)
    
    // Controller position and orientation
    float position_x;                             // X position
    float position_y;                             // Y position
    float position_z;                             // Z position
    float rotation_x;                             // X rotation (radians)
    float rotation_y;                             // Y rotation (radians)
    float rotation_z;                             // Z rotation (radians)
    
    // Controller vibration
    bool haptic_enabled;                          // Haptic feedback enabled
    float haptic_frequency;                       // Haptic frequency (Hz)
    float haptic_amplitude;                       // Haptic amplitude (0-1)
    uint32_t haptic_duration;                     // Haptic duration (ms)
    
    void* controller_data;                        // Controller-specific data
} xr_controller_t;

// XR tracker information
typedef struct xr_tracker {
    uint32_t tracker_id;                          // Unique tracker identifier
    char name[64];                                // Tracker name
    uint32_t device_id;                           // Associated device
    
    // Tracker state
    bool connected;                               // Tracker connected
    bool active;                                  // Tracker active
    bool tracking;                                // Tracker tracking
    
    // Tracker position and orientation
    float position_x;                             // X position
    float position_y;                             // Y position
    float position_z;                             // Z position
    float rotation_x;                             // X rotation (radians)
    float rotation_y;                             // Y rotation (radians)
    float rotation_z;                             // Z rotation (radians)
    
    // Tracker accuracy
    float tracking_confidence;                    // Tracking confidence (0-1)
    float tracking_accuracy;                      // Tracking accuracy (mm)
    uint32_t tracking_frequency;                  // Tracking frequency (Hz)
    
    void* tracker_data;                           // Tracker-specific data
} xr_tracker_t;

// XR display information
typedef struct xr_display {
    uint32_t display_id;                          // Unique display identifier
    char name[64];                                // Display name
    uint32_t device_id;                           // Associated device
    
    // Display properties
    xr_display_type_t type;                       // Display type
    uint32_t resolution_width;                    // Resolution width
    uint32_t resolution_height;                   // Resolution height
    float refresh_rate;                           // Refresh rate (Hz)
    float brightness;                             // Brightness (0-1)
    float contrast;                               // Contrast (0-1)
    
    // Display performance
    uint32_t latency;                             // Display latency (ms)
    uint32_t frame_rate;                          // Current frame rate (FPS)
    bool vsync_enabled;                           // V-sync enabled
    bool adaptive_sync;                           // Adaptive sync enabled
    
    // Display calibration
    float gamma;                                  // Gamma correction
    float color_temperature;                      // Color temperature (K)
    bool color_correction;                        // Color correction enabled
    float distortion_correction;                  // Distortion correction
    
    void* display_data;                           // Display-specific data
} xr_display_t;

// Extended reality system
typedef struct extended_reality_system {
    spinlock_t lock;                              // System lock
    bool initialized;                             // Initialization flag
    
    // Device management
    xr_device_t devices[MAX_XR_DEVICES];          // XR devices
    uint32_t device_count;                        // Number of devices
    
    // Application management
    xr_application_t applications[MAX_XR_APPLICATIONS]; // XR applications
    uint32_t application_count;                   // Number of applications
    
    // Scene management
    xr_scene_t scenes[MAX_XR_SCENES];             // XR scenes
    uint32_t scene_count;                         // Number of scenes
    
    // Object management
    xr_object_t objects[MAX_XR_OBJECTS];          // XR objects
    uint32_t object_count;                        // Number of objects
    
    // Controller management
    xr_controller_t controllers[MAX_XR_CONTROLLERS]; // XR controllers
    uint32_t controller_count;                    // Number of controllers
    
    // Tracker management
    xr_tracker_t trackers[MAX_XR_TRACKERS];       // XR trackers
    uint32_t tracker_count;                       // Number of trackers
    
    // Display management
    xr_display_t displays[MAX_XR_DISPLAYS];       // XR displays
    uint32_t display_count;                       // Number of displays
    
    // System configuration
    bool vr_enabled;                              // VR enabled
    bool ar_enabled;                              // AR enabled
    bool mr_enabled;                              // MR enabled
    bool hand_tracking_enabled;                   // Hand tracking enabled
    bool eye_tracking_enabled;                    // Eye tracking enabled
    bool spatial_audio_enabled;                   // Spatial audio enabled
    
    // System statistics
    uint64_t total_sessions;                      // Total XR sessions
    uint64_t total_runtime;                       // Total runtime (seconds)
    uint64_t total_applications;                  // Total applications run
    uint64_t last_update;                         // Last update time
} extended_reality_system_t;

// Extended reality statistics
typedef struct xr_stats {
    // Device statistics
    uint32_t total_devices;                       // Total devices
    uint32_t connected_devices;                   // Connected devices
    uint32_t active_devices;                      // Active devices
    uint32_t vr_devices;                          // VR devices
    uint32_t ar_devices;                          // AR devices
    uint32_t mr_devices;                          // MR devices
    
    // Application statistics
    uint32_t total_applications;                  // Total applications
    uint32_t running_applications;                // Running applications
    uint32_t game_applications;                   // Game applications
    uint32_t productivity_applications;           // Productivity applications
    uint32_t educational_applications;            // Educational applications
    
    // Performance statistics
    float average_frame_rate;                     // Average frame rate
    float average_latency;                        // Average latency
    float average_tracking_accuracy;              // Average tracking accuracy
    uint32_t total_polygons_rendered;             // Total polygons rendered
    uint32_t total_vertices_processed;            // Total vertices processed
    
    // Session statistics
    uint64_t total_sessions;                      // Total sessions
    uint64_t total_runtime;                       // Total runtime
    float average_session_length;                 // Average session length
    uint32_t concurrent_users;                    // Concurrent users
    
    // System statistics
    uint64_t last_update;                         // Last update time
} xr_stats_t;

// Function declarations

// System initialization
int xr_system_init(void);
void xr_system_shutdown(void);
extended_reality_system_t* xr_system_get_system(void);

// Device management
xr_device_t* xr_device_add(const char* name, xr_device_type_t type);
int xr_device_remove(uint32_t device_id);
int xr_device_connect(uint32_t device_id);
int xr_device_disconnect(uint32_t device_id);
xr_device_t* xr_device_find(uint32_t device_id);
xr_device_t* xr_device_find_by_name(const char* name);

// Application management
xr_application_t* xr_application_add(const char* name, xr_application_type_t type);
int xr_application_remove(uint32_t app_id);
int xr_application_start(uint32_t app_id);
int xr_application_stop(uint32_t app_id);
int xr_application_pause(uint32_t app_id);
xr_application_t* xr_application_find(uint32_t app_id);
xr_application_t* xr_application_find_by_name(const char* name);

// Scene management
xr_scene_t* xr_scene_create(const char* name, uint32_t app_id);
int xr_scene_destroy(uint32_t scene_id);
int xr_scene_load(uint32_t scene_id);
int xr_scene_unload(uint32_t scene_id);
xr_scene_t* xr_scene_find(uint32_t scene_id);
xr_scene_t* xr_scene_find_by_name(const char* name);

// Object management
xr_object_t* xr_object_create(const char* name, uint32_t scene_id);
int xr_object_destroy(uint32_t object_id);
int xr_object_set_transform(uint32_t object_id, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);
xr_object_t* xr_object_find(uint32_t object_id);
xr_object_t* xr_object_find_by_name(const char* name);

// Controller management
xr_controller_t* xr_controller_add(const char* name, uint32_t device_id);
int xr_controller_remove(uint32_t controller_id);
int xr_controller_update_input(uint32_t controller_id);
xr_controller_t* xr_controller_find(uint32_t controller_id);
xr_controller_t* xr_controller_find_by_name(const char* name);

// Tracker management
xr_tracker_t* xr_tracker_add(const char* name, uint32_t device_id);
int xr_tracker_remove(uint32_t tracker_id);
int xr_tracker_update_position(uint32_t tracker_id);
xr_tracker_t* xr_tracker_find(uint32_t tracker_id);
xr_tracker_t* xr_tracker_find_by_name(const char* name);

// Display management
xr_display_t* xr_display_add(const char* name, uint32_t device_id);
int xr_display_remove(uint32_t display_id);
int xr_display_set_resolution(uint32_t display_id, uint32_t width, uint32_t height);
xr_display_t* xr_display_find(uint32_t display_id);
xr_display_t* xr_display_find_by_name(const char* name);

// System configuration
int xr_enable_vr(bool enabled);
int xr_enable_ar(bool enabled);
int xr_enable_mr(bool enabled);
int xr_enable_hand_tracking(bool enabled);
int xr_enable_eye_tracking(bool enabled);
int xr_enable_spatial_audio(bool enabled);

// Monitoring and statistics
void xr_get_stats(xr_stats_t* stats);
void xr_reset_stats(void);
int xr_perform_benchmark(void);
int xr_generate_report(void);

// Extended reality debugging
void xr_dump_devices(void);
void xr_dump_applications(void);
void xr_dump_scenes(void);
void xr_dump_objects(void);
void xr_dump_controllers(void);
void xr_dump_trackers(void);
void xr_dump_displays(void);
void xr_dump_stats(void);

#endif // EXTENDED_REALITY_H 