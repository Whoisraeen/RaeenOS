#ifndef WIFI_SYSTEM_H
#define WIFI_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memory.h>

// WiFi System - Advanced wireless networking for RaeenOS
// Provides WPA3, mesh networking, and enterprise security

// WiFi standards
typedef enum {
    WIFI_STANDARD_802_11A = 0,   // 802.11a (5 GHz, 54 Mbps)
    WIFI_STANDARD_802_11B,       // 802.11b (2.4 GHz, 11 Mbps)
    WIFI_STANDARD_802_11G,       // 802.11g (2.4 GHz, 54 Mbps)
    WIFI_STANDARD_802_11N,       // 802.11n (2.4/5 GHz, 600 Mbps)
    WIFI_STANDARD_802_11AC,      // 802.11ac (5 GHz, 6.9 Gbps)
    WIFI_STANDARD_802_11AX,      // 802.11ax (Wi-Fi 6, 9.6 Gbps)
    WIFI_STANDARD_802_11BE,      // 802.11be (Wi-Fi 7, 46 Gbps)
} wifi_standard_t;

// WiFi security protocols
typedef enum {
    WIFI_SECURITY_OPEN = 0,      // No security
    WIFI_SECURITY_WEP,           // WEP (deprecated)
    WIFI_SECURITY_WPA,           // WPA
    WIFI_SECURITY_WPA2,          // WPA2
    WIFI_SECURITY_WPA3,          // WPA3
    WIFI_SECURITY_WPA3_ENTERPRISE, // WPA3 Enterprise
    WIFI_SECURITY_OWE,           // Opportunistic Wireless Encryption
} wifi_security_t;

// WiFi authentication methods
typedef enum {
    WIFI_AUTH_PSK = 0,           // Pre-shared key
    WIFI_AUTH_SAE,               // Simultaneous Authentication of Equals (WPA3)
    WIFI_AUTH_802_1X,            // 802.1X authentication
    WIFI_AUTH_EAP_TLS,           // EAP-TLS
    WIFI_AUTH_EAP_TTLS,          // EAP-TTLS
    WIFI_AUTH_EAP_PEAP,          // EAP-PEAP
    WIFI_AUTH_EAP_FAST,          // EAP-FAST
} wifi_auth_method_t;

// WiFi bands
typedef enum {
    WIFI_BAND_2_4GHZ = 0,        // 2.4 GHz band
    WIFI_BAND_5GHZ,              // 5 GHz band
    WIFI_BAND_6GHZ,              // 6 GHz band (Wi-Fi 6E)
} wifi_band_t;

