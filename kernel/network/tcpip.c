#include "network/include/network.h"
#include "kernel.h"
#include "memory/memory.h"
#include <string.h>

// TCP/IP configuration
#define MAX_SOCKETS 256
#define TCP_WINDOW_SIZE 65535
#define TCP_MAX_SEGMENT_SIZE 1460
#define TCP_TIMEOUT_MS 5000
#define TCP_RETRANSMIT_ATTEMPTS 3

// TCP flags
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

// TCP states
typedef enum {
    TCP_CLOSED,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,
    TCP_FIN_WAIT_2,
    TCP_CLOSE_WAIT,
    TCP_CLOSING,
    TCP_LAST_ACK,
    TCP_TIME_WAIT
} tcp_state_t;

// TCP header structure
typedef struct {
    u16 source_port;
    u16 dest_port;
    u32 sequence_number;
    u32 acknowledgment_number;
    u8 data_offset;
    u8 flags;
    u16 window_size;
    u16 checksum;
    u16 urgent_pointer;
} __attribute__((packed)) tcp_header_t;

// TCP connection structure
typedef struct {
    u32 local_ip;
    u16 local_port;
    u32 remote_ip;
    u16 remote_port;
    tcp_state_t state;
    u32 sequence_number;
    u32 acknowledgment_number;
    u16 window_size;
    u32 timeout;
    u32 retransmit_count;
    u32 last_activity;
    void* receive_buffer;
    u32 receive_buffer_size;
    u32 receive_buffer_head;
    u32 receive_buffer_tail;
    void* send_buffer;
    u32 send_buffer_size;
    u32 send_buffer_head;
    u32 send_buffer_tail;
    bool active;
} tcp_connection_t;

// Socket structure
typedef struct {
    int type;
    int protocol;
    tcp_connection_t* connection;
    bool active;
} socket_t;

// Network interface structure
typedef struct {
    char name[32];
    u32 ip_address;
    u32 netmask;
    u32 gateway;
    u32 mtu;
    bool active;
    void* driver_data;
} network_interface_t;

// Global networking state
static socket_t sockets[MAX_SOCKETS];
static tcp_connection_t tcp_connections[MAX_SOCKETS];
static network_interface_t network_interfaces[4];
static u32 num_interfaces = 0;
static u32 next_port = 1024;
static bool networking_initialized = false;

// Forward declarations
static error_t tcp_send_segment(tcp_connection_t* conn, u8 flags, const void* data, u32 length);
static error_t tcp_handle_segment(tcp_connection_t* conn, tcp_header_t* header, const void* data, u32 length);
static error_t tcp_establish_connection(tcp_connection_t* conn);
static error_t tcp_close_connection(tcp_connection_t* conn);
static tcp_connection_t* tcp_find_connection(u32 local_ip, u16 local_port, u32 remote_ip, u16 remote_port);
static u16 tcp_allocate_port(void);
static u16 tcp_calculate_checksum(const void* data, u32 length, u32 source_ip, u32 dest_ip);
static socket_t* socket_get(int fd);
static int socket_alloc(void);
static void socket_free(int fd);

