#include <kernel/drivers/usb.h>
#include <kernel/drivers/device_manager.h>
#include <kernel/memory/memory.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/core/kernel.h>
#include <kernel/libc/string.h>
#include <kernel/libc/stdio.h>
#include <kernel/pci/pci.h>

// USB Host Controller Types
#define USB_HC_UHCI    0x00
#define USB_HC_OHCI    0x10
#define USB_HC_EHCI    0x20
#define USB_HC_XHCI    0x30

// USB Descriptor Types
#define USB_DESC_DEVICE        0x01
#define USB_DESC_CONFIGURATION 0x02
#define USB_DESC_STRING        0x03
#define USB_DESC_INTERFACE     0x04
#define USB_DESC_ENDPOINT      0x05
#define USB_DESC_DEVICE_QUALIFIER 0x06
#define USB_DESC_OTHER_SPEED_CONFIGURATION 0x07
#define USB_DESC_INTERFACE_POWER 0x08

// USB Device Classes
#define USB_CLASS_MASS_STORAGE 0x08
#define USB_CLASS_HID          0x03
#define USB_CLASS_HUB          0x09

// USB Mass Storage Subclasses
#define USB_MASS_STORAGE_SCSI  0x06
#define USB_MASS_STORAGE_RBC   0x01
#define USB_MASS_STORAGE_ATAPI 0x02

// USB Mass Storage Protocols
#define USB_MASS_STORAGE_BULK_ONLY 0x50
#define USB_MASS_STORAGE_CBI       0x00
#define USB_MASS_STORAGE_CBI_WITH_INTERRUPT 0x01

// USB Request Types
#define USB_REQ_DIR_OUT        0x00
#define USB_REQ_DIR_IN         0x80
#define USB_REQ_TYPE_STANDARD  0x00
#define USB_REQ_TYPE_CLASS     0x20
#define USB_REQ_TYPE_VENDOR    0x40

// USB Standard Requests
#define USB_REQ_GET_STATUS     0x00
#define USB_REQ_CLEAR_FEATURE  0x01
#define USB_REQ_SET_FEATURE    0x03
#define USB_REQ_SET_ADDRESS    0x05
#define USB_REQ_GET_DESCRIPTOR 0x06
#define USB_REQ_SET_DESCRIPTOR 0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_GET_INTERFACE  0x0A
#define USB_REQ_SET_INTERFACE  0x0B
#define USB_REQ_SYNCH_FRAME    0x0C

// USB Mass Storage Requests
#define USB_MASS_STORAGE_BULK_RESET 0xFF
#define USB_MASS_STORAGE_GET_MAX_LUN 0xFE

// USB Device Descriptor
typedef struct {
    uint8_t  length;
    uint8_t  descriptor_type;
    uint16_t usb_version;
    uint8_t  device_class;
    uint8_t  device_subclass;
    uint8_t  device_protocol;
    uint8_t  max_packet_size;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_version;
    uint8_t  manufacturer_string;
    uint8_t  product_string;
    uint8_t  serial_number_string;
    uint8_t  num_configurations;
} __attribute__((packed)) usb_device_descriptor_t;

// USB Configuration Descriptor
typedef struct {
    uint8_t  length;
    uint8_t  descriptor_type;
    uint16_t total_length;
    uint8_t  num_interfaces;
    uint8_t  configuration_value;
    uint8_t  configuration_string;
    uint8_t  attributes;
    uint8_t  max_power;
} __attribute__((packed)) usb_configuration_descriptor_t;

// USB Interface Descriptor
typedef struct {
    uint8_t  length;
    uint8_t  descriptor_type;
    uint8_t  interface_number;
    uint8_t  alternate_setting;
    uint8_t  num_endpoints;
    uint8_t  interface_class;
    uint8_t  interface_subclass;
    uint8_t  interface_protocol;
    uint8_t  interface_string;
} __attribute__((packed)) usb_interface_descriptor_t;

