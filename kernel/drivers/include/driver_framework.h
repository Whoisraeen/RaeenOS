#ifndef DRIVER_FRAMEWORK_H
#define DRIVER_FRAMEWORK_H

#include "types.h"
#include "hal/hal.h"
#include "interrupts.h"
#include <stddef.h>
#include <stdbool.h>

// Device types
typedef enum {
    DEVICE_CLASS_UNKNOWN = 0,
    DEVICE_CLASS_STORAGE,
    DEVICE_CLASS_NETWORK,
    DEVICE_CLASS_GRAPHICS,
    DEVICE_CLASS_AUDIO,
    DEVICE_CLASS_INPUT,
    DEVICE_CLASS_USB,
    DEVICE_CLASS_PCI,
    DEVICE_CLASS_ACPI,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_THERMAL,
    DEVICE_CLASS_PLATFORM
} device_class_t;

// Device states
typedef enum {
    DEVICE_STATE_UNKNOWN = 0,
    DEVICE_STATE_INITIALIZING,
    DEVICE_STATE_ACTIVE,
    DEVICE_STATE_SUSPENDED,
    DEVICE_STATE_ERROR,
    DEVICE_STATE_REMOVED
} device_state_t;

// Power states
typedef enum {
    POWER_STATE_D0 = 0,    // Full power
    POWER_STATE_D1,        // Low power
    POWER_STATE_D2,        // Lower power
    POWER_STATE_D3_HOT,    // Off, context saved
    POWER_STATE_D3_COLD    // Off, no context
} device_power_state_t;

// Forward declarations
typedef struct device device_t;
typedef struct driver driver_t;
typedef struct device_driver_ops device_driver_ops_t;

// Device operations
struct device_driver_ops {
    // Lifecycle
    error_t (*probe)(device_t* device);
    error_t (*remove)(device_t* device);
    error_t (*init)(device_t* device);
    error_t (*shutdown)(device_t* device);
    
    // Power management
    error_t (*suspend)(device_t* device, device_power_state_t state);
    error_t (*resume)(device_t* device);
    error_t (*set_power_state)(device_t* device, device_power_state_t state);
    
    // I/O operations
    ssize_t (*read)(device_t* device, void* buffer, size_t size, off_t offset);
    ssize_t (*write)(device_t* device, const void* buffer, size_t size, off_t offset);
    error_t (*ioctl)(device_t* device, u32 cmd, void* arg);
    
    // Interrupt handling
    void (*interrupt_handler)(device_t* device, u32 irq);
    
    // DMA operations
    error_t (*dma_alloc)(device_t* device, size_t size, void** virt_addr, phys_addr_t* phys_addr);
    error_t (*dma_free)(device_t* device, void* virt_addr, phys_addr_t phys_addr, size_t size);
    
    // Configuration
    error_t (*configure)(device_t* device, void* config);
    error_t (*reset)(device_t* device);
};

// Device structure
struct device {
    char name[64];
    char description[256];
    device_class_t class;
    device_state_t state;
    device_power_state_t power_state;
    
    // Device identification
    u32 vendor_id;
    u32 device_id;
    u32 revision;
    u32 subsystem_vendor_id;
    u32 subsystem_device_id;
    
    // Resources
    struct {
        phys_addr_t base_address;
        size_t size;
        u32 flags;
    } memory_regions[6];
    
    struct {
        u16 base;
        u16 size;
    } io_regions[6];
    
    u32 irq_line;
    u32 irq_pin;
    
    // Driver binding
    driver_t* driver;
    device_driver_ops_t* ops;
    void* private_data;
    
    // Parent/child relationships
    device_t* parent;
    device_t* first_child;
    device_t* next_sibling;
    
    // Device tree
    device_t* next;
    device_t* prev;
    
    // Reference counting
    u32 ref_count;
    
    // Synchronization
    bool busy;
    struct thread* owner;
    
    // Statistics
    u64 bytes_read;
    u64 bytes_written;
    u64 operations;
    u64 errors;
    u64 uptime;
};

// Driver structure
struct driver {
    char name[64];
    char description[256];
    char version[32];
    device_class_t class;
    
    // Driver operations
    device_driver_ops_t* ops;
    
    // Device matching
    struct {
        u32 vendor_id;
        u32 device_id;
        u32 class_mask;
        u32 subclass_mask;
    } id_table[16];
    
    // Module information
    void* module_base;
    size_t module_size;
    
    // Linked list
    driver_t* next;
    driver_t* prev;
    
    // Statistics
    u32 device_count;
    u64 load_time;
};

// Device manager structure
typedef struct device_manager {
    device_t* device_list;
    driver_t* driver_list;
    u32 device_count;
    u32 driver_count;
    bool initialized;
} device_manager_t;

// PCI device structure
typedef struct pci_device {
    device_t base;
    
    // PCI-specific fields
    u8 bus;
    u8 device;
    u8 function;
    u16 vendor_id;
    u16 device_id;
    u8 class_code;
    u8 subclass;
    u8 prog_if;
    u8 revision_id;
    u8 header_type;
    
    // Configuration space
    u32 config_space[64]; // 256 bytes
} pci_device_t;