// WiFi channels
typedef enum {
    WIFI_CHANNEL_1 = 1,          // 2.4 GHz channels
    WIFI_CHANNEL_2 = 2,
    WIFI_CHANNEL_3 = 3,
    WIFI_CHANNEL_4 = 4,
    WIFI_CHANNEL_5 = 5,
    WIFI_CHANNEL_6 = 6,
    WIFI_CHANNEL_7 = 7,
    WIFI_CHANNEL_8 = 8,
    WIFI_CHANNEL_9 = 9,
    WIFI_CHANNEL_10 = 10,
    WIFI_CHANNEL_11 = 11,
    WIFI_CHANNEL_12 = 12,
    WIFI_CHANNEL_13 = 13,
    WIFI_CHANNEL_14 = 14,
    WIFI_CHANNEL_36 = 36,        // 5 GHz channels
    WIFI_CHANNEL_40 = 40,
    WIFI_CHANNEL_44 = 44,
    WIFI_CHANNEL_48 = 48,
    WIFI_CHANNEL_52 = 52,
    WIFI_CHANNEL_56 = 56,
    WIFI_CHANNEL_60 = 60,
    WIFI_CHANNEL_64 = 64,
    WIFI_CHANNEL_100 = 100,
    WIFI_CHANNEL_104 = 104,
    WIFI_CHANNEL_108 = 108,
    WIFI_CHANNEL_112 = 112,
    WIFI_CHANNEL_116 = 116,
    WIFI_CHANNEL_120 = 120,
    WIFI_CHANNEL_124 = 124,
    WIFI_CHANNEL_128 = 128,
    WIFI_CHANNEL_132 = 132,
    WIFI_CHANNEL_136 = 136,
    WIFI_CHANNEL_140 = 140,
    WIFI_CHANNEL_144 = 144,
    WIFI_CHANNEL_149 = 149,
    WIFI_CHANNEL_153 = 153,
    WIFI_CHANNEL_157 = 157,
    WIFI_CHANNEL_161 = 161,
    WIFI_CHANNEL_165 = 165,
    WIFI_CHANNEL_1_6GHZ = 200,   // 6 GHz channels (Wi-Fi 6E)
    WIFI_CHANNEL_5_6GHZ = 205,
    WIFI_CHANNEL_9_6GHZ = 209,
    WIFI_CHANNEL_13_6GHZ = 213,
    WIFI_CHANNEL_17_6GHZ = 217,
    WIFI_CHANNEL_21_6GHZ = 221,
    WIFI_CHANNEL_25_6GHZ = 225,
    WIFI_CHANNEL_29_6GHZ = 229,
    WIFI_CHANNEL_33_6GHZ = 233,
    WIFI_CHANNEL_37_6GHZ = 237,
    WIFI_CHANNEL_41_6GHZ = 241,
    WIFI_CHANNEL_45_6GHZ = 245,
    WIFI_CHANNEL_49_6GHZ = 249,
    WIFI_CHANNEL_53_6GHZ = 253,
    WIFI_CHANNEL_57_6GHZ = 257,
    WIFI_CHANNEL_61_6GHZ = 261,
    WIFI_CHANNEL_65_6GHZ = 265,
    WIFI_CHANNEL_69_6GHZ = 269,
    WIFI_CHANNEL_73_6GHZ = 273,
    WIFI_CHANNEL_77_6GHZ = 277,
    WIFI_CHANNEL_81_6GHZ = 281,
    WIFI_CHANNEL_85_6GHZ = 285,
    WIFI_CHANNEL_89_6GHZ = 289,
    WIFI_CHANNEL_93_6GHZ = 293,
} wifi_channel_t;

// WiFi connection states
typedef enum {
    WIFI_STATE_DISCONNECTED = 0, // Not connected
    WIFI_STATE_SCANNING,         // Scanning for networks
    WIFI_STATE_CONNECTING,       // Attempting to connect
    WIFI_STATE_AUTHENTICATING,   // Authenticating
    WIFI_STATE_ASSOCIATING,      // Associating with AP
    WIFI_STATE_CONNECTED,        // Connected and operational
    WIFI_STATE_DISCONNECTING,    // Disconnecting
    WIFI_STATE_ERROR,            // Error state
} wifi_state_t;

// WiFi network information
typedef struct {
    char ssid[33];               // Network name (max 32 chars + null)
    uint8_t bssid[6];            // MAC address of access point
    wifi_standard_t standard;    // WiFi standard
    wifi_security_t security;    // Security protocol
    wifi_auth_method_t auth;     // Authentication method
    wifi_band_t band;            // Frequency band
    wifi_channel_t channel;      // Channel number
    int8_t signal_strength;      // Signal strength in dBm
    uint8_t signal_quality;      // Signal quality (0-100)
    uint32_t max_rate;           // Maximum data rate in Mbps
    bool is_5ghz_capable;        // Supports 5 GHz
    bool is_6ghz_capable;        // Supports 6 GHz
    bool is_mesh_capable;        // Supports mesh networking
    bool is_enterprise;          // Enterprise network
    bool is_hidden;              // Hidden network
    uint32_t last_seen;          // Last time network was seen
} wifi_network_t;

