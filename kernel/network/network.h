#ifndef RAEENOS_NETWORK_H
#define RAEENOS_NETWORK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"

// Network protocol types
typedef enum {
    PROTO_ICMP = 1,
    PROTO_TCP = 6,
    PROTO_UDP = 17
} protocol_t;

// Network interface types
typedef enum {
    IF_TYPE_ETHERNET = 1,
    IF_TYPE_LOOPBACK = 24,
    IF_TYPE_WIFI = 71
} interface_type_t;

// Network interface states
typedef enum {
    IF_STATE_DOWN = 0,
    IF_STATE_UP = 1,
    IF_STATE_RUNNING = 2
} interface_state_t;

// IP address structure
typedef struct ip_addr {
    uint32_t addr;              // IPv4 address in network byte order
} ip_addr_t;

// MAC address structure
typedef struct mac_addr {
    uint8_t addr[6];            // MAC address
} mac_addr_t;

// Network interface structure
typedef struct network_interface {
    char name[16];              // Interface name (e.g., "eth0")
    interface_type_t type;      // Interface type
    interface_state_t state;    // Interface state
    ip_addr_t ip_addr;          // IP address
    ip_addr_t netmask;          // Subnet mask
    ip_addr_t gateway;          // Default gateway
    mac_addr_t mac_addr;        // MAC address
    uint32_t mtu;               // Maximum transmission unit
    uint32_t flags;             // Interface flags
    uint64_t rx_packets;        // Received packets
    uint64_t tx_packets;        // Transmitted packets
    uint64_t rx_bytes;          // Received bytes
    uint64_t tx_bytes;          // Transmitted bytes
    uint64_t rx_errors;         // Receive errors
    uint64_t tx_errors;         // Transmit errors
    struct network_interface* next; // Next interface
} network_interface_t;

// Socket types
typedef enum {
    SOCK_STREAM = 1,            // TCP socket
    SOCK_DGRAM = 2,             // UDP socket
    SOCK_RAW = 3                // Raw socket
} socket_type_t;

// Socket states
typedef enum {
    SOCK_STATE_CLOSED = 0,
    SOCK_STATE_LISTEN = 1,
    SOCK_STATE_SYN_SENT = 2,
    SOCK_STATE_SYN_RECEIVED = 3,
    SOCK_STATE_ESTABLISHED = 4,
    SOCK_STATE_FIN_WAIT1 = 5,
    SOCK_STATE_FIN_WAIT2 = 6,
    SOCK_STATE_CLOSE_WAIT = 7,
    SOCK_STATE_CLOSING = 8,
    SOCK_STATE_LAST_ACK = 9,
    SOCK_STATE_TIME_WAIT = 10
} socket_state_t;

// Socket structure
typedef struct socket {
    socket_type_t type;         // Socket type
    socket_state_t state;       // Socket state
    protocol_t protocol;        // Protocol
    ip_addr_t local_addr;       // Local address
    uint16_t local_port;        // Local port
    ip_addr_t remote_addr;      // Remote address
    uint16_t remote_port;       // Remote port
    network_interface_t* interface; // Associated interface
    void* protocol_data;        // Protocol-specific data
    uint32_t flags;             // Socket flags
    uint64_t rx_bytes;          // Received bytes
    uint64_t tx_bytes;          // Transmitted bytes
    struct socket* next;        // Next socket
} socket_t;

// TCP connection structure
typedef struct tcp_connection {
    socket_t* socket;           // Associated socket
    uint32_t seq_num;           // Sequence number
    uint32_t ack_num;           // Acknowledgment number
    uint16_t window_size;       // Window size
    uint32_t snd_una;           // Oldest unacknowledged sequence number
    uint32_t snd_nxt;           // Next sequence number to send
    uint32_t snd_wnd;           // Send window
    uint32_t rcv_nxt;           // Next sequence number expected
    uint32_t rcv_wnd;           // Receive window
    uint32_t ssthresh;          // Slow start threshold
    uint32_t cwnd;              // Congestion window
    uint32_t rtt;               // Round trip time
    uint32_t srtt;              // Smoothed round trip time
    uint32_t rttvar;            // Round trip time variation
    uint32_t rto;               // Retransmission timeout
    uint32_t retransmit_count;  // Number of retransmissions
    uint64_t last_activity;     // Last activity timestamp
    struct tcp_connection* next; // Next connection
} tcp_connection_t;

