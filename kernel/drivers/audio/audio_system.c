#include "audio_system.h"
#include <kernel/memory/memory.h>
#include <kernel/libc/string.h>
#include <kernel/libc/stdio.h>
#include <kernel/hal/hal.h>
#include <kernel/interrupts/interrupts.h>

// Internal structures
typedef struct {
    uint32_t id;
    audio_device_t* device;
    audio_buffer_t* buffer;
    uint32_t position;
    uint32_t volume;
    bool is_muted;
    bool is_paused;
    spatial_mode_t spatial_mode;
    spatial_params_t spatial_params;
    effect_params_t effects[8];
    uint32_t effect_count;
    float* processed_buffer;
    uint32_t processed_size;
    void* user_data;
} internal_audio_stream_t;

typedef struct {
    uint32_t id;
    audio_device_t* device;
    uint32_t duration_ms;
    uint32_t recorded_samples;
    audio_buffer_t* buffer;
    bool is_recording;
    uint64_t start_time;
} recording_session_t;

// Global state
static audio_system_t* g_audio_system = NULL;
static audio_system_error_t g_last_error = AUDIO_SYSTEM_SUCCESS;
static uint64_t g_frame_count = 0;
static float g_cpu_usage = 0.0f;
static uint32_t g_latency_ms = 0;

// Internal functions
static void set_error(audio_system_error_t error) {
    g_last_error = error;
}

static uint32_t calculate_buffer_size(uint32_t samples, uint32_t channels, audio_format_t format) {
    uint32_t bytes_per_sample = 0;
    switch (format) {
        case AUDIO_FORMAT_PCM_8: bytes_per_sample = 1; break;
        case AUDIO_FORMAT_PCM_16: bytes_per_sample = 2; break;
        case AUDIO_FORMAT_PCM_24: bytes_per_sample = 3; break;
        case AUDIO_FORMAT_PCM_32: bytes_per_sample = 4; break;
        case AUDIO_FORMAT_FLOAT_32: bytes_per_sample = 4; break;
        case AUDIO_FORMAT_FLOAT_64: bytes_per_sample = 8; break;
        default: return 0;
    }
    return samples * channels * bytes_per_sample;
}

static float apply_volume(float sample, uint32_t volume) {
    return sample * (volume / 100.0f);
}

static void apply_spatial_processing(float* input, float* output, uint32_t samples, 
                                   uint32_t channels, spatial_params_t* params) {
    if (!params || channels < 2) {
        memcpy(output, input, samples * channels * sizeof(float));
        return;
    }
    
    // Simple spatial processing - in a real implementation would use HRTF
    for (uint32_t i = 0; i < samples; i++) {
        if (channels >= 2) {
            // Apply distance attenuation
            float distance_factor = 1.0f / (1.0f + params->distance);
            
            // Apply azimuth (left/right panning)
            float left_gain = 1.0f - (params->azimuth / 360.0f);
            float right_gain = params->azimuth / 360.0f;
            
            output[i * channels + 0] = input[i * channels + 0] * left_gain * distance_factor;
            output[i * channels + 1] = input[i * channels + 1] * right_gain * distance_factor;
        }
        
        // Copy additional channels
        for (uint32_t ch = 2; ch < channels; ch++) {
            output[i * channels + ch] = input[i * channels + ch] * distance_factor;
        }
    }
}

static void apply_effect(float* buffer, uint32_t samples, effect_params_t* effect) {
    if (!effect || effect->type == AUDIO_EFFECT_NONE) return;
    
    switch (effect->type) {
        case AUDIO_EFFECT_REVERB:
            // Simple reverb implementation
            for (uint32_t i = 0; i < samples; i++) {
                if (i >= (uint32_t)(effect->frequency * 1000)) {
                    buffer[i] += buffer[i - (uint32_t)(effect->frequency * 1000)] * effect->intensity * 0.5f;
                }
            }
            break;
            
        case AUDIO_EFFECT_ECHO:
            // Simple echo implementation
            for (uint32_t i = 0; i < samples; i++) {
                if (i >= (uint32_t)(effect->frequency * 1000)) {
                    buffer[i] += buffer[i - (uint32_t)(effect->frequency * 1000)] * effect->intensity;
                }
            }
            break;
            
        case AUDIO_EFFECT_COMPRESSOR:
            // Simple compressor
            for (uint32_t i = 0; i < samples; i++) {
                float abs_sample = fabs(buffer[i]);
                if (abs_sample > effect->intensity) {
                    buffer[i] = (buffer[i] > 0 ? 1.0f : -1.0f) * effect->intensity;
                }
            }
            break;
            
        case AUDIO_EFFECT_LIMITER:
            // Simple limiter
            for (uint32_t i = 0; i < samples; i++) {
                if (buffer[i] > effect->intensity) {
                    buffer[i] = effect->intensity;
                } else if (buffer[i] < -effect->intensity) {
                    buffer[i] = -effect->intensity;
                }
            }
            break;
            
        default:
            // Other effects not implemented yet
            break;
    }
}