// USB Endpoint Descriptor
typedef struct {
    uint8_t  length;
    uint8_t  descriptor_type;
    uint8_t  endpoint_address;
    uint8_t  attributes;
    uint16_t max_packet_size;
    uint8_t  interval;
} __attribute__((packed)) usb_endpoint_descriptor_t;

// USB Setup Packet
typedef struct {
    uint8_t  request_type;
    uint8_t  request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed)) usb_setup_packet_t;

// USB Mass Storage Command Block Wrapper
typedef struct {
    uint32_t signature;
    uint32_t tag;
    uint32_t data_transfer_length;
    uint8_t  flags;
    uint8_t  lun;
    uint8_t  command_block_length;
    uint8_t  command_block[16];
} __attribute__((packed)) usb_ms_cbw_t;

// USB Mass Storage Command Status Wrapper
typedef struct {
    uint32_t signature;
    uint32_t tag;
    uint32_t data_residue;
    uint8_t  status;
} __attribute__((packed)) usb_ms_csw_t;

// USB Device Structure
typedef struct {
    uint8_t  address;
    uint8_t  device_class;
    uint8_t  device_subclass;
    uint8_t  device_protocol;
    uint16_t vendor_id;
    uint16_t product_id;
    uint8_t  max_packet_size;
    uint8_t  num_configurations;
    uint8_t  current_configuration;
    uint8_t  num_interfaces;
    usb_interface_descriptor_t interfaces[8];
    uint8_t  endpoints[8];
    uint8_t  hub_port;
    uint8_t  hub_address;
    char     manufacturer[64];
    char     product[64];
    char     serial[64];
} usb_device_t;

// USB Host Controller Structure
typedef struct {
    uint32_t base_addr;
    uint8_t  type;
    uint8_t  num_ports;
    uint8_t  initialized;
    usb_device_t devices[127];
    uint8_t  num_devices;
} usb_host_controller_t;

// Global USB host controllers
static usb_host_controller_t usb_controllers[4];
static int usb_controller_count = 0;

// Function prototypes
static int usb_find_controllers();
static int usb_init_controller(int index);
static int usb_enumerate_device(int controller, int port);
static int usb_get_descriptor(int controller, uint8_t address, uint8_t type, uint8_t index, void* buffer, uint16_t length);
static int usb_set_address(int controller, uint8_t address);
static int usb_set_configuration(int controller, uint8_t address, uint8_t configuration);
static int usb_bulk_transfer(int controller, uint8_t address, uint8_t endpoint, void* buffer, uint16_t length, int direction);
static int usb_mass_storage_init(int controller, int device_index);
static int usb_mass_storage_read_sectors(int device_index, uint32_t lba, uint32_t count, void* buffer);
static int usb_mass_storage_write_sectors(int device_index, uint32_t lba, uint32_t count, const void* buffer);
static void usb_irq_handler(struct interrupt_frame* frame);

// Initialize USB subsystem
int usb_init() {
    printf("USB: Initializing USB subsystem...\n");
    
    // Find USB controllers
    if (!usb_find_controllers()) {
        printf("USB: No USB controllers found\n");
        return -1;
    }
    
    // Initialize controllers
    for (int i = 0; i < usb_controller_count; i++) {
        if (usb_init_controller(i) == 0) {
            printf("USB: Controller %d initialized\n", i);
            
            // Enumerate devices on each port
            for (int port = 0; port < usb_controllers[i].num_ports; port++) {
                if (usb_enumerate_device(i, port) == 0) {
                    printf("USB: Device found on controller %d, port %d\n", i, port);
                }
            }
        }
    }
    
    // Register IRQ handlers
    register_irq_handler(11, usb_irq_handler); // USB IRQ
    
    printf("USB: Initialization complete, %d controllers, %d total devices\n", 
           usb_controller_count, usb_controllers[0].num_devices + usb_controllers[1].num_devices);
    
    return 0;
}

