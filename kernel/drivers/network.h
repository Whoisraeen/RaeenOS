#ifndef RAEENOS_NETWORK_H
#define RAEENOS_NETWORK_H

#include "types.h"

// Network constants
#define MAX_NETWORK_INTERFACES 16
#define MAX_NETWORK_PACKETS 1024
#define MAX_PACKET_SIZE 1518
#define MAX_MAC_ADDRESS_LEN 6
#define MAX_IP_ADDRESS_LEN 4

// Network interface types
typedef enum {
    NETWORK_INTERFACE_UNKNOWN = 0,
    NETWORK_INTERFACE_ETHERNET = 1,
    NETWORK_INTERFACE_WIFI = 2,
    NETWORK_INTERFACE_LOOPBACK = 3
} network_interface_type_t;

// Network interface states
typedef enum {
    NETWORK_INTERFACE_DOWN = 0,
    NETWORK_INTERFACE_UP = 1,
    NETWORK_INTERFACE_RUNNING = 2
} network_interface_state_t;

// IP address structure
typedef struct {
    uint8_t bytes[4];
} ip_address_t;

// MAC address structure
typedef struct {
    uint8_t bytes[6];
} mac_address_t;

// Network statistics
typedef struct {
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t errors;
    uint64_t collisions;
} network_stats_t;

// Network packet structure
typedef struct network_packet {
    void* data;
    size_t size;
    size_t length;
    network_interface_t* interface;
    struct network_packet* next;
} network_packet_t;

// Network interface structure
typedef struct {
    char name[16];
    int bus;
    int device;
    int function;
    uint16_t vendor_id;
    uint16_t device_id;
    network_interface_type_t type;
    network_interface_state_t state;
    uintptr_t base_address;
    int irq;
    mac_address_t mac_address;
    ip_address_t ip_address;
    ip_address_t netmask;
    ip_address_t gateway;
    network_stats_t stats;
    void* private_data;
} network_interface_t;

// Network driver functions
error_t network_init(void);
void network_shutdown(void);
error_t network_scan_controllers(void);
bool network_is_controller(uint16_t vendor_id, uint16_t device_id);
network_interface_t* network_interface_create(int bus, int device, int function, uint16_t vendor_id, uint16_t device_id);
network_interface_type_t network_determine_interface_type(uint16_t vendor_id, uint16_t device_id);
uintptr_t network_get_interface_base_address(int bus, int device, int function);
int network_get_interface_irq(int bus, int device, int function);
error_t network_interface_init(network_interface_t* interface);
void network_interface_shutdown(network_interface_t* interface);

// Ethernet functions
error_t network_ethernet_init(network_interface_t* interface);
void network_ethernet_shutdown(network_interface_t* interface);
error_t network_ethernet_send(network_interface_t* interface, network_packet_t* packet);
error_t network_ethernet_receive(network_interface_t* interface, network_packet_t** packet);

// WiFi functions
error_t network_wifi_init(network_interface_t* interface);
void network_wifi_shutdown(network_interface_t* interface);
error_t network_wifi_send(network_interface_t* interface, network_packet_t* packet);
error_t network_wifi_receive(network_interface_t* interface, network_packet_t** packet);

// Packet management
network_packet_t* network_packet_create(size_t size);
void network_packet_free(network_packet_t* packet);

// Interface management
error_t network_add_interface(network_interface_t* interface);
error_t network_remove_interface(const char* name);
network_interface_t* network_find_interface(const char* name);
error_t network_get_interfaces(network_interface_t** interface_list, int* count);

// Packet transmission
error_t network_send_packet(network_interface_t* interface, network_packet_t* packet);
error_t network_receive_packet(network_interface_t* interface, network_packet_t** packet);

// Interface control
error_t network_interface_up(network_interface_t* interface);
error_t network_interface_down(network_interface_t* interface);
error_t network_set_mac_address(network_interface_t* interface, const uint8_t* mac);
error_t network_get_mac_address(network_interface_t* interface, uint8_t* mac);
error_t network_set_ip_address(network_interface_t* interface, const ip_address_t* ip);
error_t network_get_ip_address(network_interface_t* interface, ip_address_t* ip);

// Statistics
void network_get_interface_stats(network_interface_t* interface, network_stats_t* stats);

// Utility functions
bool network_is_initialized(void);
int network_get_interface_count(void);

// IP address utilities
ip_address_t ip_make_address(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
bool ip_is_valid(const ip_address_t* ip);
bool ip_is_loopback(const ip_address_t* ip);
bool ip_is_broadcast(const ip_address_t* ip);
bool ip_is_multicast(const ip_address_t* ip);

// MAC address utilities
mac_address_t mac_make_address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f);
bool mac_is_broadcast(const mac_address_t* mac);
bool mac_is_multicast(const mac_address_t* mac);

#endif // RAEENOS_NETWORK_H 