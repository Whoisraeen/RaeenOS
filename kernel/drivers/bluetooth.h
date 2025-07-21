#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "../core/types.h"
#include "../core/error.h"
#include "driver_framework.h"

// Bluetooth versions
typedef enum {
    BT_VERSION_1_0 = 0,
    BT_VERSION_1_1,
    BT_VERSION_1_2,
    BT_VERSION_2_0_EDR,
    BT_VERSION_2_1_EDR,
    BT_VERSION_3_0_HS,
    BT_VERSION_4_0_LE,
    BT_VERSION_4_1,
    BT_VERSION_4_2,
    BT_VERSION_5_0,
    BT_VERSION_5_1,
    BT_VERSION_5_2,
    BT_VERSION_5_3,
    BT_VERSION_5_4
} bt_version_t;

// Bluetooth device classes
typedef enum {
    BT_CLASS_MISCELLANEOUS = 0x00,
    BT_CLASS_COMPUTER = 0x01,
    BT_CLASS_PHONE = 0x02,
    BT_CLASS_LAN_ACCESS = 0x03,
    BT_CLASS_AUDIO_VIDEO = 0x04,
    BT_CLASS_PERIPHERAL = 0x05,
    BT_CLASS_IMAGING = 0x06,
    BT_CLASS_WEARABLE = 0x07,
    BT_CLASS_TOY = 0x08,
    BT_CLASS_HEALTH = 0x09,
    BT_CLASS_UNCATEGORIZED = 0x1F
} bt_device_class_t;

// Bluetooth connection states
typedef enum {
    BT_STATE_POWERED_OFF = 0,
    BT_STATE_POWERED_ON,
    BT_STATE_DISCOVERABLE,
    BT_STATE_SCANNING,
    BT_STATE_CONNECTING,
    BT_STATE_CONNECTED,
    BT_STATE_DISCONNECTING,
    BT_STATE_PAIRING,
    BT_STATE_PAIRED,
    BT_STATE_ERROR
} bt_state_t;

// Bluetooth security levels
typedef enum {
    BT_SECURITY_NONE = 0,
    BT_SECURITY_LOW,
    BT_SECURITY_MEDIUM,
    BT_SECURITY_HIGH,
    BT_SECURITY_FIPS
} bt_security_level_t;

// Bluetooth pairing methods
typedef enum {
    BT_PAIRING_NONE = 0,
    BT_PAIRING_PIN,
    BT_PAIRING_PASSKEY,
    BT_PAIRING_CONFIRM,
    BT_PAIRING_OOB
} bt_pairing_method_t;

// Bluetooth address structure
typedef struct {
    uint8_t addr[6];  // 48-bit Bluetooth address
} bt_address_t;

// Bluetooth device information
typedef struct {
    bt_address_t address;
    char name[248];                   // Device name (max 248 bytes)
    bt_device_class_t device_class;   // Device class
    uint32_t class_of_device;         // Full CoD value
    int8_t rssi;                      // Signal strength
    bool paired;                      // Pairing status
    bool connected;                   // Connection status
    bool trusted;                     // Trust status
    bool blocked;                     // Blocked status
    uint16_t vendor_id;               // Vendor ID
    uint16_t product_id;              // Product ID
    uint16_t version;                 // Version
    uint64_t last_seen;               // Last seen timestamp
    uint32_t services;                // Supported services (bitmask)
} bt_device_info_t;

// Bluetooth service UUIDs (16-bit)
#define BT_SERVICE_SDP                  0x0001
#define BT_SERVICE_UDP                  0x0002
#define BT_SERVICE_RFCOMM               0x0003
#define BT_SERVICE_TCP                  0x0004
#define BT_SERVICE_TCS_BIN              0x0005
#define BT_SERVICE_TCS_AT               0x0006
#define BT_SERVICE_ATT                  0x0007
#define BT_SERVICE_OBEX                 0x0008
#define BT_SERVICE_IP                   0x0009
#define BT_SERVICE_FTP                  0x000A
#define BT_SERVICE_HTTP                 0x000C
#define BT_SERVICE_WSP                  0x000E
#define BT_SERVICE_BNEP                 0x000F
#define BT_SERVICE_UPNP                 0x0010
#define BT_SERVICE_HIDP                 0x0011
#define BT_SERVICE_HARDCOPY_CONTROL     0x0012
#define BT_SERVICE_HARDCOPY_DATA        0x0014
#define BT_SERVICE_HARDCOPY_NOTIFICATION 0x0016
#define BT_SERVICE_AVCTP                0x0017
#define BT_SERVICE_AVDTP                0x0019
#define BT_SERVICE_CMTP                 0x001B
#define BT_SERVICE_MCAP_CONTROL         0x001E
#define BT_SERVICE_MCAP_DATA            0x001F
#define BT_SERVICE_L2CAP                0x0100