// WiFi adapter information
typedef struct {
    uint32_t id;
    char name[64];               // Adapter name
    char driver[64];             // Driver name
    uint8_t mac_address[6];      // MAC address
    wifi_standard_t max_standard; // Maximum supported standard
    wifi_band_t supported_bands; // Supported frequency bands
    uint32_t max_rate;           // Maximum data rate
    bool is_5ghz_capable;        // Supports 5 GHz
    bool is_6ghz_capable;        // Supports 6 GHz
    bool is_mesh_capable;        // Supports mesh networking
    bool is_enterprise_capable;  // Supports enterprise features
    bool is_powered_on;          // Power state
    bool is_scanning;            // Currently scanning
    void* driver_data;           // Driver-specific data
} wifi_adapter_t;

// WiFi connection configuration
typedef struct {
    char ssid[33];               // Network SSID
    char password[65];           // Network password
    wifi_security_t security;    // Security protocol
    wifi_auth_method_t auth;     // Authentication method
    wifi_band_t preferred_band;  // Preferred frequency band
    wifi_channel_t preferred_channel; // Preferred channel
    bool auto_connect;           // Auto-connect when available
    bool hidden_network;         // Hidden network
    char identity[64];           // EAP identity (for enterprise)
    char ca_cert_path[256];      // CA certificate path
    char client_cert_path[256];  // Client certificate path
    char private_key_path[256];  // Private key path
    char username[64];           // Username (for enterprise)
    char domain[64];             // Domain (for enterprise)
} wifi_connection_config_t;

// WiFi mesh network configuration
typedef struct {
    char mesh_id[33];            // Mesh network ID
    char mesh_password[65];      // Mesh network password
    uint8_t mesh_key[32];        // Mesh encryption key
    uint32_t max_peers;          // Maximum number of peers
    bool is_gateway;             // This node is a gateway
    bool is_root;                // This node is the root
    uint32_t hop_limit;          // Maximum hop count
    float transmission_power;    // Transmission power in dBm
} wifi_mesh_config_t;

// WiFi system configuration
typedef struct {
    bool enable_wifi;            // Enable WiFi subsystem
    bool enable_auto_connect;    // Auto-connect to known networks
    bool enable_roaming;         // Enable network roaming
    bool enable_mesh;            // Enable mesh networking
    bool enable_enterprise;      // Enable enterprise features
    bool enable_wpa3;            // Enable WPA3 security
    bool enable_6ghz;            // Enable 6 GHz band
    uint32_t scan_interval;      // Network scan interval (seconds)
    uint32_t connection_timeout; // Connection timeout (seconds)
    uint32_t max_networks;       // Maximum saved networks
    float transmission_power;    // Default transmission power
    bool enable_power_save;      // Enable power saving
    bool enable_band_steering;   // Enable band steering
    bool enable_beamforming;     // Enable beamforming
} wifi_system_config_t;

// WiFi system context
typedef struct {
    wifi_system_config_t config;
    wifi_adapter_t* adapters;
    uint32_t adapter_count;
    uint32_t max_adapters;
    wifi_network_t* networks;
    uint32_t network_count;
    uint32_t max_networks;
    wifi_connection_config_t* saved_networks;
    uint32_t saved_network_count;
    uint32_t max_saved_networks;
    wifi_adapter_t* active_adapter;
    wifi_network_t* connected_network;
    wifi_state_t connection_state;
    wifi_mesh_config_t* mesh_config;
    bool initialized;
    uint64_t last_scan_time;
    uint32_t connection_attempts;
    uint32_t failed_connections;
} wifi_system_t;

// Function prototypes

// Initialization and shutdown
wifi_system_t* wifi_system_init(wifi_system_config_t* config);
void wifi_system_shutdown(wifi_system_t* wifi);
bool wifi_system_is_initialized(wifi_system_t* wifi);

