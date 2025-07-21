#include "pci.h"
#include "memory/memory.h"
#include <string.h>

// PCI driver state
static bool pci_initialized = false;
static pci_device_t* devices[MAX_PCI_DEVICES];
static int device_count = 0;
static pci_bus_t* buses[MAX_PCI_BUSES];
static int bus_count = 0;

// PCI configuration space access functions
static uint32_t pci_read_config_dword_internal(int bus, int device, int function, int offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    
    outl(0xCF8, address);
    return inl(0xCFC);
}

static void pci_write_config_dword_internal(int bus, int device, int function, int offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    
    outl(0xCF8, address);
    outl(0xCFC, value);
}

// Initialize PCI subsystem
error_t pci_init(void) {
    if (pci_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing PCI subsystem");
    
    // Clear arrays
    memset(devices, 0, sizeof(devices));
    memset(buses, 0, sizeof(buses));
    device_count = 0;
    bus_count = 0;
    
    // Scan for PCI devices
    error_t result = pci_scan_buses();
    if (result != SUCCESS) {
        KERROR("Failed to scan PCI buses");
        return result;
    }
    
    // Initialize found devices
    for (int i = 0; i < device_count; i++) {
        result = pci_device_init(devices[i]);
        if (result != SUCCESS) {
            KWARN("Failed to initialize PCI device %d", i);
        }
    }
    
    pci_initialized = true;
    
    KINFO("PCI subsystem initialized with %d devices on %d buses", device_count, bus_count);
    return SUCCESS;
}

// Shutdown PCI subsystem
void pci_shutdown(void) {
    if (!pci_initialized) {
        return;
    }
    
    KINFO("Shutting down PCI subsystem");
    
    // Shutdown all devices
    for (int i = 0; i < device_count; i++) {
        if (devices[i]) {
            pci_device_shutdown(devices[i]);
        }
    }
    
    // Free all buses
    for (int i = 0; i < bus_count; i++) {
        if (buses[i]) {
            memory_free(buses[i]);
        }
    }
    
    pci_initialized = false;
    
    KINFO("PCI subsystem shutdown complete");
}

// Scan PCI buses
error_t pci_scan_buses(void) {
    KDEBUG("Scanning PCI buses");
    
    // Scan primary bus (bus 0)
    error_t result = pci_scan_bus(0);
    if (result != SUCCESS) {
        return result;
    }
    
    // Check for PCI-to-PCI bridges and scan secondary buses
    for (int i = 0; i < device_count; i++) {
        if (devices[i] && devices[i]->class_code == 0x06 && devices[i]->subclass == 0x04) {
            // This is a PCI-to-PCI bridge
            uint32_t secondary_bus = (devices[i]->config_space[0x19] >> 8) & 0xFF;
            if (secondary_bus > 0 && secondary_bus < MAX_PCI_BUSES) {
                KDEBUG("Found PCI-to-PCI bridge, scanning secondary bus %d", secondary_bus);
                pci_scan_bus(secondary_bus);
            }
        }
    }
    
    return SUCCESS;
}

// Scan specific PCI bus
error_t pci_scan_bus(int bus) {
    KDEBUG("Scanning PCI bus %d", bus);
    
    // Create bus structure
    pci_bus_t* pci_bus = (pci_bus_t*)memory_alloc(sizeof(pci_bus_t));
    if (!pci_bus) {
        return E_NOMEM;
    }
    
    memset(pci_bus, 0, sizeof(pci_bus_t));
    pci_bus->bus_number = bus;
    buses[bus_count++] = pci_bus;
    
    // Scan all devices on this bus
    for (int device = 0; device < 32; device++) {
        for (int function = 0; function < 8; function++) {
            uint32_t vendor_id, device_id;
            
            if (pci_read_config_dword(bus, device, function, 0, &vendor_id) != SUCCESS) {
                continue;
            }
            
            device_id = vendor_id >> 16;
            vendor_id &= 0xFFFF;
            
            // Check if device exists
            if (vendor_id == 0xFFFF || vendor_id == 0) {
                continue;
            }
            
            KDEBUG("Found PCI device: bus=%d, device=%d, function=%d, vendor=0x%04x, device=0x%04x",
                   bus, device, function, vendor_id, device_id);
            
            // Create device structure
            pci_device_t* pci_device = pci_device_create(bus, device, function, vendor_id, device_id);
            if (pci_device) {
                devices[device_count++] = pci_device;
                pci_bus->devices[pci_bus->device_count++] = pci_device;
                
                if (device_count >= MAX_PCI_DEVICES) {
                    KWARN("Maximum number of PCI devices reached");
                    return SUCCESS;
                }
            }
        }
    }
    
    return SUCCESS;
}

// Create PCI device
pci_device_t* pci_device_create(int bus, int device, int function, uint16_t vendor_id, uint16_t device_id) {
    pci_device_t* pci_device = (pci_device_t*)memory_alloc(sizeof(pci_device_t));
    if (!pci_device) {
        return NULL;
    }
    
    memset(pci_device, 0, sizeof(pci_device_t));
    
    pci_device->bus = bus;
    pci_device->device = device;
    pci_device->function = function;
    pci_device->vendor_id = vendor_id;
    pci_device->device_id = device_id;
    
    // Read configuration space
    for (int i = 0; i < 64; i++) {
        uint32_t value;
        if (pci_read_config_dword(bus, device, function, i * 4, &value) == SUCCESS) {
            pci_device->config_space[i] = value;
        }
    }
    
    // Extract device information
    pci_device->revision_id = pci_device->config_space[0] & 0xFF;
    pci_device->class_code = (pci_device->config_space[2] >> 16) & 0xFF;
    pci_device->subclass = (pci_device->config_space[2] >> 8) & 0xFF;
    pci_device->programming_interface = pci_device->config_space[2] & 0xFF;
    pci_device->header_type = (pci_device->config_space[3] >> 16) & 0xFF;
    
    // Read BARs (Base Address Registers)
    for (int i = 0; i < 6; i++) {
        pci_device->bars[i] = pci_device->config_space[4 + i];
    }
    
    // Read interrupt information
    pci_device->interrupt_line = (pci_device->config_space[15] >> 8) & 0xFF;
    pci_device->interrupt_pin = (pci_device->config_space[15] >> 16) & 0xFF;
    
    return pci_device;
}

// Initialize PCI device
error_t pci_device_init(pci_device_t* device) {
    if (!device) {
        return E_INVAL;
    }
    
    KDEBUG("Initializing PCI device: vendor=0x%04x, device=0x%04x, class=0x%02x",
           device->vendor_id, device->device_id, device->class_code);
    
    // Determine device type and initialize appropriate driver
    switch (device->class_code) {
        case 0x01: // Mass storage controller
            return pci_storage_init(device);
            
        case 0x02: // Network controller
            return pci_network_init(device);
            
        case 0x03: // Display controller
            return pci_display_init(device);
            
        case 0x0C: // Serial bus controller (USB, etc.)
            return pci_serial_init(device);
            
        case 0x06: // Bridge device
            return pci_bridge_init(device);
            
        default:
            KDEBUG("Unknown PCI device class: 0x%02x", device->class_code);
            return SUCCESS;
    }
}

// Shutdown PCI device
void pci_device_shutdown(pci_device_t* device) {
    if (!device) {
        return;
    }
    
    KDEBUG("Shutting down PCI device: vendor=0x%04x, device=0x%04x",
           device->vendor_id, device->device_id);
    
    // Shutdown device-specific driver
    switch (device->class_code) {
        case 0x01: // Mass storage controller
            pci_storage_shutdown(device);
            break;
            
        case 0x02: // Network controller
            pci_network_shutdown(device);
            break;
            
        case 0x03: // Display controller
            pci_display_shutdown(device);
            break;
            
        case 0x0C: // Serial bus controller
            pci_serial_shutdown(device);
            break;
            
        case 0x06: // Bridge device
            pci_bridge_shutdown(device);
            break;
    }
    
    memory_free(device);
}

// PCI configuration space access functions
error_t pci_read_config_dword(int bus, int device, int function, int offset, uint32_t* value) {
    if (!value || offset < 0 || offset > 255 || (offset & 3) != 0) {
        return E_INVAL;
    }
    
    *value = pci_read_config_dword_internal(bus, device, function, offset);
    return SUCCESS;
}

error_t pci_write_config_dword(int bus, int device, int function, int offset, uint32_t value) {
    if (offset < 0 || offset > 255 || (offset & 3) != 0) {
        return E_INVAL;
    }
    
    pci_write_config_dword_internal(bus, device, function, offset, value);
    return SUCCESS;
}

error_t pci_read_config_word(int bus, int device, int function, int offset, uint16_t* value) {
    if (!value || offset < 0 || offset > 255 || (offset & 1) != 0) {
        return E_INVAL;
    }
    
    uint32_t dword = pci_read_config_dword_internal(bus, device, function, offset & ~3);
    *value = (dword >> ((offset & 2) * 8)) & 0xFFFF;
    return SUCCESS;
}

error_t pci_write_config_word(int bus, int device, int function, int offset, uint16_t value) {
    if (offset < 0 || offset > 255 || (offset & 1) != 0) {
        return E_INVAL;
    }
    
    uint32_t dword = pci_read_config_dword_internal(bus, device, function, offset & ~3);
    uint32_t mask = 0xFFFF << ((offset & 2) * 8);
    dword = (dword & ~mask) | (value << ((offset & 2) * 8));
    pci_write_config_dword_internal(bus, device, function, offset & ~3, dword);
    return SUCCESS;
}

error_t pci_read_config_byte(int bus, int device, int function, int offset, uint8_t* value) {
    if (!value || offset < 0 || offset > 255) {
        return E_INVAL;
    }
    
    uint32_t dword = pci_read_config_dword_internal(bus, device, function, offset & ~3);
    *value = (dword >> ((offset & 3) * 8)) & 0xFF;
    return SUCCESS;
}

error_t pci_write_config_byte(int bus, int device, int function, int offset, uint8_t value) {
    if (offset < 0 || offset > 255) {
        return E_INVAL;
    }
    
    uint32_t dword = pci_read_config_dword_internal(bus, device, function, offset & ~3);
    uint32_t mask = 0xFF << ((offset & 3) * 8);
    dword = (dword & ~mask) | (value << ((offset & 3) * 8));
    pci_write_config_dword_internal(bus, device, function, offset & ~3, dword);
    return SUCCESS;
}

// Device-specific initialization functions
error_t pci_storage_init(pci_device_t* device) {
    KDEBUG("Initializing PCI storage device");
    
    // TODO: Implement storage device initialization
    // This would involve:
    // 1. Reading device capabilities
    // 2. Setting up DMA
    // 3. Configuring interrupts
    // 4. Registering with storage subsystem
    
    return SUCCESS;
}

void pci_storage_shutdown(pci_device_t* device) {
    KDEBUG("Shutting down PCI storage device");
    
    // TODO: Implement storage device shutdown
}

error_t pci_network_init(pci_device_t* device) {
    KDEBUG("Initializing PCI network device");
    
    // TODO: Implement network device initialization
    
    return SUCCESS;
}

void pci_network_shutdown(pci_device_t* device) {
    KDEBUG("Shutting down PCI network device");
    
    // TODO: Implement network device shutdown
}

error_t pci_display_init(pci_device_t* device) {
    KDEBUG("Initializing PCI display device");
    
    // TODO: Implement display device initialization
    
    return SUCCESS;
}

void pci_display_shutdown(pci_device_t* device) {
    KDEBUG("Shutting down PCI display device");
    
    // TODO: Implement display device shutdown
}

error_t pci_serial_init(pci_device_t* device) {
    KDEBUG("Initializing PCI serial device");
    
    // TODO: Implement serial device initialization
    
    return SUCCESS;
}

void pci_serial_shutdown(pci_device_t* device) {
    KDEBUG("Shutting down PCI serial device");
    
    // TODO: Implement serial device shutdown
}

error_t pci_bridge_init(pci_device_t* device) {
    KDEBUG("Initializing PCI bridge device");
    
    // TODO: Implement bridge device initialization
    
    return SUCCESS;
}

void pci_bridge_shutdown(pci_device_t* device) {
    KDEBUG("Shutting down PCI bridge device");
    
    // TODO: Implement bridge device shutdown
}

// Device management functions
error_t pci_add_device(pci_device_t* device) {
    if (!device || device_count >= MAX_PCI_DEVICES) {
        return E_NOMEM;
    }
    
    devices[device_count++] = device;
    
    KDEBUG("Added PCI device: vendor=0x%04x, device=0x%04x",
           device->vendor_id, device->device_id);
    
    return SUCCESS;
}

error_t pci_remove_device(uint16_t vendor_id, uint16_t device_id) {
    for (int i = 0; i < device_count; i++) {
        if (devices[i] && devices[i]->vendor_id == vendor_id && devices[i]->device_id == device_id) {
            pci_device_shutdown(devices[i]);
            
            // Shift remaining devices
            for (int j = i; j < device_count - 1; j++) {
                devices[j] = devices[j + 1];
            }
            devices[device_count - 1] = NULL;
            device_count--;
            
            KDEBUG("Removed PCI device: vendor=0x%04x, device=0x%04x", vendor_id, device_id);
            return SUCCESS;
        }
    }
    
    return E_NOENT;
}

pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    for (int i = 0; i < device_count; i++) {
        if (devices[i] && devices[i]->vendor_id == vendor_id && devices[i]->device_id == device_id) {
            return devices[i];
        }
    }
    return NULL;
}

