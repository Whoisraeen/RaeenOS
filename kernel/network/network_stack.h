#ifndef RAEEEN_NETWORK_STACK_H
#define RAEEEN_NETWORK_STACK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"

// Network protocol families
#define AF_UNSPEC                  0
#define AF_UNIX                    1
#define AF_INET                    2
#define AF_INET6                   10
#define AF_PACKET                  17
#define AF_BLUETOOTH               31

// Socket types
#define SOCK_STREAM                1
#define SOCK_DGRAM                 2
#define SOCK_RAW                   3
#define SOCK_RDM                   4
#define SOCK_SEQPACKET             5
#define SOCK_DCCP                  6
#define SOCK_PACKET                10

// Protocol types
#define IPPROTO_IP                 0
#define IPPROTO_ICMP               1
#define IPPROTO_IGMP               2
#define IPPROTO_IPIP               4
#define IPPROTO_TCP                6
#define IPPROTO_EGP                8
#define IPPROTO_PUP                12
#define IPPROTO_UDP                17
#define IPPROTO_IDP                22
#define IPPROTO_TP                 29
#define IPPROTO_DCCP               33
#define IPPROTO_IPV6               41
#define IPPROTO_RSVP               46
#define IPPROTO_GRE                47
#define IPPROTO_ESP                50
#define IPPROTO_AH                 51
#define IPPROTO_ICMPV6             58
#define IPPROTO_NONE               59
#define IPPROTO_DSTOPTS            60
#define IPPROTO_MTP                92
#define IPPROTO_BEETPH             94
#define IPPROTO_ENCAP              98
#define IPPROTO_PIM                103
#define IPPROTO_COMP               108
#define IPPROTO_SCTP               132
#define IPPROTO_UDPLITE            136
#define IPPROTO_MPLS               137
#define IPPROTO_ETHERNET           143
#define IPPROTO_RAW                255
#define IPPROTO_MPTCP              262

// Socket options
#define SOL_SOCKET                 1
#define SOL_IP                     0
#define SOL_IPV6                   41
#define SOL_ICMPV6                 58
#define SOL_TCP                    6
#define SOL_UDP                    17

// Socket option levels
#define SO_DEBUG                   1
#define SO_REUSEADDR               2
#define SO_TYPE                    3
#define SO_ERROR                   4
#define SO_DONTROUTE               5
#define SO_BROADCAST               6
#define SO_SNDBUF                  7
#define SO_RCVBUF                  8
#define SO_SNDBUFFORCE             32
#define SO_RCVBUFFORCE             33
#define SO_KEEPALIVE               9
#define SO_OOBINLINE               10
#define SO_NO_CHECK                11
#define SO_PRIORITY                12
#define SO_LINGER                  13
#define SO_BSDCOMPAT               14
#define SO_REUSEPORT               15
#define SO_PASSCRED                16
#define SO_PEERCRED                17
#define SO_RCVLOWAT                18
#define SO_SNDLOWAT                19
#define SO_RCVTIMEO                20
#define SO_SNDTIMEO                21
#define SO_SECURITY_AUTHENTICATION 22
#define SO_SECURITY_ENCRYPTION_TRANSPORT 23
#define SO_SECURITY_ENCRYPTION_NETWORK 24
#define SO_BINDTODEVICE            25
#define SO_ATTACH_FILTER           26
#define SO_DETACH_FILTER           27
#define SO_PEERNAME                28
#define SO_TIMESTAMP               29
#define SO_ACCEPTCONN              30
#define SO_PEERSEC                 31
#define SO_SNDBUFFORCE             32
#define SO_RCVBUFFORCE             33
#define SO_PASSSEC                 34
#define SO_TIMESTAMPNS             35
#define SO_MARK                    36
#define SO_TIMESTAMPING            37
#define SO_PROTOCOL                38
#define SO_DOMAIN                  39
#define SO_RXQ_OVFL                40
#define SO_WIFI_STATUS             41
#define SO_PEEK_OFF                42
#define SO_NOFCS                   43
#define SO_LOCK_FILTER             44
#define SO_SELECT_ERR_QUEUE        45
#define SO_BUSY_POLL               46
#define SO_MAX_PACING_RATE         47
#define SO_BPF_EXTENSIONS          48
#define SO_INCOMING_CPU            49
#define SO_ATTACH_BPF              50
#define SO_DETACH_BPF              51
#define SO_ATTACH_REUSEPORT_CBPF   52
#define SO_ATTACH_REUSEPORT_EBPF   53
#define SO_CNX_ADVICE              54
#define SCM_TIMESTAMPING_OPT_STATS 55
#define SO_MEMINFO                 56
#define SO_INCOMING_NAPI_ID        57
#define SO_COOKIE                  58
#define SCM_TIMESTAMPING_PKTINFO   59
#define SO_PEERGROUPS              60
#define SO_ZEROCOPY                61
#define SO_TXTIME                  62
#define SO_BINDTOIFINDEX           63
#define SO_TIMESTAMP_NEW           64
#define SO_DETACH_REUSEPORT_BPF    65
#define SO_PREFER_BUSY_POLL        66
#define SO_BUSY_POLL_BUDGET        67
#define SO_NETNS_COOKIE            68
#define SO_BUF_LOCK                69
#define SO_RESERVE_MEM             70
#define SO_TXREHASH                71
#define SO_RCVMARK                 72
#define SO_PASSPIDFD               73
#define SO_PEERPIDFD               74
#define SO_RCVBUFFORCE_NEW         75
#define SO_SNDBUFFORCE_NEW         76

