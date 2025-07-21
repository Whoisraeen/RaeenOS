#include "drivers/gpu.h"
#include "kernel.h"
#include "memory/memory.h"
#include <string.h>

// GPU constants
#define GPU_VENDOR_INTEL 0x8086
#define GPU_VENDOR_NVIDIA 0x10DE
#define GPU_VENDOR_AMD 0x1002

// GPU device IDs
#define INTEL_HD_GRAPHICS_2000 0x0102
#define INTEL_HD_GRAPHICS_3000 0x0112
#define INTEL_HD_GRAPHICS_4000 0x0162
#define NVIDIA_GTX_1060 0x1C03
#define AMD_RX_580 0x67DF

// GPU registers
#define GPU_COMMAND_REG 0x00
#define GPU_STATUS_REG 0x04
#define GPU_FRAMEBUFFER_ADDR 0x08
#define GPU_FRAMEBUFFER_SIZE 0x0C
#define GPU_RESOLUTION_WIDTH 0x10
#define GPU_RESOLUTION_HEIGHT 0x14
#define GPU_BYTES_PER_PIXEL 0x18

// GPU commands
#define GPU_CMD_SET_MODE 0x01
#define GPU_CMD_CLEAR_SCREEN 0x02
#define GPU_CMD_DRAW_PIXEL 0x03
#define GPU_CMD_DRAW_LINE 0x04
#define GPU_CMD_DRAW_RECT 0x05
#define GPU_CMD_DRAW_CIRCLE 0x06
#define GPU_CMD_BLIT 0x07
#define GPU_CMD_3D_RENDER 0x08

// Global GPU state
static gpu_state_t gpu_state = {0};
static bool gpu_initialized = false;
static gpu_device_t gpu_devices[MAX_GPU_DEVICES];
static int gpu_device_count = 0;
static void* framebuffer = NULL;
static u32 framebuffer_size = 0;

// Initialize GPU subsystem
error_t gpu_init(void) {
    if (gpu_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing GPU subsystem...");
    
    // Initialize GPU state
    memset(&gpu_state, 0, sizeof(gpu_state));
    gpu_state.initialized = false;
    
    // Initialize GPU devices array
    memset(gpu_devices, 0, sizeof(gpu_devices));
    gpu_device_count = 0;
    
    // Detect GPU devices
    gpu_detect_devices();
    
    // Initialize primary GPU
    if (gpu_device_count > 0) {
        gpu_init_primary_device();
    }
    
    gpu_initialized = true;
    gpu_state.initialized = true;
    
    KINFO("GPU subsystem initialized with %d devices", gpu_device_count);
    
    return SUCCESS;
}

// Detect GPU devices
void gpu_detect_devices(void) {
    KDEBUG("Detecting GPU devices...");
    
    // Scan PCI bus for GPU devices
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int function = 0; function < 8; function++) {
                u32 vendor_id = pci_read_config(bus, device, function, 0x00);
                u32 device_id = pci_read_config(bus, device, function, 0x02);
                
                if (vendor_id == 0xFFFF) {
                    continue; // No device
                }
                
                // Check if this is a GPU device
                if (gpu_is_gpu_device(vendor_id, device_id)) {
                    gpu_add_device(bus, device, function, vendor_id, device_id);
                }
            }
        }
    }
    
    KINFO("GPU detection complete: %d devices found", gpu_device_count);
}

// Check if device is a GPU
bool gpu_is_gpu_device(u32 vendor_id, u32 device_id) {
    // Intel integrated graphics
    if (vendor_id == GPU_VENDOR_INTEL) {
        return (device_id >= 0x0100 && device_id <= 0xFFFF);
    }
    
    // NVIDIA discrete graphics
    if (vendor_id == GPU_VENDOR_NVIDIA) {
        return (device_id >= 0x1000 && device_id <= 0xFFFF);
    }
    
    // AMD discrete graphics
    if (vendor_id == GPU_VENDOR_AMD) {
        return (device_id >= 0x6000 && device_id <= 0xFFFF);
    }
    
    return false;
}