// Initialization and shutdown
audio_system_t* audio_system_init(audio_system_config_t* config) {
    if (g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    
    if (!config) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    
    g_audio_system = kmalloc(sizeof(audio_system_t));
    if (!g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    memset(g_audio_system, 0, sizeof(audio_system_t));
    
    // Copy configuration
    memcpy(&g_audio_system->config, config, sizeof(audio_system_config_t));
    
    // Initialize device arrays
    g_audio_system->max_devices = 16;
    g_audio_system->devices = kmalloc(sizeof(audio_device_t) * g_audio_system->max_devices);
    if (!g_audio_system->devices) {
        set_error(AUDIO_SYSTEM_ERROR_OUT_OF_MEMORY);
        kfree(g_audio_system);
        g_audio_system = NULL;
        return NULL;
    }
    memset(g_audio_system->devices, 0, sizeof(audio_device_t) * g_audio_system->max_devices);
    
    // Initialize stream arrays
    g_audio_system->max_streams = config->max_streams;
    g_audio_system->streams = kmalloc(sizeof(audio_stream_t) * g_audio_system->max_streams);
    if (!g_audio_system->streams) {
        set_error(AUDIO_SYSTEM_ERROR_OUT_OF_MEMORY);
        kfree(g_audio_system->devices);
        kfree(g_audio_system);
        g_audio_system = NULL;
        return NULL;
    }
    memset(g_audio_system->streams, 0, sizeof(audio_stream_t) * g_audio_system->max_streams);
    
    // Initialize default devices
    g_audio_system->default_output = NULL;
    g_audio_system->default_input = NULL;
    
    // Set initial state
    g_audio_system->initialized = true;
    g_audio_system->frame_count = 0;
    g_audio_system->cpu_usage = 0.0f;
    g_audio_system->latency_ms = 0;
    
    printf("Audio System: Initialized with %d max streams\n", config->max_streams);
    printf("Audio System: Sample rate: %d, Format: %d, Channels: %d\n", 
           config->default_sample_rate, config->default_format, config->default_channels);
    
    return g_audio_system;
}

void audio_system_shutdown(audio_system_t* audio) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    printf("Audio System: Shutting down...\n");
    
    // Stop all streams
    for (uint32_t i = 0; i < audio->stream_count; i++) {
        if (audio->streams[i].is_playing) {
            audio_system_stop_stream(audio, audio->streams[i].id);
        }
    }
    
    // Free device arrays
    if (audio->devices) {
        kfree(audio->devices);
    }
    
    // Free stream arrays
    if (audio->streams) {
        kfree(audio->streams);
    }
    
    kfree(audio);
    g_audio_system = NULL;
    
    printf("Audio System: Shutdown complete\n");
}

bool audio_system_is_initialized(audio_system_t* audio) {
    return audio && audio == g_audio_system && audio->initialized;
}

// Device management
uint32_t audio_system_add_device(audio_system_t* audio, audio_device_t* device) {
    if (!audio || audio != g_audio_system || !device) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0;
    }
    
    if (audio->device_count >= audio->max_devices) {
        set_error(AUDIO_SYSTEM_ERROR_DEVICE_BUSY);
        return 0;
    }
    
    // Assign device ID
    device->id = audio->device_count + 1;
    
    // Add to device list
    memcpy(&audio->devices[audio->device_count], device, sizeof(audio_device_t));
    audio->device_count++;
    
    // Set as default if first device of its type
    if (device->type == AUDIO_DEVICE_SPEAKERS && !audio->default_output) {
        audio->default_output = &audio->devices[audio->device_count - 1];
    } else if (device->type == AUDIO_DEVICE_MICROPHONE && !audio->default_input) {
        audio->default_input = &audio->devices[audio->device_count - 1];
    }
    
    printf("Audio System: Added device '%s' (ID: %d, Type: %d)\n", 
           device->name, device->id, device->type);
    
    return device->id;
}

