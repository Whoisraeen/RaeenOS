#ifndef AUDIO_H
#define AUDIO_H

#include "../core/include/types.h"
#include "../core/include/error.h"
#include "driver_framework.h"

// Audio device types
typedef enum {
    AUDIO_DEVICE_PLAYBACK = 0,
    AUDIO_DEVICE_CAPTURE,
    AUDIO_DEVICE_DUPLEX,
    AUDIO_DEVICE_MIDI,
    AUDIO_DEVICE_DIGITAL
} audio_device_type_t;

// Audio formats
typedef enum {
    AUDIO_FORMAT_PCM_8 = 0,
    AUDIO_FORMAT_PCM_16,
    AUDIO_FORMAT_PCM_24,
    AUDIO_FORMAT_PCM_32,
    AUDIO_FORMAT_FLOAT_32,
    AUDIO_FORMAT_FLOAT_64,
    AUDIO_FORMAT_AC3,
    AUDIO_FORMAT_DTS,
    AUDIO_FORMAT_FLAC,
    AUDIO_FORMAT_MP3
} audio_format_t;

// Audio sample rates
typedef enum {
    AUDIO_RATE_8000 = 8000,
    AUDIO_RATE_11025 = 11025,
    AUDIO_RATE_16000 = 16000,
    AUDIO_RATE_22050 = 22050,
    AUDIO_RATE_44100 = 44100,
    AUDIO_RATE_48000 = 48000,
    AUDIO_RATE_88200 = 88200,
    AUDIO_RATE_96000 = 96000,
    AUDIO_RATE_176400 = 176400,
    AUDIO_RATE_192000 = 192000,
    AUDIO_RATE_384000 = 384000
} audio_sample_rate_t;

// Audio buffer structure
typedef struct {
    void* data;
    size_t size;
    size_t frames;
    audio_format_t format;
    uint32_t channels;
    uint32_t sample_rate;
    uint64_t timestamp;
    bool is_playing;
} audio_buffer_t;

// Audio device capabilities
typedef struct {
    uint32_t min_channels;
    uint32_t max_channels;
    uint32_t min_sample_rate;
    uint32_t max_sample_rate;
    uint32_t supported_formats;
    uint32_t buffer_sizes[8];
    uint32_t num_buffer_sizes;
    bool supports_mmap;
    bool supports_pause;
    bool supports_resume;
    bool supports_drain;
} audio_caps_t;

// Audio device structure
typedef struct audio_device {
    device_t base;
    char name[128];
    char manufacturer[64];
    audio_device_type_t type;
    audio_caps_t capabilities;
    
    // Current configuration
    uint32_t channels;
    uint32_t sample_rate;
    audio_format_t format;
    uint32_t buffer_size;
    uint32_t period_size;
    
    // State
    bool is_open;
    bool is_running;
    bool is_paused;
    
    // Hardware specific
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t subsystem_id;
    void* hw_data;
    
    // Operations
    error_t (*open)(struct audio_device* dev);
    error_t (*close)(struct audio_device* dev);
    error_t (*start)(struct audio_device* dev);
    error_t (*stop)(struct audio_device* dev);
    error_t (*pause)(struct audio_device* dev);
    error_t (*resume)(struct audio_device* dev);
    error_t (*write)(struct audio_device* dev, const void* data, size_t size);
    error_t (*read)(struct audio_device* dev, void* data, size_t size);
    error_t (*set_volume)(struct audio_device* dev, uint32_t volume);
    error_t (*get_volume)(struct audio_device* dev, uint32_t* volume);
    error_t (*set_mute)(struct audio_device* dev, bool mute);
    error_t (*get_position)(struct audio_device* dev, uint64_t* position);
    
    // List management
    struct audio_device* next;
} audio_device_t;

// Audio mixer structure
typedef struct {
    uint32_t num_channels;
    uint32_t master_volume;
    bool master_mute;
    uint32_t channel_volumes[32];
    bool channel_mutes[32];
    audio_device_t* devices[16];
    uint32_t num_devices;
} audio_mixer_t;

