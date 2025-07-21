#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memory.h>

// Audio System - Professional audio subsystem for RaeenOS
// Provides spatial audio, surround sound, and real-time effects

// Audio formats
typedef enum {
    AUDIO_FORMAT_PCM_8 = 0,      // 8-bit PCM
    AUDIO_FORMAT_PCM_16,         // 16-bit PCM
    AUDIO_FORMAT_PCM_24,         // 24-bit PCM
    AUDIO_FORMAT_PCM_32,         // 32-bit PCM
    AUDIO_FORMAT_FLOAT_32,       // 32-bit float
    AUDIO_FORMAT_FLOAT_64,       // 64-bit float
} audio_format_t;

// Sample rates
typedef enum {
    SAMPLE_RATE_8000 = 8000,     // 8 kHz
    SAMPLE_RATE_11025 = 11025,   // 11.025 kHz
    SAMPLE_RATE_16000 = 16000,   // 16 kHz
    SAMPLE_RATE_22050 = 22050,   // 22.05 kHz
    SAMPLE_RATE_32000 = 32000,   // 32 kHz
    SAMPLE_RATE_44100 = 44100,   // 44.1 kHz
    SAMPLE_RATE_48000 = 48000,   // 48 kHz
    SAMPLE_RATE_96000 = 96000,   // 96 kHz
    SAMPLE_RATE_192000 = 192000, // 192 kHz
} sample_rate_t;

// Channel configurations
typedef enum {
    CHANNELS_MONO = 1,           // Mono
    CHANNELS_STEREO = 2,         // Stereo
    CHANNELS_2_1 = 3,            // 2.1 (stereo + subwoofer)
    CHANNELS_5_1 = 6,            // 5.1 surround
    CHANNELS_7_1 = 8,            // 7.1 surround
    CHANNELS_9_1 = 10,           // 9.1 surround
    CHANNELS_11_1 = 12,          // 11.1 surround
    CHANNELS_ATMOS = 16,         // Dolby Atmos
} channel_config_t;

// Audio device types
typedef enum {
    AUDIO_DEVICE_SPEAKERS = 0,   // Speakers
    AUDIO_DEVICE_HEADPHONES,     // Headphones
    AUDIO_DEVICE_MICROPHONE,     // Microphone
    AUDIO_DEVICE_LINE_IN,        // Line input
    AUDIO_DEVICE_LINE_OUT,       // Line output
    AUDIO_DEVICE_DIGITAL,        // Digital audio (S/PDIF, HDMI)
    AUDIO_DEVICE_BLUETOOTH,      // Bluetooth audio
    AUDIO_DEVICE_USB,            // USB audio
} audio_device_type_t;

// Audio driver types
typedef enum {
    AUDIO_DRIVER_AC97 = 0,       // AC'97
    AUDIO_DRIVER_HD_AUDIO,       // Intel HD Audio
    AUDIO_DRIVER_ASIO,           // ASIO
    AUDIO_DRIVER_DIRECTSOUND,    // DirectSound
    AUDIO_DRIVER_WASAPI,         // Windows Audio Session API
    AUDIO_DRIVER_COREAUDIO,      // Core Audio (macOS compatibility)
    AUDIO_DRIVER_ALSA,           // ALSA (Linux compatibility)
    AUDIO_DRIVER_PULSE,          // PulseAudio
} audio_driver_type_t;

// Spatial audio modes
typedef enum {
    SPATIAL_MODE_OFF = 0,        // No spatial processing
    SPATIAL_MODE_STEREO,         // Stereo spatialization
    SPATIAL_MODE_5_1,            // 5.1 surround spatialization
    SPATIAL_MODE_7_1,            // 7.1 surround spatialization
    SPATIAL_MODE_ATMOS,          // Dolby Atmos spatialization
    SPATIAL_MODE_3D,             // 3D spatial audio
    SPATIAL_MODE_VR,             // VR spatial audio
} spatial_mode_t;

// Audio effects
typedef enum {
    AUDIO_EFFECT_NONE = 0,       // No effects
    AUDIO_EFFECT_REVERB,         // Reverb
    AUDIO_EFFECT_ECHO,           // Echo/delay
    AUDIO_EFFECT_CHORUS,         // Chorus
    AUDIO_EFFECT_FLANGER,        // Flanger
    AUDIO_EFFECT_PHASER,         // Phaser
    AUDIO_EFFECT_DISTORTION,     // Distortion
    AUDIO_EFFECT_COMPRESSOR,     // Dynamic compression
    AUDIO_EFFECT_LIMITER,        // Limiter
    AUDIO_EFFECT_EQUALIZER,      // Equalizer
    AUDIO_EFFECT_NOISE_GATE,     // Noise gate
    AUDIO_EFFECT_AUTOTUNE,       // Auto-tune
} audio_effect_t;

