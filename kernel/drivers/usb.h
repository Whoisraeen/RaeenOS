#ifndef RAEENOS_USB_H
#define RAEENOS_USB_H

#include "types.h"

// USB constants
#define MAX_USB_CONTROLLERS 8
#define MAX_USB_DEVICES 128
#define MAX_USB_ENDPOINTS 16
#define MAX_USB_INTERFACES 8
#define MAX_USB_CONFIGURATIONS 8

// USB speeds
typedef enum {
    USB_SPEED_LOW = 0,      // 1.5 Mbps
    USB_SPEED_FULL = 1,     // 12 Mbps
    USB_SPEED_HIGH = 2,     // 480 Mbps
    USB_SPEED_SUPER = 3,    // 5 Gbps
    USB_SPEED_SUPER_PLUS = 4 // 10 Gbps
} usb_speed_t;

// USB controller types
typedef enum {
    USB_CONTROLLER_UHCI = 0, // USB 1.1
    USB_CONTROLLER_EHCI = 1, // USB 2.0
    USB_CONTROLLER_XHCI = 2  // USB 3.0
} usb_controller_type_t;

// USB device states
typedef enum {
    USB_DEVICE_DEFAULT = 0,
    USB_DEVICE_ADDRESSED = 1,
    USB_DEVICE_CONFIGURED = 2,
    USB_DEVICE_SUSPENDED = 3
} usb_device_state_t;

// USB request types
#define USB_REQUEST_TYPE_STANDARD 0x00
#define USB_REQUEST_TYPE_CLASS    0x20
#define USB_REQUEST_TYPE_VENDOR   0x40
#define USB_REQUEST_TYPE_RESERVED 0x60

#define USB_REQUEST_DIR_OUT       0x00
#define USB_REQUEST_DIR_IN        0x80

// USB standard requests
#define USB_GET_STATUS            0x00
#define USB_CLEAR_FEATURE         0x01
#define USB_SET_FEATURE           0x03
#define USB_SET_ADDRESS           0x05
#define USB_GET_DESCRIPTOR        0x06
#define USB_SET_DESCRIPTOR        0x07
#define USB_GET_CONFIGURATION     0x08
#define USB_SET_CONFIGURATION     0x09
#define USB_GET_INTERFACE         0x0A
#define USB_SET_INTERFACE         0x0B
#define USB_SYNCH_FRAME           0x0C

// USB descriptor types
#define USB_DESCRIPTOR_DEVICE             0x01
#define USB_DESCRIPTOR_CONFIGURATION      0x02
#define USB_DESCRIPTOR_STRING             0x03
#define USB_DESCRIPTOR_INTERFACE          0x04
#define USB_DESCRIPTOR_ENDPOINT           0x05
#define USB_DESCRIPTOR_DEVICE_QUALIFIER   0x06
#define USB_DESCRIPTOR_OTHER_SPEED_CONFIG 0x07
#define USB_DESCRIPTOR_INTERFACE_POWER    0x08

// USB setup packet
typedef struct {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed)) usb_setup_packet_t;

// USB device descriptor
typedef struct {
    uint8_t length;
    uint8_t descriptor_type;
    uint16_t bcd_usb;
    uint8_t device_class;
    uint8_t device_subclass;
    uint8_t device_protocol;
    uint8_t max_packet_size;
    uint16_t id_vendor;
    uint16_t id_product;
    uint16_t bcd_device;
    uint8_t manufacturer_string;
    uint8_t product_string;
    uint8_t serial_number_string;
    uint8_t num_configurations;
} __attribute__((packed)) usb_device_descriptor_t;

// USB configuration descriptor
typedef struct {
    uint8_t length;
    uint8_t descriptor_type;
    uint16_t total_length;
    uint8_t num_interfaces;
    uint8_t configuration_value;
    uint8_t configuration_string;
    uint8_t attributes;
    uint8_t max_power;
} __attribute__((packed)) usb_config_descriptor_t;