// Initialize networking stack
error_t networking_init(void) {
    if (networking_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing TCP/IP networking stack");
    
    // Initialize socket table
    memset(sockets, 0, sizeof(sockets));
    for (u32 i = 0; i < MAX_SOCKETS; i++) {
        sockets[i].active = false;
    }
    
    // Initialize TCP connections
    memset(tcp_connections, 0, sizeof(tcp_connections));
    for (u32 i = 0; i < MAX_SOCKETS; i++) {
        tcp_connections[i].active = false;
    }
    
    // Initialize network interfaces
    memset(network_interfaces, 0, sizeof(network_interfaces));
    
    // Set up loopback interface
    network_interfaces[0].ip_address = 0x7F000001; // 127.0.0.1
    network_interfaces[0].netmask = 0xFF000000;    // 255.0.0.0
    network_interfaces[0].gateway = 0x7F000001;    // 127.0.0.1
    network_interfaces[0].mtu = 1500;
    network_interfaces[0].active = true;
    strcpy(network_interfaces[0].name, "lo");
    num_interfaces = 1;
    
    networking_initialized = true;
    
    KINFO("Networking stack initialized with %u interfaces", num_interfaces);
    return SUCCESS;
}

// Create a socket
int socket_create(int domain, int type, int protocol) {
    if (!networking_initialized) {
        return -E_NOTINIT;
    }
    
    if (domain != AF_INET) {
        return -E_AFNOSUPPORT;
    }
    
    if (type != SOCK_STREAM && type != SOCK_DGRAM) {
        return -E_SOCKTNOSUPPORT;
    }
    
    KDEBUG("Creating socket: domain=%d, type=%d, protocol=%d", domain, type, protocol);
    
    int fd = socket_alloc();
    if (fd < 0) {
        return -E_NOMEM;
    }
    
    socket_t* sock = socket_get(fd);
    sock->type = type;
    sock->protocol = protocol;
    sock->active = true;
    
    // Initialize TCP connection for stream sockets
    if (type == SOCK_STREAM) {
        tcp_connection_t* conn = &tcp_connections[fd];
        conn->state = TCP_CLOSED;
        conn->window_size = TCP_WINDOW_SIZE;
        conn->receive_buffer_size = TCP_WINDOW_SIZE;
        conn->send_buffer_size = TCP_WINDOW_SIZE;
        conn->receive_buffer = memory_alloc(conn->receive_buffer_size);
        conn->send_buffer = memory_alloc(conn->send_buffer_size);
        conn->active = true;
        sock->connection = conn;
    }
    
    KDEBUG("Created socket: fd=%d", fd);
    return fd;
}

// Bind a socket to an address
error_t socket_bind(int fd, const struct sockaddr* addr, socklen_t addrlen) {
    if (fd < 0 || fd >= MAX_SOCKETS || !addr) {
        return E_INVAL;
    }
    
    socket_t* sock = socket_get(fd);
    if (!sock || !sock->active) {
        return E_BADF;
    }
    
    if (addr->sa_family != AF_INET) {
        return E_AFNOSUPPORT;
    }
    
    struct sockaddr_in* addr_in = (struct sockaddr_in*)addr;
    u32 ip = addr_in->sin_addr.s_addr;
    u16 port = addr_in->sin_port;
    
    KDEBUG("Binding socket %d to %u.%u.%u.%u:%u", fd,
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF,
           ntohs(port));
    
    if (sock->type == SOCK_STREAM && sock->connection) {
        sock->connection->local_ip = ip;
        sock->connection->local_port = port;
    }
    
    return SUCCESS;
}

// Listen for connections
error_t socket_listen(int fd, int backlog) {
    if (fd < 0 || fd >= MAX_SOCKETS) {
        return E_INVAL;
    }
    
    socket_t* sock = socket_get(fd);
    if (!sock || !sock->active || sock->type != SOCK_STREAM) {
        return E_BADF;
    }
    
    if (!sock->connection) {
        return E_INVAL;
    }
    
    KDEBUG("Socket %d listening with backlog %d", fd, backlog);
    
    sock->connection->state = TCP_LISTEN;
    
    return SUCCESS;
}

// Accept a connection
int socket_accept(int fd, struct sockaddr* addr, socklen_t* addrlen) {
    if (fd < 0 || fd >= MAX_SOCKETS) {
        return -E_INVAL;
    }
    
    socket_t* sock = socket_get(fd);
    if (!sock || !sock->active || sock->type != SOCK_STREAM) {
        return -E_BADF;
    }
    
    if (!sock->connection || sock->connection->state != TCP_LISTEN) {
        return -E_INVAL;
    }
    
    KDEBUG("Accepting connection on socket %d", fd);
    
    // Wait for incoming connection
    // This is a simplified implementation
    // In a real system, this would wait for SYN packets
    
    // Create new socket for accepted connection
    int new_fd = socket_create(AF_INET, SOCK_STREAM, 0);
    if (new_fd < 0) {
        return new_fd;
    }
    
    socket_t* new_sock = socket_get(new_fd);
    tcp_connection_t* new_conn = new_sock->connection;
    
    // Set up connection
    new_conn->local_ip = sock->connection->local_ip;
    new_conn->local_port = sock->connection->local_port;
    new_conn->state = TCP_ESTABLISHED;
    
    // Fill in client address if provided
    if (addr && addrlen) {
        struct sockaddr_in* addr_in = (struct sockaddr_in*)addr;
        addr_in->sin_family = AF_INET;
        addr_in->sin_addr.s_addr = new_conn->remote_ip;
        addr_in->sin_port = new_conn->remote_port;
        *addrlen = sizeof(struct sockaddr_in);
    }
    
    KDEBUG("Accepted connection: fd=%d", new_fd);
    return new_fd;
}

// Connect to a remote address
error_t socket_connect(int fd, const struct sockaddr* addr, socklen_t addrlen) {
    if (fd < 0 || fd >= MAX_SOCKETS || !addr) {
        return E_INVAL;
    }
    
    socket_t* sock = socket_get(fd);
    if (!sock || !sock->active || sock->type != SOCK_STREAM) {
        return E_BADF;
    }
    
    if (addr->sa_family != AF_INET) {
        return E_AFNOSUPPORT;
    }
    
    struct sockaddr_in* addr_in = (struct sockaddr_in*)addr;
    u32 remote_ip = addr_in->sin_addr.s_addr;
    u16 remote_port = addr_in->sin_port;
    
    KDEBUG("Connecting socket %d to %u.%u.%u.%u:%u", fd,
           (remote_ip >> 24) & 0xFF, (remote_ip >> 16) & 0xFF, 
           (remote_ip >> 8) & 0xFF, remote_ip & 0xFF, ntohs(remote_port));
    
    if (!sock->connection) {
        return E_INVAL;
    }
    
    // Set up connection parameters
    sock->connection->remote_ip = remote_ip;
    sock->connection->remote_port = remote_port;
    
    // Allocate local port if not bound
    if (sock->connection->local_port == 0) {
        sock->connection->local_port = tcp_allocate_port();
    }
    
    // Establish connection
    error_t result = tcp_establish_connection(sock->connection);
    if (result != SUCCESS) {
        KERROR("Failed to establish TCP connection");
        return result;
    }
    
    return SUCCESS;
}

// Send data
ssize_t socket_send(int fd, const void* buffer, size_t length, int flags) {
    if (fd < 0 || fd >= MAX_SOCKETS || !buffer) {
        return -E_INVAL;
    }
    
    socket_t* sock = socket_get(fd);
    if (!sock || !sock->active) {
        return -E_BADF;
    }
    
    if (sock->type == SOCK_STREAM) {
        if (!sock->connection || sock->connection->state != TCP_ESTABLISHED) {
            return -E_NOTCONN;
        }
        
        // Add data to send buffer
        tcp_connection_t* conn = sock->connection;
        u32 available = conn->send_buffer_size - 
                       ((conn->send_buffer_tail - conn->send_buffer_head) % conn->send_buffer_size);
        
        if (length > available) {
            return -E_NOMEM;
        }
        
        // Copy data to send buffer
        u8* buf = (u8*)conn->send_buffer;
        for (u32 i = 0; i < length; i++) {
            buf[(conn->send_buffer_tail + i) % conn->send_buffer_size] = ((u8*)buffer)[i];
        }
        conn->send_buffer_tail = (conn->send_buffer_tail + length) % conn->send_buffer_size;
        
        // Send data
        error_t result = tcp_send_segment(conn, TCP_PSH | TCP_ACK, buffer, length);
        if (result != SUCCESS) {
            return -result;
        }
        
        return length;
    } else {
        // UDP implementation would go here
        return -E_NOSYS;
    }
}

// Receive data
ssize_t socket_recv(int fd, void* buffer, size_t length, int flags) {
    if (fd < 0 || fd >= MAX_SOCKETS || !buffer) {
        return -E_INVAL;
    }
    
    socket_t* sock = socket_get(fd);
    if (!sock || !sock->active) {
        return -E_BADF;
    }
    
    if (sock->type == SOCK_STREAM) {
        if (!sock->connection || sock->connection->state != TCP_ESTABLISHED) {
            return -E_NOTCONN;
        }
        
        // Get data from receive buffer
        tcp_connection_t* conn = sock->connection;
        u32 available = (conn->receive_buffer_tail - conn->receive_buffer_head) % conn->receive_buffer_size;
        
        if (available == 0) {
            return 0; // No data available
        }
        
        if (length > available) {
            length = available;
        }
        
        // Copy data from receive buffer
        u8* buf = (u8*)conn->receive_buffer;
        for (u32 i = 0; i < length; i++) {
            ((u8*)buffer)[i] = buf[(conn->receive_buffer_head + i) % conn->receive_buffer_size];
        }
        conn->receive_buffer_head = (conn->receive_buffer_head + length) % conn->receive_buffer_size;
        
        return length;
    } else {
        // UDP implementation would go here
        return -E_NOSYS;
    }
}

// Close a socket
error_t socket_close(int fd) {
    if (fd < 0 || fd >= MAX_SOCKETS) {
        return E_INVAL;
    }
    
    socket_t* sock = socket_get(fd);
    if (!sock || !sock->active) {
        return E_BADF;
    }
    
    KDEBUG("Closing socket %d", fd);
    
    if (sock->type == SOCK_STREAM && sock->connection) {
        tcp_close_connection(sock->connection);
        
        // Free buffers
        if (sock->connection->receive_buffer) {
            memory_free(sock->connection->receive_buffer);
        }
        if (sock->connection->send_buffer) {
            memory_free(sock->connection->send_buffer);
        }
    }
    
    socket_free(fd);
    return SUCCESS;
}

// Send TCP segment
static error_t tcp_send_segment(tcp_connection_t* conn, u8 flags, const void* data, u32 length) {
    if (!conn || !conn->active) {
        return E_INVAL;
    }
    
    // Allocate buffer for TCP segment
    u32 total_length = sizeof(tcp_header_t) + length;
    void* segment = memory_alloc(total_length);
    if (!segment) {
        return E_NOMEM;
    }
    
    // Fill TCP header
    tcp_header_t* header = (tcp_header_t*)segment;
    header->source_port = conn->local_port;
    header->dest_port = conn->remote_port;
    header->sequence_number = htonl(conn->sequence_number);
    header->acknowledgment_number = htonl(conn->acknowledgment_number);
    header->data_offset = (sizeof(tcp_header_t) / 4) << 4;
    header->flags = flags;
    header->window_size = htons(conn->window_size);
    header->checksum = 0;
    header->urgent_pointer = 0;
    
    // Copy data
    if (data && length > 0) {
        memcpy((u8*)segment + sizeof(tcp_header_t), data, length);
    }
    
    // Calculate checksum
    header->checksum = tcp_calculate_checksum(segment, total_length, conn->local_ip, conn->remote_ip);
    
    // Send via IP layer (simplified)
    // In a real implementation, this would call the IP layer
    KDEBUG("Sending TCP segment: flags=0x%x, length=%u", flags, length);
    
    // Update sequence number
    if (data && length > 0) {
        conn->sequence_number += length;
    }
    if (flags & TCP_SYN) {
        conn->sequence_number++;
    }
    if (flags & TCP_FIN) {
        conn->sequence_number++;
    }
    
    memory_free(segment);
    return SUCCESS;
}

// Handle incoming TCP segment
static error_t tcp_handle_segment(tcp_connection_t* conn, tcp_header_t* header, const void* data, u32 length) {
    if (!conn || !header) {
        return E_INVAL;
    }
    
    u32 seq = ntohl(header->sequence_number);
    u32 ack = ntohl(header->acknowledgment_number);
    u8 flags = header->flags;
    
    KDEBUG("Received TCP segment: flags=0x%x, seq=%u, ack=%u, length=%u", 
           flags, seq, ack, length);
    
    // Handle different TCP states
    switch (conn->state) {
        case TCP_LISTEN:
            if (flags & TCP_SYN) {
                // Accept connection
                conn->remote_ip = 0; // Would be set from IP header
                conn->remote_port = header->source_port;
                conn->acknowledgment_number = seq + 1;
                conn->sequence_number = 0x12345678; // Random initial sequence number
                conn->state = TCP_SYN_RECEIVED;
                
                // Send SYN+ACK
                tcp_send_segment(conn, TCP_SYN | TCP_ACK, NULL, 0);
            }
            break;
            
        case TCP_SYN_SENT:
            if ((flags & (TCP_SYN | TCP_ACK)) == (TCP_SYN | TCP_ACK)) {
                // Connection established
                conn->acknowledgment_number = seq + 1;
                conn->state = TCP_ESTABLISHED;
                
                // Send ACK
                tcp_send_segment(conn, TCP_ACK, NULL, 0);
            }
            break;
            
        case TCP_SYN_RECEIVED:
            if (flags & TCP_ACK) {
                // Connection established
                conn->state = TCP_ESTABLISHED;
            }
            break;
            
        case TCP_ESTABLISHED:
            if (flags & TCP_FIN) {
                // Remote closing
                conn->acknowledgment_number = seq + 1;
                conn->state = TCP_CLOSE_WAIT;
                
                // Send ACK
                tcp_send_segment(conn, TCP_ACK, NULL, 0);
            } else if (data && length > 0) {
                // Data received
                conn->acknowledgment_number = seq + length;
                
                // Add to receive buffer
                u8* buf = (u8*)conn->receive_buffer;
                for (u32 i = 0; i < length; i++) {
                    buf[(conn->receive_buffer_tail + i) % conn->receive_buffer_size] = ((u8*)data)[i];
                }
                conn->receive_buffer_tail = (conn->receive_buffer_tail + length) % conn->receive_buffer_size;
                
                // Send ACK
                tcp_send_segment(conn, TCP_ACK, NULL, 0);
            }
            break;
            
        default:
            break;
    }
    
    return SUCCESS;
}

// Establish TCP connection
static error_t tcp_establish_connection(tcp_connection_t* conn) {
    if (!conn) {
        return E_INVAL;
    }
    
    KDEBUG("Establishing TCP connection");
    
    conn->state = TCP_SYN_SENT;
    conn->sequence_number = 0x12345678; // Random initial sequence number
    
    // Send SYN
    return tcp_send_segment(conn, TCP_SYN, NULL, 0);
}

// Close TCP connection
static error_t tcp_close_connection(tcp_connection_t* conn) {
    if (!conn) {
        return E_INVAL;
    }
    
    KDEBUG("Closing TCP connection");
    
    if (conn->state == TCP_ESTABLISHED) {
        conn->state = TCP_FIN_WAIT_1;
        
        // Send FIN
        tcp_send_segment(conn, TCP_FIN | TCP_ACK, NULL, 0);
    }
    
    conn->state = TCP_CLOSED;
    conn->active = false;
    
    return SUCCESS;
}

// Find TCP connection
static tcp_connection_t* tcp_find_connection(u32 local_ip, u16 local_port, u32 remote_ip, u16 remote_port) {
    for (u32 i = 0; i < MAX_SOCKETS; i++) {
        tcp_connection_t* conn = &tcp_connections[i];
        if (conn->active &&
            conn->local_ip == local_ip &&
            conn->local_port == local_port &&
            conn->remote_ip == remote_ip &&
            conn->remote_port == remote_port) {
            return conn;
        }
    }
    return NULL;
}

// Allocate TCP port
static u16 tcp_allocate_port(void) {
    u16 port = next_port++;
    if (next_port > 65535) {
        next_port = 1024;
    }
    return port;
}

// Calculate TCP checksum
static u16 tcp_calculate_checksum(const void* data, u32 length, u32 source_ip, u32 dest_ip) {
    // Simplified checksum calculation
    // In a real implementation, this would calculate the proper TCP checksum
    u32 sum = 0;
    u16* ptr = (u16*)data;
    
    for (u32 i = 0; i < length / 2; i++) {
        sum += ntohs(ptr[i]);
    }
    
    if (length % 2) {
        sum += ((u8*)data)[length - 1] << 8;
    }
    
    // Add pseudo-header
    sum += (source_ip >> 16) & 0xFFFF;
    sum += source_ip & 0xFFFF;
    sum += (dest_ip >> 16) & 0xFFFF;
    sum += dest_ip & 0xFFFF;
    sum += htons(6); // TCP protocol
    sum += htons(length);
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

// Get socket
static socket_t* socket_get(int fd) {
    if (fd < 0 || fd >= MAX_SOCKETS) {
        return NULL;
    }
    return &sockets[fd];
}

// Allocate socket
static int socket_alloc(void) {
    for (u32 i = 0; i < MAX_SOCKETS; i++) {
        if (!sockets[i].active) {
            return i;
        }
    }
    return -1;
}

// Free socket
static void socket_free(int fd) {
    if (fd >= 0 && fd < MAX_SOCKETS) {
        sockets[fd].active = false;
        sockets[fd].connection = NULL;
    }
}

// Add network interface
error_t network_add_interface(const char* name, u32 ip_address, u32 netmask, u32 gateway) {
    if (!networking_initialized || num_interfaces >= 4) {
        return E_NOMEM;
    }
    
    network_interface_t* iface = &network_interfaces[num_interfaces];
    strncpy(iface->name, name, sizeof(iface->name) - 1);
    iface->ip_address = ip_address;
    iface->netmask = netmask;
    iface->gateway = gateway;
    iface->mtu = 1500;
    iface->active = true;
    
    num_interfaces++;
    
    KINFO("Added network interface: %s (%u.%u.%u.%u)", name,
           (ip_address >> 24) & 0xFF, (ip_address >> 16) & 0xFF,
           (ip_address >> 8) & 0xFF, ip_address & 0xFF);
    
    return SUCCESS;
}

// Get network interface by IP
network_interface_t* network_get_interface(u32 ip_address) {
    for (u32 i = 0; i < num_interfaces; i++) {
        if (network_interfaces[i].active &&
            (network_interfaces[i].ip_address & network_interfaces[i].netmask) ==
            (ip_address & network_interfaces[i].netmask)) {
            return &network_interfaces[i];
        }
    }
    return NULL;
}

// Dump networking information
void networking_dump_info(void) {
    KINFO("=== Networking Information ===");
    KINFO("Initialized: %s", networking_initialized ? "Yes" : "No");
    KINFO("Interfaces: %u", num_interfaces);
    
    for (u32 i = 0; i < num_interfaces; i++) {
        network_interface_t* iface = &network_interfaces[i];
        if (iface->active) {
            KINFO("  Interface %s: %u.%u.%u.%u/%u.%u.%u.%u",
                  iface->name,
                  (iface->ip_address >> 24) & 0xFF, (iface->ip_address >> 16) & 0xFF,
                  (iface->ip_address >> 8) & 0xFF, iface->ip_address & 0xFF,
                  (iface->netmask >> 24) & 0xFF, (iface->netmask >> 16) & 0xFF,
                  (iface->netmask >> 8) & 0xFF, iface->netmask & 0xFF);
        }
    }
    
    u32 active_sockets = 0;
    u32 active_connections = 0;
    for (u32 i = 0; i < MAX_SOCKETS; i++) {
        if (sockets[i].active) {
            active_sockets++;
        }
        if (tcp_connections[i].active) {
            active_connections++;
        }
    }
    
    KINFO("Active sockets: %u", active_sockets);
    KINFO("Active TCP connections: %u", active_connections);
} 