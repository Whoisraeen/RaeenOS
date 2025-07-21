#include "drivers/include/driver_framework.h"
#include "kernel.h"
#include "memory/include/memory.h"
#include "hal/hal.h"
#include <string.h>

// Global device manager instance
static device_manager_t device_manager = {0};
device_manager_t* global_device_manager = &device_manager;

// Device statistics
static device_stats_t device_stats = {0};

// Forward declarations
static error_t device_manager_match_driver(device_t* device);
static void device_add_to_tree(device_t* device, device_t* parent);
static void device_remove_from_tree(device_t* device);

error_t device_manager_init(void) {
    if (device_manager.initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing Device Manager");
    
    // Initialize device manager structure
    memset(&device_manager, 0, sizeof(device_manager_t));
    memset(&device_stats, 0, sizeof(device_stats_t));
    
    device_manager.initialized = true;
    
    KINFO("Device Manager initialized successfully");
    return SUCCESS;
}

void device_manager_shutdown(void) {
    if (!device_manager.initialized) {
        return;
    }
    
    KINFO("Shutting down Device Manager");
    
    // Unregister all devices
    device_t* device = device_manager.device_list;
    while (device) {
        device_t* next = device->next;
        device_unregister(device);
        device = next;
    }
    
    // Unregister all drivers
    driver_t* driver = device_manager.driver_list;
    while (driver) {
        driver_t* next = driver->next;
        driver_unregister(driver);
        driver = next;
    }
    
    device_manager.initialized = false;
    KINFO("Device Manager shutdown complete");
}

device_t* device_create(const char* name, device_class_t class) {
    if (!name) {
        return NULL;
    }
    
    device_t* device = (device_t*)memory_alloc(sizeof(device_t));
    if (!device) {
        return NULL;
    }
    
    // Initialize device structure
    memset(device, 0, sizeof(device_t));
    
    strncpy(device->name, name, sizeof(device->name) - 1);
    device->class = class;
    device->state = DEVICE_STATE_UNKNOWN;
    device->power_state = POWER_STATE_D3_COLD;
    device->ref_count = 1;
    
    // Set default description
    snprintf(device->description, sizeof(device->description), 
             "%s device", name);
    
    KDEBUG("Created device: %s (class=%d)", name, class);
    return device;
}

error_t device_destroy(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    // Check if device is still registered
    if (device->next || device->prev || 
        device_manager.device_list == device) {
        KWARN("Destroying registered device: %s", device->name);
        device_unregister(device);
    }
    
    // Unbind driver if bound
    if (device->driver) {
        driver_unbind_device(device);
    }
    
    KDEBUG("Destroying device: %s", device->name);
    memory_free(device);
    
    return SUCCESS;
}

error_t device_register(device_t* device) {
    if (!device_manager.initialized || !device) {
        return E_INVAL;
    }
    
    // Check if device already registered
    if (device->next || device->prev || device_manager.device_list == device) {
        return E_ALREADY;
    }
    
    KINFO("Registering device: %s", device->name);
    
    // Add to device list
    if (device_manager.device_list) {
        device_manager.device_list->prev = device;
    }
    device->next = device_manager.device_list;
    device_manager.device_list = device;
    
    device_manager.device_count++;
    device_stats.total_devices++;
    
    // Set initial state
    device->state = DEVICE_STATE_INITIALIZING;
    
    // Try to match with a driver
    error_t result = device_manager_match_driver(device);
    if (result == SUCCESS) {
        device->state = DEVICE_STATE_ACTIVE;
        device_stats.active_devices++;
    } else {
        KWARN("No driver found for device: %s", device->name);
    }
    
    KINFO("Device registered: %s (driver=%s)", 
          device->name, 
          device->driver ? device->driver->name : "none");
    
    return SUCCESS;
}

error_t device_unregister(device_t* device) {
    if (!device_manager.initialized || !device) {
        return E_INVAL;
    }
    
    KINFO("Unregistering device: %s", device->name);
    
    // Unbind driver
    if (device->driver) {
        driver_unbind_device(device);
    }
    
    // Remove from device tree
    device_remove_from_tree(device);
    
    // Remove from device list
    if (device->prev) {
        device->prev->next = device->next;
    } else {
        device_manager.device_list = device->next;
    }
    
    if (device->next) {
        device->next->prev = device->prev;
    }
    
    device->next = NULL;
    device->prev = NULL;
    
    device_manager.device_count--;
    
    if (device->state == DEVICE_STATE_ACTIVE) {
        device_stats.active_devices--;
    }
    
    device->state = DEVICE_STATE_REMOVED;
    
    KINFO("Device unregistered: %s", device->name);
    return SUCCESS;
}

device_t* device_find_by_name(const char* name) {
    if (!device_manager.initialized || !name) {
        return NULL;
    }
    
    device_t* device = device_manager.device_list;
    while (device) {
        if (strcmp(device->name, name) == 0) {
            return device;
        }
        device = device->next;
    }
    
    return NULL;
}

device_t* device_find_by_class(device_class_t class) {
    if (!device_manager.initialized) {
        return NULL;
    }
    
    device_t* device = device_manager.device_list;
    while (device) {
        if (device->class == class) {
            return device;
        }
        device = device->next;
    }
    
    return NULL;
}

error_t driver_register(driver_t* driver) {
    if (!device_manager.initialized || !driver) {
        return E_INVAL;
    }
    
    // Check if driver already registered
    if (driver->next || driver->prev || device_manager.driver_list == driver) {
        return E_ALREADY;
    }
    
    KINFO("Registering driver: %s v%s", driver->name, driver->version);
    
    // Add to driver list
    if (device_manager.driver_list) {
        device_manager.driver_list->prev = driver;
    }
    driver->next = device_manager.driver_list;
    device_manager.driver_list = driver;
    
    device_manager.driver_count++;
    device_stats.total_drivers++;
    
    driver->load_time = hal_get_timestamp();
    
    // Try to bind to existing devices
    device_t* device = device_manager.device_list;
    while (device) {
        if (!device->driver && device->class == driver->class) {
            // Check ID table for specific device match
            for (int i = 0; i < 16 && driver->id_table[i].vendor_id != 0; i++) {
                if (driver->id_table[i].vendor_id == device->vendor_id &&
                    driver->id_table[i].device_id == device->device_id) {
                    
                    error_t result = driver_bind_device(driver, device);
                    if (result == SUCCESS) {
                        KINFO("Driver %s bound to device %s", 
                              driver->name, device->name);
                    }
                    break;
                }
            }
        }
        device = device->next;
    }
    
    KINFO("Driver registered: %s", driver->name);
    return SUCCESS;
}

error_t driver_unregister(driver_t* driver) {
    if (!device_manager.initialized || !driver) {
        return E_INVAL;
    }
    
    KINFO("Unregistering driver: %s", driver->name);
    
    // Unbind from all devices
    device_t* device = device_manager.device_list;
    while (device) {
        if (device->driver == driver) {
            driver_unbind_device(device);
        }
        device = device->next;
    }
    
    // Remove from driver list
    if (driver->prev) {
        driver->prev->next = driver->next;
    } else {
        device_manager.driver_list = driver->next;
    }
    
    if (driver->next) {
        driver->next->prev = driver->prev;
    }
    
    driver->next = NULL;
    driver->prev = NULL;
    
    device_manager.driver_count--;
    
    KINFO("Driver unregistered: %s", driver->name);
    return SUCCESS;
}

driver_t* driver_find_by_name(const char* name) {
    if (!device_manager.initialized || !name) {
        return NULL;
    }
    
    driver_t* driver = device_manager.driver_list;
    while (driver) {
        if (strcmp(driver->name, name) == 0) {
            return driver;
        }
        driver = driver->next;
    }
    
    return NULL;
}

error_t driver_bind_device(driver_t* driver, device_t* device) {
    if (!driver || !device) {
        return E_INVAL;
    }
    
    // Check if device already has a driver
    if (device->driver) {
        return E_DEVICE_BUSY;
    }
    
    KDEBUG("Binding driver %s to device %s", driver->name, device->name);
    
    // Set up binding
    device->driver = driver;
    device->ops = driver->ops;
    driver->device_count++;
    
    // Call driver probe function
    if (driver->ops && driver->ops->probe) {
        error_t result = driver->ops->probe(device);
        if (result != SUCCESS) {
            KWARN("Driver probe failed for %s: %d", device->name, result);
            
            // Unbind on probe failure
            device->driver = NULL;
            device->ops = NULL;
            driver->device_count--;
            
            return E_DEVICE_PROBE_FAILED;
        }
    }
    
    // Initialize device if probe succeeded
    if (driver->ops && driver->ops->init) {
        error_t result = driver->ops->init(device);
        if (result != SUCCESS) {
            KWARN("Device initialization failed for %s: %d", device->name, result);
            
            // Call remove if probe succeeded but init failed
            if (driver->ops->remove) {
                driver->ops->remove(device);
            }
            
            device->driver = NULL;
            device->ops = NULL;
            driver->device_count--;
            
            return result;
        }
    }
    
    device->state = DEVICE_STATE_ACTIVE;
    
    KINFO("Driver %s successfully bound to device %s", 
          driver->name, device->name);
    
    return SUCCESS;
}

error_t driver_unbind_device(device_t* device) {
    if (!device || !device->driver) {
        return E_INVAL;
    }
    
    driver_t* driver = device->driver;
    
    KDEBUG("Unbinding driver %s from device %s", driver->name, device->name);
    
    // Call driver remove function
    if (device->ops && device->ops->remove) {
        device->ops->remove(device);
    }
    
    // Clean up binding
    device->driver = NULL;
    device->ops = NULL;
    driver->device_count--;
    
    device->state = DEVICE_STATE_UNKNOWN;
    
    KINFO("Driver %s unbound from device %s", driver->name, device->name);
    return SUCCESS;
}

error_t device_init(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    if (device->ops && device->ops->init) {
        return device->ops->init(device);
    }
    
    return E_OPNOTSUPP;
}

error_t device_start(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    if (device->state != DEVICE_STATE_ACTIVE) {
        return E_DEVICE_NOT_READY;
    }
    
    device->power_state = POWER_STATE_D0;
    return SUCCESS;
}

error_t device_stop(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    device->power_state = POWER_STATE_D3_HOT;
    return SUCCESS;
}

error_t device_reset(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    if (device->ops && device->ops->reset) {
        return device->ops->reset(device);
    }
    
    return E_OPNOTSUPP;
}

error_t device_suspend(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    if (device->ops && device->ops->suspend) {
        error_t result = device->ops->suspend(device, POWER_STATE_D3_HOT);
        if (result == SUCCESS) {
            device->state = DEVICE_STATE_SUSPENDED;
        }
        return result;
    }
    
    device->state = DEVICE_STATE_SUSPENDED;
    return SUCCESS;
}

error_t device_resume(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    if (device->ops && device->ops->resume) {
        error_t result = device->ops->resume(device);
        if (result == SUCCESS) {
            device->state = DEVICE_STATE_ACTIVE;
        }
        return result;
    }
    
    device->state = DEVICE_STATE_ACTIVE;
    return SUCCESS;
}

// Resource management implementation
error_t device_request_irq(device_t* device, u32 irq, interrupt_handler_func_t handler) {
    if (!device || !handler) {
        return E_INVAL;
    }
    
    // TODO: Implement IRQ allocation and registration
    device->irq_line = irq;
    KDEBUG("IRQ %u requested for device %s", irq, device->name);
    
    return SUCCESS;
}

error_t device_free_irq(device_t* device, u32 irq) {
    if (!device) {
        return E_INVAL;
    }
    
    // TODO: Implement IRQ deallocation
    KDEBUG("IRQ %u freed for device %s", irq, device->name);
    
    return SUCCESS;
}

error_t device_request_memory_region(device_t* device, phys_addr_t base, size_t size) {
    if (!device) {
        return E_INVAL;
    }
    
    // TODO: Implement memory region allocation
    KDEBUG("Memory region 0x%lx-0x%lx requested for device %s", 
           base, base + size, device->name);
    
    return SUCCESS;
}

error_t device_release_memory_region(device_t* device, phys_addr_t base, size_t size) {
    if (!device) {
        return E_INVAL;
    }
    
    // TODO: Implement memory region deallocation
    KDEBUG("Memory region 0x%lx-0x%lx released for device %s", 
           base, base + size, device->name);
    
    return SUCCESS;
}

error_t device_request_io_region(device_t* device, u16 base, u16 size) {
    if (!device) {
        return E_INVAL;
    }
    
    // TODO: Implement I/O region allocation
    KDEBUG("I/O region 0x%x-0x%x requested for device %s", 
           base, base + size, device->name);
    
    return SUCCESS;
}

error_t device_release_io_region(device_t* device, u16 base, u16 size) {
    if (!device) {
        return E_INVAL;
    }
    
    // TODO: Implement I/O region deallocation
    KDEBUG("I/O region 0x%x-0x%x released for device %s", 
           base, base + size, device->name);
    
    return SUCCESS;
}

// DMA management implementation
error_t device_dma_alloc_coherent(device_t* device, size_t size, void** virt_addr, phys_addr_t* phys_addr) {
    if (!device || !virt_addr || !phys_addr) {
        return E_INVAL;
    }
    
    // TODO: Implement DMA coherent allocation
    *virt_addr = memory_alloc(size);
    if (!*virt_addr) {
        return E_NOMEM;
    }
    
    *phys_addr = memory_virt_to_phys(*virt_addr);
    device_stats.total_dma_allocations++;
    
    KDEBUG("DMA coherent memory allocated: virt=0x%p phys=0x%lx size=%zu", 
           *virt_addr, *phys_addr, size);
    
    return SUCCESS;
}

error_t device_dma_free_coherent(device_t* device, size_t size, void* virt_addr, phys_addr_t phys_addr) {
    if (!device || !virt_addr) {
        return E_INVAL;
    }
    
    // TODO: Implement DMA coherent deallocation
    memory_free(virt_addr);
    
    KDEBUG("DMA coherent memory freed: virt=0x%p phys=0x%lx size=%zu", 
           virt_addr, phys_addr, size);
    
    return SUCCESS;
}

error_t device_dma_map_single(device_t* device, void* ptr, size_t size, phys_addr_t* phys_addr) {
    if (!device || !ptr || !phys_addr) {
        return E_INVAL;
    }
    
    // TODO: Implement DMA mapping
    *phys_addr = memory_virt_to_phys(ptr);
    
    KDEBUG("DMA single mapping: virt=0x%p phys=0x%lx size=%zu", 
           ptr, *phys_addr, size);
    
    return SUCCESS;
}

error_t device_dma_unmap_single(device_t* device, phys_addr_t phys_addr, size_t size) {
    if (!device) {
        return E_INVAL;
    }
    
    // TODO: Implement DMA unmapping
    KDEBUG("DMA single unmapped: phys=0x%lx size=%zu", phys_addr, size);
    
    return SUCCESS;
}

// Hot-plug support
error_t device_hotplug_add(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    KINFO("Hot-plug device added: %s", device->name);
    return device_register(device);
}

error_t device_hotplug_remove(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    KINFO("Hot-plug device removed: %s", device->name);
    return device_unregister(device);
}

// Device enumeration
error_t device_enumerate_all(void) {
    if (!device_manager.initialized) {
        return E_INVAL;
    }
    
    KINFO("Enumerating all devices");
    
    // TODO: Implement device enumeration for different bus types
    // This would involve scanning PCI, USB, ACPI, etc.
    
    return SUCCESS;
}

error_t device_enumerate_class(device_class_t class) {
    if (!device_manager.initialized) {
        return E_INVAL;
    }
    
    KINFO("Enumerating devices of class %d", class);
    
    // TODO: Implement class-specific enumeration
    
    return SUCCESS;
}

void device_tree_dump(device_t* root, int depth) {
    if (!root) {
        root = device_manager.device_list;
        depth = 0;
    }
    
    device_t* device = root;
    while (device) {
        for (int i = 0; i < depth; i++) {
            hal_console_print("  ");
        }
        
        hal_console_print("Device: %s (class=%d, state=%d, driver=%s)\n",
                         device->name,
                         device->class,
                         device->state,
                         device->driver ? device->driver->name : "none");
        
        // Print children
        if (device->first_child) {
            device_tree_dump(device->first_child, depth + 1);
        }
        
        device = device->next_sibling ? device->next_sibling : device->next;
    }
}

// Statistics and debugging
device_stats_t* device_get_stats(void) {
    return &device_stats;
}

void device_dump_stats(void) {
    hal_console_print("Device Manager Statistics:\n");
    hal_console_print("  Total devices: %u\n", device_stats.total_devices);
    hal_console_print("  Active devices: %u\n", device_stats.active_devices);
    hal_console_print("  Failed devices: %u\n", device_stats.failed_devices);
    hal_console_print("  Total drivers: %u\n", device_stats.total_drivers);
    hal_console_print("  Total interrupts: %llu\n", device_stats.total_interrupts);
    hal_console_print("  Total DMA allocations: %llu\n", device_stats.total_dma_allocations);
}

void device_dump_tree(void) {
    hal_console_print("Device Tree:\n");
    device_tree_dump(NULL, 0);
}

void driver_dump_list(void) {
    hal_console_print("Registered Drivers:\n");
    
    driver_t* driver = device_manager.driver_list;
    while (driver) {
        hal_console_print("  Driver: %s v%s (class=%d, devices=%u)\n",
                         driver->name,
                         driver->version,
                         driver->class,
                         driver->device_count);
        driver = driver->next;
    }
}

// Helper functions
static error_t device_manager_match_driver(device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    driver_t* driver = device_manager.driver_list;
    while (driver) {
        // Check class match
        if (driver->class == device->class) {
            // Check ID table for specific device match
            for (int i = 0; i < 16 && driver->id_table[i].vendor_id != 0; i++) {
                if (driver->id_table[i].vendor_id == device->vendor_id &&
                    driver->id_table[i].device_id == device->device_id) {
                    
                    return driver_bind_device(driver, device);
                }
            }
        }
        driver = driver->next;
    }
    
    return E_DEVICE_NO_DRIVER;
}

static void device_add_to_tree(device_t* device, device_t* parent) {
    if (!device) {
        return;
    }
    
    device->parent = parent;
    
    if (parent) {
        device->next_sibling = parent->first_child;
        parent->first_child = device;
    }
}

static void device_remove_from_tree(device_t* device) {
    if (!device) {
        return;
    }
    
    // Remove from parent's child list
    if (device->parent) {
        if (device->parent->first_child == device) {
            device->parent->first_child = device->next_sibling;
        } else {
            device_t* sibling = device->parent->first_child;
            while (sibling && sibling->next_sibling != device) {
                sibling = sibling->next_sibling;
            }
            if (sibling) {
                sibling->next_sibling = device->next_sibling;
            }
        }
    }
    
    device->parent = NULL;
    device->next_sibling = NULL;
}