// IP socket options
#define IP_TOS                     1
#define IP_TTL                     2
#define IP_HDRINCL                 3
#define IP_OPTIONS                 4
#define IP_ROUTER_ALERT            5
#define IP_RECVOPTS                6
#define IP_RETOPTS                 7
#define IP_PKTINFO                 8
#define IP_PKTOPTIONS              9
#define IP_MTU_DISCOVER            10
#define IP_RECVERR                 11
#define IP_RECVTTL                 12
#define IP_RECVTOS                 13
#define IP_MTU                     14
#define IP_FREEBIND                15
#define IP_IPSEC_POLICY            16
#define IP_XFRM_POLICY             17
#define IP_PASSSEC                 18
#define IP_TRANSPARENT             19
#define IP_ORIGDSTADDR             20
#define IP_RECVORIGDSTADDR         21
#define IP_MINTTL                  22
#define IP_NODEFRAG                23
#define IP_CHECKSUM                24
#define IP_BIND_ADDRESS_NO_PORT    25
#define IP_RECVFRAGSIZE            26
#define IP_RECVERR_RFC4884         27
#define IP_RECVERR_NOBIND          28
#define IP_UNICAST_IF              50
#define IP_MULTICAST_IF            32
#define IP_MULTICAST_TTL           33
#define IP_MULTICAST_LOOP          34
#define IP_ADD_MEMBERSHIP          35
#define IP_DROP_MEMBERSHIP         36
#define IP_UNBLOCK_SOURCE          37
#define IP_BLOCK_SOURCE            38
#define IP_ADD_SOURCE_MEMBERSHIP   39
#define IP_DROP_SOURCE_MEMBERSHIP  40
#define IP_MSFILTER                41
#define IP_MULTICAST_ALL           49
#define IP_UNICAST_IF              50