error_t pci_get_devices(pci_device_t** device_list, int* count) {
    if (!device_list || !count) {
        return E_INVAL;
    }
    
    *device_list = devices;
    *count = device_count;
    
    return SUCCESS;
}

// BAR (Base Address Register) functions
uintptr_t pci_get_bar_address(pci_device_t* device, int bar_index) {
    if (!device || bar_index < 0 || bar_index >= 6) {
        return 0;
    }
    
    uint32_t bar = device->bars[bar_index];
    
    // Check if it's memory-mapped
    if (bar & 1) {
        return 0; // I/O space, not supported yet
    }
    
    return bar & ~0xF; // Clear flags
}

size_t pci_get_bar_size(pci_device_t* device, int bar_index) {
    if (!device || bar_index < 0 || bar_index >= 6) {
        return 0;
    }
    
    uint32_t bar = device->bars[bar_index];
    
    // Check if it's memory-mapped
    if (bar & 1) {
        return 0; // I/O space, not supported yet
    }
    
    // Determine size by writing all 1s and reading back
    uint32_t original_bar = bar;
    pci_write_config_dword(device->bus, device->device, device->function, 0x10 + bar_index * 4, 0xFFFFFFFF);
    uint32_t size_bar = pci_read_config_dword_internal(device->bus, device->device, device->function, 0x10 + bar_index * 4);
    pci_write_config_dword(device->bus, device->device, device->function, 0x10 + bar_index * 4, original_bar);
    
    return ~(size_bar & ~0xF) + 1;
}

// Utility functions
bool pci_is_initialized(void) {
    return pci_initialized;
}

int pci_get_device_count(void) {
    return device_count;
}

int pci_get_bus_count(void) {
    return bus_count;
}

// I/O port access functions
static inline void outl(uint16_t port, uint32_t value) {
    asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
} 