#ifndef CREATIVE_SUITE_H
#define CREATIVE_SUITE_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// Creative suite constants
#define MAX_CREATIVE_APPS 50
#define MAX_AUDIO_DEVICES 16
#define MAX_VIDEO_DEVICES 8
#define MAX_AUDIO_TRACKS 128
#define MAX_VIDEO_TRACKS 64
#define MAX_AUDIO_EFFECTS 32
#define MAX_VIDEO_EFFECTS 32
#define MAX_CODECS 20
#define MAX_COLOR_PROFILES 10

// Creative application types
typedef enum {
    CREATIVE_APP_VIDEO_EDITOR = 0,
    CREATIVE_APP_AUDIO_EDITOR = 1,
    CREATIVE_APP_IMAGE_EDITOR = 2,
    CREATIVE_APP_3D_MODELER = 3,
    CREATIVE_APP_CAD = 4,
    CREATIVE_APP_ANIMATION = 5,
    CREATIVE_APP_COMPOSITING = 6,
    CREATIVE_APP_MOTION_GRAPHICS = 7,
    CREATIVE_APP_VFX = 8,
    CREATIVE_APP_COLOR_GRADING = 9
} creative_app_type_t;

// Audio formats
typedef enum {
    AUDIO_FORMAT_PCM = 0,
    AUDIO_FORMAT_MP3 = 1,
    AUDIO_FORMAT_AAC = 2,
    AUDIO_FORMAT_FLAC = 3,
    AUDIO_FORMAT_WAV = 4,
    AUDIO_FORMAT_OGG = 5,
    AUDIO_FORMAT_ALAC = 6,
    AUDIO_FORMAT_WMA = 7
} audio_format_t;

// Video formats
typedef enum {
    VIDEO_FORMAT_H264 = 0,
    VIDEO_FORMAT_H265 = 1,
    VIDEO_FORMAT_AV1 = 2,
    VIDEO_FORMAT_VP9 = 3,
    VIDEO_FORMAT_MPEG2 = 4,
    VIDEO_FORMAT_MPEG4 = 5,
    VIDEO_FORMAT_PRO_RES = 6,
    VIDEO_FORMAT_DNX = 7,
    VIDEO_FORMAT_RAW = 8
} video_format_t;

// Color spaces
typedef enum {
    COLOR_SPACE_SRGB = 0,
    COLOR_SPACE_ADOBE_RGB = 1,
    COLOR_SPACE_DCI_P3 = 2,
    COLOR_SPACE_REC_709 = 3,
    COLOR_SPACE_REC_2020 = 4,
    COLOR_SPACE_ACES = 5,
    COLOR_SPACE_PRO_PHOTO = 6
} color_space_t;

// Audio device types
typedef enum {
    AUDIO_DEVICE_INPUT = 0,
    AUDIO_DEVICE_OUTPUT = 1,
    AUDIO_DEVICE_INPUT_OUTPUT = 2
} audio_device_type_t;

// Video device types
typedef enum {
    VIDEO_DEVICE_CAMERA = 0,
    VIDEO_DEVICE_DISPLAY = 1,
    VIDEO_DEVICE_CAPTURE = 2
} video_device_type_t;

// Creative application
typedef struct creative_app {
    uint32_t app_id;                             // Application identifier
    char name[128];                              // Application name
    char version[32];                            // Application version
    creative_app_type_t type;                    // Application type
    bool is_running;                             // Currently running
    uint32_t process_id;                         // Process ID when running
    uint64_t last_used;                          // Last used timestamp
    uint64_t total_usage_time;                   // Total usage time
    bool hardware_acceleration;                  // Hardware acceleration enabled
    bool real_time_preview;                      // Real-time preview enabled
    uint32_t memory_usage;                       // Memory usage (MB)
    uint32_t cpu_usage;                          // CPU usage (%)
    uint32_t gpu_usage;                          // GPU usage (%)
    void* app_data;                              // Application-specific data
} creative_app_t;