// TCP socket options
#define TCP_NODELAY                1
#define TCP_MAXSEG                 2
#define TCP_CORK                   3
#define TCP_KEEPIDLE               4
#define TCP_KEEPINTVL              5
#define TCP_KEEPCNT                6
#define TCP_SYNCNT                 7
#define TCP_LINGER2                8
#define TCP_DEFER_ACCEPT           9
#define TCP_WINDOW_CLAMP           10
#define TCP_INFO                   11
#define TCP_QUICKACK               12
#define TCP_CONGESTION             13
#define TCP_MD5SIG                 14
#define TCP_COOKIE_TRANSACTIONS    15
#define TCP_THIN_LINEAR_TIMEOUTS   16
#define TCP_THIN_DUPACK            17
#define TCP_USER_TIMEOUT           18
#define TCP_REPAIR                 19
#define TCP_REPAIR_QUEUE           20
#define TCP_QUEUE_SEQ              21
#define TCP_REPAIR_OPTIONS         22
#define TCP_FASTOPEN               23
#define TCP_TIMESTAMP              24
#define TCP_NOTSENT_LOWAT          25
#define TCP_CC_INFO                26
#define TCP_SAVE_SYN               27
#define TCP_SAVED_SYN              28
#define TCP_REPAIR_WINDOW          29
#define TCP_FASTOPEN_CONNECT       30
#define TCP_ULP                    31
#define TCP_MD5SIG_EXT             32
#define TCP_FASTOPEN_KEY           33
#define TCP_FASTOPEN_NO_COOKIE     34
#define TCP_ZEROCOPY_RECEIVE       35
#define TCP_INQ                    36
#define TCP_CM_INQ                 37
#define TCP_TX_DELAY               38
#define TCP_TIMESTAMP              39
#define TCP_NOTSENT_LOWAT          40
#define TCP_CC_INFO                41
#define TCP_SAVE_SYN               42
#define TCP_SAVED_SYN              43
#define TCP_REPAIR_WINDOW          44
#define TCP_FASTOPEN_CONNECT       45
#define TCP_ULP                    46
#define TCP_MD5SIG_EXT             47
#define TCP_FASTOPEN_KEY           48
#define TCP_FASTOPEN_NO_COOKIE     49
#define TCP_ZEROCOPY_RECEIVE       50
#define TCP_INQ                    51
#define TCP_CM_INQ                 52
#define TCP_TX_DELAY               53
#define TCP_TIMESTAMP              54
#define TCP_NOTSENT_LOWAT          55
#define TCP_CC_INFO                56
#define TCP_SAVE_SYN               57
#define TCP_SAVED_SYN              58
#define TCP_REPAIR_WINDOW          59
#define TCP_FASTOPEN_CONNECT       60
#define TCP_ULP                    61
#define TCP_MD5SIG_EXT             62
#define TCP_FASTOPEN_KEY           63
#define TCP_FASTOPEN_NO_COOKIE     64
#define TCP_ZEROCOPY_RECEIVE       65
#define TCP_INQ                    66
#define TCP_CM_INQ                 67
#define TCP_TX_DELAY               68
#define TCP_TIMESTAMP              69
#define TCP_NOTSENT_LOWAT          70
#define TCP_CC_INFO                71
#define TCP_SAVE_SYN               72
#define TCP_SAVED_SYN              73
#define TCP_REPAIR_WINDOW          74
#define TCP_FASTOPEN_CONNECT       75
#define TCP_ULP                    76
#define TCP_MD5SIG_EXT             77
#define TCP_FASTOPEN_KEY           78
#define TCP_FASTOPEN_NO_COOKIE     79
#define TCP_ZEROCOPY_RECEIVE       80
#define TCP_INQ                    81
#define TCP_CM_INQ                 82
#define TCP_TX_DELAY               83
#define TCP_TIMESTAMP              84
#define TCP_NOTSENT_LOWAT          85
#define TCP_CC_INFO                86
#define TCP_SAVE_SYN               87
#define TCP_SAVED_SYN              88
#define TCP_REPAIR_WINDOW          89
#define TCP_FASTOPEN_CONNECT       90
#define TCP_ULP                    91
#define TCP_MD5SIG_EXT             92
#define TCP_FASTOPEN_KEY           93
#define TCP_FASTOPEN_NO_COOKIE     94
#define TCP_ZEROCOPY_RECEIVE       95
#define TCP_INQ                    96
#define TCP_CM_INQ                 97
#define TCP_TX_DELAY               98
#define TCP_TIMESTAMP              99
#define TCP_NOTSENT_LOWAT          100
#define TCP_CC_INFO                101
#define TCP_SAVE_SYN               102
#define TCP_SAVED_SYN              103
#define TCP_REPAIR_WINDOW          104
#define TCP_FASTOPEN_CONNECT       105
#define TCP_ULP                    106
#define TCP_MD5SIG_EXT             107
#define TCP_FASTOPEN_KEY           108
#define TCP_FASTOPEN_NO_COOKIE     109
#define TCP_ZEROCOPY_RECEIVE       110
#define TCP_INQ                    111
#define TCP_CM_INQ                 112
#define TCP_TX_DELAY               113
#define TCP_TIMESTAMP              114
#define TCP_NOTSENT_LOWAT          115
#define TCP_CC_INFO                116
#define TCP_SAVE_SYN               117
#define TCP_SAVED_SYN              118
#define TCP_REPAIR_WINDOW          119
#define TCP_FASTOPEN_CONNECT       120
#define TCP_ULP                    121
#define TCP_MD5SIG_EXT             122
#define TCP_FASTOPEN_KEY           123
#define TCP_FASTOPEN_NO_COOKIE     124
#define TCP_ZEROCOPY_RECEIVE       125
#define TCP_INQ                    126
#define TCP_CM_INQ                 127
#define TCP_TX_DELAY               128
#define TCP_TIMESTAMP              129
#define TCP_NOTSENT_LOWAT          130
#define TCP_CC_INFO                131
#define TCP_SAVE_SYN               132
#define TCP_SAVED_SYN              133
#define TCP_REPAIR_WINDOW          134
#define TCP_FASTOPEN_CONNECT       135
#define TCP_ULP                    136
#define TCP_MD5SIG_EXT             137
#define TCP_FASTOPEN_KEY           138
#define TCP_FASTOPEN_NO_COOKIE     139
#define TCP_ZEROCOPY_RECEIVE       140
#define TCP_INQ                    141
#define TCP_CM_INQ                 142
#define TCP_TX_DELAY               143
#define TCP_TIMESTAMP              144
#define TCP_NOTSENT_LOWAT          145
#define TCP_CC_INFO                146
#define TCP_SAVE_SYN               147
#define TCP_SAVED_SYN              148
#define TCP_REPAIR_WINDOW          149
#define TCP_FASTOPEN_CONNECT       150
#define TCP_ULP                    151
#define TCP_MD5SIG_EXT             152
#define TCP_FASTOPEN_KEY           153
#define TCP_FASTOPEN_NO_COOKIE     154
#define TCP_ZEROCOPY_RECEIVE       155
#define TCP_INQ                    156
#define TCP_CM_INQ                 157
#define TCP_TX_DELAY               158
#define TCP_TIMESTAMP              159
#define TCP_NOTSENT_LOWAT          160
#define TCP_CC_INFO                161
#define TCP_SAVE_SYN               162
#define TCP_SAVED_SYN              163
#define TCP_REPAIR_WINDOW          164
#define TCP_FASTOPEN_CONNECT       165
#define TCP_ULP                    166
#define TCP_MD5SIG_EXT             167
#define TCP_FASTOPEN_KEY           168
#define TCP_FASTOPEN_NO_COOKIE     169
#define TCP_ZEROCOPY_RECEIVE       170
#define TCP_INQ                    171
#define TCP_CM_INQ                 172
#define TCP_TX_DELAY               173
#define TCP_TIMESTAMP              174
#define TCP_NOTSENT_LOWAT          175
#define TCP_CC_INFO                176
#define TCP_SAVE_SYN               177
#define TCP_SAVED_SYN              178
#define TCP_REPAIR_WINDOW          179
#define TCP_FASTOPEN_CONNECT       180
#define TCP_ULP                    181
#define TCP_MD5SIG_EXT             182
#define TCP_FASTOPEN_KEY           183
#define TCP_FASTOPEN_NO_COOKIE     184
#define TCP_ZEROCOPY_RECEIVE       185
#define TCP_INQ                    186
#define TCP_CM_INQ                 187
#define TCP_TX_DELAY               188
#define TCP_TIMESTAMP              189
#define TCP_NOTSENT_LOWAT          190
#define TCP_CC_INFO                191
#define TCP_SAVE_SYN               192
#define TCP_SAVED_SYN              193
#define TCP_REPAIR_WINDOW          194
#define TCP_FASTOPEN_CONNECT       195
#define TCP_ULP                    196
#define TCP_MD5SIG_EXT             197
#define TCP_FASTOPEN_KEY           198
#define TCP_FASTOPEN_NO_COOKIE     199
#define TCP_ZEROCOPY_RECEIVE       200
#define TCP_INQ                    201
#define TCP_CM_INQ                 202
#define TCP_TX_DELAY               203
#define TCP_TIMESTAMP              204
#define TCP_NOTSENT_LOWAT          205
#define TCP_CC_INFO                206
#define TCP_SAVE_SYN               207
#define TCP_SAVED_SYN              208
#define TCP_REPAIR_WINDOW          209
#define TCP_FASTOPEN_CONNECT       210
#define TCP_ULP                    211
#define TCP_MD5SIG_EXT             212
#define TCP_FASTOPEN_KEY           213
#define TCP_FASTOPEN_NO_COOKIE     214
#define TCP_ZEROCOPY_RECEIVE       215
#define TCP_INQ                    216
#define TCP_CM_INQ                 217
#define TCP_TX_DELAY               218
#define TCP_TIMESTAMP              219
#define TCP_NOTSENT_LOWAT          220
#define TCP_CC_INFO                221
#define TCP_SAVE_SYN               222
#define TCP_SAVED_SYN              223
#define TCP_REPAIR_WINDOW          224
#define TCP_FASTOPEN_CONNECT       225
#define TCP_ULP                    226
#define TCP_MD5SIG_EXT             227
#define TCP_FASTOPEN_KEY           228
#define TCP_FASTOPEN_NO_COOKIE     229
#define TCP_ZEROCOPY_RECEIVE       230
#define TCP_INQ                    231
#define TCP_CM_INQ                 232
#define TCP_TX_DELAY               233
#define TCP_TIMESTAMP              234
#define TCP_NOTSENT_LOWAT          235
#define TCP_CC_INFO                236
#define TCP_SAVE_SYN               237
#define TCP_SAVED_SYN              238
#define TCP_REPAIR_WINDOW          239
#define TCP_FASTOPEN_CONNECT       240
#define TCP_ULP                    241
#define TCP_MD5SIG_EXT             242
#define TCP_FASTOPEN_KEY           243
#define TCP_FASTOPEN_NO_COOKIE     244
#define TCP_ZEROCOPY_RECEIVE       245
#define TCP_INQ                    246
#define TCP_CM_INQ                 247
#define TCP_TX_DELAY               248
#define TCP_TIMESTAMP              249
#define TCP_NOTSENT_LOWAT          250
#define TCP_CC_INFO                251
#define TCP_SAVE_SYN               252
#define TCP_SAVED_SYN              253
#define TCP_REPAIR_WINDOW          254
#define TCP_FASTOPEN_CONNECT       255