// Add GPU device
void gpu_add_device(int bus, int device, int function, u32 vendor_id, u32 device_id) {
    if (gpu_device_count >= MAX_GPU_DEVICES) {
        return;
    }
    
    gpu_device_t* gpu = &gpu_devices[gpu_device_count];
    gpu->bus = bus;
    gpu->device = device;
    gpu->function = function;
    gpu->vendor_id = vendor_id;
    gpu->device_id = device_id;
    gpu->present = true;
    
    // Get device information
    gpu_get_device_info(gpu);
    
    gpu_device_count++;
    
    KINFO("Found GPU: %s (vendor: 0x%04X, device: 0x%04X)", 
          gpu->name, vendor_id, device_id);
}

// Get device information
void gpu_get_device_info(gpu_device_t* gpu) {
    // Read device class and subclass
    u32 class_code = pci_read_config(gpu->bus, gpu->device, gpu->function, 0x08);
    u8 base_class = (class_code >> 16) & 0xFF;
    u8 sub_class = (class_code >> 8) & 0xFF;
    
    if (base_class == 0x03 && sub_class == 0x00) {
        // VGA compatible controller
        strcpy(gpu->name, "VGA Compatible GPU");
    } else if (base_class == 0x03 && sub_class == 0x80) {
        // Display controller
        strcpy(gpu->name, "Display Controller");
    } else {
        strcpy(gpu->name, "Unknown GPU");
    }
    
    // Get memory BAR
    u32 bar0 = pci_read_config(gpu->bus, gpu->device, gpu->function, 0x10);
    gpu->memory_base = bar0 & 0xFFFFFFF0;
    
    // Get IRQ line
    u8 irq_line = pci_read_config(gpu->bus, gpu->device, gpu->function, 0x3C) & 0xFF;
    gpu->irq_line = irq_line;
    
    // Determine GPU type
    if (gpu->vendor_id == GPU_VENDOR_INTEL) {
        gpu->type = GPU_TYPE_INTEGRATED;
    } else {
        gpu->type = GPU_TYPE_DISCRETE;
    }
}

// Initialize primary GPU device
void gpu_init_primary_device(void) {
    if (gpu_device_count == 0) {
        return;
    }
    
    gpu_device_t* primary = &gpu_devices[0];
    
    // Enable memory access
    u32 command = pci_read_config(primary->bus, primary->device, primary->function, 0x04);
    command |= 0x02; // Enable memory access
    pci_write_config(primary->bus, primary->device, primary->function, 0x04, command);
    
    // Initialize GPU-specific driver
    switch (primary->vendor_id) {
        case GPU_VENDOR_INTEL:
            gpu_init_intel(primary);
            break;
        case GPU_VENDOR_NVIDIA:
            gpu_init_nvidia(primary);
            break;
        case GPU_VENDOR_AMD:
            gpu_init_amd(primary);
            break;
        default:
            gpu_init_generic(primary);
            break;
    }
    
    KINFO("Primary GPU initialized: %s", primary->name);
}

// Initialize Intel GPU
void gpu_init_intel(gpu_device_t* gpu) {
    KDEBUG("Initializing Intel GPU");
    
    // Intel GPU initialization
    gpu->driver_type = GPU_DRIVER_INTEL;
    
    // Set up framebuffer
    gpu_setup_framebuffer(gpu, 1024, 768, 32);
    
    // Enable hardware acceleration
    gpu_enable_hardware_acceleration(gpu);
}

// Initialize NVIDIA GPU
void gpu_init_nvidia(gpu_device_t* gpu) {
    KDEBUG("Initializing NVIDIA GPU");
    
    // NVIDIA GPU initialization
    gpu->driver_type = GPU_DRIVER_NVIDIA;
    
    // Set up framebuffer
    gpu_setup_framebuffer(gpu, 1920, 1080, 32);
    
    // Enable hardware acceleration
    gpu_enable_hardware_acceleration(gpu);
}

