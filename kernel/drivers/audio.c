#include "audio.h"
#include "../core/kernel.h"
#include "../memory/memory.h"
#include "../core/interrupts.h"
#include "../hal/hal.h"
#include "pci.h"
#include <string.h>

// Global audio subsystem
static audio_subsystem_t audio_subsystem = {0};

// Audio device list
static audio_device_t* audio_devices = NULL;
static uint32_t num_audio_devices = 0;

// PCI audio device IDs (Intel HDA, Realtek, etc.)
static const struct {
    uint16_t vendor_id;
    uint16_t device_id;
    const char* name;
} audio_pci_devices[] = {
    {0x8086, 0x2668, "Intel ICH6 Audio"},
    {0x8086, 0x27D8, "Intel ICH7 Audio"},
    {0x8086, 0x284B, "Intel ICH8 Audio"},
    {0x8086, 0x293E, "Intel ICH9 Audio"},
    {0x8086, 0x3A3E, "Intel ICH10 Audio"},
    {0x8086, 0x1C20, "Intel 6 Series Audio"},
    {0x8086, 0x1E20, "Intel 7 Series Audio"},
    {0x8086, 0x8C20, "Intel 8 Series Audio"},
    {0x8086, 0x9C20, "Intel 9 Series Audio"},
    {0x8086, 0xA170, "Intel 100 Series Audio"},
    {0x10DE, 0x0371, "NVIDIA MCP55 Audio"},
    {0x10DE, 0x03F0, "NVIDIA MCP61 Audio"},
    {0x1002, 0x4370, "ATI SB400 Audio"},
    {0x1002, 0x4380, "ATI SB600 Audio"},
    {0x1106, 0x3058, "VIA VT82C686 Audio"},
    {0x1106, 0x3059, "VIA VT8233 Audio"},
    {0x0000, 0x0000, NULL}
};

// Forward declarations
static error_t audio_pci_probe(pci_device_t* pci_dev);
static error_t audio_hda_init(audio_device_t* device, pci_device_t* pci_dev);
static error_t audio_ac97_init(audio_device_t* device, pci_device_t* pci_dev);

error_t audio_init(void) {
    KINFO("Initializing audio subsystem...");
    
    memset(&audio_subsystem, 0, sizeof(audio_subsystem));
    
    // Initialize mutexes
    mutex_init(&audio_subsystem.device_mutex);
    mutex_init(&audio_subsystem.mixer_mutex);
    
    // Initialize mixer
    audio_subsystem.mixer.master_volume = 75; // 75% default volume
    audio_subsystem.mixer.master_mute = false;
    audio_subsystem.mixer.num_channels = 2; // Stereo default
    
    for (uint32_t i = 0; i < 32; i++) {
        audio_subsystem.mixer.channel_volumes[i] = 75;
        audio_subsystem.mixer.channel_mutes[i] = false;
    }
    
    // Scan for PCI audio devices
    pci_device_t* pci_dev = pci_get_first_device();
    while (pci_dev) {
        if (pci_dev->class_code == 0x04) { // Multimedia device
            if (pci_dev->subclass == 0x01 || pci_dev->subclass == 0x03) { // Audio or HDA
                audio_pci_probe(pci_dev);
            }
        }
        pci_dev = pci_get_next_device(pci_dev);
    }
    
    // Create default software audio device if no hardware found
    if (num_audio_devices == 0) {
        KWARN("No hardware audio devices found, creating software device");
        // TODO: Implement software audio device
    }
    
    audio_subsystem.initialized = true;
    KINFO("Audio subsystem initialized with %d devices", num_audio_devices);
    
    return SUCCESS;
}

void audio_shutdown(void) {
    if (!audio_subsystem.initialized) {
        return;
    }
    
    KINFO("Shutting down audio subsystem...");
    
    // Stop and close all devices
    audio_device_t* device = audio_devices;
    while (device) {
        if (device->is_running) {
            audio_device_stop(device);
        }
        if (device->is_open) {
            audio_device_close(device);
        }
        device = device->next;
    }
    
    // Free device list
    device = audio_devices;
    while (device) {
        audio_device_t* next = device->next;
        kfree(device);
        device = next;
    }
    
    audio_subsystem.initialized = false;
    KINFO("Audio subsystem shutdown complete");
}