// Network interface flags
#define IFF_UP                     0x1
#define IFF_BROADCAST              0x2
#define IFF_DEBUG                  0x4
#define IFF_LOOPBACK               0x8
#define IFF_POINTOPOINT            0x10
#define IFF_NOTRAILERS             0x20
#define IFF_RUNNING                0x40
#define IFF_NOARP                  0x80
#define IFF_PROMISC                0x100
#define IFF_ALLMULTI               0x200
#define IFF_MASTER                 0x400
#define IFF_SLAVE                  0x800
#define IFF_MULTICAST              0x8000
#define IFF_PORTSEL                0x2000
#define IFF_AUTOMEDIA              0x4000
#define IFF_DYNAMIC                0x8000

// Socket states
#define SS_FREE                    0
#define SS_UNCONNECTED             1
#define SS_CONNECTING              2
#define SS_CONNECTED               3
#define SS_DISCONNECTING           4

// Network packet structure
typedef struct network_packet {
    void *data;
    size_t size;
    uint32_t protocol;
    uint32_t flags;
    struct network_packet *next;
    struct network_packet *prev;
} network_packet_t;

// Socket structure
typedef struct socket {
    int domain;
    int type;
    int protocol;
    int state;
    uint32_t flags;
    struct sockaddr *local_addr;
    struct sockaddr *remote_addr;
    size_t send_buffer_size;
    size_t recv_buffer_size;
    network_packet_t *send_queue;
    network_packet_t *recv_queue;
    void *private_data;
    struct socket_ops *ops;
    spinlock_t lock;
    int ref_count;
} socket_t;

