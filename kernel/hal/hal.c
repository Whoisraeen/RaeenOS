#include "hal.h"
#include "memory/memory.h"
#include "interrupts/interrupts.h"
#include <string.h>
#include <stdio.h>

// Global HAL state
static bool hal_initialized = false;
static hal_info_t hal_info;
static cpu_info_t cpu_info;
static memory_info_t memory_info;

// Device detection
static device_list_t device_list;
static u32 device_count = 0;

// HAL initialization
error_t hal_init(void) {
    if (hal_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing Hardware Abstraction Layer...");
    
    // Initialize HAL info
    memset(&hal_info, 0, sizeof(hal_info));
    hal_info.version_major = HAL_VERSION_MAJOR;
    hal_info.version_minor = HAL_VERSION_MINOR;
    hal_info.version_patch = HAL_VERSION_PATCH;
    
    // Detect CPU
    error_t result = hal_detect_cpu(&cpu_info);
    if (result != SUCCESS) {
        KERROR("Failed to detect CPU");
        return result;
    }
    
    // Detect memory
    result = hal_detect_memory(&memory_info);
    if (result != SUCCESS) {
        KERROR("Failed to detect memory");
        return result;
    }
    
    // Initialize architecture-specific HAL
#ifdef __x86_64__
    result = hal_init_x64();
#else
    result = hal_init_x86();
#endif
    
    if (result != SUCCESS) {
        KERROR("Failed to initialize architecture-specific HAL");
        return result;
    }
    
    // Initialize console
    result = hal_early_console_init();
    if (result != SUCCESS) {
        KWARN("Failed to initialize early console");
    }
    
    // Detect devices
    result = hal_detect_devices(&device_list, &device_count);
    if (result != SUCCESS) {
        KWARN("Failed to detect some devices");
    }
    
    hal_initialized = true;
    
    KINFO("HAL initialized successfully");
    KINFO("CPU: %s %s", cpu_info.vendor, cpu_info.brand);
    KINFO("Memory: %llu MB total, %llu MB available", 
          memory_info.total_memory / (1024 * 1024),
          memory_info.available_memory / (1024 * 1024));
    KINFO("Devices detected: %u", device_count);
    
    return SUCCESS;
}

void hal_shutdown(void) {
    if (!hal_initialized) {
        return;
    }
    
    KINFO("Shutting down Hardware Abstraction Layer...");
    
    // Shutdown devices
    for (u32 i = 0; i < device_count; i++) {
        if (device_list.devices[i].shutdown) {
            device_list.devices[i].shutdown(&device_list.devices[i]);
        }
    }
    
    // Shutdown architecture-specific HAL
#ifdef __x86_64__
    hal_shutdown_x64();
#else
    hal_shutdown_x86();
#endif
    
    hal_initialized = false;
    KINFO("HAL shutdown complete");
}

// CPU detection and information
error_t hal_detect_cpu(cpu_info_t* info) {
    if (!info) {
        return E_INVAL;
    }
    
    memset(info, 0, sizeof(cpu_info_t));
    
    // Check for CPUID support
    u32 eax, ebx, ecx, edx;
    
    // Get CPU vendor string
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    
    memcpy(info->vendor, &ebx, 4);
    memcpy(info->vendor + 4, &edx, 4);
    memcpy(info->vendor + 8, &ecx, 4);
    info->vendor[12] = '\0';
    
    // Get CPU brand string
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000002));
    memcpy(info->brand, &eax, 4);
    memcpy(info->brand + 4, &ebx, 4);
    memcpy(info->brand + 8, &ecx, 4);
    memcpy(info->brand + 12, &edx, 4);
    
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000003));
    memcpy(info->brand + 16, &eax, 4);
    memcpy(info->brand + 20, &ebx, 4);
    memcpy(info->brand + 24, &ecx, 4);
    memcpy(info->brand + 28, &edx, 4);
    
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000004));
    memcpy(info->brand + 32, &eax, 4);
    memcpy(info->brand + 36, &ebx, 4);
    memcpy(info->brand + 40, &ecx, 4);
    memcpy(info->brand + 44, &edx, 4);
    info->brand[48] = '\0';
    
    // Get CPU family, model, stepping
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    info->family = ((eax >> 8) & 0xF) + ((eax >> 20) & 0xFF);
    info->model = ((eax >> 4) & 0xF) + ((eax >> 12) & 0xF0);
    info->stepping = eax & 0xF;
    info->features = edx;
    
    // Get cache information
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(2));
    
    // Parse cache information (simplified)
    info->cache_size_l1 = 32 * 1024;  // Assume 32KB L1
    info->cache_size_l2 = 256 * 1024; // Assume 256KB L2
    info->cache_size_l3 = 8 * 1024 * 1024; // Assume 8MB L3
    
    // Get core count (simplified)
    info->core_count = 4;  // Assume 4 cores
    info->thread_count = 8; // Assume 8 threads
    
    return SUCCESS;
}