// Find USB controllers via PCI
static int usb_find_controllers() {
    usb_controller_count = 0;
    
    // Search for USB controllers in PCI space
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int function = 0; function < 8; function++) {
                uint32_t vendor_id = pci_read_config_dword(bus, device, function, 0);
                uint16_t vendor = vendor_id & 0xFFFF;
                uint16_t device_id = (vendor_id >> 16) & 0xFFFF;
                
                if (vendor == 0x8086) { // Intel
                    // UHCI
                    if (device_id >= 0x7020 && device_id <= 0x702F) {
                        usb_controllers[usb_controller_count].base_addr = pci_read_config_dword(bus, device, function, 0x20) & 0xFFFFFFF0;
                        usb_controllers[usb_controller_count].type = USB_HC_UHCI;
                        usb_controllers[usb_controller_count].num_ports = 2;
                        usb_controller_count++;
                        printf("USB: Found UHCI controller at 0x%x\n", usb_controllers[usb_controller_count-1].base_addr);
                    }
                    // OHCI
                    else if (device_id >= 0x7030 && device_id <= 0x703F) {
                        usb_controllers[usb_controller_count].base_addr = pci_read_config_dword(bus, device, function, 0x20) & 0xFFFFFFF0;
                        usb_controllers[usb_controller_count].type = USB_HC_OHCI;
                        usb_controllers[usb_controller_count].num_ports = 2;
                        usb_controller_count++;
                        printf("USB: Found OHCI controller at 0x%x\n", usb_controllers[usb_controller_count-1].base_addr);
                    }
                    // EHCI
                    else if (device_id >= 0x3A34 && device_id <= 0x3A3F) {
                        usb_controllers[usb_controller_count].base_addr = pci_read_config_dword(bus, device, function, 0x20) & 0xFFFFFFF0;
                        usb_controllers[usb_controller_count].type = USB_HC_EHCI;
                        usb_controllers[usb_controller_count].num_ports = 6;
                        usb_controller_count++;
                        printf("USB: Found EHCI controller at 0x%x\n", usb_controllers[usb_controller_count-1].base_addr);
                    }
                    // xHCI
                    else if (device_id >= 0x1E31 && device_id <= 0x1E3F) {
                        usb_controllers[usb_controller_count].base_addr = pci_read_config_dword(bus, device, function, 0x20) & 0xFFFFFFF0;
                        usb_controllers[usb_controller_count].type = USB_HC_XHCI;
                        usb_controllers[usb_controller_count].num_ports = 8;
                        usb_controller_count++;
                        printf("USB: Found xHCI controller at 0x%x\n", usb_controllers[usb_controller_count-1].base_addr);
                    }
                }
            }
        }
    }
    
    return usb_controller_count > 0;
}

