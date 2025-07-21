#ifndef USB_SYSTEM_H
#define USB_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memory.h>

// USB System - Advanced USB device management for RaeenOS
// Provides USB 3.2, USB-C, Thunderbolt, and device hotplugging support

// USB versions
typedef enum {
    USB_VERSION_1_0 = 0x0100,        // USB 1.0 (12 Mbps)
    USB_VERSION_1_1 = 0x0110,        // USB 1.1 (12 Mbps)
    USB_VERSION_2_0 = 0x0200,        // USB 2.0 (480 Mbps)
    USB_VERSION_3_0 = 0x0300,        // USB 3.0 (5 Gbps)
    USB_VERSION_3_1 = 0x0310,        // USB 3.1 (10 Gbps)
    USB_VERSION_3_2 = 0x0320,        // USB 3.2 (20 Gbps)
    USB_VERSION_4_0 = 0x0400,        // USB 4.0 (40 Gbps)
} usb_version_t;

// USB device classes
typedef enum {
    USB_CLASS_DEVICE = 0x00,         // Device class
    USB_CLASS_AUDIO = 0x01,          // Audio device
    USB_CLASS_COMMUNICATIONS = 0x02, // Communications device
    USB_CLASS_HID = 0x03,            // Human Interface Device
    USB_CLASS_PHYSICAL = 0x05,       // Physical device
    USB_CLASS_IMAGE = 0x06,          // Image device
    USB_CLASS_PRINTER = 0x07,        // Printer
    USB_CLASS_MASS_STORAGE = 0x08,   // Mass storage
    USB_CLASS_HUB = 0x09,            // Hub
    USB_CLASS_CDC_DATA = 0x0A,       // CDC data
    USB_CLASS_SMART_CARD = 0x0B,     // Smart card
    USB_CLASS_CONTENT_SECURITY = 0x0D, // Content security
    USB_CLASS_VIDEO = 0x0E,          // Video device
    USB_CLASS_PERSONAL_HEALTHCARE = 0x0F, // Healthcare
    USB_CLASS_AUDIO_VIDEO = 0x10,    // Audio/Video device
    USB_CLASS_BILLBOARD = 0x11,      // Billboard device
    USB_CLASS_USB_C_BRIDGE = 0x12,   // USB-C bridge
    USB_CLASS_DIAGNOSTIC = 0xDC,     // Diagnostic device
    USB_CLASS_WIRELESS = 0xE0,       // Wireless device
    USB_CLASS_MISCELLANEOUS = 0xEF,  // Miscellaneous
    USB_CLASS_VENDOR_SPECIFIC = 0xFF, // Vendor specific
} usb_device_class_t;

// USB transfer types
typedef enum {
    USB_TRANSFER_CONTROL = 0,        // Control transfer
    USB_TRANSFER_ISOCHRONOUS = 1,    // Isochronous transfer
    USB_TRANSFER_BULK = 2,           // Bulk transfer
    USB_TRANSFER_INTERRUPT = 3,      // Interrupt transfer
} usb_transfer_type_t;

// USB device states
typedef enum {
    USB_DEVICE_STATE_DISCONNECTED = 0, // Device disconnected
    USB_DEVICE_STATE_ATTACHED,       // Device attached
    USB_DEVICE_STATE_POWERED,        // Device powered
    USB_DEVICE_STATE_DEFAULT,        // Device in default state
    USB_DEVICE_STATE_ADDRESS,        // Device addressed
    USB_DEVICE_STATE_CONFIGURED,     // Device configured
    USB_DEVICE_STATE_SUSPENDED,      // Device suspended
} usb_device_state_t;

// USB device information
typedef struct {
    uint32_t id;
    char name[64];                   // Device name
    char manufacturer[64];            // Manufacturer
    char product[64];                // Product name
    char serial[64];                 // Serial number
    usb_version_t version;           // USB version
    usb_device_class_t device_class; // Device class
    usb_device_class_t interface_class; // Interface class
    uint16_t vendor_id;              // Vendor ID
    uint16_t product_id;             // Product ID
    uint8_t device_address;          // Device address
    uint8_t configuration;           // Current configuration
    uint8_t interface_count;         // Number of interfaces
    uint8_t endpoint_count;          // Number of endpoints
    uint32_t max_packet_size;        // Maximum packet size
    usb_device_state_t state;        // Device state
    bool is_connected;               // Is connected
    bool is_enabled;                 // Is enabled
    bool is_suspended;               // Is suspended
    uint64_t connected_time;         // Connection time
    uint64_t last_activity_time;     // Last activity time
    void* device_data;               // Device-specific data
} usb_device_t;