// Memory detection
error_t hal_detect_memory(memory_info_t* info) {
    if (!info) {
        return E_INVAL;
    }
    
    memset(info, 0, sizeof(memory_info_t));
    
    // Get memory map from multiboot or BIOS
    // For now, assume 8GB total memory
    info->total_memory = 8ULL * 1024 * 1024 * 1024;
    info->available_memory = 7ULL * 1024 * 1024 * 1024; // Reserve 1GB for kernel
    
    // Add memory regions
    hal_add_memory_region(0x00000000, 0x0009FFFF, MEMORY_TYPE_CONVENTIONAL, 0);
    hal_add_memory_region(0x00100000, 0x07FFFFFF, MEMORY_TYPE_CONVENTIONAL, 0);
    hal_add_memory_region(0x08000000, 0x1FFFFFFF, MEMORY_TYPE_EXTENDED, 0);
    
    return SUCCESS;
}

// Device detection
error_t hal_detect_devices(device_list_t* list, u32* count) {
    if (!list || !count) {
        return E_INVAL;
    }
    
    memset(list, 0, sizeof(device_list_t));
    *count = 0;
    
    // Detect PCI devices
    error_t result = hal_detect_pci_devices(list, count);
    if (result != SUCCESS) {
        KWARN("Failed to detect PCI devices");
    }
    
    // Detect legacy devices
    result = hal_detect_legacy_devices(list, count);
    if (result != SUCCESS) {
        KWARN("Failed to detect legacy devices");
    }
    
    return SUCCESS;
}

// PCI device detection
error_t hal_detect_pci_devices(device_list_t* list, u32* count) {
    // Scan PCI buses
    for (u8 bus = 0; bus < 256; bus++) {
        for (u8 device = 0; device < 32; device++) {
            for (u8 function = 0; function < 8; function++) {
                u32 vendor_id, device_id;
                
                // Read vendor and device IDs
                if (hal_pci_read_config(bus, device, function, 0, &vendor_id) == SUCCESS) {
                    if (vendor_id != 0xFFFF) {
                        // Valid device found
                        device_info_t* dev = &list->devices[*count];
                        
                        dev->type = DEVICE_TYPE_PCI;
                        dev->bus = bus;
                        dev->device = device;
                        dev->function = function;
                        dev->vendor_id = vendor_id & 0xFFFF;
                        dev->device_id = (vendor_id >> 16) & 0xFFFF;
                        
                        // Read device class
                        u32 class_code;
                        hal_pci_read_config(bus, device, function, 8, &class_code);
                        dev->class_code = (class_code >> 16) & 0xFF;
                        dev->subclass = (class_code >> 8) & 0xFF;
                        dev->prog_if = class_code & 0xFF;
                        
                        // Initialize device
                        hal_init_device(dev);
                        
                        (*count)++;
                        if (*count >= MAX_DEVICES) {
                            return SUCCESS;
                        }
                    }
                }
            }
        }
    }
    
    return SUCCESS;
}