// Initialize USB host controller
static int usb_init_controller(int index) {
    usb_host_controller_t* hc = &usb_controllers[index];
    
    // Initialize controller based on type
    switch (hc->type) {
        case USB_HC_UHCI:
            // Initialize UHCI controller
            outw(hc->base_addr + 0x00, 0x0000); // Disable controller
            outw(hc->base_addr + 0x02, 0x0000); // Clear status
            outw(hc->base_addr + 0x04, 0x0000); // Clear interrupt enable
            outw(hc->base_addr + 0x06, 0x0000); // Clear frame number
            outw(hc->base_addr + 0x08, 0x0000); // Clear frame base address
            outw(hc->base_addr + 0x0A, 0x0000); // Clear start of frame
            outw(hc->base_addr + 0x0C, 0x0000); // Clear port status
            outw(hc->base_addr + 0x0E, 0x0000); // Clear port status
            outw(hc->base_addr + 0x00, 0x0001); // Enable controller
            break;
            
        case USB_HC_OHCI:
            // Initialize OHCI controller
            mmio_write32(hc->base_addr + 0x00, 0x00000000); // Disable controller
            mmio_write32(hc->base_addr + 0x04, 0x00000000); // Clear status
            mmio_write32(hc->base_addr + 0x08, 0x00000000); // Clear interrupt enable
            mmio_write32(hc->base_addr + 0x0C, 0x00000000); // Clear frame number
            mmio_write32(hc->base_addr + 0x10, 0x00000000); // Clear frame base address
            mmio_write32(hc->base_addr + 0x14, 0x00000000); // Clear start of frame
            mmio_write32(hc->base_addr + 0x18, 0x00000000); // Clear port status
            mmio_write32(hc->base_addr + 0x1C, 0x00000000); // Clear port status
            mmio_write32(hc->base_addr + 0x00, 0x00000001); // Enable controller
            break;
            
        case USB_HC_EHCI:
            // Initialize EHCI controller
            mmio_write32(hc->base_addr + 0x00, 0x00000000); // Disable controller
            mmio_write32(hc->base_addr + 0x04, 0x00000000); // Clear status
            mmio_write32(hc->base_addr + 0x08, 0x00000000); // Clear interrupt enable
            mmio_write32(hc->base_addr + 0x0C, 0x00000000); // Clear frame number
            mmio_write32(hc->base_addr + 0x10, 0x00000000); // Clear frame base address
            mmio_write32(hc->base_addr + 0x14, 0x00000000); // Clear start of frame
            mmio_write32(hc->base_addr + 0x18, 0x00000000); // Clear port status
            mmio_write32(hc->base_addr + 0x1C, 0x00000000); // Clear port status
            mmio_write32(hc->base_addr + 0x00, 0x00000001); // Enable controller
            break;
            
        case USB_HC_XHCI:
            // Initialize xHCI controller
            mmio_write32(hc->base_addr + 0x00, 0x00000000); // Disable controller
            mmio_write32(hc->base_addr + 0x04, 0x00000000); // Clear status
            mmio_write32(hc->base_addr + 0x08, 0x00000000); // Clear interrupt enable
            mmio_write32(hc->base_addr + 0x0C, 0x00000000); // Clear frame number
            mmio_write32(hc->base_addr + 0x10, 0x00000000); // Clear frame base address
            mmio_write32(hc->base_addr + 0x14, 0x00000000); // Clear start of frame
            mmio_write32(hc->base_addr + 0x18, 0x00000000); // Clear port status
            mmio_write32(hc->base_addr + 0x1C, 0x00000000); // Clear port status
            mmio_write32(hc->base_addr + 0x00, 0x00000001); // Enable controller
            break;
    }
    
    hc->initialized = 1;
    hc->num_devices = 0;
    
    return 0;
}

