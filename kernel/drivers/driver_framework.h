#ifndef RAEENOS_DRIVER_FRAMEWORK_H
#define RAEENOS_DRIVER_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// Device types
typedef enum {
    DEVICE_TYPE_UNKNOWN = 0,
    DEVICE_TYPE_CHAR,           // Character device (keyboard, serial, etc.)
    DEVICE_TYPE_BLOCK,          // Block device (disk, flash, etc.)
    DEVICE_TYPE_NETWORK,        // Network device (ethernet, wifi, etc.)
    DEVICE_TYPE_DISPLAY,        // Display device (VGA, framebuffer, etc.)
    DEVICE_TYPE_AUDIO,          // Audio device (sound card, etc.)
    DEVICE_TYPE_INPUT,          // Input device (mouse, touchpad, etc.)
    DEVICE_TYPE_STORAGE,        // Storage device (hard drive, SSD, etc.)
    DEVICE_TYPE_BUS,            // Bus device (PCI, USB, etc.)
    DEVICE_TYPE_SENSOR,         // Sensor device (temperature, accelerometer, etc.)
    DEVICE_TYPE_MISC            // Miscellaneous device
} device_type_t;

// Device states
typedef enum {
    DEVICE_STATE_UNKNOWN = 0,
    DEVICE_STATE_DETECTED,      // Device detected but not initialized
    DEVICE_STATE_INITIALIZING,  // Device is being initialized
    DEVICE_STATE_READY,         // Device is ready for use
    DEVICE_STATE_ERROR,         // Device has encountered an error
    DEVICE_STATE_SUSPENDED,     // Device is suspended
    DEVICE_STATE_OFFLINE        // Device is offline
} device_state_t;

// Bus types
typedef enum {
    BUS_TYPE_UNKNOWN = 0,
    BUS_TYPE_PCI,               // PCI/PCIe bus
    BUS_TYPE_USB,               // USB bus
    BUS_TYPE_I2C,               // I2C bus
    BUS_TYPE_SPI,               // SPI bus
    BUS_TYPE_ISA,               // ISA bus
    BUS_TYPE_ACPI,              // ACPI bus
    BUS_TYPE_PLATFORM           // Platform bus
} bus_type_t;

// Device operations structure
typedef struct device_ops {
    // Basic operations
    int (*init)(struct device* dev);
    int (*shutdown)(struct device* dev);
    int (*reset)(struct device* dev);
    int (*suspend)(struct device* dev);
    int (*resume)(struct device* dev);
    
    // I/O operations
    ssize_t (*read)(struct device* dev, void* buffer, size_t size, off_t offset);
    ssize_t (*write)(struct device* dev, const void* buffer, size_t size, off_t offset);
    int (*ioctl)(struct device* dev, unsigned long request, void* arg);
    
    // Power management
    int (*set_power_state)(struct device* dev, int state);
    int (*get_power_state)(struct device* dev);
    
    // Interrupt handling
    int (*enable_interrupt)(struct device* dev, int irq);
    int (*disable_interrupt)(struct device* dev, int irq);
    void (*interrupt_handler)(struct device* dev);
    
    // DMA operations
    int (*dma_alloc)(struct device* dev, size_t size, void** addr, uintptr_t* dma_addr);
    int (*dma_free)(struct device* dev, void* addr, uintptr_t dma_addr);
    int (*dma_sync)(struct device* dev, void* addr, size_t size, int direction);
    
    // Device-specific operations
    int (*probe)(struct device* dev);
    int (*remove)(struct device* dev);
    int (*suspend_late)(struct device* dev);
    int (*resume_early)(struct device* dev);
} device_ops_t;

// Device structure
typedef struct device {
    char name[64];              // Device name
    char description[256];      // Device description
    device_type_t type;         // Device type
    device_state_t state;       // Device state
    uint32_t id;                // Device ID
    uint32_t vendor_id;         // Vendor ID
    uint32_t device_id;         // Device ID
    uint32_t revision;          // Device revision
    uint32_t class_code;        // Device class code
    uint32_t subclass_code;     // Device subclass code
    uint32_t prog_if;           // Programming interface
    
    // Bus information
    bus_type_t bus_type;        // Bus type
    uint32_t bus_number;        // Bus number
    uint32_t device_number;     // Device number on bus
    uint32_t function_number;   // Function number
    
    // Resource information
    uintptr_t io_base;          // I/O base address
    uintptr_t mem_base;         // Memory base address
    size_t mem_size;            // Memory size
    int irq;                    // Interrupt number
    uint32_t dma_channel;       // DMA channel
    
    // Driver information
    struct device_driver* driver; // Associated driver
    device_ops_t* ops;          // Device operations
    void* driver_data;          // Driver-specific data
    void* platform_data;        // Platform-specific data
    
    // Device tree
    struct device* parent;      // Parent device
    struct device* children;    // First child device
    struct device* next_sibling; // Next sibling device
    struct device* prev_sibling; // Previous sibling device
    
    // Reference counting
    uint32_t ref_count;         // Reference count
    spinlock_t lock;            // Device lock
    
    // Statistics
    uint64_t read_operations;   // Number of read operations
    uint64_t write_operations;  // Number of write operations
    uint64_t errors;            // Number of errors
    uint64_t interrupts;        // Number of interrupts
} device_t;