// Audio buffer structure
typedef struct {
    void* data;                  // Audio data buffer
    uint32_t size;               // Buffer size in bytes
    uint32_t samples;            // Number of samples
    uint32_t channels;           // Number of channels
    audio_format_t format;       // Audio format
    sample_rate_t sample_rate;   // Sample rate
    uint64_t timestamp;          // Timestamp
    bool is_playing;             // Is currently playing
    bool is_looping;             // Is looping
} audio_buffer_t;

// Audio device structure
typedef struct {
    uint32_t id;
    char name[64];
    audio_device_type_t type;
    audio_driver_type_t driver;
    uint32_t channels;
    sample_rate_t sample_rate;
    audio_format_t format;
    uint32_t buffer_size;
    bool is_active;
    bool is_default;
    void* driver_data;
} audio_device_t;

// Audio stream structure
typedef struct {
    uint32_t id;
    audio_device_t* device;
    audio_buffer_t* buffer;
    uint32_t position;
    uint32_t volume;
    bool is_muted;
    bool is_paused;
    spatial_mode_t spatial_mode;
    audio_effect_t effects[8];
    void* user_data;
} audio_stream_t;

// Spatial audio parameters
typedef struct {
    float x, y, z;               // 3D position
    float distance;              // Distance from listener
    float azimuth;               // Horizontal angle
    float elevation;             // Vertical angle
    float doppler_shift;         // Doppler effect
    float room_size;             // Room size for reverb
    float reverb_level;          // Reverb intensity
    float absorption;            // Sound absorption
} spatial_params_t;

// Audio effect parameters
typedef struct {
    audio_effect_t type;
    float intensity;             // Effect intensity (0.0 - 1.0)
    float frequency;             // Frequency for modulation effects
    float depth;                 // Depth for modulation effects
    float feedback;              // Feedback for delay effects
    float wet_dry;               // Wet/dry mix
    float* parameters;           // Additional parameters
    uint32_t param_count;        // Number of parameters
} effect_params_t;

// Audio system configuration
typedef struct {
    uint32_t default_sample_rate;
    audio_format_t default_format;
    channel_config_t default_channels;
    uint32_t buffer_size;
    uint32_t max_streams;
    bool enable_spatial_audio;
    bool enable_effects;
    bool enable_low_latency;
    bool enable_hardware_acceleration;
    float master_volume;
    bool enable_automatic_gain_control;
    bool enable_noise_reduction;
} audio_system_config_t;

// Audio system context
typedef struct {
    audio_system_config_t config;
    audio_device_t* devices;
    uint32_t device_count;
    uint32_t max_devices;
    audio_stream_t* streams;
    uint32_t stream_count;
    uint32_t max_streams;
    audio_device_t* default_output;
    audio_device_t* default_input;
    bool initialized;
    uint64_t frame_count;
    float cpu_usage;
    uint32_t latency_ms;
} audio_system_t;

// Function prototypes

// Initialization and shutdown
audio_system_t* audio_system_init(audio_system_config_t* config);
void audio_system_shutdown(audio_system_t* audio);
bool audio_system_is_initialized(audio_system_t* audio);

// Device management
uint32_t audio_system_add_device(audio_system_t* audio, audio_device_t* device);
void audio_system_remove_device(audio_system_t* audio, uint32_t device_id);
audio_device_t* audio_system_get_device(audio_system_t* audio, uint32_t device_id);
audio_device_t* audio_system_get_default_output(audio_system_t* audio);
audio_device_t* audio_system_get_default_input(audio_system_t* audio);
void audio_system_set_default_output(audio_system_t* audio, uint32_t device_id);
void audio_system_set_default_input(audio_system_t* audio, uint32_t device_id);

// Stream management
uint32_t audio_system_create_stream(audio_system_t* audio, audio_device_t* device);
void audio_system_destroy_stream(audio_system_t* audio, uint32_t stream_id);
audio_stream_t* audio_system_get_stream(audio_system_t* audio, uint32_t stream_id);
void audio_system_play_stream(audio_system_t* audio, uint32_t stream_id);
void audio_system_pause_stream(audio_system_t* audio, uint32_t stream_id);
void audio_system_stop_stream(audio_system_t* audio, uint32_t stream_id);

// Buffer management
audio_buffer_t* audio_system_create_buffer(audio_system_t* audio, uint32_t samples, 
                                          uint32_t channels, audio_format_t format, 
                                          sample_rate_t sample_rate);
void audio_system_destroy_buffer(audio_system_t* audio, audio_buffer_t* buffer);
void audio_system_load_buffer_data(audio_system_t* audio, audio_buffer_t* buffer, 
                                  void* data, uint32_t size);
void audio_system_attach_buffer_to_stream(audio_system_t* audio, uint32_t stream_id, 
                                         audio_buffer_t* buffer);