void audio_system_remove_device(audio_system_t* audio, uint32_t device_id) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Find device
    for (uint32_t i = 0; i < audio->device_count; i++) {
        if (audio->devices[i].id == device_id) {
            // Stop all streams using this device
            for (uint32_t j = 0; j < audio->stream_count; j++) {
                if (audio->streams[j].device && audio->streams[j].device->id == device_id) {
                    audio_system_stop_stream(audio, audio->streams[j].id);
                }
            }
            
            // Remove device
            for (uint32_t j = i; j < audio->device_count - 1; j++) {
                memcpy(&audio->devices[j], &audio->devices[j + 1], sizeof(audio_device_t));
            }
            audio->device_count--;
            
            printf("Audio System: Removed device ID %d\n", device_id);
            return;
        }
    }
    
    set_error(AUDIO_SYSTEM_ERROR_INVALID_DEVICE);
}

audio_device_t* audio_system_get_device(audio_system_t* audio, uint32_t device_id) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    
    for (uint32_t i = 0; i < audio->device_count; i++) {
        if (audio->devices[i].id == device_id) {
            return &audio->devices[i];
        }
    }
    
    set_error(AUDIO_SYSTEM_ERROR_INVALID_DEVICE);
    return NULL;
}

audio_device_t* audio_system_get_default_output(audio_system_t* audio) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    return audio->default_output;
}

audio_device_t* audio_system_get_default_input(audio_system_t* audio) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    return audio->default_input;
}

// Stream management
uint32_t audio_system_create_stream(audio_system_t* audio, audio_device_t* device) {
    if (!audio || audio != g_audio_system || !device) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0;
    }
    
    if (audio->stream_count >= audio->max_streams) {
        set_error(AUDIO_SYSTEM_ERROR_DEVICE_BUSY);
        return 0;
    }
    
    // Create stream
    audio_stream_t* stream = &audio->streams[audio->stream_count];
    stream->id = audio->stream_count + 1;
    stream->device = device;
    stream->buffer = NULL;
    stream->position = 0;
    stream->volume = 100;
    stream->is_muted = false;
    stream->is_paused = false;
    stream->spatial_mode = SPATIAL_MODE_OFF;
    stream->user_data = NULL;
    
    audio->stream_count++;
    
    printf("Audio System: Created stream ID %d for device '%s'\n", 
           stream->id, device->name);
    
    return stream->id;
}

void audio_system_destroy_stream(audio_system_t* audio, uint32_t stream_id) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Find and stop stream
    for (uint32_t i = 0; i < audio->stream_count; i++) {
        if (audio->streams[i].id == stream_id) {
            if (audio->streams[i].is_playing) {
                audio_system_stop_stream(audio, stream_id);
            }
            
            // Remove stream
            for (uint32_t j = i; j < audio->stream_count - 1; j++) {
                memcpy(&audio->streams[j], &audio->streams[j + 1], sizeof(audio_stream_t));
            }
            audio->stream_count--;
            
            printf("Audio System: Destroyed stream ID %d\n", stream_id);
            return;
        }
    }
    
    set_error(AUDIO_SYSTEM_ERROR_INVALID_STREAM);
}

audio_stream_t* audio_system_get_stream(audio_system_t* audio, uint32_t stream_id) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    
    for (uint32_t i = 0; i < audio->stream_count; i++) {
        if (audio->streams[i].id == stream_id) {
            return &audio->streams[i];
        }
    }
    
    set_error(AUDIO_SYSTEM_ERROR_INVALID_STREAM);
    return NULL;
}

void audio_system_play_stream(audio_system_t* audio, uint32_t stream_id) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    audio_stream_t* stream = audio_system_get_stream(audio, stream_id);
    if (!stream) return;
    
    if (!stream->buffer) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_BUFFER);
        return;
    }
    
    stream->is_playing = true;
    stream->is_paused = false;
    
    printf("Audio System: Started playing stream ID %d\n", stream_id);
}

void audio_system_pause_stream(audio_system_t* audio, uint32_t stream_id) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    audio_stream_t* stream = audio_system_get_stream(audio, stream_id);
    if (!stream) return;
    
    stream->is_paused = true;
    
    printf("Audio System: Paused stream ID %d\n", stream_id);
}