// Network packet structure
typedef struct network_packet {
    void* data;                 // Packet data
    size_t size;                // Packet size
    network_interface_t* interface; // Source interface
    ip_addr_t src_addr;         // Source address
    ip_addr_t dst_addr;         // Destination address
    protocol_t protocol;        // Protocol
    uint16_t src_port;          // Source port
    uint16_t dst_port;          // Destination port
    uint32_t flags;             // Packet flags
    struct network_packet* next; // Next packet
} network_packet_t;

// Network buffer structure
typedef struct network_buffer {
    void* data;                 // Buffer data
    size_t size;                // Buffer size
    size_t offset;              // Current offset
    size_t length;              // Data length
    struct network_buffer* next; // Next buffer
} network_buffer_t;

// Network statistics
typedef struct network_stats {
    uint64_t packets_sent;      // Total packets sent
    uint64_t packets_received;  // Total packets received
    uint64_t bytes_sent;        // Total bytes sent
    uint64_t bytes_received;    // Total bytes received
    uint64_t errors;            // Total errors
    uint64_t collisions;        // Total collisions
    uint64_t dropped_packets;   // Dropped packets
} network_stats_t;

// Function prototypes

// Network initialization
error_t network_init(void);
void network_shutdown(void);

// Interface management
network_interface_t* network_interface_create(const char* name, interface_type_t type);
void network_interface_destroy(network_interface_t* interface);
int network_interface_register(network_interface_t* interface);
int network_interface_unregister(network_interface_t* interface);
network_interface_t* network_interface_find(const char* name);
network_interface_t* network_interface_find_by_addr(ip_addr_t addr);

// Interface operations
int network_interface_up(network_interface_t* interface);
int network_interface_down(network_interface_t* interface);
int network_interface_set_addr(network_interface_t* interface, ip_addr_t addr, ip_addr_t netmask);
int network_interface_set_gateway(network_interface_t* interface, ip_addr_t gateway);

// Socket management
socket_t* socket_create(socket_type_t type, protocol_t protocol);
void socket_destroy(socket_t* socket);
int socket_bind(socket_t* socket, ip_addr_t addr, uint16_t port);
int socket_listen(socket_t* socket, int backlog);
socket_t* socket_accept(socket_t* socket, ip_addr_t* addr, uint16_t* port);
int socket_connect(socket_t* socket, ip_addr_t addr, uint16_t port);
int socket_close(socket_t* socket);

// Socket I/O
ssize_t socket_send(socket_t* socket, const void* data, size_t size);
ssize_t socket_recv(socket_t* socket, void* data, size_t size);
ssize_t socket_sendto(socket_t* socket, const void* data, size_t size, ip_addr_t addr, uint16_t port);
ssize_t socket_recvfrom(socket_t* socket, void* data, size_t size, ip_addr_t* addr, uint16_t* port);

// TCP operations
tcp_connection_t* tcp_connection_create(socket_t* socket);
void tcp_connection_destroy(tcp_connection_t* conn);
int tcp_connection_connect(tcp_connection_t* conn, ip_addr_t addr, uint16_t port);
int tcp_connection_listen(tcp_connection_t* conn, int backlog);
tcp_connection_t* tcp_connection_accept(tcp_connection_t* conn);
int tcp_connection_close(tcp_connection_t* conn);
ssize_t tcp_connection_send(tcp_connection_t* conn, const void* data, size_t size);
ssize_t tcp_connection_recv(tcp_connection_t* conn, void* data, size_t size);

// IP operations
int ip_send_packet(network_interface_t* interface, ip_addr_t dst_addr, protocol_t protocol, 
                   const void* data, size_t size);
int ip_receive_packet(network_interface_t* interface, const void* data, size_t size);
ip_addr_t ip_route_packet(ip_addr_t dst_addr);
bool ip_is_local_addr(ip_addr_t addr);
bool ip_is_broadcast_addr(ip_addr_t addr);

