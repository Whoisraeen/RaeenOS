#ifndef WIFI_H
#define WIFI_H

#include "../core/types.h"
#include "../core/error.h"
#include "driver_framework.h"
#include "network.h"

// WiFi standards
typedef enum {
    WIFI_STANDARD_80211A = 0,
    WIFI_STANDARD_80211B,
    WIFI_STANDARD_80211G,
    WIFI_STANDARD_80211N,
    WIFI_STANDARD_80211AC,
    WIFI_STANDARD_80211AX, // WiFi 6
    WIFI_STANDARD_80211BE  // WiFi 7
} wifi_standard_t;

// WiFi security types
typedef enum {
    WIFI_SECURITY_NONE = 0,
    WIFI_SECURITY_WEP,
    WIFI_SECURITY_WPA,
    WIFI_SECURITY_WPA2,
    WIFI_SECURITY_WPA3,
    WIFI_SECURITY_WPS,
    WIFI_SECURITY_ENTERPRISE
} wifi_security_t;

// WiFi encryption types
typedef enum {
    WIFI_ENCRYPTION_NONE = 0,
    WIFI_ENCRYPTION_WEP64,
    WIFI_ENCRYPTION_WEP128,
    WIFI_ENCRYPTION_TKIP,
    WIFI_ENCRYPTION_AES,
    WIFI_ENCRYPTION_CCMP,
    WIFI_ENCRYPTION_GCMP
} wifi_encryption_t;

// WiFi frequency bands
typedef enum {
    WIFI_BAND_2_4GHZ = 0,
    WIFI_BAND_5GHZ,
    WIFI_BAND_6GHZ,
    WIFI_BAND_60GHZ
} wifi_band_t;

// WiFi channel width
typedef enum {
    WIFI_WIDTH_20MHZ = 20,
    WIFI_WIDTH_40MHZ = 40,
    WIFI_WIDTH_80MHZ = 80,
    WIFI_WIDTH_160MHZ = 160,
    WIFI_WIDTH_320MHZ = 320
} wifi_channel_width_t;

// WiFi power management modes
typedef enum {
    WIFI_POWER_ACTIVE = 0,
    WIFI_POWER_SAVE,
    WIFI_POWER_DEEP_SLEEP,
    WIFI_POWER_OFF
} wifi_power_mode_t;

// WiFi connection state
typedef enum {
    WIFI_STATE_DISCONNECTED = 0,
    WIFI_STATE_SCANNING,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_AUTHENTICATING,
    WIFI_STATE_ASSOCIATING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_DISCONNECTING,
    WIFI_STATE_ERROR
} wifi_state_t;

// WiFi network information
typedef struct {
    char ssid[33];                    // Network name (max 32 chars + null)
    uint8_t bssid[6];                 // MAC address of access point
    wifi_security_t security;         // Security type
    wifi_encryption_t encryption;     // Encryption type
    wifi_standard_t standard;         // WiFi standard
    wifi_band_t band;                 // Frequency band
    uint32_t frequency;               // Frequency in MHz
    uint8_t channel;                  // Channel number
    wifi_channel_width_t width;       // Channel width
    int8_t rssi;                      // Signal strength (dBm)
    uint8_t quality;                  // Signal quality (0-100%)
    bool hidden;                      // Hidden network
    uint32_t capabilities;            // Capability flags
    uint64_t timestamp;               // Last seen timestamp
} wifi_network_t;

// WiFi connection parameters
typedef struct {
    char ssid[33];                    // Network SSID
    char password[256];               // Network password
    wifi_security_t security;         // Security type
    bool auto_connect;                // Auto-connect on boot
    uint32_t priority;                // Connection priority
    uint32_t timeout;                 // Connection timeout (ms)
} wifi_connection_params_t;

// WiFi statistics
typedef struct {
    uint64_t tx_packets;              // Transmitted packets
    uint64_t rx_packets;              // Received packets
    uint64_t tx_bytes;                // Transmitted bytes
    uint64_t rx_bytes;                // Received bytes
    uint64_t tx_errors;               // Transmission errors
    uint64_t rx_errors;               // Reception errors
    uint64_t tx_dropped;              // Dropped TX packets
    uint64_t rx_dropped;              // Dropped RX packets
    uint32_t retries;                 // Retry count
    uint32_t failed;                  // Failed transmissions
    int8_t noise;                     // Noise level (dBm)
    uint32_t link_speed;              // Link speed (Mbps)
} wifi_stats_t;