// Volume and mixing
void audio_system_set_master_volume(audio_system_t* audio, float volume);
float audio_system_get_master_volume(audio_system_t* audio);
void audio_system_set_stream_volume(audio_system_t* audio, uint32_t stream_id, uint32_t volume);
uint32_t audio_system_get_stream_volume(audio_system_t* audio, uint32_t stream_id);
void audio_system_mute_stream(audio_system_t* audio, uint32_t stream_id, bool mute);
bool audio_system_is_stream_muted(audio_system_t* audio, uint32_t stream_id);

// Spatial audio
void audio_system_set_spatial_mode(audio_system_t* audio, uint32_t stream_id, spatial_mode_t mode);
spatial_mode_t audio_system_get_spatial_mode(audio_system_t* audio, uint32_t stream_id);
void audio_system_set_spatial_params(audio_system_t* audio, uint32_t stream_id, spatial_params_t* params);
void audio_system_get_spatial_params(audio_system_t* audio, uint32_t stream_id, spatial_params_t* params);
void audio_system_set_listener_position(audio_system_t* audio, float x, float y, float z);
void audio_system_set_listener_orientation(audio_system_t* audio, float yaw, float pitch, float roll);

// Audio effects
void audio_system_add_effect(audio_system_t* audio, uint32_t stream_id, effect_params_t* effect);
void audio_system_remove_effect(audio_system_t* audio, uint32_t stream_id, audio_effect_t effect_type);
void audio_system_set_effect_params(audio_system_t* audio, uint32_t stream_id, 
                                   audio_effect_t effect_type, effect_params_t* params);
void audio_system_get_effect_params(audio_system_t* audio, uint32_t stream_id, 
                                   audio_effect_t effect_type, effect_params_t* params);

// Real-time processing
void audio_system_process_frame(audio_system_t* audio, float* input, float* output, uint32_t samples);
void audio_system_update(audio_system_t* audio, float delta_time);
void audio_system_render(audio_system_t* audio);

// Recording and capture
uint32_t audio_system_start_recording(audio_system_t* audio, audio_device_t* device, 
                                     uint32_t duration_ms);
void audio_system_stop_recording(audio_system_t* audio, uint32_t recording_id);
audio_buffer_t* audio_system_get_recording_buffer(audio_system_t* audio, uint32_t recording_id);

// Audio analysis
void audio_system_get_spectrum(audio_system_t* audio, uint32_t stream_id, float* spectrum, uint32_t size);
void audio_system_get_waveform(audio_system_t* audio, uint32_t stream_id, float* waveform, uint32_t size);
float audio_system_get_rms_level(audio_system_t* audio, uint32_t stream_id);
float audio_system_get_peak_level(audio_system_t* audio, uint32_t stream_id);

// System information
uint32_t audio_system_get_device_count(audio_system_t* audio);
uint32_t audio_system_get_stream_count(audio_system_t* audio);
float audio_system_get_cpu_usage(audio_system_t* audio);
uint32_t audio_system_get_latency(audio_system_t* audio);
uint64_t audio_system_get_frame_count(audio_system_t* audio);

// Driver-specific functions
bool audio_system_detect_devices(audio_system_t* audio);
bool audio_system_initialize_driver(audio_system_t* audio, audio_driver_type_t driver_type);
void audio_system_set_driver_parameters(audio_system_t* audio, audio_driver_type_t driver_type, void* params);

// Utility functions
sample_rate_t audio_system_get_supported_sample_rate(audio_system_t* audio, sample_rate_t preferred);
audio_format_t audio_system_get_supported_format(audio_system_t* audio, audio_format_t preferred);
uint32_t audio_system_calculate_buffer_size(audio_system_t* audio, uint32_t samples, 
                                           uint32_t channels, audio_format_t format);
void audio_system_convert_format(audio_system_t* audio, void* input, audio_format_t input_format,
                                void* output, audio_format_t output_format, uint32_t samples);

// Error handling
typedef enum {
    AUDIO_SYSTEM_SUCCESS = 0,
    AUDIO_SYSTEM_ERROR_INVALID_CONTEXT,
    AUDIO_SYSTEM_ERROR_INVALID_DEVICE,
    AUDIO_SYSTEM_ERROR_INVALID_STREAM,
    AUDIO_SYSTEM_ERROR_INVALID_BUFFER,
    AUDIO_SYSTEM_ERROR_DRIVER_FAILED,
    AUDIO_SYSTEM_ERROR_OUT_OF_MEMORY,
    AUDIO_SYSTEM_ERROR_UNSUPPORTED_FORMAT,
    AUDIO_SYSTEM_ERROR_DEVICE_BUSY,
} audio_system_error_t;

audio_system_error_t audio_system_get_last_error(void);
const char* audio_system_get_error_string(audio_system_error_t error);

#endif // AUDIO_SYSTEM_H 