// Adapter management
uint32_t wifi_system_add_adapter(wifi_system_t* wifi, wifi_adapter_t* adapter);
void wifi_system_remove_adapter(wifi_system_t* wifi, uint32_t adapter_id);
wifi_adapter_t* wifi_system_get_adapter(wifi_system_t* wifi, uint32_t adapter_id);
wifi_adapter_t* wifi_system_get_active_adapter(wifi_system_t* wifi);
void wifi_system_set_active_adapter(wifi_system_t* wifi, uint32_t adapter_id);
bool wifi_system_power_adapter(wifi_system_t* wifi, uint32_t adapter_id, bool power_on);

// Network scanning
bool wifi_system_start_scan(wifi_system_t* wifi);
bool wifi_system_stop_scan(wifi_system_t* wifi);
bool wifi_system_is_scanning(wifi_system_t* wifi);
wifi_network_t* wifi_system_get_networks(wifi_system_t* wifi, uint32_t* count);
wifi_network_t* wifi_system_find_network(wifi_system_t* wifi, const char* ssid);
void wifi_system_clear_networks(wifi_system_t* wifi);

// Connection management
bool wifi_system_connect(wifi_system_t* wifi, wifi_connection_config_t* config);
bool wifi_system_disconnect(wifi_system_t* wifi);
bool wifi_system_is_connected(wifi_system_t* wifi);
wifi_state_t wifi_system_get_connection_state(wifi_system_t* wifi);
wifi_network_t* wifi_system_get_connected_network(wifi_system_t* wifi);
int8_t wifi_system_get_signal_strength(wifi_system_t* wifi);
uint8_t wifi_system_get_signal_quality(wifi_system_t* wifi);
uint32_t wifi_system_get_data_rate(wifi_system_t* wifi);

// Saved networks
uint32_t wifi_system_save_network(wifi_system_t* wifi, wifi_connection_config_t* config);
void wifi_system_remove_saved_network(wifi_system_t* wifi, uint32_t network_id);
wifi_connection_config_t* wifi_system_get_saved_networks(wifi_system_t* wifi, uint32_t* count);
wifi_connection_config_t* wifi_system_get_saved_network(wifi_system_t* wifi, uint32_t network_id);
bool wifi_system_connect_saved_network(wifi_system_t* wifi, uint32_t network_id);

// Mesh networking
bool wifi_system_create_mesh(wifi_system_t* wifi, wifi_mesh_config_t* config);
bool wifi_system_join_mesh(wifi_system_t* wifi, wifi_mesh_config_t* config);
bool wifi_system_leave_mesh(wifi_system_t* wifi);
bool wifi_system_is_mesh_member(wifi_system_t* wifi);
uint32_t wifi_system_get_mesh_peer_count(wifi_system_t* wifi);
wifi_mesh_config_t* wifi_system_get_mesh_config(wifi_system_t* wifi);

// Security and authentication
bool wifi_system_set_security(wifi_system_t* wifi, wifi_security_t security);
wifi_security_t wifi_system_get_security(wifi_system_t* wifi);
bool wifi_system_set_auth_method(wifi_system_t* wifi, wifi_auth_method_t auth);
wifi_auth_method_t wifi_system_get_auth_method(wifi_system_t* wifi);
bool wifi_system_validate_password(wifi_system_t* wifi, const char* password);
bool wifi_system_generate_wpa3_key(wifi_system_t* wifi, const char* password, uint8_t* key);

// Band and channel management
bool wifi_system_set_band(wifi_system_t* wifi, wifi_band_t band);
wifi_band_t wifi_system_get_band(wifi_system_t* wifi);
bool wifi_system_set_channel(wifi_system_t* wifi, wifi_channel_t channel);
wifi_channel_t wifi_system_get_channel(wifi_system_t* wifi);
wifi_channel_t* wifi_system_get_available_channels(wifi_system_t* wifi, wifi_band_t band, uint32_t* count);
bool wifi_system_is_channel_available(wifi_system_t* wifi, wifi_channel_t channel);