// WiFi capabilities
typedef struct {
    wifi_standard_t max_standard;     // Maximum supported standard
    uint32_t supported_bands;         // Supported frequency bands (bitmask)
    uint32_t max_tx_power;            // Maximum TX power (mW)
    uint32_t max_channels;            // Maximum concurrent channels
    bool supports_monitor;            // Monitor mode support
    bool supports_ap;                 // Access Point mode support
    bool supports_mesh;               // Mesh networking support
    bool supports_p2p;                // WiFi Direct support
    bool supports_wps;                // WPS support
    bool supports_enterprise;         // Enterprise security support
    uint32_t max_scan_ssids;          // Maximum SSIDs per scan
    uint32_t max_sched_scans;         // Maximum scheduled scans
} wifi_capabilities_t;

// WiFi device structure
typedef struct wifi_device {
    device_t base;                    // Base device structure
    char name[64];                    // Device name
    char manufacturer[64];            // Manufacturer
    char model[64];                   // Model name
    char firmware_version[32];        // Firmware version
    uint8_t mac_address[6];           // MAC address
    
    // Current state
    wifi_state_t state;               // Connection state
    wifi_network_t* current_network;  // Currently connected network
    wifi_power_mode_t power_mode;     // Power management mode
    
    // Capabilities
    wifi_capabilities_t capabilities; // Device capabilities
    
    // Configuration
    bool enabled;                     // Device enabled
    bool auto_scan;                   // Automatic scanning
    uint32_t scan_interval;           // Scan interval (ms)
    uint32_t roaming_threshold;       // Roaming RSSI threshold
    
    // Network lists
    wifi_network_t* available_networks; // Scanned networks
    uint32_t num_available;           // Number of available networks
    wifi_connection_params_t* saved_networks; // Saved network profiles
    uint32_t num_saved;               // Number of saved networks
    
    // Statistics
    wifi_stats_t stats;               // Connection statistics
    
    // Hardware specific
    uint32_t vendor_id;               // PCI vendor ID
    uint32_t device_id;               // PCI device ID
    void* hw_data;                    // Hardware-specific data
    
    // Operations
    error_t (*enable)(struct wifi_device* dev);
    error_t (*disable)(struct wifi_device* dev);
    error_t (*scan)(struct wifi_device* dev);
    error_t (*connect)(struct wifi_device* dev, const wifi_connection_params_t* params);
    error_t (*disconnect)(struct wifi_device* dev);
    error_t (*set_power_mode)(struct wifi_device* dev, wifi_power_mode_t mode);
    error_t (*get_stats)(struct wifi_device* dev, wifi_stats_t* stats);
    error_t (*set_channel)(struct wifi_device* dev, uint8_t channel);
    error_t (*set_tx_power)(struct wifi_device* dev, uint32_t power);
    
    // List management
    struct wifi_device* next;
} wifi_device_t;

// WiFi subsystem structure
typedef struct {
    bool initialized;
    wifi_device_t* device_list;
    uint32_t num_devices;
    wifi_device_t* default_device;
    
    // Global settings
    bool auto_connect;
    uint32_t scan_timeout;
    uint32_t connect_timeout;
    
    // Thread synchronization
    mutex_t device_mutex;
    mutex_t scan_mutex;
} wifi_subsystem_t;

// Event types for WiFi notifications
typedef enum {
    WIFI_EVENT_SCAN_COMPLETE = 0,
    WIFI_EVENT_CONNECTED,
    WIFI_EVENT_DISCONNECTED,
    WIFI_EVENT_CONNECTION_FAILED,
    WIFI_EVENT_SIGNAL_CHANGED,
    WIFI_EVENT_ROAMING,
    WIFI_EVENT_DEVICE_ADDED,
    WIFI_EVENT_DEVICE_REMOVED
} wifi_event_type_t;

// WiFi event structure
typedef struct {
    wifi_event_type_t type;
    wifi_device_t* device;
    wifi_network_t* network;
    void* data;
    size_t data_size;
    uint64_t timestamp;
} wifi_event_t;

// WiFi event callback
typedef void (*wifi_event_callback_t)(const wifi_event_t* event);