// Enumerate USB device
static int usb_enumerate_device(int controller, int port) {
    usb_host_controller_t* hc = &usb_controllers[controller];
    
    // Check if device is present
    uint32_t port_status = 0;
    switch (hc->type) {
        case USB_HC_UHCI:
            port_status = inw(hc->base_addr + 0x10 + (port * 2));
            break;
        case USB_HC_OHCI:
            port_status = mmio_read32(hc->base_addr + 0x54 + (port * 4));
            break;
        case USB_HC_EHCI:
            port_status = mmio_read32(hc->base_addr + 0x44 + (port * 4));
            break;
        case USB_HC_XHCI:
            port_status = mmio_read32(hc->base_addr + 0x480 + (port * 4));
            break;
    }
    
    if (!(port_status & 0x00000001)) { // No device connected
        return -1;
    }
    
    // Allocate device address
    uint8_t address = hc->num_devices + 1;
    if (address > 127) {
        return -1; // Too many devices
    }
    
    usb_device_t* device = &hc->devices[hc->num_devices];
    memset(device, 0, sizeof(usb_device_t));
    device->address = address;
    device->hub_port = port;
    device->hub_address = 0; // Root hub
    
    // Set device address
    if (usb_set_address(controller, address) != 0) {
        return -1;
    }
    
    // Get device descriptor
    usb_device_descriptor_t dev_desc;
    if (usb_get_descriptor(controller, address, USB_DESC_DEVICE, 0, &dev_desc, sizeof(dev_desc)) != 0) {
        return -1;
    }
    
    // Fill device information
    device->device_class = dev_desc.device_class;
    device->device_subclass = dev_desc.device_subclass;
    device->device_protocol = dev_desc.device_protocol;
    device->vendor_id = dev_desc.vendor_id;
    device->product_id = dev_desc.product_id;
    device->max_packet_size = dev_desc.max_packet_size;
    device->num_configurations = dev_desc.num_configurations;
    
    // Get manufacturer string
    if (dev_desc.manufacturer_string > 0) {
        char manufacturer[64];
        if (usb_get_descriptor(controller, address, USB_DESC_STRING, dev_desc.manufacturer_string, manufacturer, sizeof(manufacturer)) == 0) {
            strncpy(device->manufacturer, manufacturer, sizeof(device->manufacturer) - 1);
        }
    }
    
    // Get product string
    if (dev_desc.product_string > 0) {
        char product[64];
        if (usb_get_descriptor(controller, address, USB_DESC_STRING, dev_desc.product_string, product, sizeof(product)) == 0) {
            strncpy(device->product, product, sizeof(device->product) - 1);
        }
    }
    
    // Get serial string
    if (dev_desc.serial_number_string > 0) {
        char serial[64];
        if (usb_get_descriptor(controller, address, USB_DESC_STRING, dev_desc.serial_number_string, serial, sizeof(serial)) == 0) {
            strncpy(device->serial, serial, sizeof(device->serial) - 1);
        }
    }
    
    // Get configuration descriptor
    uint8_t config_buffer[512];
    if (usb_get_descriptor(controller, address, USB_DESC_CONFIGURATION, 0, config_buffer, sizeof(config_buffer)) == 0) {
        usb_configuration_descriptor_t* config_desc = (usb_configuration_descriptor_t*)config_buffer;
        device->num_interfaces = config_desc->num_interfaces;
        
        // Parse interface descriptors
        uint8_t* ptr = config_buffer + config_desc->length;
        for (int i = 0; i < device->num_interfaces && i < 8; i++) {
            if (ptr[1] == USB_DESC_INTERFACE) {
                usb_interface_descriptor_t* iface_desc = (usb_interface_descriptor_t*)ptr;
                device->interfaces[i] = *iface_desc;
                ptr += iface_desc->length;
                
                // Parse endpoint descriptors
                for (int j = 0; j < iface_desc->num_endpoints && j < 8; j++) {
                    if (ptr[1] == USB_DESC_ENDPOINT) {
                        usb_endpoint_descriptor_t* ep_desc = (usb_endpoint_descriptor_t*)ptr;
                        device->endpoints[j] = ep_desc->endpoint_address;
                        ptr += ep_desc->length;
                    }
                }
            }
        }
    }
    
    // Set configuration
    if (usb_set_configuration(controller, address, 1) != 0) {
        return -1;
    }
    device->current_configuration = 1;
    
    // Initialize device based on class
    if (device->device_class == USB_CLASS_MASS_STORAGE) {
        if (usb_mass_storage_init(controller, hc->num_devices) == 0) {
            printf("USB: Mass storage device initialized: %s %s\n", device->manufacturer, device->product);
        }
    }
    
    hc->num_devices++;
    printf("USB: Device enumerated: %s %s (VID:0x%04X, PID:0x%04X)\n", 
           device->manufacturer, device->product, device->vendor_id, device->product_id);
    
    return 0;
}

// Get USB descriptor
static int usb_get_descriptor(int controller, uint8_t address, uint8_t type, uint8_t index, void* buffer, uint16_t length) {
    usb_setup_packet_t setup;
    setup.request_type = USB_REQ_DIR_IN | USB_REQ_TYPE_STANDARD;
    setup.request = USB_REQ_GET_DESCRIPTOR;
    setup.value = (type << 8) | index;
    setup.index = 0;
    setup.length = length;
    
    // Send setup packet
    // This is a simplified implementation - real USB would need proper transaction handling
    return 0;
}