// Device driver structure
typedef struct device_driver {
    char name[64];              // Driver name
    char description[256];      // Driver description
    device_type_t device_type;  // Supported device type
    bus_type_t bus_type;        // Supported bus type
    
    // Driver operations
    int (*probe)(struct device* dev);
    int (*remove)(struct device* dev);
    int (*suspend)(struct device* dev);
    int (*resume)(struct device* dev);
    
    // Device operations
    device_ops_t ops;           // Device operations
    
    // Driver information
    uint32_t version;           // Driver version
    char* author;               // Driver author
    char* license;              // Driver license
    
    // Driver tree
    struct device_driver* next; // Next driver in list
} device_driver_t;

// Bus driver structure
typedef struct bus_driver {
    char name[64];              // Bus driver name
    bus_type_t bus_type;        // Bus type
    
    // Bus operations
    int (*init)(void);
    int (*shutdown)(void);
    int (*scan)(void);
    int (*add_device)(struct device* dev);
    int (*remove_device)(struct device* dev);
    
    // Bus information
    struct bus_driver* next;    // Next bus driver
} bus_driver_t;

// Device manager structure
typedef struct device_manager {
    device_t* devices;          // List of devices
    device_driver_t* drivers;   // List of drivers
    bus_driver_t* bus_drivers;  // List of bus drivers
    uint32_t device_count;      // Number of devices
    uint32_t driver_count;      // Number of drivers
    spinlock_t lock;            // Manager lock
} device_manager_t;

// Function prototypes

// Device manager
int device_manager_init(void);
void device_manager_shutdown(void);
device_manager_t* device_manager_get(void);

// Device management
device_t* device_create(const char* name, device_type_t type);
void device_destroy(device_t* dev);
int device_register(device_t* dev);
int device_unregister(device_t* dev);
device_t* device_find_by_name(const char* name);
device_t* device_find_by_id(uint32_t id);
device_t* device_find_by_type(device_type_t type);

// Driver management
int driver_register(device_driver_t* driver);
int driver_unregister(device_driver_t* driver);
device_driver_t* driver_find_by_name(const char* name);
device_driver_t* driver_find_by_type(device_type_t type);

// Bus driver management
int bus_driver_register(bus_driver_t* bus_driver);
int bus_driver_unregister(bus_driver_t* bus_driver);
bus_driver_t* bus_driver_find_by_type(bus_type_t bus_type);

// Device operations
int device_init(device_t* dev);
int device_shutdown(device_t* dev);
int device_reset(device_t* dev);
int device_suspend(device_t* dev);
int device_resume(device_t* dev);

// I/O operations
ssize_t device_read(device_t* dev, void* buffer, size_t size, off_t offset);
ssize_t device_write(device_t* dev, const void* buffer, size_t size, off_t offset);
int device_ioctl(device_t* dev, unsigned long request, void* arg);

// Power management
int device_set_power_state(device_t* dev, int state);
int device_get_power_state(device_t* dev);

// Interrupt handling
int device_enable_interrupt(device_t* dev, int irq);
int device_disable_interrupt(device_t* dev, int irq);
void device_interrupt_handler(device_t* dev);

// DMA operations
int device_dma_alloc(device_t* dev, size_t size, void** addr, uintptr_t* dma_addr);
int device_dma_free(device_t* dev, void* addr, uintptr_t dma_addr);
int device_dma_sync(device_t* dev, void* addr, size_t size, int direction);

// Device tree operations
int device_add_child(device_t* parent, device_t* child);
int device_remove_child(device_t* parent, device_t* child);
device_t* device_find_child(device_t* parent, const char* name);

// Reference counting
void device_ref(device_t* dev);
void device_unref(device_t* dev);

// Device enumeration
int device_scan_all(void);
int device_scan_bus(bus_type_t bus_type);

// Driver matching
device_driver_t* device_find_driver(device_t* dev);
int device_bind_driver(device_t* dev, device_driver_t* driver);
int device_unbind_driver(device_t* dev);

// Device statistics
void device_get_stats(device_t* dev, device_stats_t* stats);
void device_reset_stats(device_t* dev);

// Device debugging
void device_dump_info(device_t* dev);
void device_dump_tree(device_t* root, int depth);
void device_list_all(void);

// Platform-specific operations
int device_set_platform_data(device_t* dev, void* data);
void* device_get_platform_data(device_t* dev);
int device_set_driver_data(device_t* dev, void* data);
void* device_get_driver_data(device_t* dev);

// Device properties
int device_set_property(device_t* dev, const char* name, const void* value, size_t size);
int device_get_property(device_t* dev, const char* name, void* value, size_t size);
int device_has_property(device_t* dev, const char* name);

// Error handling
int device_set_error(device_t* dev, int error);
int device_get_error(device_t* dev);
const char* device_error_string(int error);

// Device notifications
typedef void (*device_notify_callback)(device_t* dev, int event, void* data);
int device_register_notify(device_t* dev, device_notify_callback callback);
int device_unregister_notify(device_t* dev, device_notify_callback callback);

// Device events
#define DEVICE_EVENT_ADDED      1
#define DEVICE_EVENT_REMOVED    2
#define DEVICE_EVENT_SUSPENDED  3
#define DEVICE_EVENT_RESUMED    4
#define DEVICE_EVENT_ERROR      5
#define DEVICE_EVENT_RESET      6

// Device statistics structure
typedef struct device_stats {
    uint64_t read_operations;
    uint64_t write_operations;
    uint64_t errors;
    uint64_t interrupts;
    uint64_t dma_transfers;
    uint64_t power_state_changes;
    uint64_t uptime;
} device_stats_t;

#endif // RAEENOS_DRIVER_FRAMEWORK_H 