static error_t audio_pci_probe(pci_device_t* pci_dev) {
    // Check if this is a known audio device
    for (int i = 0; audio_pci_devices[i].name != NULL; i++) {
        if (pci_dev->vendor_id == audio_pci_devices[i].vendor_id &&
            pci_dev->device_id == audio_pci_devices[i].device_id) {
            
            KINFO("Found audio device: %s", audio_pci_devices[i].name);
            
            // Create audio device structure
            audio_device_t* device = kmalloc(sizeof(audio_device_t));
            if (!device) {
                return E_NOMEM;
            }
            
            memset(device, 0, sizeof(audio_device_t));
            strncpy(device->name, audio_pci_devices[i].name, sizeof(device->name) - 1);
            device->vendor_id = pci_dev->vendor_id;
            device->device_id = pci_dev->device_id;
            device->type = AUDIO_DEVICE_DUPLEX;
            
            // Initialize based on device type
            error_t result;
            if (pci_dev->subclass == 0x03) { // HDA
                result = audio_hda_init(device, pci_dev);
            } else { // AC97 or other
                result = audio_ac97_init(device, pci_dev);
            }
            
            if (result == SUCCESS) {
                audio_register_device(device);
                return SUCCESS;
            } else {
                kfree(device);
                return result;
            }
        }
    }
    
    return E_NOENT;
}

error_t audio_register_device(audio_device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    mutex_lock(&audio_subsystem.device_mutex);
    
    // Add to device list
    device->next = audio_devices;
    audio_devices = device;
    num_audio_devices++;
    
    // Set as default if first device
    if (!audio_subsystem.default_playback && 
        (device->type == AUDIO_DEVICE_PLAYBACK || device->type == AUDIO_DEVICE_DUPLEX)) {
        audio_subsystem.default_playback = device;
    }
    
    if (!audio_subsystem.default_capture && 
        (device->type == AUDIO_DEVICE_CAPTURE || device->type == AUDIO_DEVICE_DUPLEX)) {
        audio_subsystem.default_capture = device;
    }
    
    mutex_unlock(&audio_subsystem.device_mutex);
    
    KINFO("Registered audio device: %s", device->name);
    return SUCCESS;
}

error_t audio_unregister_device(audio_device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    mutex_lock(&audio_subsystem.device_mutex);
    
    // Remove from device list
    if (audio_devices == device) {
        audio_devices = device->next;
    } else {
        audio_device_t* current = audio_devices;
        while (current && current->next != device) {
            current = current->next;
        }
        if (current) {
            current->next = device->next;
        }
    }
    
    num_audio_devices--;
    
    // Update default devices
    if (audio_subsystem.default_playback == device) {
        audio_subsystem.default_playback = NULL;
        // Find new default
        audio_device_t* dev = audio_devices;
        while (dev) {
            if (dev->type == AUDIO_DEVICE_PLAYBACK || dev->type == AUDIO_DEVICE_DUPLEX) {
                audio_subsystem.default_playback = dev;
                break;
            }
            dev = dev->next;
        }
    }
    
    if (audio_subsystem.default_capture == device) {
        audio_subsystem.default_capture = NULL;
        // Find new default
        audio_device_t* dev = audio_devices;
        while (dev) {
            if (dev->type == AUDIO_DEVICE_CAPTURE || dev->type == AUDIO_DEVICE_DUPLEX) {
                audio_subsystem.default_capture = dev;
                break;
            }
            dev = dev->next;
        }
    }
    
    mutex_unlock(&audio_subsystem.device_mutex);
    
    KINFO("Unregistered audio device: %s", device->name);
    return SUCCESS;
}

audio_device_t* audio_find_device(const char* name) {
    if (!name) {
        return NULL;
    }
    
    mutex_lock(&audio_subsystem.device_mutex);
    
    audio_device_t* device = audio_devices;
    while (device) {
        if (strcmp(device->name, name) == 0) {
            mutex_unlock(&audio_subsystem.device_mutex);
            return device;
        }
        device = device->next;
    }
    
    mutex_unlock(&audio_subsystem.device_mutex);
    return NULL;
}