// Bluetooth profiles
#define BT_PROFILE_A2DP                 0x110D  // Advanced Audio Distribution
#define BT_PROFILE_AVRCP                0x110E  // Audio/Video Remote Control
#define BT_PROFILE_HFP                  0x111E  // Hands-Free Profile
#define BT_PROFILE_HSP                  0x1108  // Headset Profile
#define BT_PROFILE_HID                  0x1124  // Human Interface Device
#define BT_PROFILE_SPP                  0x1101  // Serial Port Profile
#define BT_PROFILE_OPP                  0x1105  // Object Push Profile
#define BT_PROFILE_FTP                  0x1106  // File Transfer Profile
#define BT_PROFILE_PAN                  0x1115  // Personal Area Network
#define BT_PROFILE_BIP                  0x111B  // Basic Imaging Profile
#define BT_PROFILE_PBAP                 0x1130  // Phone Book Access Profile

// Bluetooth capabilities
typedef struct {
    bt_version_t version;             // Bluetooth version
    bool supports_le;                 // Low Energy support
    bool supports_edr;                // Enhanced Data Rate support
    bool supports_hs;                 // High Speed support
    bool supports_sco;                // SCO audio support
    bool supports_esco;               // eSCO audio support
    uint32_t max_connections;         // Maximum concurrent connections
    uint32_t supported_profiles;      // Supported profiles (bitmask)
    uint8_t max_tx_power;             // Maximum TX power (dBm)
    uint8_t min_tx_power;             // Minimum TX power (dBm)
} bt_capabilities_t;

// Bluetooth statistics
typedef struct {
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t errors;
    uint64_t retransmissions;
    uint32_t connections_established;
    uint32_t connections_failed;
    uint32_t scan_count;
    uint32_t pair_count;
} bt_statistics_t;

// Bluetooth connection parameters
typedef struct {
    bt_address_t remote_address;
    bt_security_level_t security_level;
    uint16_t connection_interval;     // Connection interval (1.25ms units)
    uint16_t connection_latency;      // Connection latency
    uint16_t supervision_timeout;     // Supervision timeout (10ms units)
    bool auto_connect;                // Auto-connect on discovery
} bt_connection_params_t;

// Bluetooth pairing parameters
typedef struct {
    bt_address_t remote_address;
    bt_pairing_method_t method;
    char pin[17];                     // PIN code (max 16 digits + null)
    uint32_t passkey;                 // Numeric passkey
    bool mitm_protection;             // Man-in-the-middle protection
    bool bonding;                     // Create bond after pairing
} bt_pairing_params_t;

// Bluetooth adapter structure
typedef struct bt_adapter {
    device_t base;                    // Base device structure
    char name[64];                    // Adapter name
    bt_address_t address;             // Adapter address
    char friendly_name[248];          // User-friendly name
    
    // State
    bt_state_t state;                 // Current state
    bool powered;                     // Power state
    bool discoverable;                // Discoverable mode
    bool pairable;                    // Pairable mode
    uint32_t discoverable_timeout;    // Discoverable timeout (seconds)
    uint32_t pairable_timeout;        // Pairable timeout (seconds)
    
    // Capabilities
    bt_capabilities_t capabilities;   // Adapter capabilities
    
    // Device lists
    bt_device_info_t* paired_devices; // Paired devices
    uint32_t num_paired;              // Number of paired devices
    bt_device_info_t* discovered_devices; // Discovered devices
    uint32_t num_discovered;          // Number of discovered devices
    
    // Statistics
    bt_statistics_t stats;            // Connection statistics
    
    // Hardware specific
    uint16_t vendor_id;               // USB vendor ID
    uint16_t product_id;              // USB product ID
    void* hw_data;                    // Hardware-specific data
    
    // Operations
    error_t (*power_on)(struct bt_adapter* adapter);
    error_t (*power_off)(struct bt_adapter* adapter);
    error_t (*start_discovery)(struct bt_adapter* adapter);
    error_t (*stop_discovery)(struct bt_adapter* adapter);
    error_t (*connect_device)(struct bt_adapter* adapter, const bt_connection_params_t* params);
    error_t (*disconnect_device)(struct bt_adapter* adapter, const bt_address_t* address);
    error_t (*pair_device)(struct bt_adapter* adapter, const bt_pairing_params_t* params);
    error_t (*unpair_device)(struct bt_adapter* adapter, const bt_address_t* address);
    error_t (*set_discoverable)(struct bt_adapter* adapter, bool discoverable, uint32_t timeout);
    error_t (*set_pairable)(struct bt_adapter* adapter, bool pairable, uint32_t timeout);
    
    // List management
    struct bt_adapter* next;
} bt_adapter_t;