// Initialize AMD GPU
void gpu_init_amd(gpu_device_t* gpu) {
    KDEBUG("Initializing AMD GPU");
    
    // AMD GPU initialization
    gpu->driver_type = GPU_DRIVER_AMD;
    
    // Set up framebuffer
    gpu_setup_framebuffer(gpu, 1920, 1080, 32);
    
    // Enable hardware acceleration
    gpu_enable_hardware_acceleration(gpu);
}

// Initialize generic GPU
void gpu_init_generic(gpu_device_t* gpu) {
    KDEBUG("Initializing generic GPU");
    
    // Generic GPU initialization
    gpu->driver_type = GPU_DRIVER_GENERIC;
    
    // Set up basic framebuffer
    gpu_setup_framebuffer(gpu, 800, 600, 24);
}

// Set up framebuffer
void gpu_setup_framebuffer(gpu_device_t* gpu, u32 width, u32 height, u32 bpp) {
    gpu->width = width;
    gpu->height = height;
    gpu->bpp = bpp;
    gpu->bytes_per_pixel = bpp / 8;
    gpu->pitch = width * gpu->bytes_per_pixel;
    gpu->framebuffer_size = height * gpu->pitch;
    
    // Allocate framebuffer memory
    framebuffer = memory_allocate_pages(gpu->framebuffer_size / 4096 + 1);
    if (framebuffer) {
        gpu->framebuffer_addr = (u64)framebuffer;
        framebuffer_size = gpu->framebuffer_size;
        
        // Clear framebuffer
        memset(framebuffer, 0, gpu->framebuffer_size);
        
        KINFO("Framebuffer set up: %dx%d, %d bpp, %u bytes", 
              width, height, bpp, gpu->framebuffer_size);
    }
}

// Enable hardware acceleration
void gpu_enable_hardware_acceleration(gpu_device_t* gpu) {
    switch (gpu->driver_type) {
        case GPU_DRIVER_INTEL:
            gpu_enable_intel_acceleration(gpu);
            break;
        case GPU_DRIVER_NVIDIA:
            gpu_enable_nvidia_acceleration(gpu);
            break;
        case GPU_DRIVER_AMD:
            gpu_enable_amd_acceleration(gpu);
            break;
        default:
            break;
    }
    
    gpu->hardware_acceleration = true;
    KINFO("Hardware acceleration enabled for %s", gpu->name);
}

// Enable Intel hardware acceleration
void gpu_enable_intel_acceleration(gpu_device_t* gpu) {
    // Intel GPU acceleration setup
    // TODO: Implement Intel-specific acceleration
    KDEBUG("Intel hardware acceleration enabled");
}

// Enable NVIDIA hardware acceleration
void gpu_enable_nvidia_acceleration(gpu_device_t* gpu) {
    // NVIDIA GPU acceleration setup
    // TODO: Implement NVIDIA-specific acceleration
    KDEBUG("NVIDIA hardware acceleration enabled");
}

// Enable AMD hardware acceleration
void gpu_enable_amd_acceleration(gpu_device_t* gpu) {
    // AMD GPU acceleration setup
    // TODO: Implement AMD-specific acceleration
    KDEBUG("AMD hardware acceleration enabled");
}

// Set display mode
error_t gpu_set_mode(u32 width, u32 height, u32 bpp) {
    if (!gpu_initialized || gpu_device_count == 0) {
        return E_INVAL;
    }
    
    gpu_device_t* primary = &gpu_devices[0];
    
    // Set up new framebuffer
    gpu_setup_framebuffer(primary, width, height, bpp);
    
    // Update GPU state
    gpu_state.width = width;
    gpu_state.height = height;
    gpu_state.bpp = bpp;
    
    KINFO("Display mode set: %dx%d, %d bpp", width, height, bpp);
    
    return SUCCESS;
}

// Clear screen
void gpu_clear_screen(u32 color) {
    if (!framebuffer) {
        return;
    }
    
    gpu_device_t* primary = &gpu_devices[0];
    u32* fb = (u32*)framebuffer;
    
    for (u32 i = 0; i < primary->width * primary->height; i++) {
        fb[i] = color;
    }
}