// Set USB device address
static int usb_set_address(int controller, uint8_t address) {
    usb_setup_packet_t setup;
    setup.request_type = USB_REQ_DIR_OUT | USB_REQ_TYPE_STANDARD;
    setup.request = USB_REQ_SET_ADDRESS;
    setup.value = address;
    setup.index = 0;
    setup.length = 0;
    
    // Send setup packet
    // This is a simplified implementation - real USB would need proper transaction handling
    return 0;
}

// Set USB configuration
static int usb_set_configuration(int controller, uint8_t address, uint8_t configuration) {
    usb_setup_packet_t setup;
    setup.request_type = USB_REQ_DIR_OUT | USB_REQ_TYPE_STANDARD;
    setup.request = USB_REQ_SET_CONFIGURATION;
    setup.value = configuration;
    setup.index = 0;
    setup.length = 0;
    
    // Send setup packet
    // This is a simplified implementation - real USB would need proper transaction handling
    return 0;
}

// USB bulk transfer
static int usb_bulk_transfer(int controller, uint8_t address, uint8_t endpoint, void* buffer, uint16_t length, int direction) {
    // This is a simplified implementation - real USB would need proper transaction handling
    return 0;
}

// Initialize USB mass storage device
static int usb_mass_storage_init(int controller, int device_index) {
    usb_device_t* device = &usb_controllers[controller].devices[device_index];
    
    // Find bulk endpoints
    uint8_t bulk_in_ep = 0;
    uint8_t bulk_out_ep = 0;
    
    for (int i = 0; i < 8; i++) {
        if (device->endpoints[i] != 0) {
            if ((device->endpoints[i] & 0x80) && (device->endpoints[i] & 0x02)) {
                bulk_in_ep = device->endpoints[i];
            } else if (!(device->endpoints[i] & 0x80) && (device->endpoints[i] & 0x02)) {
                bulk_out_ep = device->endpoints[i];
            }
        }
    }
    
    if (!bulk_in_ep || !bulk_out_ep) {
        return -1;
    }
    
    // Reset bulk-only mass storage device
    usb_setup_packet_t setup;
    setup.request_type = USB_REQ_DIR_OUT | USB_REQ_TYPE_CLASS;
    setup.request = USB_MASS_STORAGE_BULK_RESET;
    setup.value = 0;
    setup.index = 0;
    setup.length = 0;
    
    // Send setup packet
    // This is a simplified implementation
    
    return 0;
}

// Read sectors from USB mass storage device
static int usb_mass_storage_read_sectors(int device_index, uint32_t lba, uint32_t count, void* buffer) {
    // This is a simplified implementation - real USB mass storage would need SCSI commands
    return 0;
}

// Write sectors to USB mass storage device
static int usb_mass_storage_write_sectors(int device_index, uint32_t lba, uint32_t count, const void* buffer) {
    // This is a simplified implementation - real USB mass storage would need SCSI commands
    return 0;
}

// USB interrupt handler
static void usb_irq_handler(struct interrupt_frame* frame) {
    // Handle USB interrupts
    // This is a simplified implementation - real USB would need proper interrupt handling
    
    // Acknowledge interrupt
    outb(0x20, 0x20);
}

// Get USB device information
usb_device_t* usb_get_device(int controller, int device_index) {
    if (controller >= 0 && controller < usb_controller_count &&
        device_index >= 0 && device_index < usb_controllers[controller].num_devices) {
        return &usb_controllers[controller].devices[device_index];
    }
    return NULL;
}

// Get number of USB devices
int usb_get_device_count(int controller) {
    if (controller >= 0 && controller < usb_controller_count) {
        return usb_controllers[controller].num_devices;
    }
    return 0;
} 