// Legacy device detection
error_t hal_detect_legacy_devices(device_list_t* list, u32* count) {
    // Detect keyboard controller
    device_info_t* keyboard = &list->devices[*count];
    keyboard->type = DEVICE_TYPE_LEGACY;
    keyboard->name = "i8042 Keyboard Controller";
    keyboard->io_base = 0x60;
    keyboard->irq = IRQ_KEYBOARD;
    hal_init_device(keyboard);
    (*count)++;
    
    // Detect timer
    device_info_t* timer = &list->devices[*count];
    timer->type = DEVICE_TYPE_LEGACY;
    timer->name = "8253/8254 Timer";
    timer->io_base = 0x40;
    timer->irq = IRQ_TIMER;
    hal_init_device(timer);
    (*count)++;
    
    return SUCCESS;
}

// Device initialization
error_t hal_init_device(device_info_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    // Set default handlers
    device->init = hal_default_device_init;
    device->shutdown = hal_default_device_shutdown;
    device->read = hal_default_device_read;
    device->write = hal_default_device_write;
    device->ioctl = hal_default_device_ioctl;
    
    // Initialize device based on type
    switch (device->type) {
        case DEVICE_TYPE_PCI:
            return hal_init_pci_device(device);
        case DEVICE_TYPE_LEGACY:
            return hal_init_legacy_device(device);
        default:
            return E_INVAL;
    }
}

// Default device handlers
error_t hal_default_device_init(device_info_t* device) {
    KINFO("Initializing device: %s", device->name);
    return SUCCESS;
}

error_t hal_default_device_shutdown(device_info_t* device) {
    KINFO("Shutting down device: %s", device->name);
    return SUCCESS;
}

error_t hal_default_device_read(device_info_t* device, void* buffer, size_t size, off_t offset) {
    return E_NOSYS;
}

error_t hal_default_device_write(device_info_t* device, const void* buffer, size_t size, off_t offset) {
    return E_NOSYS;
}

error_t hal_default_device_ioctl(device_info_t* device, u32 request, void* arg) {
    return E_NOSYS;
}

// Memory region management
error_t hal_add_memory_region(phys_addr_t start, phys_addr_t end, u32 type, u32 flags) {
    return memory_add_region(start, end, type, flags);
}

// Console functions
error_t hal_early_console_init(void) {
    // Initialize VGA text mode console
    // This is a simplified implementation
    return SUCCESS;
}

bool hal_is_console_ready(void) {
    return true;
}

void hal_console_print(const char* format, ...) {
    // Simplified console output
    // In a real implementation, this would write to VGA or serial
    printf(format);
}

void hal_console_vprint(const char* format, va_list args) {
    vprintf(format, args);
}

void hal_console_set_color(u32 color) {
    // Set console color
}

void hal_console_clear(void) {
    // Clear console
}

// Time and timing functions
uint64_t hal_get_timestamp(void) {
    // Read from TSC (Time Stamp Counter)
    uint64_t tsc;
    __asm__ volatile("rdtsc" : "=A"(tsc));
    return tsc;
}

uint64_t hal_get_cpu_frequency(void) {
    // Calculate CPU frequency from TSC
    return 2400000000ULL; // Assume 2.4GHz
}

void hal_sleep_ms(u32 milliseconds) {
    // Sleep for specified milliseconds
    // This is a simplified implementation
}

void hal_sleep_us(u32 microseconds) {
    // Sleep for specified microseconds
    // This is a simplified implementation
}

// CPU control functions
void hal_enable_interrupts(void) {
    __asm__ volatile("sti");
}

void hal_disable_interrupts(void) {
    __asm__ volatile("cli");
}

bool hal_are_interrupts_enabled(void) {
    u64 rflags;
    __asm__ volatile("pushfq; popq %0" : "=r"(rflags));
    return (rflags & 0x200) != 0;
}