void audio_system_stop_stream(audio_system_t* audio, uint32_t stream_id) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    audio_stream_t* stream = audio_system_get_stream(audio, stream_id);
    if (!stream) return;
    
    stream->is_playing = false;
    stream->is_paused = false;
    stream->position = 0;
    
    printf("Audio System: Stopped stream ID %d\n", stream_id);
}

// Buffer management
audio_buffer_t* audio_system_create_buffer(audio_system_t* audio, uint32_t samples, 
                                          uint32_t channels, audio_format_t format, 
                                          sample_rate_t sample_rate) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    
    audio_buffer_t* buffer = kmalloc(sizeof(audio_buffer_t));
    if (!buffer) {
        set_error(AUDIO_SYSTEM_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    memset(buffer, 0, sizeof(audio_buffer_t));
    
    buffer->samples = samples;
    buffer->channels = channels;
    buffer->format = format;
    buffer->sample_rate = sample_rate;
    buffer->size = calculate_buffer_size(samples, channels, format);
    buffer->is_playing = false;
    buffer->is_looping = false;
    buffer->timestamp = hal_get_tick_count();
    
    // Allocate data buffer
    buffer->data = kmalloc(buffer->size);
    if (!buffer->data) {
        set_error(AUDIO_SYSTEM_ERROR_OUT_OF_MEMORY);
        kfree(buffer);
        return NULL;
    }
    memset(buffer->data, 0, buffer->size);
    
    printf("Audio System: Created buffer with %d samples, %d channels, format %d\n", 
           samples, channels, format);
    
    return buffer;
}

void audio_system_destroy_buffer(audio_system_t* audio, audio_buffer_t* buffer) {
    if (!audio || audio != g_audio_system || !buffer) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    if (buffer->data) {
        kfree(buffer->data);
    }
    kfree(buffer);
    
    printf("Audio System: Destroyed audio buffer\n");
}

void audio_system_load_buffer_data(audio_system_t* audio, audio_buffer_t* buffer, 
                                  void* data, uint32_t size) {
    if (!audio || audio != g_audio_system || !buffer || !data) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    if (size > buffer->size) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_BUFFER);
        return;
    }
    
    memcpy(buffer->data, data, size);
    buffer->timestamp = hal_get_tick_count();
    
    printf("Audio System: Loaded %d bytes into buffer\n", size);
}

void audio_system_attach_buffer_to_stream(audio_system_t* audio, uint32_t stream_id, 
                                         audio_buffer_t* buffer) {
    if (!audio || audio != g_audio_system || !buffer) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    audio_stream_t* stream = audio_system_get_stream(audio, stream_id);
    if (!stream) return;
    
    stream->buffer = buffer;
    stream->position = 0;
    
    printf("Audio System: Attached buffer to stream ID %d\n", stream_id);
}

// Volume and mixing
void audio_system_set_master_volume(audio_system_t* audio, float volume) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Clamp volume to 0.0 - 1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    audio->config.master_volume = volume;
    
    printf("Audio System: Set master volume to %.2f\n", volume);
}

float audio_system_get_master_volume(audio_system_t* audio) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0.0f;
    }
    return audio->config.master_volume;
}

void audio_system_set_stream_volume(audio_system_t* audio, uint32_t stream_id, uint32_t volume) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    audio_stream_t* stream = audio_system_get_stream(audio, stream_id);
    if (!stream) return;
    
    // Clamp volume to 0 - 100
    if (volume > 100) volume = 100;
    
    stream->volume = volume;
    
    printf("Audio System: Set stream %d volume to %d\n", stream_id, volume);
}

uint32_t audio_system_get_stream_volume(audio_system_t* audio, uint32_t stream_id) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0;
    }
    
    audio_stream_t* stream = audio_system_get_stream(audio, stream_id);
    if (!stream) return 0;
    
    return stream->volume;
}

void audio_system_mute_stream(audio_system_t* audio, uint32_t stream_id, bool mute) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    audio_stream_t* stream = audio_system_get_stream(audio, stream_id);
    if (!stream) return;
    
    stream->is_muted = mute;
    
    printf("Audio System: %s stream %d\n", mute ? "Muted" : "Unmuted", stream_id);
}

bool audio_system_is_stream_muted(audio_system_t* audio, uint32_t stream_id) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return false;
    }
    
    audio_stream_t* stream = audio_system_get_stream(audio, stream_id);
    if (!stream) return false;
    
    return stream->is_muted;
}