// Audio device
typedef struct audio_device {
    uint32_t device_id;                          // Device identifier
    char name[128];                              // Device name
    char manufacturer[64];                       // Manufacturer
    audio_device_type_t type;                    // Device type
    bool connected;                              // Connected
    bool active;                                 // Active
    uint32_t sample_rate;                        // Sample rate (Hz)
    uint32_t bit_depth;                          // Bit depth
    uint32_t channels;                           // Number of channels
    uint32_t buffer_size;                        // Buffer size (samples)
    uint32_t latency;                            // Latency (ms)
    bool asio_support;                           // ASIO support
    bool wasapi_support;                         // WASAPI support
    bool core_audio_support;                     // Core Audio support
    bool jack_support;                           // JACK support
    bool pulse_audio_support;                    // PulseAudio support
    void* device_data;                           // Device-specific data
} audio_device_t;

// Video device
typedef struct video_device {
    uint32_t device_id;                          // Device identifier
    char name[128];                              // Device name
    char manufacturer[64];                       // Manufacturer
    video_device_type_t type;                    // Device type
    bool connected;                              // Connected
    bool active;                                 // Active
    uint32_t width;                              // Width
    uint32_t height;                             // Height
    uint32_t frame_rate;                         // Frame rate (fps)
    uint32_t bit_depth;                          // Bit depth
    color_space_t color_space;                   // Color space
    bool hdr_support;                            // HDR support
    bool hardware_encoding;                      // Hardware encoding
    bool hardware_decoding;                      // Hardware decoding
    void* device_data;                           // Device-specific data
} video_device_t;

// Audio track
typedef struct audio_track {
    uint32_t track_id;                           // Track identifier
    char name[64];                               // Track name
    bool enabled;                                // Track enabled
    bool muted;                                  // Track muted
    bool solo;                                   // Track solo
    float volume;                                // Volume (0.0 - 1.0)
    float pan;                                   // Pan (-1.0 to 1.0)
    uint32_t sample_rate;                        // Sample rate (Hz)
    uint32_t bit_depth;                          // Bit depth
    uint32_t channels;                           // Number of channels
    uint64_t duration;                           // Duration (samples)
    uint64_t position;                           // Current position (samples)
    audio_format_t format;                       // Audio format
    void* track_data;                            // Track-specific data
} audio_track_t;

// Video track
typedef struct video_track {
    uint32_t track_id;                           // Track identifier
    char name[64];                               // Track name
    bool enabled;                                // Track enabled
    bool visible;                                // Track visible
    bool locked;                                 // Track locked
    float opacity;                               // Opacity (0.0 - 1.0)
    uint32_t width;                              // Width
    uint32_t height;                             // Height
    uint32_t frame_rate;                         // Frame rate (fps)
    uint64_t duration;                           // Duration (frames)
    uint64_t position;                           // Current position (frames)
    video_format_t format;                       // Video format
    color_space_t color_space;                   // Color space
    void* track_data;                            // Track-specific data
} video_track_t;

// Audio effect
typedef struct audio_effect {
    uint32_t effect_id;                          // Effect identifier
    char name[64];                               // Effect name
    char type[32];                               // Effect type
    bool enabled;                                // Effect enabled
    bool bypass;                                 // Effect bypass
    uint32_t track_id;                           // Associated track
    float wet_mix;                               // Wet mix (0.0 - 1.0)
    float dry_mix;                               // Dry mix (0.0 - 1.0)
    void* parameters;                            // Effect parameters
    void* effect_data;                           // Effect-specific data
} audio_effect_t;

// Video effect
typedef struct video_effect {
    uint32_t effect_id;                          // Effect identifier
    char name[64];                               // Effect name
    char type[32];                               // Effect type
    bool enabled;                                // Effect enabled
    bool bypass;                                 // Effect bypass
    uint32_t track_id;                           // Associated track
    float intensity;                             // Effect intensity (0.0 - 1.0)
    void* parameters;                            // Effect parameters
    void* effect_data;                           // Effect-specific data
} video_effect_t;

// Codec information
typedef struct codec_info {
    uint32_t codec_id;                           // Codec identifier
    char name[64];                               // Codec name
    char description[256];                       // Codec description
    bool is_audio;                               // Audio codec
    bool is_video;                               // Video codec
    bool hardware_accelerated;                   // Hardware accelerated
    bool encoding_supported;                     // Encoding supported
    bool decoding_supported;                     // Decoding supported
    uint32_t max_resolution_width;               // Max resolution width
    uint32_t max_resolution_height;              // Max resolution height
    uint32_t max_bitrate;                        // Max bitrate (Mbps)
    uint32_t max_framerate;                      // Max framerate (fps)
    void* codec_data;                            // Codec-specific data
} codec_info_t;