// Audio subsystem structure
typedef struct {
    bool initialized;
    audio_mixer_t mixer;
    audio_device_t* device_list;
    uint32_t num_devices;
    
    // Default devices
    audio_device_t* default_playback;
    audio_device_t* default_capture;
    
    // Thread synchronization
    mutex_t device_mutex;
    mutex_t mixer_mutex;
} audio_subsystem_t;

// Function prototypes
error_t audio_init(void);
void audio_shutdown(void);

// Device management
error_t audio_register_device(audio_device_t* device);
error_t audio_unregister_device(audio_device_t* device);
audio_device_t* audio_find_device(const char* name);
audio_device_t* audio_get_default_playback(void);
audio_device_t* audio_get_default_capture(void);

// Device operations
error_t audio_device_open(audio_device_t* device, uint32_t channels, 
                         uint32_t sample_rate, audio_format_t format);
error_t audio_device_close(audio_device_t* device);
error_t audio_device_start(audio_device_t* device);
error_t audio_device_stop(audio_device_t* device);
error_t audio_device_pause(audio_device_t* device);
error_t audio_device_resume(audio_device_t* device);

// Audio I/O
error_t audio_write(audio_device_t* device, const void* data, size_t size);
error_t audio_read(audio_device_t* device, void* data, size_t size);

// Volume control
error_t audio_set_master_volume(uint32_t volume);
error_t audio_get_master_volume(uint32_t* volume);
error_t audio_set_master_mute(bool mute);
error_t audio_set_device_volume(audio_device_t* device, uint32_t volume);
error_t audio_get_device_volume(audio_device_t* device, uint32_t* volume);

// Format utilities
size_t audio_format_get_bytes_per_sample(audio_format_t format);
size_t audio_format_get_frame_size(audio_format_t format, uint32_t channels);
const char* audio_format_to_string(audio_format_t format);

// Buffer management
audio_buffer_t* audio_buffer_create(size_t frames, audio_format_t format, 
                                   uint32_t channels, uint32_t sample_rate);
void audio_buffer_destroy(audio_buffer_t* buffer);
error_t audio_buffer_copy(audio_buffer_t* dst, const audio_buffer_t* src);

// High-level audio API
error_t audio_play_buffer(const audio_buffer_t* buffer);
error_t audio_play_file(const char* filename);
error_t audio_record_start(audio_device_t* device, const char* filename);
error_t audio_record_stop(audio_device_t* device);

// Audio effects and processing
typedef struct {
    float gain;
    float bass;
    float treble;
    float balance;
    bool reverb_enabled;
    float reverb_level;
    bool equalizer_enabled;
    float eq_bands[10];
} audio_effects_t;

error_t audio_apply_effects(audio_buffer_t* buffer, const audio_effects_t* effects);

// Spatial audio support
typedef struct {
    float x, y, z;
    float velocity_x, velocity_y, velocity_z;
} audio_position_t;

error_t audio_set_3d_position(audio_device_t* device, const audio_position_t* pos);
error_t audio_set_listener_position(const audio_position_t* pos);

// MIDI support
typedef struct {
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
    uint32_t timestamp;
} midi_message_t;

error_t audio_midi_send(audio_device_t* device, const midi_message_t* msg);
error_t audio_midi_receive(audio_device_t* device, midi_message_t* msg);

// Audio codec support
typedef enum {
    AUDIO_CODEC_PCM = 0,
    AUDIO_CODEC_MP3,
    AUDIO_CODEC_AAC,
    AUDIO_CODEC_FLAC,
    AUDIO_CODEC_OGG,
    AUDIO_CODEC_AC3,
    AUDIO_CODEC_DTS
} audio_codec_t;

error_t audio_decode(const void* input, size_t input_size, 
                    audio_codec_t codec, audio_buffer_t** output);
error_t audio_encode(const audio_buffer_t* input, audio_codec_t codec, 
                    void** output, size_t* output_size);

#endif // AUDIO_H