// USB endpoint
typedef struct {
    uint8_t address;                 // Endpoint address
    uint8_t attributes;              // Endpoint attributes
    uint16_t max_packet_size;        // Maximum packet size
    uint8_t interval;                // Polling interval
    usb_transfer_type_t transfer_type; // Transfer type
    bool is_in;                      // Is IN endpoint
    bool is_out;                     // Is OUT endpoint
    bool is_control;                 // Is control endpoint
    bool is_bulk;                    // Is bulk endpoint
    bool is_interrupt;               // Is interrupt endpoint
    bool is_isochronous;             // Is isochronous endpoint
} usb_endpoint_t;

// USB interface
typedef struct {
    uint8_t number;                  // Interface number
    uint8_t alternate_setting;       // Alternate setting
    uint8_t endpoint_count;          // Number of endpoints
    usb_device_class_t interface_class; // Interface class
    uint8_t interface_subclass;      // Interface subclass
    uint8_t interface_protocol;      // Interface protocol
    usb_endpoint_t* endpoints;       // Endpoints
    uint32_t max_endpoints;          // Maximum endpoints
    void* interface_data;            // Interface-specific data
} usb_interface_t;

// USB configuration
typedef struct {
    uint8_t number;                  // Configuration number
    uint8_t interface_count;         // Number of interfaces
    uint16_t total_length;           // Total length
    uint8_t attributes;              // Configuration attributes
    uint8_t max_power;               // Maximum power in mA
    usb_interface_t* interfaces;     // Interfaces
    uint32_t max_interfaces;         // Maximum interfaces
    bool is_active;                  // Is active configuration
} usb_configuration_t;

// USB hub
typedef struct {
    uint32_t id;
    char name[64];                   // Hub name
    uint8_t port_count;              // Number of ports
    uint8_t power_on_delay;          // Power-on delay
    uint8_t current_limit;           // Current limit
    bool is_root_hub;                // Is root hub
    bool is_self_powered;            // Is self-powered
    bool is_removable;               // Is removable
    uint8_t* port_status;            // Port status array
    uint8_t* port_change;            // Port change array
    usb_device_t* connected_devices; // Connected devices
    uint32_t max_devices;            // Maximum devices
} usb_hub_t;

// USB controller
typedef struct {
    uint32_t id;
    char name[64];                   // Controller name
    usb_version_t version;           // USB version
    uint32_t port_count;             // Number of ports
    uint32_t max_devices;            // Maximum devices
    uint32_t max_endpoints;          // Maximum endpoints
    uint32_t max_transfer_size;      // Maximum transfer size
    bool is_high_speed;              // Supports high speed
    bool is_super_speed;             // Supports super speed
    bool is_enhanced;                // Enhanced controller
    usb_hub_t* root_hub;             // Root hub
    void* controller_data;           // Controller-specific data
} usb_controller_t;

// USB system configuration
typedef struct {
    bool enable_usb_1_0;             // Enable USB 1.0 support
    bool enable_usb_2_0;             // Enable USB 2.0 support
    bool enable_usb_3_0;             // Enable USB 3.0 support
    bool enable_usb_3_1;             // Enable USB 3.1 support
    bool enable_usb_3_2;             // Enable USB 3.2 support
    bool enable_usb_4_0;             // Enable USB 4.0 support
    bool enable_usb_c;               // Enable USB-C support
    bool enable_thunderbolt;         // Enable Thunderbolt support
    bool enable_hotplugging;         // Enable device hotplugging
    bool enable_power_management;    // Enable power management
    bool enable_suspend_resume;      // Enable suspend/resume
    bool enable_remote_wakeup;       // Enable remote wakeup
    uint32_t max_controllers;        // Maximum controllers
    uint32_t max_devices;            // Maximum devices
    uint32_t max_hubs;               // Maximum hubs
    uint32_t max_endpoints;          // Maximum endpoints
    uint32_t max_transfers;          // Maximum transfers
    uint32_t transfer_timeout;       // Transfer timeout in ms
    uint32_t enumeration_timeout;    // Enumeration timeout in ms
} usb_system_config_t;

// USB system context
typedef struct {
    usb_system_config_t config;
    usb_controller_t* controllers;
    uint32_t controller_count;
    uint32_t max_controllers;
    usb_device_t* devices;
    uint32_t device_count;
    uint32_t max_devices;
    usb_hub_t* hubs;
    uint32_t hub_count;
    uint32_t max_hubs;
    bool initialized;
    bool hotplugging_enabled;
    uint32_t next_device_id;
    uint32_t next_controller_id;
    uint32_t next_hub_id;
    uint64_t last_update_time;
} usb_system_t;

// Function prototypes

// Initialization and shutdown
usb_system_t* usb_system_init(usb_system_config_t* config);
void usb_system_shutdown(usb_system_t* usb);
bool usb_system_is_initialized(usb_system_t* usb);