// Color profile
typedef struct color_profile {
    uint32_t profile_id;                         // Profile identifier
    char name[64];                               // Profile name
    char description[256];                       // Profile description
    color_space_t color_space;                   // Color space
    float gamma;                                 // Gamma value
    float white_point_x;                         // White point X
    float white_point_y;                         // White point Y
    float red_primary_x;                         // Red primary X
    float red_primary_y;                         // Red primary Y
    float green_primary_x;                       // Green primary X
    float green_primary_y;                       // Green primary Y
    float blue_primary_x;                        // Blue primary X
    float blue_primary_y;                        // Blue primary Y
    bool hdr;                                    // HDR profile
    float max_luminance;                         // Max luminance (nits)
    float min_luminance;                         // Min luminance (nits)
    void* profile_data;                          // Profile-specific data
} color_profile_t;

// Creative suite
typedef struct creative_suite {
    spinlock_t lock;                             // System lock
    bool initialized;                            // Initialization flag
    
    // Application management
    creative_app_t apps[MAX_CREATIVE_APPS];      // Creative applications
    uint32_t app_count;                          // Number of applications
    creative_app_t* current_app;                 // Currently active application
    
    // Audio management
    audio_device_t audio_devices[MAX_AUDIO_DEVICES]; // Audio devices
    uint32_t audio_device_count;                 // Number of audio devices
    audio_device_t* primary_audio_device;        // Primary audio device
    audio_track_t audio_tracks[MAX_AUDIO_TRACKS]; // Audio tracks
    uint32_t audio_track_count;                  // Number of audio tracks
    audio_effect_t audio_effects[MAX_AUDIO_EFFECTS]; // Audio effects
    uint32_t audio_effect_count;                 // Number of audio effects
    
    // Video management
    video_device_t video_devices[MAX_VIDEO_DEVICES]; // Video devices
    uint32_t video_device_count;                 // Number of video devices
    video_device_t* primary_video_device;        // Primary video device
    video_track_t video_tracks[MAX_VIDEO_TRACKS]; // Video tracks
    uint32_t video_track_count;                  // Number of video tracks
    video_effect_t video_effects[MAX_VIDEO_EFFECTS]; // Video effects
    uint32_t video_effect_count;                 // Number of video effects
    
    // Codec management
    codec_info_t codecs[MAX_CODECS];             // Available codecs
    uint32_t codec_count;                        // Number of codecs
    
    // Color management
    color_profile_t color_profiles[MAX_COLOR_PROFILES]; // Color profiles
    uint32_t color_profile_count;                // Number of color profiles
    color_profile_t* active_color_profile;       // Active color profile
    
    // System configuration
    bool low_latency_audio;                      // Low latency audio
    bool hardware_acceleration;                  // Hardware acceleration
    bool real_time_preview;                      // Real-time preview
    bool color_management;                       // Color management
    bool hdr_support;                            // HDR support
    bool surround_sound;                         // Surround sound
    bool midi_support;                           // MIDI support
    
    // Performance monitoring
    uint32_t audio_latency;                      // Audio latency (ms)
    uint32_t video_latency;                      // Video latency (ms)
    uint32_t render_time;                        // Render time (ms)
    uint32_t memory_usage;                       // Memory usage (MB)
    uint32_t cpu_usage;                          // CPU usage (%)
    uint32_t gpu_usage;                          // GPU usage (%)
    
    // Statistics
    uint64_t total_usage_time;                   // Total usage time
    uint64_t projects_created;                   // Projects created
    uint64_t files_exported;                     // Files exported
    uint64_t last_update;                        // Last update time
} creative_suite_t;

// Function declarations

// System initialization
int creative_suite_init(void);
void creative_suite_shutdown(void);
creative_suite_t* creative_suite_get_system(void);

// Application management
creative_app_t* creative_app_add(const char* name, const char* version, creative_app_type_t type);
int creative_app_remove(uint32_t app_id);
int creative_app_launch(uint32_t app_id);
int creative_app_terminate(uint32_t app_id);
creative_app_t* creative_app_find(uint32_t app_id);
creative_app_t* creative_app_find_by_name(const char* name);
creative_app_t* creative_app_get_current(void);