void hal_halt_cpu(void) {
    __asm__ volatile("hlt");
}

void hal_pause_cpu(void) {
    __asm__ volatile("pause");
}

// Memory management functions
phys_addr_t hal_get_total_memory(void) {
    return memory_info.total_memory;
}

phys_addr_t hal_get_available_memory(void) {
    return memory_info.available_memory;
}

void* hal_map_physical(phys_addr_t phys_addr, size_t size, u32 flags) {
    return memory_map_device(phys_addr, size, NULL);
}

void hal_unmap_physical(void* virt_addr, size_t size) {
    memory_unmap_device(virt_addr, size);
}

// I/O functions
u8 hal_inb(u16 port) {
    u8 value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

u16 hal_inw(u16 port) {
    u16 value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

u32 hal_inl(u16 port) {
    u32 value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void hal_outb(u16 port, u8 value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void hal_outw(u16 port, u16 value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

void hal_outl(u16 port, u32 value) {
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

// PCI functions
error_t hal_pci_read_config(u8 bus, u8 device, u8 function, u8 offset, u32* value) {
    u32 address = 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    
    hal_outl(0xCF8, address);
    *value = hal_inl(0xCFC);
    
    return SUCCESS;
}

error_t hal_pci_write_config(u8 bus, u8 device, u8 function, u8 offset, u32 value) {
    u32 address = 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    
    hal_outl(0xCF8, address);
    hal_outl(0xCFC, value);
    
    return SUCCESS;
}

// Debugging functions
void hal_dump_registers(void) {
    // Dump CPU registers
}

void hal_dump_stack_trace(void) {
    // Dump stack trace
}

void hal_breakpoint(void) {
    __asm__ volatile("int3");
}

// Power management
error_t hal_set_power_state(u32 state) {
    // Set system power state
    return SUCCESS;
}

u32 hal_get_power_state(void) {
    // Get current power state
    return POWER_STATE_S0;
}

// Memory barriers
void hal_memory_barrier(void) {
    __asm__ volatile("mfence" : : : "memory");
}

void hal_read_barrier(void) {
    __asm__ volatile("lfence" : : : "memory");
}

void hal_write_barrier(void) {
    __asm__ volatile("sfence" : : : "memory");
}

// Cache management
void hal_flush_cache(void) {
    __asm__ volatile("wbinvd");
}

void hal_invalidate_cache(void) {
    __asm__ volatile("invd");
}

void hal_flush_tlb(void) {
    __asm__ volatile("invlpg (%0)" : : "r"(0) : "memory");
}

void hal_invalidate_tlb_entry(virt_addr_t addr) {
    __asm__ volatile("invlpg (%0)" : : "r"(addr) : "memory");
} 
// AI Accelerator Abstraction Layer
#define AI_ACCEL_PRIORITY 0x01

struct ai_hardware {
    uint8_t  accelerator_type;  // NPU/GPU/TPU/SIMD
    uint16_t ops_per_joule;    // Efficiency metric
    uint32_t max_matrix_size;  // Largest supported tensor
    bool     supports_quant;   // 8-bit quantization
};

// Neural Network Hardware Discovery
void detect_ai_accelerators() {
    // Check for vendor-specific AI chips
    if (cpuid_check(INTEL_NPU_FEATURE)) {
        register_accel(INTEL_NPU, 85000, 8192, true);
    }
    if (amd_ai_supported()) {
        register_accel(AMD_XDNA, 92000, 16384, true);
    }
    
    // Fallback to GPU acceleration
    if (!ai_accel_count && pci_find_gpu()) {
        init_gpu_matrix_ops();
        register_accel(GPU_OPENCL, 45000, 4096, false);
    }
    
    // Final fallback to AVX-512
    if (!ai_accel_count && cpu_has_avx512()) {
        enable_avx512_ai_mode();
        register_accel(CPU_SIMD, 12000, 1024, false);
    }
}