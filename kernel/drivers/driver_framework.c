#include "drivers/include/drivers.h"
#include "kernel.h"
#include "memory/memory.h"
#include "hal/hal.h"
#include <string.h>

// Driver framework configuration
#define MAX_DRIVERS 256
#define MAX_DEVICES 1024
#define MAX_DEVICE_CLASSES 64
#define DRIVER_NAME_MAX 64
#define DEVICE_NAME_MAX 64
#define MAX_DRIVER_DEPENDENCIES 16

// Device types
typedef enum {
    DEVICE_TYPE_UNKNOWN,
    DEVICE_TYPE_CHAR,
    DEVICE_TYPE_BLOCK,
    DEVICE_TYPE_NETWORK,
    DEVICE_TYPE_DISPLAY,
    DEVICE_TYPE_INPUT,
    DEVICE_TYPE_AUDIO,
    DEVICE_TYPE_STORAGE,
    DEVICE_TYPE_SENSOR,
    DEVICE_TYPE_BUS,
    DEVICE_TYPE_BRIDGE
} device_type_t;

// Device states
typedef enum {
    DEVICE_STATE_UNREGISTERED,
    DEVICE_STATE_REGISTERED,
    DEVICE_STATE_PROBING,
    DEVICE_STATE_PROBED,
    DEVICE_STATE_BOUND,
    DEVICE_STATE_RUNNING,
    DEVICE_STATE_SUSPENDED,
    DEVICE_STATE_ERROR
} device_state_t;

// Driver states
typedef enum {
    DRIVER_STATE_UNLOADED,
    DRIVER_STATE_LOADED,
    DRIVER_STATE_INITIALIZING,
    DRIVER_STATE_READY,
    DRIVER_STATE_RUNNING,
    DRIVER_STATE_ERROR
} driver_state_t;

// Device structure
typedef struct {
    u32 id;
    char name[DEVICE_NAME_MAX];
    device_type_t type;
    device_state_t state;
    u32 major;
    u32 minor;
    u32 vendor_id;
    u32 device_id;
    u32 revision;
    void* driver_data;
    void* private_data;
    struct device* parent;
    struct device* children;
    struct device* next_sibling;
    struct device* prev_sibling;
    struct driver* driver;
    u64 resources[8];
    u32 num_resources;
    bool active;
} device_t;

// Driver structure
typedef struct {
    u32 id;
    char name[DRIVER_NAME_MAX];
    char version[32];
    char description[128];
    driver_state_t state;
    device_type_t supported_types[8];
    u32 num_supported_types;
    u32 vendor_id;
    u32 device_id;
    u32 revision;
    u32 dependencies[MAX_DRIVER_DEPENDENCIES];
    u32 num_dependencies;
    void* driver_data;
    struct device* devices;
    u32 num_devices;
    bool active;
} driver_t;

// Device class structure
typedef struct {
    u32 id;
    char name[64];
    device_type_t type;
    driver_t* default_driver;
    bool active;
} device_class_t;

// Driver operations structure
typedef struct {
    error_t (*probe)(device_t* device);
    error_t (*remove)(device_t* device);
    error_t (*suspend)(device_t* device);
    error_t (*resume)(device_t* device);
    error_t (*shutdown)(device_t* device);
    error_t (*open)(device_t* device, int flags);
    error_t (*close)(device_t* device);
    ssize_t (*read)(device_t* device, void* buffer, size_t size, off_t offset);
    ssize_t (*write)(device_t* device, const void* buffer, size_t size, off_t offset);
    error_t (*ioctl)(device_t* device, u32 request, void* arg);
    error_t (*mmap)(device_t* device, void* addr, size_t length, int prot, int flags, off_t offset);
    error_t (*poll)(device_t* device, u32 events);
} driver_ops_t;

// Resource types
typedef enum {
    RESOURCE_TYPE_MEMORY,
    RESOURCE_TYPE_IO,
    RESOURCE_TYPE_IRQ,
    RESOURCE_TYPE_DMA,
    RESOURCE_TYPE_BUS
} resource_type_t;

// Resource structure
typedef struct {
    resource_type_t type;
    u64 start;
    u64 end;
    u64 flags;
    char name[32];
} resource_t;