// Function prototypes
error_t wifi_init(void);
void wifi_shutdown(void);

// Device management
error_t wifi_register_device(wifi_device_t* device);
error_t wifi_unregister_device(wifi_device_t* device);
wifi_device_t* wifi_find_device(const char* name);
wifi_device_t* wifi_get_default_device(void);
error_t wifi_set_default_device(wifi_device_t* device);

// Device operations
error_t wifi_enable_device(wifi_device_t* device);
error_t wifi_disable_device(wifi_device_t* device);
error_t wifi_scan(wifi_device_t* device);
error_t wifi_scan_async(wifi_device_t* device, wifi_event_callback_t callback);

// Network operations
error_t wifi_connect(wifi_device_t* device, const wifi_connection_params_t* params);
error_t wifi_disconnect(wifi_device_t* device);
error_t wifi_auto_connect(wifi_device_t* device);
wifi_network_t* wifi_get_current_network(wifi_device_t* device);
wifi_state_t wifi_get_state(wifi_device_t* device);

// Network management
error_t wifi_save_network(wifi_device_t* device, const wifi_connection_params_t* params);
error_t wifi_remove_network(wifi_device_t* device, const char* ssid);
wifi_network_t* wifi_get_available_networks(wifi_device_t* device, uint32_t* count);
wifi_connection_params_t* wifi_get_saved_networks(wifi_device_t* device, uint32_t* count);

// Configuration
error_t wifi_set_power_mode(wifi_device_t* device, wifi_power_mode_t mode);
wifi_power_mode_t wifi_get_power_mode(wifi_device_t* device);
error_t wifi_set_auto_scan(wifi_device_t* device, bool enabled, uint32_t interval);
error_t wifi_set_roaming_threshold(wifi_device_t* device, int8_t threshold);

// Statistics and monitoring
error_t wifi_get_stats(wifi_device_t* device, wifi_stats_t* stats);
error_t wifi_get_signal_strength(wifi_device_t* device, int8_t* rssi, uint8_t* quality);
error_t wifi_get_link_speed(wifi_device_t* device, uint32_t* speed);

// Event system
error_t wifi_register_event_callback(wifi_event_callback_t callback);
error_t wifi_unregister_event_callback(wifi_event_callback_t callback);
void wifi_notify_event(const wifi_event_t* event);

// Utility functions
const char* wifi_standard_to_string(wifi_standard_t standard);
const char* wifi_security_to_string(wifi_security_t security);
const char* wifi_state_to_string(wifi_state_t state);
const char* wifi_band_to_string(wifi_band_t band);
uint32_t wifi_channel_to_frequency(uint8_t channel, wifi_band_t band);
uint8_t wifi_frequency_to_channel(uint32_t frequency);
bool wifi_is_valid_ssid(const char* ssid);
bool wifi_is_valid_password(const char* password, wifi_security_t security);

// WPA/WPA2/WPA3 authentication
error_t wifi_wpa_authenticate(wifi_device_t* device, const wifi_connection_params_t* params);
error_t wifi_wpa_generate_psk(const char* ssid, const char* password, uint8_t* psk);

// Enterprise authentication
typedef struct {
    char identity[256];
    char password[256];
    char ca_cert_path[512];
    char client_cert_path[512];
    char private_key_path[512];
    char private_key_password[256];
} wifi_enterprise_config_t;

error_t wifi_enterprise_authenticate(wifi_device_t* device, 
                                   const wifi_enterprise_config_t* config);

// WiFi Direct (P2P) support
typedef struct {
    char device_name[64];
    uint8_t device_address[6];
    uint16_t config_methods;
    uint8_t primary_device_type[8];
    uint8_t group_capability;
    uint8_t device_capability;
} wifi_p2p_device_t;

error_t wifi_p2p_enable(wifi_device_t* device);
error_t wifi_p2p_disable(wifi_device_t* device);
error_t wifi_p2p_discover(wifi_device_t* device);
error_t wifi_p2p_connect(wifi_device_t* device, const wifi_p2p_device_t* peer);

// Monitor mode support
error_t wifi_set_monitor_mode(wifi_device_t* device, bool enabled);
error_t wifi_set_monitor_channel(wifi_device_t* device, uint8_t channel);
error_t wifi_capture_packet(wifi_device_t* device, void* buffer, size_t* size);

#endif // WIFI_H