// Audio management
int audio_device_enumerate(void);
audio_device_t* audio_device_get_info(uint32_t device_id);
audio_device_t* audio_device_get_primary(void);
int audio_device_set_primary(uint32_t device_id);
int audio_device_configure(uint32_t device_id, uint32_t sample_rate, uint32_t bit_depth, uint32_t channels);
int audio_device_set_latency(uint32_t device_id, uint32_t latency);

// Video management
int video_device_enumerate(void);
video_device_t* video_device_get_info(uint32_t device_id);
video_device_t* video_device_get_primary(void);
int video_device_set_primary(uint32_t device_id);
int video_device_configure(uint32_t device_id, uint32_t width, uint32_t height, uint32_t frame_rate);

// Track management
audio_track_t* audio_track_create(const char* name);
int audio_track_destroy(uint32_t track_id);
int audio_track_enable(uint32_t track_id, bool enabled);
int audio_track_mute(uint32_t track_id, bool muted);
int audio_track_solo(uint32_t track_id, bool solo);
int audio_track_set_volume(uint32_t track_id, float volume);
int audio_track_set_pan(uint32_t track_id, float pan);
audio_track_t* audio_track_find(uint32_t track_id);

video_track_t* video_track_create(const char* name);
int video_track_destroy(uint32_t track_id);
int video_track_enable(uint32_t track_id, bool enabled);
int video_track_set_visible(uint32_t track_id, bool visible);
int video_track_set_opacity(uint32_t track_id, float opacity);
video_track_t* video_track_find(uint32_t track_id);

// Effect management
audio_effect_t* audio_effect_add(const char* name, const char* type, uint32_t track_id);
int audio_effect_remove(uint32_t effect_id);
int audio_effect_enable(uint32_t effect_id, bool enabled);
int audio_effect_bypass(uint32_t effect_id, bool bypass);
int audio_effect_set_wet_mix(uint32_t effect_id, float wet_mix);
audio_effect_t* audio_effect_find(uint32_t effect_id);

video_effect_t* video_effect_add(const char* name, const char* type, uint32_t track_id);
int video_effect_remove(uint32_t effect_id);
int video_effect_enable(uint32_t effect_id, bool enabled);
int video_effect_bypass(uint32_t effect_id, bool bypass);
int video_effect_set_intensity(uint32_t effect_id, float intensity);
video_effect_t* video_effect_find(uint32_t effect_id);

// Codec management
int codec_enumerate(void);
codec_info_t* codec_get_info(uint32_t codec_id);
codec_info_t* codec_find_by_name(const char* name);
bool codec_supports_encoding(uint32_t codec_id);
bool codec_supports_decoding(uint32_t codec_id);
bool codec_is_hardware_accelerated(uint32_t codec_id);

// Color management
color_profile_t* color_profile_create(const char* name, const char* description);
int color_profile_destroy(uint32_t profile_id);
int color_profile_apply(uint32_t profile_id);
color_profile_t* color_profile_find(uint32_t profile_id);
color_profile_t* color_profile_find_by_name(const char* name);
color_profile_t* color_profile_get_active(void);

// System configuration
int low_latency_audio_enable(bool enabled);
int hardware_acceleration_enable(bool enabled);
int real_time_preview_enable(bool enabled);
int color_management_enable(bool enabled);
int hdr_support_enable(bool enabled);
int surround_sound_enable(bool enabled);
int midi_support_enable(bool enabled);

// Performance monitoring
int creative_performance_start_monitoring(void);
int creative_performance_stop_monitoring(void);
int creative_performance_get_metrics(uint32_t* audio_latency, uint32_t* video_latency, uint32_t* render_time);

// Professional tools
int video_editing_init(void);
int audio_editing_init(void);
int image_editing_init(void);
int modeling_3d_init(void);
int cad_init(void);
int animation_init(void);
int compositing_init(void);
int motion_graphics_init(void);
int vfx_init(void);
int color_grading_init(void);

// Content creation
int codec_support_init(void);
int hardware_encoding_init(void);
int hardware_decoding_init(void);
int color_profiles_init(void);
int audio_formats_init(void);
int video_formats_init(void);

// Statistics
void creative_suite_get_stats(creative_suite_t* stats);
void creative_suite_reset_stats(void);

#endif // CREATIVE_SUITE_H 