// USB device structure
typedef struct usb_device {
    device_t base;
    
    // USB-specific fields
    u8 address;
    u8 speed;
    u16 vendor_id;
    u16 product_id;
    u8 class;
    u8 subclass;
    u8 protocol;
    
    // Descriptors
    void* device_descriptor;
    void* config_descriptor;
} usb_device_t;

// Core device management functions
error_t device_manager_init(void);
void device_manager_shutdown(void);

// Device registration and management
device_t* device_create(const char* name, device_class_t class);
error_t device_destroy(device_t* device);
error_t device_register(device_t* device);
error_t device_unregister(device_t* device);
device_t* device_find_by_name(const char* name);
device_t* device_find_by_class(device_class_t class);

// Driver registration and management
error_t driver_register(driver_t* driver);
error_t driver_unregister(driver_t* driver);
driver_t* driver_find_by_name(const char* name);
error_t driver_bind_device(driver_t* driver, device_t* device);
error_t driver_unbind_device(device_t* device);

// Device operations
error_t device_init(device_t* device);
error_t device_start(device_t* device);
error_t device_stop(device_t* device);
error_t device_reset(device_t* device);
error_t device_suspend(device_t* device);
error_t device_resume(device_t* device);

// Resource management
error_t device_request_irq(device_t* device, u32 irq, interrupt_handler_func_t handler);
error_t device_free_irq(device_t* device, u32 irq);
error_t device_request_memory_region(device_t* device, phys_addr_t base, size_t size);
error_t device_release_memory_region(device_t* device, phys_addr_t base, size_t size);
error_t device_request_io_region(device_t* device, u16 base, u16 size);
error_t device_release_io_region(device_t* device, u16 base, u16 size);

// DMA management
error_t device_dma_alloc_coherent(device_t* device, size_t size, void** virt_addr, phys_addr_t* phys_addr);
error_t device_dma_free_coherent(device_t* device, size_t size, void* virt_addr, phys_addr_t phys_addr);
error_t device_dma_map_single(device_t* device, void* ptr, size_t size, phys_addr_t* phys_addr);
error_t device_dma_unmap_single(device_t* device, phys_addr_t phys_addr, size_t size);

// Hot-plug support
error_t device_hotplug_add(device_t* device);
error_t device_hotplug_remove(device_t* device);

// PCI bus support
error_t pci_bus_init(void);
error_t pci_scan_bus(void);
pci_device_t* pci_device_create(u8 bus, u8 device, u8 function);
error_t pci_device_enable(pci_device_t* pci_dev);
error_t pci_device_disable(pci_device_t* pci_dev);
u32 pci_config_read(pci_device_t* pci_dev, u8 offset);
error_t pci_config_write(pci_device_t* pci_dev, u8 offset, u32 value);

// USB bus support
error_t usb_bus_init(void);
usb_device_t* usb_device_create(u8 address);
error_t usb_device_enumerate(usb_device_t* usb_dev);

// Platform device support
error_t platform_device_register(device_t* device);
error_t platform_driver_register(driver_t* driver);

// Device tree and enumeration
error_t device_enumerate_all(void);
error_t device_enumerate_class(device_class_t class);
void device_tree_dump(device_t* root, int depth);

// Device I/O wrappers
static inline ssize_t device_read(device_t* device, void* buffer, size_t size, off_t offset) {
    if (device && device->ops && device->ops->read) {
        return device->ops->read(device, buffer, size, offset);
    }
    return -1;
}

static inline ssize_t device_write(device_t* device, const void* buffer, size_t size, off_t offset) {
    if (device && device->ops && device->ops->write) {
        return device->ops->write(device, buffer, size, offset);
    }
    return -1;
}

static inline error_t device_ioctl(device_t* device, u32 cmd, void* arg) {
    if (device && device->ops && device->ops->ioctl) {
        return device->ops->ioctl(device, cmd, arg);
    }
    return E_OPNOTSUPP;
}

// Built-in drivers
extern driver_t pci_driver;
extern driver_t usb_driver;
extern driver_t ata_driver;
extern driver_t ethernet_driver;
extern driver_t graphics_driver;
extern driver_t audio_driver;
extern driver_t keyboard_driver;
extern driver_t mouse_driver;

// Global device manager
extern device_manager_t* global_device_manager;

// Statistics and debugging
typedef struct device_stats {
    u32 total_devices;
    u32 active_devices;
    u32 failed_devices;
    u32 total_drivers;
    u64 total_interrupts;
    u64 total_dma_allocations;
} device_stats_t;

device_stats_t* device_get_stats(void);
void device_dump_stats(void);
void device_dump_tree(void);
void driver_dump_list(void);

// Error codes specific to device management
#define E_DEVICE_NOT_FOUND      -4000
#define E_DEVICE_BUSY           -4001
#define E_DEVICE_NOT_READY      -4002
#define E_DEVICE_TIMEOUT        -4003
#define E_DEVICE_IO_ERROR       -4004
#define E_DEVICE_NO_DRIVER      -4005
#define E_DEVICE_PROBE_FAILED   -4006
#define E_DEVICE_RESOURCE_CONFLICT -4007
#define E_DEVICE_POWER_ERROR    -4008
#define E_DEVICE_DMA_ERROR      -4009

#endif // DRIVER_FRAMEWORK_H