// Controller management
uint32_t usb_system_add_controller(usb_system_t* usb, usb_controller_t* controller);
void usb_system_remove_controller(usb_system_t* usb, uint32_t controller_id);
usb_controller_t* usb_system_get_controller(usb_system_t* usb, uint32_t controller_id);
usb_controller_t* usb_system_get_controllers(usb_system_t* usb, uint32_t* count);
uint32_t usb_system_get_controller_count(usb_system_t* usb);
bool usb_system_is_controller_enabled(usb_system_t* usb, uint32_t controller_id);
void usb_system_enable_controller(usb_system_t* usb, uint32_t controller_id, bool enable);

// Device management
uint32_t usb_system_add_device(usb_system_t* usb, usb_device_t* device);
void usb_system_remove_device(usb_system_t* usb, uint32_t device_id);
usb_device_t* usb_system_get_device(usb_system_t* usb, uint32_t device_id);
usb_device_t* usb_system_get_devices(usb_system_t* usb, uint32_t* count);
uint32_t usb_system_get_device_count(usb_system_t* usb);
usb_device_t* usb_system_get_devices_by_class(usb_system_t* usb, usb_device_class_t class, uint32_t* count);
usb_device_t* usb_system_get_device_by_vendor_product(usb_system_t* usb, uint16_t vendor_id, uint16_t product_id);
bool usb_system_is_device_connected(usb_system_t* usb, uint32_t device_id);
bool usb_system_is_device_enabled(usb_system_t* usb, uint32_t device_id);

// Hub management
uint32_t usb_system_add_hub(usb_system_t* usb, usb_hub_t* hub);
void usb_system_remove_hub(usb_system_t* usb, uint32_t hub_id);
usb_hub_t* usb_system_get_hub(usb_system_t* usb, uint32_t hub_id);
usb_hub_t* usb_system_get_hubs(usb_system_t* usb, uint32_t* count);
uint32_t usb_system_get_hub_count(usb_system_t* usb);
usb_hub_t* usb_system_get_root_hub(usb_system_t* usb, uint32_t controller_id);
bool usb_system_is_hub_enabled(usb_system_t* usb, uint32_t hub_id);
void usb_system_enable_hub(usb_system_t* usb, uint32_t hub_id, bool enable);

// Device enumeration
bool usb_system_enumerate_device(usb_system_t* usb, uint32_t controller_id, uint8_t port);
bool usb_system_de_enumerate_device(usb_system_t* usb, uint32_t device_id);
bool usb_system_reset_device(usb_system_t* usb, uint32_t device_id);
bool usb_system_suspend_device(usb_system_t* usb, uint32_t device_id);
bool usb_system_resume_device(usb_system_t* usb, uint32_t device_id);

// Device configuration
bool usb_system_set_device_address(usb_system_t* usb, uint32_t device_id, uint8_t address);
uint8_t usb_system_get_device_address(usb_system_t* usb, uint32_t device_id);
bool usb_system_set_device_configuration(usb_system_t* usb, uint32_t device_id, uint8_t configuration);
uint8_t usb_system_get_device_configuration(usb_system_t* usb, uint32_t device_id);
usb_configuration_t* usb_system_get_device_configurations(usb_system_t* usb, uint32_t device_id, uint32_t* count);
usb_interface_t* usb_system_get_device_interfaces(usb_system_t* usb, uint32_t device_id, uint32_t* count);
usb_endpoint_t* usb_system_get_device_endpoints(usb_system_t* usb, uint32_t device_id, uint32_t* count);

// Transfer management
bool usb_system_control_transfer(usb_system_t* usb, uint32_t device_id, uint8_t endpoint, uint8_t* data, uint32_t length, uint32_t timeout);
bool usb_system_bulk_transfer(usb_system_t* usb, uint32_t device_id, uint8_t endpoint, uint8_t* data, uint32_t length, uint32_t timeout);
bool usb_system_interrupt_transfer(usb_system_t* usb, uint32_t device_id, uint8_t endpoint, uint8_t* data, uint32_t length, uint32_t timeout);
bool usb_system_isochronous_transfer(usb_system_t* usb, uint32_t device_id, uint8_t endpoint, uint8_t* data, uint32_t length, uint32_t timeout);

// Hotplugging
void usb_system_enable_hotplugging(usb_system_t* usb, bool enable);
bool usb_system_is_hotplugging_enabled(usb_system_t* usb);
void usb_system_handle_device_connected(usb_system_t* usb, uint32_t controller_id, uint8_t port);
void usb_system_handle_device_disconnected(usb_system_t* usb, uint32_t controller_id, uint8_t port);
bool usb_system_is_device_hotpluggable(usb_system_t* usb, uint32_t device_id);