// Bluetooth subsystem structure
typedef struct {
    bool initialized;
    bt_adapter_t* adapter_list;
    uint32_t num_adapters;
    bt_adapter_t* default_adapter;
    
    // Global settings
    bool auto_power_on;
    uint32_t scan_timeout;
    uint32_t connect_timeout;
    uint32_t pair_timeout;
    
    // Thread synchronization
    mutex_t adapter_mutex;
    mutex_t discovery_mutex;
} bt_subsystem_t;

// Bluetooth event types
typedef enum {
    BT_EVENT_ADAPTER_ADDED = 0,
    BT_EVENT_ADAPTER_REMOVED,
    BT_EVENT_ADAPTER_POWERED_ON,
    BT_EVENT_ADAPTER_POWERED_OFF,
    BT_EVENT_DISCOVERY_STARTED,
    BT_EVENT_DISCOVERY_STOPPED,
    BT_EVENT_DEVICE_DISCOVERED,
    BT_EVENT_DEVICE_CONNECTED,
    BT_EVENT_DEVICE_DISCONNECTED,
    BT_EVENT_DEVICE_PAIRED,
    BT_EVENT_DEVICE_UNPAIRED,
    BT_EVENT_PAIRING_REQUEST,
    BT_EVENT_PAIRING_FAILED,
    BT_EVENT_SERVICE_DISCOVERED
} bt_event_type_t;

// Bluetooth event structure
typedef struct {
    bt_event_type_t type;
    bt_adapter_t* adapter;
    bt_device_info_t* device;
    void* data;
    size_t data_size;
    uint64_t timestamp;
} bt_event_t;

// Bluetooth event callback
typedef void (*bt_event_callback_t)(const bt_event_t* event);

// Function prototypes
error_t bluetooth_init(void);
void bluetooth_shutdown(void);

// Adapter management
error_t bt_register_adapter(bt_adapter_t* adapter);
error_t bt_unregister_adapter(bt_adapter_t* adapter);
bt_adapter_t* bt_find_adapter(const char* name);
bt_adapter_t* bt_get_default_adapter(void);
error_t bt_set_default_adapter(bt_adapter_t* adapter);

// Power management
error_t bt_power_on(bt_adapter_t* adapter);
error_t bt_power_off(bt_adapter_t* adapter);
bool bt_is_powered(bt_adapter_t* adapter);

// Discovery operations
error_t bt_start_discovery(bt_adapter_t* adapter);
error_t bt_stop_discovery(bt_adapter_t* adapter);
bool bt_is_discovering(bt_adapter_t* adapter);
bt_device_info_t* bt_get_discovered_devices(bt_adapter_t* adapter, uint32_t* count);

// Device operations
error_t bt_connect_device(bt_adapter_t* adapter, const bt_connection_params_t* params);
error_t bt_disconnect_device(bt_adapter_t* adapter, const bt_address_t* address);
bool bt_is_device_connected(bt_adapter_t* adapter, const bt_address_t* address);
bt_device_info_t* bt_find_device(bt_adapter_t* adapter, const bt_address_t* address);