audio_device_t* audio_get_default_playback(void) {
    return audio_subsystem.default_playback;
}

audio_device_t* audio_get_default_capture(void) {
    return audio_subsystem.default_capture;
}

error_t audio_device_open(audio_device_t* device, uint32_t channels, 
                         uint32_t sample_rate, audio_format_t format) {
    if (!device || device->is_open) {
        return E_INVAL;
    }
    
    // Validate parameters
    if (channels < device->capabilities.min_channels || 
        channels > device->capabilities.max_channels) {
        return E_INVAL;
    }
    
    if (sample_rate < device->capabilities.min_sample_rate || 
        sample_rate > device->capabilities.max_sample_rate) {
        return E_INVAL;
    }
    
    // Set configuration
    device->channels = channels;
    device->sample_rate = sample_rate;
    device->format = format;
    
    // Call device-specific open function
    error_t result = SUCCESS;
    if (device->open) {
        result = device->open(device);
    }
    
    if (result == SUCCESS) {
        device->is_open = true;
        KDEBUG("Opened audio device: %s (%d channels, %d Hz)", 
               device->name, channels, sample_rate);
    }
    
    return result;
}

error_t audio_device_close(audio_device_t* device) {
    if (!device || !device->is_open) {
        return E_INVAL;
    }
    
    // Stop device if running
    if (device->is_running) {
        audio_device_stop(device);
    }
    
    // Call device-specific close function
    error_t result = SUCCESS;
    if (device->close) {
        result = device->close(device);
    }
    
    device->is_open = false;
    device->is_running = false;
    device->is_paused = false;
    
    KDEBUG("Closed audio device: %s", device->name);
    return result;
}

error_t audio_device_start(audio_device_t* device) {
    if (!device || !device->is_open || device->is_running) {
        return E_INVAL;
    }
    
    error_t result = SUCCESS;
    if (device->start) {
        result = device->start(device);
    }
    
    if (result == SUCCESS) {
        device->is_running = true;
        device->is_paused = false;
        KDEBUG("Started audio device: %s", device->name);
    }
    
    return result;
}

error_t audio_device_stop(audio_device_t* device) {
    if (!device || !device->is_running) {
        return E_INVAL;
    }
    
    error_t result = SUCCESS;
    if (device->stop) {
        result = device->stop(device);
    }
    
    device->is_running = false;
    device->is_paused = false;
    
    KDEBUG("Stopped audio device: %s", device->name);
    return result;
}

error_t audio_write(audio_device_t* device, const void* data, size_t size) {
    if (!device || !device->is_open || !data || size == 0) {
        return E_INVAL;
    }
    
    if (device->type != AUDIO_DEVICE_PLAYBACK && device->type != AUDIO_DEVICE_DUPLEX) {
        return E_INVAL;
    }
    
    if (device->write) {
        return device->write(device, data, size);
    }
    
    return E_NOSYS;
}

error_t audio_read(audio_device_t* device, void* data, size_t size) {
    if (!device || !device->is_open || !data || size == 0) {
        return E_INVAL;
    }
    
    if (device->type != AUDIO_DEVICE_CAPTURE && device->type != AUDIO_DEVICE_DUPLEX) {
        return E_INVAL;
    }
    
    if (device->read) {
        return device->read(device, data, size);
    }
    
    return E_NOSYS;
}

error_t audio_set_master_volume(uint32_t volume) {
    if (volume > 100) {
        return E_INVAL;
    }
    
    mutex_lock(&audio_subsystem.mixer_mutex);
    audio_subsystem.mixer.master_volume = volume;
    mutex_unlock(&audio_subsystem.mixer_mutex);
    
    return SUCCESS;
}

error_t audio_get_master_volume(uint32_t* volume) {
    if (!volume) {
        return E_INVAL;
    }
    
    mutex_lock(&audio_subsystem.mixer_mutex);
    *volume = audio_subsystem.mixer.master_volume;
    mutex_unlock(&audio_subsystem.mixer_mutex);
    
    return SUCCESS;
}