// Spatial audio
void audio_system_set_spatial_mode(audio_system_t* audio, uint32_t stream_id, spatial_mode_t mode) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    audio_stream_t* stream = audio_system_get_stream(audio, stream_id);
    if (!stream) return;
    
    stream->spatial_mode = mode;
    
    printf("Audio System: Set stream %d spatial mode to %d\n", stream_id, mode);
}

spatial_mode_t audio_system_get_spatial_mode(audio_system_t* audio, uint32_t stream_id) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return SPATIAL_MODE_OFF;
    }
    
    audio_stream_t* stream = audio_system_get_stream(audio, stream_id);
    if (!stream) return SPATIAL_MODE_OFF;
    
    return stream->spatial_mode;
}

// Real-time processing
void audio_system_process_frame(audio_system_t* audio, float* input, float* output, uint32_t samples) {
    if (!audio || audio != g_audio_system || !input || !output) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Apply master volume
    for (uint32_t i = 0; i < samples; i++) {
        output[i] = input[i] * audio->config.master_volume;
    }
    
    // Process active streams
    for (uint32_t i = 0; i < audio->stream_count; i++) {
        audio_stream_t* stream = &audio->streams[i];
        if (!stream->is_playing || stream->is_paused || !stream->buffer) continue;
        
        // Mix stream into output
        // This is a simplified implementation
        // In a real system, you'd have proper mixing and spatial processing
    }
}

void audio_system_update(audio_system_t* audio, float delta_time) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Update stream positions
    for (uint32_t i = 0; i < audio->stream_count; i++) {
        audio_stream_t* stream = &audio->streams[i];
        if (!stream->is_playing || stream->is_paused || !stream->buffer) continue;
        
        // Advance position based on time
        uint32_t samples_per_frame = (uint32_t)(stream->buffer->sample_rate * delta_time);
        stream->position += samples_per_frame;
        
        // Check for end of buffer
        if (stream->position >= stream->buffer->samples) {
            if (stream->buffer->is_looping) {
                stream->position = 0;
            } else {
                stream->is_playing = false;
                stream->position = 0;
            }
        }
    }
    
    // Update performance metrics
    g_frame_count++;
    g_cpu_usage = 0.0f; // TODO: Calculate actual CPU usage
    g_latency_ms = 10; // TODO: Calculate actual latency
}

void audio_system_render(audio_system_t* audio) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // In a real implementation, this would send audio data to the hardware
    // For now, we just update the frame count
    audio->frame_count = g_frame_count;
    audio->cpu_usage = g_cpu_usage;
    audio->latency_ms = g_latency_ms;
}

// System information
uint32_t audio_system_get_device_count(audio_system_t* audio) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0;
    }
    return audio->device_count;
}

uint32_t audio_system_get_stream_count(audio_system_t* audio) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0;
    }
    return audio->stream_count;
}

float audio_system_get_cpu_usage(audio_system_t* audio) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0.0f;
    }
    return audio->cpu_usage;
}

uint32_t audio_system_get_latency(audio_system_t* audio) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0;
    }
    return audio->latency_ms;
}

uint64_t audio_system_get_frame_count(audio_system_t* audio) {
    if (!audio || audio != g_audio_system) {
        set_error(AUDIO_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0;
    }
    return audio->frame_count;
}

// Error handling
audio_system_error_t audio_system_get_last_error(void) {
    return g_last_error;
}

const char* audio_system_get_error_string(audio_system_error_t error) {
    switch (error) {
        case AUDIO_SYSTEM_SUCCESS:
            return "Success";
        case AUDIO_SYSTEM_ERROR_INVALID_CONTEXT:
            return "Invalid audio system context";
        case AUDIO_SYSTEM_ERROR_INVALID_DEVICE:
            return "Invalid audio device";
        case AUDIO_SYSTEM_ERROR_INVALID_STREAM:
            return "Invalid audio stream";
        case AUDIO_SYSTEM_ERROR_INVALID_BUFFER:
            return "Invalid audio buffer";
        case AUDIO_SYSTEM_ERROR_DRIVER_FAILED:
            return "Audio driver failed";
        case AUDIO_SYSTEM_ERROR_OUT_OF_MEMORY:
            return "Out of memory";
        case AUDIO_SYSTEM_ERROR_UNSUPPORTED_FORMAT:
            return "Unsupported audio format";
        case AUDIO_SYSTEM_ERROR_DEVICE_BUSY:
            return "Audio device busy";
        default:
            return "Unknown error";
    }
} 