// Pairing operations
error_t bt_pair_device(bt_adapter_t* adapter, const bt_pairing_params_t* params);
error_t bt_unpair_device(bt_adapter_t* adapter, const bt_address_t* address);
bool bt_is_device_paired(bt_adapter_t* adapter, const bt_address_t* address);
bt_device_info_t* bt_get_paired_devices(bt_adapter_t* adapter, uint32_t* count);

// Configuration
error_t bt_set_discoverable(bt_adapter_t* adapter, bool discoverable, uint32_t timeout);
error_t bt_set_pairable(bt_adapter_t* adapter, bool pairable, uint32_t timeout);
error_t bt_set_friendly_name(bt_adapter_t* adapter, const char* name);
error_t bt_set_device_trust(bt_adapter_t* adapter, const bt_address_t* address, bool trusted);
error_t bt_set_device_block(bt_adapter_t* adapter, const bt_address_t* address, bool blocked);

// Statistics and monitoring
error_t bt_get_statistics(bt_adapter_t* adapter, bt_statistics_t* stats);
error_t bt_get_signal_strength(bt_adapter_t* adapter, const bt_address_t* address, int8_t* rssi);

// Event system
error_t bt_register_event_callback(bt_event_callback_t callback);
error_t bt_unregister_event_callback(bt_event_callback_t callback);
void bt_notify_event(const bt_event_t* event);

// Utility functions
const char* bt_version_to_string(bt_version_t version);
const char* bt_state_to_string(bt_state_t state);
const char* bt_device_class_to_string(bt_device_class_t class);
const char* bt_security_level_to_string(bt_security_level_t level);
void bt_address_to_string(const bt_address_t* address, char* str);
error_t bt_string_to_address(const char* str, bt_address_t* address);
bool bt_address_equal(const bt_address_t* addr1, const bt_address_t* addr2);
void bt_address_copy(bt_address_t* dest, const bt_address_t* src);

// Service discovery
typedef struct {
    uint16_t service_class;
    uint16_t protocol;
    uint16_t port;
    char name[64];
    char description[256];
} bt_service_info_t;

error_t bt_discover_services(bt_adapter_t* adapter, const bt_address_t* address);
bt_service_info_t* bt_get_device_services(bt_adapter_t* adapter, 
                                        const bt_address_t* address, 
                                        uint32_t* count);

// Audio profiles (A2DP, HFP, HSP)
typedef struct {
    bt_address_t device_address;
    uint16_t profile;
    bool connected;
    uint32_t codec;
    uint32_t sample_rate;
    uint8_t channels;
    uint8_t volume;
} bt_audio_connection_t;

error_t bt_audio_connect(bt_adapter_t* adapter, const bt_address_t* address, uint16_t profile);
error_t bt_audio_disconnect(bt_adapter_t* adapter, const bt_address_t* address, uint16_t profile);
error_t bt_audio_set_volume(bt_adapter_t* adapter, const bt_address_t* address, uint8_t volume);
bt_audio_connection_t* bt_get_audio_connections(bt_adapter_t* adapter, uint32_t* count);

// HID profile
typedef struct {
    uint8_t report_id;
    uint8_t report_type;
    uint16_t data_length;
    uint8_t data[64];
} bt_hid_report_t;

error_t bt_hid_connect(bt_adapter_t* adapter, const bt_address_t* address);
error_t bt_hid_disconnect(bt_adapter_t* adapter, const bt_address_t* address);
error_t bt_hid_send_report(bt_adapter_t* adapter, const bt_address_t* address, 
                          const bt_hid_report_t* report);
error_t bt_hid_get_report(bt_adapter_t* adapter, const bt_address_t* address, 
                         bt_hid_report_t* report);

// Low Energy (LE) support
typedef struct {
    uint16_t connection_interval_min;
    uint16_t connection_interval_max;
    uint16_t connection_latency;
    uint16_t supervision_timeout;
    uint16_t minimum_ce_length;
    uint16_t maximum_ce_length;
} bt_le_connection_params_t;

error_t bt_le_start_advertising(bt_adapter_t* adapter);
error_t bt_le_stop_advertising(bt_adapter_t* adapter);
error_t bt_le_start_scan(bt_adapter_t* adapter);
error_t bt_le_stop_scan(bt_adapter_t* adapter);
error_t bt_le_connect(bt_adapter_t* adapter, const bt_address_t* address, 
                     const bt_le_connection_params_t* params);

#endif // BLUETOOTH_H