// USB interface descriptor
typedef struct {
    uint8_t length;
    uint8_t descriptor_type;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t num_endpoints;
    uint8_t interface_class;
    uint8_t interface_subclass;
    uint8_t interface_protocol;
    uint8_t interface_string;
} __attribute__((packed)) usb_interface_descriptor_t;

// USB endpoint descriptor
typedef struct {
    uint8_t length;
    uint8_t descriptor_type;
    uint8_t endpoint_address;
    uint8_t attributes;
    uint16_t max_packet_size;
    uint8_t interval;
} __attribute__((packed)) usb_endpoint_descriptor_t;

// USB string descriptor
typedef struct {
    uint8_t length;
    uint8_t descriptor_type;
    uint16_t string[0];
} __attribute__((packed)) usb_string_descriptor_t;

// USB controller structure
typedef struct {
    int bus;
    int device;
    int function;
    uint16_t vendor_id;
    uint16_t device_id;
    usb_controller_type_t type;
    uintptr_t base_address;
    int irq;
    bool initialized;
    void* private_data;
} usb_controller_t;

// USB device structure
typedef struct {
    uint8_t address;
    uint8_t port;
    usb_speed_t speed;
    usb_device_state_t state;
    usb_device_descriptor_t* device_descriptor;
    usb_config_descriptor_t** config_descriptors;
    int num_configs;
    int current_config;
    void* private_data;
} usb_device_t;

// USB transfer structure
typedef struct {
    uint8_t device_address;
    uint8_t endpoint;
    void* data;
    size_t length;
    size_t transferred;
    error_t status;
    void (*callback)(struct usb_transfer* transfer);
    void* user_data;
} usb_transfer_t;

// USB driver functions
error_t usb_init(void);
void usb_shutdown(void);
error_t usb_scan_controllers(void);
bool usb_is_controller(uint16_t vendor_id, uint16_t device_id);
usb_controller_t* usb_controller_create(int bus, int device, int function, uint16_t vendor_id, uint16_t device_id);
usb_controller_type_t usb_determine_controller_type(uint16_t vendor_id, uint16_t device_id);
uintptr_t usb_get_controller_base_address(int bus, int device, int function);
int usb_get_controller_irq(int bus, int device, int function);
error_t usb_controller_init(usb_controller_t* controller);
void usb_controller_shutdown(usb_controller_t* controller);

// UHCI functions
error_t usb_uhci_init(usb_controller_t* controller);
void usb_uhci_shutdown(usb_controller_t* controller);

// EHCI functions
error_t usb_ehci_init(usb_controller_t* controller);
void usb_ehci_shutdown(usb_controller_t* controller);

// xHCI functions
error_t usb_xhci_init(usb_controller_t* controller);
void usb_xhci_shutdown(usb_controller_t* controller);

// Device management
usb_device_t* usb_device_create(uint8_t address, uint8_t port, usb_speed_t speed);
void usb_device_free(usb_device_t* device);
error_t usb_add_device(usb_device_t* device);
error_t usb_remove_device(uint8_t address);
usb_device_t* usb_find_device(uint8_t address);
error_t usb_get_devices(usb_device_t** device_list, int* count);
error_t usb_enumerate_device(usb_device_t* device);

// Transfer functions
error_t usb_control_transfer(usb_device_t* device, uint8_t endpoint, 
                           usb_setup_packet_t* setup, void* data, size_t length);
error_t usb_bulk_transfer(usb_device_t* device, uint8_t endpoint, 
                         void* data, size_t length, size_t* transferred);
error_t usb_interrupt_transfer(usb_device_t* device, uint8_t endpoint,
                              void* data, size_t length, size_t* transferred);

// Utility functions
bool usb_is_initialized(void);
int usb_get_controller_count(void);
int usb_get_device_count(void);

#endif // RAEENOS_USB_H 