// Address operations
ip_addr_t ip_addr_make(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
ip_addr_t ip_addr_from_string(const char* str);
void ip_addr_to_string(ip_addr_t addr, char* str);
bool ip_addr_equal(ip_addr_t addr1, ip_addr_t addr2);
ip_addr_t ip_addr_network(ip_addr_t addr, ip_addr_t netmask);
ip_addr_t ip_addr_broadcast(ip_addr_t addr, ip_addr_t netmask);

// MAC address operations
void mac_addr_copy(mac_addr_t* dst, const mac_addr_t* src);
bool mac_addr_equal(const mac_addr_t* addr1, const mac_addr_t* addr2);
bool mac_addr_is_broadcast(const mac_addr_t* addr);
bool mac_addr_is_multicast(const mac_addr_t* addr);

// Network buffer operations
network_buffer_t* network_buffer_create(size_t size);
void network_buffer_destroy(network_buffer_t* buffer);
int network_buffer_write(network_buffer_t* buffer, const void* data, size_t size);
int network_buffer_read(network_buffer_t* buffer, void* data, size_t size);
void network_buffer_reset(network_buffer_t* buffer);

// Packet operations
network_packet_t* network_packet_create(size_t size);
void network_packet_destroy(network_packet_t* packet);
int network_packet_send(network_packet_t* packet);
int network_packet_receive(network_packet_t* packet);

// Network utilities
uint16_t network_checksum(const void* data, size_t size);
uint16_t network_htons(uint16_t hostshort);
uint32_t network_htonl(uint32_t hostlong);
uint16_t network_ntohs(uint16_t netshort);
uint32_t network_ntohl(uint32_t netlong);

// Network statistics
void network_get_stats(network_stats_t* stats);
void network_reset_stats(void);
void network_interface_get_stats(network_interface_t* interface, network_stats_t* stats);

// Network debugging
void network_dump_interfaces(void);
void network_dump_sockets(void);
void network_dump_connections(void);
void network_dump_packet(const network_packet_t* packet);

// Network configuration
int network_set_default_gateway(ip_addr_t gateway);
ip_addr_t network_get_default_gateway(void);
int network_add_route(ip_addr_t network, ip_addr_t netmask, ip_addr_t gateway);
int network_remove_route(ip_addr_t network, ip_addr_t netmask);

// Network timing
void network_timer_tick(void);
uint64_t network_get_timestamp(void);

// Network protocols
int icmp_send_echo_request(ip_addr_t dst_addr);
int icmp_handle_packet(const void* data, size_t size, ip_addr_t src_addr);
int tcp_handle_packet(const void* data, size_t size, ip_addr_t src_addr, ip_addr_t dst_addr);
int udp_handle_packet(const void* data, size_t size, ip_addr_t src_addr, ip_addr_t dst_addr);

// Network constants
#define IP_ADDR_ANY         0x00000000
#define IP_ADDR_LOOPBACK    0x7F000001
#define IP_ADDR_BROADCAST   0xFFFFFFFF

#define MAC_ADDR_BROADCAST  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

#define TCP_PORT_ANY        0
#define TCP_PORT_ECHO       7
#define TCP_PORT_FTP        21
#define TCP_PORT_SSH        22
#define TCP_PORT_TELNET     23
#define TCP_PORT_SMTP       25
#define TCP_PORT_HTTP       80
#define TCP_PORT_HTTPS      443

#define UDP_PORT_ANY        0
#define UDP_PORT_ECHO       7
#define UDP_PORT_DNS        53
#define UDP_PORT_DHCP_CLIENT 68
#define UDP_PORT_DHCP_SERVER 67

#define NETWORK_MTU_DEFAULT 1500
#define NETWORK_TCP_MSS     1460
#define NETWORK_TCP_WINDOW  65535

#define NETWORK_TIMEOUT_DEFAULT 5000  // 5 seconds
#define NETWORK_RETRY_MAX    3

#endif // RAEENOS_NETWORK_H