// Performance and monitoring
uint32_t wifi_system_get_connection_attempts(wifi_system_t* wifi);
uint32_t wifi_system_get_failed_connections(wifi_system_t* wifi);
float wifi_system_get_connection_success_rate(wifi_system_t* wifi);
uint64_t wifi_system_get_uptime(wifi_system_t* wifi);
uint32_t wifi_system_get_bytes_sent(wifi_system_t* wifi);
uint32_t wifi_system_get_bytes_received(wifi_system_t* wifi);
float wifi_system_get_packet_loss_rate(wifi_system_t* wifi);

// Power management
bool wifi_system_enable_power_save(wifi_system_t* wifi, bool enable);
bool wifi_system_is_power_save_enabled(wifi_system_t* wifi);
void wifi_system_set_transmission_power(wifi_system_t* wifi, float power_dbm);
float wifi_system_get_transmission_power(wifi_system_t* wifi);
bool wifi_system_set_power_mode(wifi_system_t* wifi, uint32_t mode);

// Advanced features
bool wifi_system_enable_beamforming(wifi_system_t* wifi, bool enable);
bool wifi_system_is_beamforming_enabled(wifi_system_t* wifi);
bool wifi_system_enable_band_steering(wifi_system_t* wifi, bool enable);
bool wifi_system_is_band_steering_enabled(wifi_system_t* wifi);
bool wifi_system_enable_roaming(wifi_system_t* wifi, bool enable);
bool wifi_system_is_roaming_enabled(wifi_system_t* wifi);

// Enterprise features
bool wifi_system_configure_enterprise(wifi_system_t* wifi, const char* identity, 
                                     const char* ca_cert, const char* client_cert, 
                                     const char* private_key);
bool wifi_system_set_enterprise_credentials(wifi_system_t* wifi, const char* username, 
                                           const char* password, const char* domain);
bool wifi_system_validate_enterprise_config(wifi_system_t* wifi);

// Utility functions
bool wifi_system_is_ssid_valid(const char* ssid);
bool wifi_system_is_password_valid(const char* password, wifi_security_t security);
uint32_t wifi_system_calculate_signal_quality(int8_t signal_strength);
const char* wifi_system_get_security_name(wifi_security_t security);
const char* wifi_system_get_standard_name(wifi_standard_t standard);
const char* wifi_system_get_band_name(wifi_band_t band);
uint32_t wifi_system_get_channel_frequency(wifi_channel_t channel);
wifi_band_t wifi_system_get_channel_band(wifi_channel_t channel);

// Error handling
typedef enum {
    WIFI_SYSTEM_SUCCESS = 0,
    WIFI_SYSTEM_ERROR_INVALID_CONTEXT,
    WIFI_SYSTEM_ERROR_INVALID_ADAPTER,
    WIFI_SYSTEM_ERROR_INVALID_NETWORK,
    WIFI_SYSTEM_ERROR_INVALID_CONFIG,
    WIFI_SYSTEM_ERROR_DRIVER_FAILED,
    WIFI_SYSTEM_ERROR_OUT_OF_MEMORY,
    WIFI_SYSTEM_ERROR_AUTHENTICATION_FAILED,
    WIFI_SYSTEM_ERROR_CONNECTION_FAILED,
    WIFI_SYSTEM_ERROR_TIMEOUT,
    WIFI_SYSTEM_ERROR_UNSUPPORTED_FEATURE,
    WIFI_SYSTEM_ERROR_INVALID_SECURITY,
    WIFI_SYSTEM_ERROR_SCAN_FAILED,
    WIFI_SYSTEM_ERROR_MESH_FAILED,
} wifi_system_error_t;

wifi_system_error_t wifi_system_get_last_error(void);
const char* wifi_system_get_error_string(wifi_system_error_t error);

#endif // WIFI_SYSTEM_H 