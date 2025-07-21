#ifndef ADVANCED_NETWORKING_H
#define ADVANCED_NETWORKING_H

#include "network.h"
#include <stdint.h>
#include <stdbool.h>

// Advanced networking constants
#define MAX_WIRELESS_NETWORKS 50
#define MAX_BLUETOOTH_DEVICES 20
#define MAX_VPN_CONNECTIONS 10
#define MAX_WIFI_SSID_LENGTH 32
#define MAX_BT_NAME_LENGTH 64
#define MAX_VPN_NAME_LENGTH 64
#define WIFI_MAX_PASSWORD_LENGTH 64
#define BT_MAX_PIN_LENGTH 6

// Wireless network types
typedef enum {
    WIFI_TYPE_OPEN = 0,
    WIFI_TYPE_WEP = 1,
    WIFI_TYPE_WPA = 2,
    WIFI_TYPE_WPA2 = 3,
    WIFI_TYPE_WPA3 = 4
} wifi_security_type_t;

// Wireless network information
typedef struct wifi_network {
    char ssid[MAX_WIFI_SSID_LENGTH];        // Network SSID
    uint8_t bssid[6];                       // BSSID (MAC address)
    wifi_security_type_t security;          // Security type
    int8_t signal_strength;                 // Signal strength (dBm)
    uint8_t channel;                        // Channel number
    uint32_t frequency;                     // Frequency (MHz)
    uint32_t max_rate;                      // Maximum data rate (Mbps)
    bool is_connected;                      // Currently connected
    char password[WIFI_MAX_PASSWORD_LENGTH]; // Network password
    uint64_t last_seen;                     // Last time network was seen
} wifi_network_t;

// Bluetooth device types
typedef enum {
    BT_DEVICE_UNKNOWN = 0,
    BT_DEVICE_PHONE = 1,
    BT_DEVICE_LAPTOP = 2,
    BT_DEVICE_HEADSET = 3,
    BT_DEVICE_SPEAKER = 4,
    BT_DEVICE_KEYBOARD = 5,
    BT_DEVICE_MOUSE = 6,
    BT_DEVICE_PRINTER = 7,
    BT_DEVICE_GAMEPAD = 8
} bt_device_type_t;

// Bluetooth device information
typedef struct bt_device {
    uint8_t address[6];                     // Bluetooth address
    char name[MAX_BT_NAME_LENGTH];          // Device name
    bt_device_type_t type;                  // Device type
    uint16_t class_of_device;               // Class of device
    int8_t signal_strength;                 // Signal strength (dBm)
    bool is_paired;                         // Paired with this device
    bool is_connected;                      // Currently connected
    char pin[MAX_BT_PIN_LENGTH];            // PIN code
    uint64_t last_seen;                     // Last time device was seen
    void* connection_data;                  // Connection-specific data
} bt_device_t;

// VPN protocol types
typedef enum {
    VPN_PROTOCOL_OPENVPN = 0,
    VPN_PROTOCOL_WIREGUARD = 1,
    VPN_PROTOCOL_IPSEC = 2,
    VPN_PROTOCOL_L2TP = 3,
    VPN_PROTOCOL_PPTP = 4
} vpn_protocol_t;

// VPN connection states
typedef enum {
    VPN_STATE_DISCONNECTED = 0,
    VPN_STATE_CONNECTING = 1,
    VPN_STATE_CONNECTED = 2,
    VPN_STATE_DISCONNECTING = 3,
    VPN_STATE_ERROR = 4
} vpn_state_t;

// VPN connection information
typedef struct vpn_connection {
    char name[MAX_VPN_NAME_LENGTH];         // Connection name
    vpn_protocol_t protocol;                // VPN protocol
    vpn_state_t state;                      // Connection state
    ip_addr_t server_addr;                  // VPN server address
    uint16_t server_port;                   // VPN server port
    char username[64];                      // Username
    char password[64];                      // Password
    char certificate_path[256];             // Certificate file path
    char private_key_path[256];             // Private key file path
    ip_addr_t local_addr;                   // Local VPN address
    ip_addr_t remote_addr;                  // Remote VPN address
    uint64_t bytes_sent;                    // Bytes sent
    uint64_t bytes_received;                // Bytes received
    uint64_t connect_time;                  // Connection start time
    void* protocol_data;                    // Protocol-specific data
} vpn_connection_t;

// Advanced network interface
typedef struct advanced_network_interface {
    network_interface_t* base_interface;    // Base network interface
    bool is_wireless;                       // Wireless capability
    bool is_bluetooth;                      // Bluetooth capability
    bool is_vpn;                           // VPN capability
    
    // Wireless capabilities
    wifi_network_t* networks[MAX_WIRELESS_NETWORKS];
    uint32_t network_count;
    wifi_network_t* current_network;
    bool wifi_enabled;
    bool wifi_scanning;
    
    // Bluetooth capabilities
    bt_device_t* devices[MAX_BLUETOOTH_DEVICES];
    uint32_t device_count;
    bool bt_enabled;
    bool bt_discoverable;
    bool bt_scanning;
    
    // VPN capabilities
    vpn_connection_t* vpn_connections[MAX_VPN_CONNECTIONS];
    uint32_t vpn_count;
    vpn_connection_t* active_vpn;
    
    // Advanced features
    bool qos_enabled;                       // Quality of Service
    bool traffic_shaping;                   // Traffic shaping
    bool bandwidth_limiting;                // Bandwidth limiting
    uint32_t max_bandwidth;                 // Maximum bandwidth (Mbps)
} advanced_network_interface_t;