// Socket operations structure
typedef struct socket_ops {
    int (*bind)(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen);
    int (*connect)(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen);
    int (*listen)(socket_t *sock, int backlog);
    int (*accept)(socket_t *sock, struct sockaddr *addr, socklen_t *addrlen);
    ssize_t (*send)(socket_t *sock, const void *buf, size_t len, int flags);
    ssize_t (*recv)(socket_t *sock, void *buf, size_t len, int flags);
    ssize_t (*sendto)(socket_t *sock, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
    ssize_t (*recvfrom)(socket_t *sock, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
    int (*shutdown)(socket_t *sock, int how);
    int (*close)(socket_t *sock);
    int (*setsockopt)(socket_t *sock, int level, int optname, const void *optval, socklen_t optlen);
    int (*getsockopt)(socket_t *sock, int level, int optname, void *optval, socklen_t *optlen);
    int (*ioctl)(socket_t *sock, unsigned long request, void *arg);
} socket_ops_t;

// Network interface structure
typedef struct network_interface {
    char name[32];
    uint32_t flags;
    uint32_t mtu;
    uint8_t mac_addr[6];
    uint32_t ip_addr;
    uint32_t netmask;
    uint32_t broadcast;
    uint32_t gateway;
    void *driver_data;
    struct network_interface *next;
    struct network_interface *prev;
} network_interface_t;

// TCP connection structure
typedef struct tcp_connection {
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;
    uint32_t state;
    uint32_t seq_num;
    uint32_t ack_num;
    uint32_t window_size;
    uint32_t mss;
    uint32_t rtt;
    uint32_t timeout;
    network_packet_t *send_queue;
    network_packet_t *recv_queue;
    socket_t *socket;
    struct tcp_connection *next;
    struct tcp_connection *prev;
} tcp_connection_t;

// UDP socket structure
typedef struct udp_socket {
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;
    network_packet_t *recv_queue;
    socket_t *socket;
    struct udp_socket *next;
    struct udp_socket *prev;
} udp_socket_t;

// DNS resolver structure
typedef struct dns_resolver {
    uint32_t nameserver[4];
    uint32_t timeout;
    uint32_t retries;
    struct dns_resolver *next;
} dns_resolver_t;

// DHCP client structure
typedef struct dhcp_client {
    uint32_t ip_addr;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t dns_server[4];
    uint32_t lease_time;
    uint32_t renew_time;
    uint32_t rebind_time;
    uint8_t mac_addr[6];
    uint32_t state;
    uint32_t xid;
    uint32_t timeout;
} dhcp_client_t;

// Function prototypes

// Network stack initialization
void network_stack_init(void);
int network_interface_register(network_interface_t *iface);
int network_interface_unregister(network_interface_t *iface);
network_interface_t *network_interface_find(const char *name);
void network_interface_list(void);

// Socket API
int socket_create(int domain, int type, int protocol, socket_t **sock);
int socket_destroy(socket_t *sock);
int socket_bind(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen);
int socket_connect(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen);
int socket_listen(socket_t *sock, int backlog);
int socket_accept(socket_t *sock, struct sockaddr *addr, socklen_t *addrlen);
ssize_t socket_send(socket_t *sock, const void *buf, size_t len, int flags);
ssize_t socket_recv(socket_t *sock, void *buf, size_t len, int flags);
ssize_t socket_sendto(socket_t *sock, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t socket_recvfrom(socket_t *sock, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
int socket_shutdown(socket_t *sock, int how);
int socket_close(socket_t *sock);
int socket_setsockopt(socket_t *sock, int level, int optname, const void *optval, socklen_t optlen);
int socket_getsockopt(socket_t *sock, int level, int optname, void *optval, socklen_t *optlen);
int socket_ioctl(socket_t *sock, unsigned long request, void *arg);

// TCP protocol
void tcp_init(void);
int tcp_connect(uint32_t local_ip, uint16_t local_port, uint32_t remote_ip, uint16_t remote_port, tcp_connection_t **conn);
int tcp_disconnect(tcp_connection_t *conn);
int tcp_send(tcp_connection_t *conn, const void *data, size_t size);
int tcp_recv(tcp_connection_t *conn, void *data, size_t size);
int tcp_listen(uint32_t ip, uint16_t port, int backlog);
int tcp_accept(tcp_connection_t *listener, tcp_connection_t **conn);
void tcp_process_packet(network_packet_t *packet);

// UDP protocol
void udp_init(void);
int udp_socket_create(uint32_t local_ip, uint16_t local_port, udp_socket_t **sock);
int udp_socket_destroy(udp_socket_t *sock);
int udp_send(udp_socket_t *sock, uint32_t remote_ip, uint16_t remote_port, const void *data, size_t size);
int udp_recv(udp_socket_t *sock, uint32_t *remote_ip, uint16_t *remote_port, void *data, size_t size);
void udp_process_packet(network_packet_t *packet);

// IP protocol
void ip_init(void);
int ip_send_packet(uint32_t src_ip, uint32_t dest_ip, uint8_t protocol, network_packet_t *packet);
int ip_recv_packet(network_packet_t *packet);
uint32_t ip_checksum(void *data, size_t size);
uint32_t ip_route_packet(uint32_t dest_ip);

// ICMP protocol
void icmp_init(void);
int icmp_send_echo_request(uint32_t dest_ip, uint16_t id, uint16_t seq);
int icmp_send_echo_reply(uint32_t dest_ip, uint16_t id, uint16_t seq);
void icmp_process_packet(network_packet_t *packet);

// ARP protocol
void arp_init(void);
int arp_resolve(uint32_t ip_addr, uint8_t *mac_addr);
int arp_add_entry(uint32_t ip_addr, uint8_t *mac_addr);
void arp_process_packet(network_packet_t *packet);

// DNS resolver
void dns_init(void);
int dns_resolve(const char *hostname, uint32_t *ip_addr);
int dns_add_nameserver(uint32_t ip_addr);
void dns_process_packet(network_packet_t *packet);

// DHCP client
void dhcp_init(void);
int dhcp_discover(network_interface_t *iface);
int dhcp_request(network_interface_t *iface, uint32_t ip_addr);
int dhcp_renew(network_interface_t *iface);
int dhcp_release(network_interface_t *iface);
void dhcp_process_packet(network_packet_t *packet);

// Network utilities
uint32_t ip_string_to_addr(const char *ip_str);
char *ip_addr_to_string(uint32_t ip_addr);
uint32_t ip_calc_netmask(int prefix_len);
int ip_is_in_network(uint32_t ip_addr, uint32_t network, uint32_t netmask);
uint32_t ip_get_broadcast(uint32_t ip_addr, uint32_t netmask);

// Network packet management
network_packet_t *network_packet_alloc(size_t size);
void network_packet_free(network_packet_t *packet);
int network_packet_send(network_interface_t *iface, network_packet_t *packet);
int network_packet_recv(network_interface_t *iface, network_packet_t *packet);

// Network statistics
typedef struct {
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t errors;
    uint64_t collisions;
    uint64_t dropped;
} network_stats_t;

void network_get_stats(network_interface_t *iface, network_stats_t *stats);
void network_reset_stats(network_interface_t *iface);

// Network debugging
void network_dump_packet(network_packet_t *packet);
void network_dump_interface(network_interface_t *iface);
void network_dump_connections(void);

// Wireless networking (placeholder for future implementation)
typedef struct {
    char ssid[32];
    uint8_t bssid[6];
    uint32_t channel;
    uint32_t frequency;
    int32_t signal_strength;
    uint32_t security;
} wireless_network_t;

int wireless_scan_networks(wireless_network_t *networks, int max_networks);
int wireless_connect(const char *ssid, const char *password);
int wireless_disconnect(void);
int wireless_get_status(wireless_network_t *network);

// Bluetooth networking (placeholder for future implementation)
typedef struct {
    uint8_t address[6];
    char name[32];
    uint32_t class;
    uint32_t rssi;
} bluetooth_device_t;

int bluetooth_scan_devices(bluetooth_device_t *devices, int max_devices);
int bluetooth_connect(uint8_t *address);
int bluetooth_disconnect(uint8_t *address);
int bluetooth_send_data(uint8_t *address, const void *data, size_t size);

#endif // RAEEEN_NETWORK_STACK_H 