error_t audio_set_master_mute(bool mute) {
    mutex_lock(&audio_subsystem.mixer_mutex);
    audio_subsystem.mixer.master_mute = mute;
    mutex_unlock(&audio_subsystem.mixer_mutex);
    
    return SUCCESS;
}

size_t audio_format_get_bytes_per_sample(audio_format_t format) {
    switch (format) {
        case AUDIO_FORMAT_PCM_8: return 1;
        case AUDIO_FORMAT_PCM_16: return 2;
        case AUDIO_FORMAT_PCM_24: return 3;
        case AUDIO_FORMAT_PCM_32: return 4;
        case AUDIO_FORMAT_FLOAT_32: return 4;
        case AUDIO_FORMAT_FLOAT_64: return 8;
        default: return 2; // Default to 16-bit
    }
}

size_t audio_format_get_frame_size(audio_format_t format, uint32_t channels) {
    return audio_format_get_bytes_per_sample(format) * channels;
}

const char* audio_format_to_string(audio_format_t format) {
    switch (format) {
        case AUDIO_FORMAT_PCM_8: return "PCM 8-bit";
        case AUDIO_FORMAT_PCM_16: return "PCM 16-bit";
        case AUDIO_FORMAT_PCM_24: return "PCM 24-bit";
        case AUDIO_FORMAT_PCM_32: return "PCM 32-bit";
        case AUDIO_FORMAT_FLOAT_32: return "Float 32-bit";
        case AUDIO_FORMAT_FLOAT_64: return "Float 64-bit";
        case AUDIO_FORMAT_AC3: return "AC3";
        case AUDIO_FORMAT_DTS: return "DTS";
        case AUDIO_FORMAT_FLAC: return "FLAC";
        case AUDIO_FORMAT_MP3: return "MP3";
        default: return "Unknown";
    }
}

// HDA (High Definition Audio) initialization
static error_t audio_hda_init(audio_device_t* device, pci_device_t* pci_dev) {
    KINFO("Initializing HDA device: %s", device->name);
    
    // Set capabilities for HDA
    device->capabilities.min_channels = 1;
    device->capabilities.max_channels = 8;
    device->capabilities.min_sample_rate = 8000;
    device->capabilities.max_sample_rate = 192000;
    device->capabilities.supported_formats = 
        (1 << AUDIO_FORMAT_PCM_16) | 
        (1 << AUDIO_FORMAT_PCM_24) | 
        (1 << AUDIO_FORMAT_PCM_32);
    device->capabilities.supports_mmap = true;
    device->capabilities.supports_pause = true;
    device->capabilities.supports_resume = true;
    
    // TODO: Implement HDA-specific initialization
    // - Map MMIO registers
    // - Initialize codec
    // - Set up DMA buffers
    // - Configure streams
    
    strncpy(device->manufacturer, "Intel/Realtek", sizeof(device->manufacturer) - 1);
    
    return SUCCESS;
}

// AC97 initialization
static error_t audio_ac97_init(audio_device_t* device, pci_device_t* pci_dev) {
    KINFO("Initializing AC97 device: %s", device->name);
    
    // Set capabilities for AC97
    device->capabilities.min_channels = 1;
    device->capabilities.max_channels = 6; // 5.1 surround
    device->capabilities.min_sample_rate = 8000;
    device->capabilities.max_sample_rate = 48000;
    device->capabilities.supported_formats = 
        (1 << AUDIO_FORMAT_PCM_16) | 
        (1 << AUDIO_FORMAT_PCM_24);
    device->capabilities.supports_mmap = false;
    device->capabilities.supports_pause = true;
    device->capabilities.supports_resume = true;
    
    // TODO: Implement AC97-specific initialization
    // - Map I/O ports
    // - Initialize codec
    // - Set up DMA buffers
    
    strncpy(device->manufacturer, "AC97", sizeof(device->manufacturer) - 1);
    
    return SUCCESS;
}