// Global driver framework state
static driver_t drivers[MAX_DRIVERS];
static device_t devices[MAX_DEVICES];
static device_class_t device_classes[MAX_DEVICE_CLASSES];
static u32 num_drivers = 0;
static u32 num_devices = 0;
static u32 num_device_classes = 0;
static u32 next_driver_id = 1;
static u32 next_device_id = 1;
static u32 next_device_class_id = 1;
static bool driver_framework_initialized = false;

// Forward declarations
static error_t driver_register_internal(driver_t* driver, driver_ops_t* ops);
static error_t device_register_internal(device_t* device);
static error_t device_probe(device_t* device);
static error_t device_bind_driver(device_t* device, driver_t* driver);
static error_t device_unbind_driver(device_t* device);
static driver_t* driver_find_compatible(device_t* device);
static bool driver_supports_device(driver_t* driver, device_t* device);
static error_t resource_allocate(device_t* device, resource_type_t type, u64 size);
static void resource_free(device_t* device, resource_type_t type);

// Initialize driver framework
error_t driver_framework_init(void) {
    if (driver_framework_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing device driver framework");
    
    // Initialize driver table
    memset(drivers, 0, sizeof(drivers));
    for (u32 i = 0; i < MAX_DRIVERS; i++) {
        drivers[i].active = false;
    }
    
    // Initialize device table
    memset(devices, 0, sizeof(devices));
    for (u32 i = 0; i < MAX_DEVICES; i++) {
        devices[i].active = false;
    }
    
    // Initialize device classes
    memset(device_classes, 0, sizeof(device_classes));
    for (u32 i = 0; i < MAX_DEVICE_CLASSES; i++) {
        device_classes[i].active = false;
    }
    
    // Create default device classes
    device_class_t* char_class = &device_classes[0];
    char_class->id = next_device_class_id++;
    strcpy(char_class->name, "char");
    char_class->type = DEVICE_TYPE_CHAR;
    char_class->active = true;
    num_device_classes++;
    
    device_class_t* block_class = &device_classes[1];
    block_class->id = next_device_class_id++;
    strcpy(block_class->name, "block");
    block_class->type = DEVICE_TYPE_BLOCK;
    block_class->active = true;
    num_device_classes++;
    
    device_class_t* network_class = &device_classes[2];
    network_class->id = next_device_class_id++;
    strcpy(network_class->name, "network");
    network_class->type = DEVICE_TYPE_NETWORK;
    network_class->active = true;
    num_device_classes++;
    
    driver_framework_initialized = true;
    
    KINFO("Driver framework initialized with %u device classes", num_device_classes);
    return SUCCESS;
}

// Register a device driver
error_t driver_register(const char* name, const char* version, const char* description,
                       device_type_t* supported_types, u32 num_types, u32 vendor_id, 
                       u32 device_id, u32 revision, driver_ops_t* ops, driver_t** driver) {
    if (!driver_framework_initialized || !name || !ops || !driver) {
        return E_INVAL;
    }
    
    if (num_drivers >= MAX_DRIVERS) {
        return E_NOMEM;
    }
    
    KDEBUG("Registering driver: %s v%s", name, version);
    
    // Find free driver slot
    driver_t* drv = NULL;
    for (u32 i = 0; i < MAX_DRIVERS; i++) {
        if (!drivers[i].active) {
            drv = &drivers[i];
            break;
        }
    }
    
    if (!drv) {
        return E_NOMEM;
    }
    
    // Initialize driver
    memset(drv, 0, sizeof(driver_t));
    drv->id = next_driver_id++;
    strncpy(drv->name, name, sizeof(drv->name) - 1);
    strncpy(drv->version, version, sizeof(drv->version) - 1);
    strncpy(drv->description, description, sizeof(drv->description) - 1);
    drv->state = DRIVER_STATE_UNLOADED;
    drv->vendor_id = vendor_id;
    drv->device_id = device_id;
    drv->revision = revision;
    drv->num_devices = 0;
    drv->active = true;
    
    // Copy supported device types
    if (supported_types && num_types > 0) {
        u32 copy_count = (num_types > 8) ? 8 : num_types;
        memcpy(drv->supported_types, supported_types, copy_count * sizeof(device_type_t));
        drv->num_supported_types = copy_count;
    }
    
    // Register driver operations
    error_t result = driver_register_internal(drv, ops);
    if (result != SUCCESS) {
        drv->active = false;
        return result;
    }
    
    num_drivers++;
    *driver = drv;
    
    KINFO("Registered driver: %s (ID: %u)", name, drv->id);
    return SUCCESS;
}

// Internal driver registration
static error_t driver_register_internal(driver_t* driver, driver_ops_t* ops) {
    if (!driver || !ops) {
        return E_INVAL;
    }
    
    // Store driver operations
    driver->driver_data = ops;
    
    // Set driver state
    driver->state = DRIVER_STATE_LOADED;
    
    return SUCCESS;
}

// Unregister a driver
error_t driver_unregister(driver_t* driver) {
    if (!driver || !driver->active) {
        return E_INVAL;
    }
    
    KDEBUG("Unregistering driver: %s", driver->name);
    
    // Check if driver has bound devices
    if (driver->num_devices > 0) {
        KERROR("Cannot unregister driver with bound devices");
        return E_BUSY;
    }
    
    driver->active = false;
    driver->state = DRIVER_STATE_UNLOADED;
    num_drivers--;
    
    KINFO("Unregistered driver: %s", driver->name);
    return SUCCESS;
}

// Register a device
error_t device_register(const char* name, device_type_t type, u32 major, u32 minor,
                       u32 vendor_id, u32 device_id, u32 revision, device_t* parent,
                       device_t** device) {
    if (!driver_framework_initialized || !name || !device) {
        return E_INVAL;
    }
    
    if (num_devices >= MAX_DEVICES) {
        return E_NOMEM;
    }
    
    KDEBUG("Registering device: %s (%u:%u)", name, major, minor);
    
    // Find free device slot
    device_t* dev = NULL;
    for (u32 i = 0; i < MAX_DEVICES; i++) {
        if (!devices[i].active) {
            dev = &devices[i];
            break;
        }
    }
    
    if (!dev) {
        return E_NOMEM;
    }
    
    // Initialize device
    memset(dev, 0, sizeof(device_t));
    dev->id = next_device_id++;
    strncpy(dev->name, name, sizeof(dev->name) - 1);
    dev->type = type;
    dev->state = DEVICE_STATE_UNREGISTERED;
    dev->major = major;
    dev->minor = minor;
    dev->vendor_id = vendor_id;
    dev->device_id = device_id;
    dev->revision = revision;
    dev->parent = parent;
    dev->driver = NULL;
    dev->num_resources = 0;
    dev->active = true;
    
    // Add to parent's child list
    if (parent) {
        dev->next_sibling = parent->children;
        if (parent->children) {
            parent->children->prev_sibling = dev;
        }
        parent->children = dev;
    }
    
    // Register device
    error_t result = device_register_internal(dev);
    if (result != SUCCESS) {
        dev->active = false;
        return result;
    }
    
    num_devices++;
    *device = dev;
    
    KINFO("Registered device: %s (ID: %u)", name, dev->id);
    return SUCCESS;
}

// Internal device registration
static error_t device_register_internal(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    device->state = DEVICE_STATE_REGISTERED;
    
    // Try to find and bind a compatible driver
    error_t result = device_probe(device);
    if (result != SUCCESS) {
        KDEBUG("No compatible driver found for device: %s", device->name);
    }
    
    return SUCCESS;
}

// Probe device for compatible drivers
static error_t device_probe(device_t* device) {
    if (!device || device->state != DEVICE_STATE_REGISTERED) {
        return E_INVAL;
    }
    
    KDEBUG("Probing device: %s", device->name);
    
    device->state = DEVICE_STATE_PROBING;
    
    // Find compatible driver
    driver_t* driver = driver_find_compatible(device);
    if (!driver) {
        device->state = DEVICE_STATE_REGISTERED;
        return E_NOENT;
    }
    
    // Bind driver to device
    error_t result = device_bind_driver(device, driver);
    if (result != SUCCESS) {
        device->state = DEVICE_STATE_REGISTERED;
        return result;
    }
    
    device->state = DEVICE_STATE_PROBED;
    
    KDEBUG("Device %s bound to driver %s", device->name, driver->name);
    return SUCCESS;
}

// Find compatible driver for device
static driver_t* driver_find_compatible(device_t* device) {
    if (!device) {
        return NULL;
    }
    
    // Search for compatible driver
    for (u32 i = 0; i < num_drivers; i++) {
        driver_t* driver = &drivers[i];
        if (driver->active && driver->state >= DRIVER_STATE_LOADED) {
            if (driver_supports_device(driver, device)) {
                return driver;
            }
        }
    }
    
    return NULL;
}

// Check if driver supports device
static bool driver_supports_device(driver_t* driver, device_t* device) {
    if (!driver || !device) {
        return false;
    }
    
    // Check device type support
    bool type_supported = false;
    for (u32 i = 0; i < driver->num_supported_types; i++) {
        if (driver->supported_types[i] == device->type) {
            type_supported = true;
            break;
        }
    }
    
    if (!type_supported) {
        return false;
    }
    
    // Check vendor/device ID match
    if (driver->vendor_id != 0 && driver->vendor_id != device->vendor_id) {
        return false;
    }
    
    if (driver->device_id != 0 && driver->device_id != device->device_id) {
        return false;
    }
    
    return true;
}

// Bind driver to device
static error_t device_bind_driver(device_t* device, driver_t* driver) {
    if (!device || !driver) {
        return E_INVAL;
    }
    
    // Call driver probe function
    driver_ops_t* ops = (driver_ops_t*)driver->driver_data;
    if (ops && ops->probe) {
        error_t result = ops->probe(device);
        if (result != SUCCESS) {
            KERROR("Driver probe failed for device %s: %d", device->name, result);
            return result;
        }
    }
    
    // Bind driver to device
    device->driver = driver;
    device->state = DEVICE_STATE_BOUND;
    
    // Add device to driver's device list
    device->next_sibling = driver->devices;
    if (driver->devices) {
        driver->devices->prev_sibling = device;
    }
    driver->devices = device;
    driver->num_devices++;
    
    return SUCCESS;
}

// Unbind driver from device
static error_t device_unbind_driver(device_t* device) {
    if (!device || !device->driver) {
        return E_INVAL;
    }
    
    driver_t* driver = device->driver;
    driver_ops_t* ops = (driver_ops_t*)driver->driver_data;
    
    // Call driver remove function
    if (ops && ops->remove) {
        error_t result = ops->remove(device);
        if (result != SUCCESS) {
            KERROR("Driver remove failed for device %s: %d", device->name, result);
            return result;
        }
    }
    
    // Remove device from driver's device list
    if (device->prev_sibling) {
        device->prev_sibling->next_sibling = device->next_sibling;
    } else {
        driver->devices = device->next_sibling;
    }
    
    if (device->next_sibling) {
        device->next_sibling->prev_sibling = device->prev_sibling;
    }
    
    driver->num_devices--;
    device->driver = NULL;
    device->state = DEVICE_STATE_REGISTERED;
    
    return SUCCESS;
}

// Unregister a device
error_t device_unregister(device_t* device) {
    if (!device || !device->active) {
        return E_INVAL;
    }
    
    KDEBUG("Unregistering device: %s", device->name);
    
    // Unbind driver if bound
    if (device->driver) {
        device_unbind_driver(device);
    }
    
    // Remove from parent's child list
    if (device->parent) {
        if (device->prev_sibling) {
            device->prev_sibling->next_sibling = device->next_sibling;
        } else {
            device->parent->children = device->next_sibling;
        }
        
        if (device->next_sibling) {
            device->next_sibling->prev_sibling = device->prev_sibling;
        }
    }
    
    // Free resources
    for (u32 i = 0; i < device->num_resources; i++) {
        resource_free(device, (resource_type_t)i);
    }
    
    device->active = false;
    device->state = DEVICE_STATE_UNREGISTERED;
    num_devices--;
    
    KINFO("Unregistered device: %s", device->name);
    return SUCCESS;
}

// Open a device
error_t device_open(device_t* device, int flags) {
    if (!device || !device->driver) {
        return E_INVAL;
    }
    
    driver_ops_t* ops = (driver_ops_t*)device->driver->driver_data;
    if (!ops || !ops->open) {
        return E_NOSYS;
    }
    
    return ops->open(device, flags);
}

// Close a device
error_t device_close(device_t* device) {
    if (!device || !device->driver) {
        return E_INVAL;
    }
    
    driver_ops_t* ops = (driver_ops_t*)device->driver->driver_data;
    if (!ops || !ops->close) {
        return E_NOSYS;
    }
    
    return ops->close(device);
}

// Read from device
ssize_t device_read(device_t* device, void* buffer, size_t size, off_t offset) {
    if (!device || !device->driver || !buffer) {
        return -E_INVAL;
    }
    
    driver_ops_t* ops = (driver_ops_t*)device->driver->driver_data;
    if (!ops || !ops->read) {
        return -E_NOSYS;
    }
    
    return ops->read(device, buffer, size, offset);
}

// Write to device
ssize_t device_write(device_t* device, const void* buffer, size_t size, off_t offset) {
    if (!device || !device->driver || !buffer) {
        return -E_INVAL;
    }
    
    driver_ops_t* ops = (driver_ops_t*)device->driver->driver_data;
    if (!ops || !ops->write) {
        return -E_NOSYS;
    }
    
    return ops->write(device, buffer, size, offset);
}

// IOCTL on device
error_t device_ioctl(device_t* device, u32 request, void* arg) {
    if (!device || !device->driver) {
        return E_INVAL;
    }
    
    driver_ops_t* ops = (driver_ops_t*)device->driver->driver_data;
    if (!ops || !ops->ioctl) {
        return E_NOSYS;
    }
    
    return ops->ioctl(device, request, arg);
}

// Allocate resource for device
static error_t resource_allocate(device_t* device, resource_type_t type, u64 size) {
    if (!device || device->num_resources >= 8) {
        return E_INVAL;
    }
    
    // Simple resource allocation (in real implementation, this would be more sophisticated)
    u64 start = 0;
    u64 end = size - 1;
    
    switch (type) {
        case RESOURCE_TYPE_MEMORY:
            start = 0x1000000; // 16MB
            break;
        case RESOURCE_TYPE_IO:
            start = 0x1000; // 4KB
            break;
        case RESOURCE_TYPE_IRQ:
            start = 1;
            break;
        case RESOURCE_TYPE_DMA:
            start = 0;
            break;
        default:
            return E_INVAL;
    }
    
    device->resources[device->num_resources] = start;
    device->num_resources++;
    
    KDEBUG("Allocated resource for device %s: type=%d, start=0x%llx, size=0x%llx", 
           device->name, type, start, size);
    
    return SUCCESS;
}

// Free resource for device
static void resource_free(device_t* device, resource_type_t type) {
    if (!device) {
        return;
    }
    
    // Simple resource deallocation
    KDEBUG("Freed resource for device %s: type=%d", device->name, type);
}

// Get device by ID
device_t* device_get_by_id(u32 id) {
    for (u32 i = 0; i < num_devices; i++) {
        if (devices[i].active && devices[i].id == id) {
            return &devices[i];
        }
    }
    return NULL;
}

// Get device by major/minor numbers
device_t* device_get_by_numbers(u32 major, u32 minor) {
    for (u32 i = 0; i < num_devices; i++) {
        if (devices[i].active && devices[i].major == major && devices[i].minor == minor) {
            return &devices[i];
        }
    }
    return NULL;
}

// Get driver by ID
driver_t* driver_get_by_id(u32 id) {
    for (u32 i = 0; i < num_drivers; i++) {
        if (drivers[i].active && drivers[i].id == id) {
            return &drivers[i];
        }
    }
    return NULL;
}

// Get driver by name
driver_t* driver_get_by_name(const char* name) {
    for (u32 i = 0; i < num_drivers; i++) {
        if (drivers[i].active && strcmp(drivers[i].name, name) == 0) {
            return &drivers[i];
        }
    }
    return NULL;
}

// Dump driver framework information
void driver_framework_dump_info(void) {
    KINFO("=== Driver Framework Information ===");
    KINFO("Initialized: %s", driver_framework_initialized ? "Yes" : "No");
    KINFO("Drivers: %u", num_drivers);
    KINFO("Devices: %u", num_devices);
    KINFO("Device classes: %u", num_device_classes);
    
    for (u32 i = 0; i < num_drivers; i++) {
        if (drivers[i].active) {
            driver_t* drv = &drivers[i];
            KINFO("  Driver: %s v%s (ID: %u, State: %d, Devices: %u)", 
                  drv->name, drv->version, drv->id, drv->state, drv->num_devices);
        }
    }
    
    for (u32 i = 0; i < num_devices; i++) {
        if (devices[i].active) {
            device_t* dev = &devices[i];
            KINFO("  Device: %s (%u:%u, Type: %d, State: %d, Driver: %s)", 
                  dev->name, dev->major, dev->minor, dev->type, dev->state,
                  dev->driver ? dev->driver->name : "none");
        }
    }
    
    for (u32 i = 0; i < num_device_classes; i++) {
        if (device_classes[i].active) {
            device_class_t* cls = &device_classes[i];
            KINFO("  Device class: %s (ID: %u, Type: %d)", 
                  cls->name, cls->id, cls->type);
        }
    }
} 