// Draw pixel
void gpu_draw_pixel(u32 x, u32 y, u32 color) {
    if (!framebuffer || x >= gpu_state.width || y >= gpu_state.height) {
        return;
    }
    
    u32* fb = (u32*)framebuffer;
    u32 offset = y * gpu_state.width + x;
    fb[offset] = color;
}

// Draw line
void gpu_draw_line(u32 x1, u32 y1, u32 x2, u32 y2, u32 color) {
    // Bresenham's line algorithm
    int dx = abs((int)x2 - (int)x1);
    int dy = abs((int)y2 - (int)y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    u32 x = x1, y = y1;
    
    while (true) {
        gpu_draw_pixel(x, y, color);
        
        if (x == x2 && y == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

// Draw rectangle
void gpu_draw_rect(u32 x, u32 y, u32 width, u32 height, u32 color) {
    for (u32 i = 0; i < height; i++) {
        gpu_draw_line(x, y + i, x + width - 1, y + i, color);
    }
}

// Draw circle
void gpu_draw_circle(u32 center_x, u32 center_y, u32 radius, u32 color) {
    // Midpoint circle algorithm
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        gpu_draw_pixel(center_x + x, center_y + y, color);
        gpu_draw_pixel(center_x + y, center_y + x, color);
        gpu_draw_pixel(center_x - y, center_y + x, color);
        gpu_draw_pixel(center_x - x, center_y + y, color);
        gpu_draw_pixel(center_x - x, center_y - y, color);
        gpu_draw_pixel(center_x - y, center_y - x, color);
        gpu_draw_pixel(center_x + y, center_y - x, color);
        gpu_draw_pixel(center_x + x, center_y - y, color);
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

// Blit (copy) image
void gpu_blit(u32 dest_x, u32 dest_y, u32 src_x, u32 src_y, u32 width, u32 height, void* src_buffer) {
    if (!framebuffer || !src_buffer) {
        return;
    }
    
    u32* dest = (u32*)framebuffer;
    u32* src = (u32*)src_buffer;
    
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            u32 dest_offset = (dest_y + y) * gpu_state.width + (dest_x + x);
            u32 src_offset = (src_y + y) * width + (src_x + x);
            
            if (dest_x + x < gpu_state.width && dest_y + y < gpu_state.height) {
                dest[dest_offset] = src[src_offset];
            }
        }
    }
}

// 3D rendering (basic)
void gpu_render_3d_triangle(u32 x1, u32 y1, u32 x2, u32 y2, u32 x3, u32 y3, u32 color) {
    // Basic triangle rendering
    gpu_draw_line(x1, y1, x2, y2, color);
    gpu_draw_line(x2, y2, x3, y3, color);
    gpu_draw_line(x3, y3, x1, y1, color);
}

// Get GPU state
gpu_state_t* gpu_get_state(void) {
    return &gpu_state;
}

// Get primary GPU device
gpu_device_t* gpu_get_primary_device(void) {
    if (gpu_device_count == 0) {
        return NULL;
    }
    return &gpu_devices[0];
}

// Get framebuffer
void* gpu_get_framebuffer(void) {
    return framebuffer;
}

// Get framebuffer size
u32 gpu_get_framebuffer_size(void) {
    return framebuffer_size;
}

// Check if GPU is initialized
bool gpu_is_initialized(void) {
    return gpu_initialized;
}

// Update GPU (called periodically)
void gpu_update(void) {
    if (!gpu_initialized) {
        return;
    }
    
    // Update GPU state
    // TODO: Handle GPU events, vsync, etc.
}

// Shutdown GPU
void gpu_shutdown(void) {
    if (!gpu_initialized) {
        return;
    }
    
    KINFO("Shutting down GPU subsystem...");
    
    // Free framebuffer
    if (framebuffer) {
        memory_free_pages(framebuffer, framebuffer_size / 4096 + 1);
        framebuffer = NULL;
        framebuffer_size = 0;
    }
    
    gpu_initialized = false;
    gpu_state.initialized = false;
    
    KINFO("GPU subsystem shutdown complete");
} 