// Advanced networking system
typedef struct advanced_networking_system {
    spinlock_t lock;                        // System lock
    bool initialized;                       // Initialization flag
    advanced_network_interface_t* interfaces[MAX_INTERFACES];
    uint32_t interface_count;               // Number of interfaces
    
    // Wireless management
    bool wifi_system_enabled;
    uint32_t wifi_scan_interval;            // Scan interval (seconds)
    wifi_network_t* known_networks[MAX_WIRELESS_NETWORKS];
    uint32_t known_network_count;
    
    // Bluetooth management
    bool bt_system_enabled;
    uint32_t bt_scan_interval;              // Scan interval (seconds)
    bt_device_t* paired_devices[MAX_BLUETOOTH_DEVICES];
    uint32_t paired_device_count;
    
    // VPN management
    bool vpn_system_enabled;
    vpn_connection_t* vpn_configs[MAX_VPN_CONNECTIONS];
    uint32_t vpn_config_count;
    
    // Advanced features
    bool qos_system_enabled;
    bool traffic_shaping_enabled;
    bool bandwidth_limiting_enabled;
} advanced_networking_system_t;

// Advanced networking statistics
typedef struct advanced_networking_stats {
    // Wireless statistics
    uint32_t wifi_networks_found;
    uint32_t wifi_connections_made;
    uint32_t wifi_disconnections;
    uint64_t wifi_bytes_sent;
    uint64_t wifi_bytes_received;
    
    // Bluetooth statistics
    uint32_t bt_devices_found;
    uint32_t bt_pairings_made;
    uint32_t bt_connections_made;
    uint64_t bt_bytes_sent;
    uint64_t bt_bytes_received;
    
    // VPN statistics
    uint32_t vpn_connections_made;
    uint32_t vpn_disconnections;
    uint64_t vpn_bytes_sent;
    uint64_t vpn_bytes_received;
    uint64_t vpn_total_time;
    
    // System statistics
    uint64_t last_update;
} advanced_networking_stats_t;

// Function declarations

// System initialization
int advanced_networking_init(void);
void advanced_networking_shutdown(void);
advanced_networking_system_t* advanced_networking_get_system(void);

// Advanced interface management
advanced_network_interface_t* advanced_interface_create(network_interface_t* base);
int advanced_interface_destroy(advanced_network_interface_t* interface);
advanced_network_interface_t* advanced_interface_find(const char* name);

// Wireless networking
int wifi_enable(advanced_network_interface_t* interface);
int wifi_disable(advanced_network_interface_t* interface);
int wifi_scan(advanced_network_interface_t* interface);
int wifi_connect(advanced_network_interface_t* interface, const char* ssid, const char* password);
int wifi_disconnect(advanced_network_interface_t* interface);
int wifi_add_known_network(const char* ssid, const char* password, wifi_security_type_t security);
int wifi_remove_known_network(const char* ssid);
wifi_network_t* wifi_find_network(const char* ssid);
wifi_network_t* wifi_get_current_network(advanced_network_interface_t* interface);

// Bluetooth networking
int bt_enable(advanced_network_interface_t* interface);
int bt_disable(advanced_network_interface_t* interface);
int bt_scan(advanced_network_interface_t* interface);
int bt_pair(advanced_network_interface_t* interface, bt_device_t* device, const char* pin);
int bt_unpair(advanced_network_interface_t* interface, bt_device_t* device);
int bt_connect(advanced_network_interface_t* interface, bt_device_t* device);
int bt_disconnect(advanced_network_interface_t* interface, bt_device_t* device);
int bt_set_discoverable(advanced_network_interface_t* interface, bool discoverable);
bt_device_t* bt_find_device(const uint8_t* address);
bt_device_t* bt_find_device_by_name(const char* name);

// VPN networking
int vpn_create_connection(const char* name, vpn_protocol_t protocol, ip_addr_t server, uint16_t port);
int vpn_delete_connection(const char* name);
int vpn_connect(const char* name, const char* username, const char* password);
int vpn_disconnect(const char* name);
int vpn_set_certificate(const char* name, const char* cert_path, const char* key_path);
vpn_connection_t* vpn_find_connection(const char* name);
vpn_connection_t* vpn_get_active_connection(void);

// Advanced features
int qos_enable(advanced_network_interface_t* interface);
int qos_disable(advanced_network_interface_t* interface);
int qos_set_priority(advanced_network_interface_t* interface, ip_addr_t addr, uint8_t priority);
int traffic_shaping_enable(advanced_network_interface_t* interface);
int traffic_shaping_disable(advanced_network_interface_t* interface);
int traffic_shaping_set_limit(advanced_network_interface_t* interface, uint32_t limit);
int bandwidth_limit_set(advanced_network_interface_t* interface, uint32_t limit);

// Network utilities
int network_interface_upgrade(network_interface_t* base);
int network_interface_downgrade(advanced_network_interface_t* advanced);
bool network_interface_is_wireless(network_interface_t* interface);
bool network_interface_is_bluetooth(network_interface_t* interface);
bool network_interface_is_vpn(network_interface_t* interface);

// Advanced networking statistics
void advanced_networking_get_stats(advanced_networking_stats_t* stats);
void advanced_networking_reset_stats(void);

// Advanced networking debugging
void advanced_networking_dump_interfaces(void);
void advanced_networking_dump_wifi_networks(void);
void advanced_networking_dump_bt_devices(void);
void advanced_networking_dump_vpn_connections(void);
void advanced_networking_dump_stats(void);

#endif // ADVANCED_NETWORKING_H 