// Power management
void usb_system_enable_power_management(usb_system_t* usb, bool enable);
bool usb_system_is_power_management_enabled(usb_system_t* usb);
bool usb_system_suspend_device(usb_system_t* usb, uint32_t device_id);
bool usb_system_resume_device(usb_system_t* usb, uint32_t device_id);
bool usb_system_enable_remote_wakeup(usb_system_t* usb, uint32_t device_id, bool enable);
bool usb_system_is_remote_wakeup_enabled(usb_system_t* usb, uint32_t device_id);

// USB-C and Thunderbolt
bool usb_system_is_usb_c_device(usb_system_t* usb, uint32_t device_id);
bool usb_system_is_thunderbolt_device(usb_system_t* usb, uint32_t device_id);
bool usb_system_support_alternate_modes(usb_system_t* usb, uint32_t device_id);
bool usb_system_enter_alternate_mode(usb_system_t* usb, uint32_t device_id, uint32_t mode);
bool usb_system_exit_alternate_mode(usb_system_t* usb, uint32_t device_id);

// Device drivers
bool usb_system_load_driver(usb_system_t* usb, uint32_t device_id, const char* driver_path);
bool usb_system_unload_driver(usb_system_t* usb, uint32_t device_id);
bool usb_system_is_driver_loaded(usb_system_t* usb, uint32_t device_id);
const char* usb_system_get_driver_name(usb_system_t* usb, uint32_t device_id);

// Information
uint32_t usb_system_get_connected_device_count(usb_system_t* usb);
uint32_t usb_system_get_enabled_device_count(usb_system_t* usb);
uint32_t usb_system_get_suspended_device_count(usb_system_t* usb);
bool usb_system_has_high_speed_devices(usb_system_t* usb);
bool usb_system_has_super_speed_devices(usb_system_t* usb);
uint32_t usb_system_get_total_bandwidth_usage(usb_system_t* usb);

// Utility functions
char* usb_system_get_version_name(usb_version_t version);
char* usb_system_get_class_name(usb_device_class_t class);
char* usb_system_get_transfer_type_name(usb_transfer_type_t type);
char* usb_system_get_state_name(usb_device_state_t state);
bool usb_system_is_version_supported(usb_version_t version);
bool usb_system_is_class_supported(usb_device_class_t class);
uint32_t usb_system_get_max_speed(usb_version_t version);
uint32_t usb_system_calculate_bandwidth(usb_transfer_type_t type, uint32_t packet_size, uint32_t interval);

// Callbacks
typedef void (*usb_device_connected_callback_t)(usb_system_t* usb, uint32_t device_id, void* user_data);
typedef void (*usb_device_disconnected_callback_t)(usb_system_t* usb, uint32_t device_id, void* user_data);
typedef void (*usb_device_suspended_callback_t)(usb_system_t* usb, uint32_t device_id, void* user_data);
typedef void (*usb_device_resumed_callback_t)(usb_system_t* usb, uint32_t device_id, void* user_data);

void usb_system_set_device_connected_callback(usb_system_t* usb, usb_device_connected_callback_t callback, void* user_data);
void usb_system_set_device_disconnected_callback(usb_system_t* usb, usb_device_disconnected_callback_t callback, void* user_data);
void usb_system_set_device_suspended_callback(usb_system_t* usb, usb_device_suspended_callback_t callback, void* user_data);
void usb_system_set_device_resumed_callback(usb_system_t* usb, usb_device_resumed_callback_t callback, void* user_data);

// Preset configurations
usb_system_config_t usb_system_preset_normal_style(void);
usb_system_config_t usb_system_preset_high_performance_style(void);
usb_system_config_t usb_system_preset_power_saving_style(void);
usb_system_config_t usb_system_preset_enterprise_style(void);

// Error handling
typedef enum {
    USB_SYSTEM_SUCCESS = 0,
    USB_SYSTEM_ERROR_INVALID_CONTEXT,
    USB_SYSTEM_ERROR_INVALID_DEVICE,
    USB_SYSTEM_ERROR_INVALID_CONTROLLER,
    USB_SYSTEM_ERROR_DEVICE_NOT_FOUND,
    USB_SYSTEM_ERROR_CONTROLLER_NOT_FOUND,
    USB_SYSTEM_ERROR_TRANSFER_FAILED,
    USB_SYSTEM_ERROR_TIMEOUT,
    USB_SYSTEM_ERROR_OUT_OF_MEMORY,
    USB_SYSTEM_ERROR_DRIVER_FAILED,
} usb_system_error_t;

usb_system_error_t usb_system_get_last_error(void);
const char* usb_system_get_error_string(usb_system_error_t error);

#endif // USB_SYSTEM_H 