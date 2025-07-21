#ifndef HIGH_AVAILABILITY_H
#define HIGH_AVAILABILITY_H

#include "kernel.h"
#include <stdint.h>
#include <stdbool.h>

// High availability constants
#define MAX_CLUSTER_NODES 16
#define MAX_CLUSTER_SERVICES 50
#define MAX_FAILOVER_GROUPS 20
#define MAX_REPLICATION_PAIRS 100
#define MAX_LOAD_BALANCERS 10
#define MAX_HEARTBEAT_INTERVAL 5000  // 5 seconds
#define MAX_FAILOVER_TIMEOUT 30000   // 30 seconds

// Node states
typedef enum {
    NODE_STATE_OFFLINE = 0,
    NODE_STATE_ONLINE = 1,
    NODE_STATE_MAINTENANCE = 2,
    NODE_STATE_FAILED = 3,
    NODE_STATE_RECOVERING = 4,
    NODE_STATE_STANDBY = 5
} node_state_t;

// Service states
typedef enum {
    SERVICE_STATE_STOPPED = 0,
    SERVICE_STATE_STARTING = 1,
    SERVICE_STATE_RUNNING = 2,
    SERVICE_STATE_STOPPING = 3,
    SERVICE_STATE_FAILED = 4,
    SERVICE_STATE_DEGRADED = 5
} service_state_t;

// Failover strategies
typedef enum {
    FAILOVER_STRATEGY_ACTIVE_PASSIVE = 0,
    FAILOVER_STRATEGY_ACTIVE_ACTIVE = 1,
    FAILOVER_STRATEGY_N_PLUS_1 = 2,
    FAILOVER_STRATEGY_LOAD_BALANCED = 3
} failover_strategy_t;

// Load balancing algorithms
typedef enum {
    LB_ALGORITHM_ROUND_ROBIN = 0,
    LB_ALGORITHM_LEAST_CONNECTIONS = 1,
    LB_ALGORITHM_WEIGHTED_ROUND_ROBIN = 2,
    LB_ALGORITHM_IP_HASH = 3,
    LB_ALGORITHM_LEAST_RESPONSE_TIME = 4
} load_balancing_algorithm_t;

// Cluster node information
typedef struct cluster_node {
    uint32_t node_id;                           // Unique node identifier
    char hostname[64];                          // Node hostname
    char ip_address[16];                        // Node IP address
    node_state_t state;                         // Current node state
    uint64_t last_heartbeat;                    // Last heartbeat timestamp
    uint64_t uptime;                            // Node uptime
    uint32_t cpu_cores;                         // Number of CPU cores
    uint64_t memory_total;                      // Total memory (bytes)
    uint64_t memory_used;                       // Used memory (bytes)
    uint64_t disk_total;                        // Total disk space (bytes)
    uint64_t disk_used;                         // Used disk space (bytes)
    float cpu_usage;                            // CPU usage percentage
    float memory_usage;                         // Memory usage percentage
    float disk_usage;                           // Disk usage percentage
    uint32_t network_interfaces;                // Number of network interfaces
    uint64_t network_rx_bytes;                  // Received bytes
    uint64_t network_tx_bytes;                  // Transmitted bytes
    bool is_primary;                            // Is primary node
    bool is_backup;                             // Is backup node
    uint32_t priority;                          // Node priority (lower = higher)
    uint32_t weight;                            // Load balancing weight
    void* node_data;                            // Node-specific data
} cluster_node_t;

// Cluster service information
typedef struct cluster_service {
    uint32_t service_id;                        // Unique service identifier
    char name[64];                              // Service name
    char description[256];                      // Service description
    service_state_t state;                      // Current service state
    uint32_t primary_node_id;                   // Primary node ID
    uint32_t backup_node_id;                    // Backup node ID
    uint32_t current_node_id;                   // Current running node ID
    uint64_t start_time;                        // Service start time
    uint64_t last_health_check;                 // Last health check time
    uint32_t health_check_interval;             // Health check interval (ms)
    uint32_t restart_count;                     // Number of restarts
    uint32_t max_restarts;                      // Maximum restart attempts
    uint64_t restart_timeout;                   // Restart timeout (ms)
    bool auto_start;                            // Auto-start on boot
    bool auto_failover;                         // Auto-failover enabled
    uint32_t failover_timeout;                  // Failover timeout (ms)
    uint32_t dependencies[MAX_CLUSTER_SERVICES]; // Service dependencies
    uint32_t dependency_count;                  // Number of dependencies
    void* service_data;                         // Service-specific data
} cluster_service_t;

// Failover group information
typedef struct failover_group {
    uint32_t group_id;                          // Unique group identifier
    char name[64];                              // Group name
    char description[256];                      // Group description
    failover_strategy_t strategy;               // Failover strategy
    uint32_t primary_node_id;                   // Primary node ID
    uint32_t backup_node_id;                    // Backup node ID
    uint32_t current_node_id;                   // Current active node ID
    bool auto_failover;                         // Auto-failover enabled
    uint32_t failover_timeout;                  // Failover timeout (ms)
    uint32_t health_check_interval;             // Health check interval (ms)
    uint64_t last_failover;                     // Last failover time
    uint32_t failover_count;                    // Number of failovers
    uint32_t services[MAX_CLUSTER_SERVICES];    // Services in group
    uint32_t service_count;                     // Number of services
    void* group_data;                           // Group-specific data
} failover_group_t;

// Load balancer information
typedef struct load_balancer {
    uint32_t lb_id;                             // Unique load balancer ID
    char name[64];                              // Load balancer name
    char description[256];                      // Description
    load_balancing_algorithm_t algorithm;       // Load balancing algorithm
    uint32_t port;                              // Service port
    uint32_t nodes[MAX_CLUSTER_NODES];          // Backend nodes
    uint32_t node_count;                        // Number of backend nodes
    uint32_t max_connections;                   // Maximum connections
    uint32_t current_connections;               // Current connections
    uint32_t health_check_interval;             // Health check interval (ms)
    uint32_t health_check_timeout;              // Health check timeout (ms)
    bool sticky_sessions;                       // Sticky sessions enabled
    uint32_t session_timeout;                   // Session timeout (ms)
    uint64_t total_requests;                    // Total requests handled
    uint64_t total_bytes_sent;                  // Total bytes sent
    uint64_t total_bytes_received;              // Total bytes received
    void* lb_data;                              // Load balancer-specific data
} load_balancer_t;

// Replication pair information
typedef struct replication_pair {
    uint32_t pair_id;                           // Unique pair identifier
    char name[64];                              // Pair name
    uint32_t source_node_id;                    // Source node ID
    uint32_t target_node_id;                    // Target node ID
    char source_path[256];                      // Source path
    char target_path[256];                      // Target path
    bool bidirectional;                         // Bidirectional replication
    uint32_t sync_interval;                     // Sync interval (ms)
    uint64_t last_sync;                         // Last sync time
    uint64_t total_files_synced;                // Total files synced
    uint64_t total_bytes_synced;                // Total bytes synced
    bool real_time_sync;                        // Real-time sync enabled
    bool compression_enabled;                   // Compression enabled
    bool encryption_enabled;                    // Encryption enabled
    uint32_t sync_status;                       // Sync status (0=idle, 1=syncing, 2=error)
    void* pair_data;                            // Pair-specific data
} replication_pair_t;

// High availability system
typedef struct high_availability_system {
    spinlock_t lock;                            // System lock
    bool initialized;                           // Initialization flag
    
    // Cluster management
    cluster_node_t nodes[MAX_CLUSTER_NODES];    // Cluster nodes
    uint32_t node_count;                        // Number of nodes
    uint32_t primary_node_id;                   // Primary node ID
    uint32_t current_node_id;                   // Current node ID
    bool is_cluster_member;                     // Is cluster member
    
    // Service management
    cluster_service_t services[MAX_CLUSTER_SERVICES]; // Cluster services
    uint32_t service_count;                     // Number of services
    
    // Failover management
    failover_group_t groups[MAX_FAILOVER_GROUPS]; // Failover groups
    uint32_t group_count;                       // Number of groups
    
    // Load balancing
    load_balancer_t load_balancers[MAX_LOAD_BALANCERS]; // Load balancers
    uint32_t lb_count;                          // Number of load balancers
    
    // Replication
    replication_pair_t replication_pairs[MAX_REPLICATION_PAIRS]; // Replication pairs
    uint32_t pair_count;                        // Number of pairs
    
    // System configuration
    uint32_t heartbeat_interval;                // Heartbeat interval (ms)
    uint32_t failover_timeout;                  // Default failover timeout (ms)
    uint32_t health_check_interval;             // Default health check interval (ms)
    bool auto_failover_enabled;                 // Auto-failover enabled
    bool load_balancing_enabled;                // Load balancing enabled
    bool replication_enabled;                   // Replication enabled
    
    // System statistics
    uint64_t total_failovers;                   // Total failovers
    uint64_t total_requests_handled;            // Total requests handled
    uint64_t total_bytes_transferred;           // Total bytes transferred
    uint64_t last_update;                       // Last update time
} high_availability_system_t;

// High availability statistics
typedef struct ha_stats {
    // Cluster statistics
    uint32_t total_nodes;                       // Total nodes
    uint32_t online_nodes;                      // Online nodes
    uint32_t failed_nodes;                      // Failed nodes
    uint32_t total_services;                    // Total services
    uint32_t running_services;                  // Running services
    uint32_t failed_services;                   // Failed services
    
    // Failover statistics
    uint32_t total_groups;                      // Total failover groups
    uint32_t active_groups;                     // Active groups
    uint64_t total_failovers;                   // Total failovers
    uint64_t successful_failovers;              // Successful failovers
    uint64_t failed_failovers;                  // Failed failovers
    float average_failover_time;                // Average failover time (ms)
    
    // Load balancing statistics
    uint32_t total_load_balancers;              // Total load balancers
    uint32_t active_load_balancers;             // Active load balancers
    uint64_t total_requests;                    // Total requests
    uint64_t successful_requests;               // Successful requests
    uint64_t failed_requests;                   // Failed requests
    float average_response_time;                // Average response time (ms)
    
    // Replication statistics
    uint32_t total_replication_pairs;           // Total replication pairs
    uint32_t active_pairs;                      // Active pairs
    uint64_t total_files_synced;                // Total files synced
    uint64_t total_bytes_synced;                // Total bytes synced
    uint64_t sync_errors;                       // Sync errors
    float average_sync_time;                    // Average sync time (ms)
    
    // System statistics
    uint64_t last_update;                       // Last update time
} ha_stats_t;

// Function declarations

// System initialization
int ha_system_init(void);
void ha_system_shutdown(void);
high_availability_system_t* ha_system_get_system(void);

// Node management
cluster_node_t* ha_node_add(const char* hostname, const char* ip_address);
int ha_node_remove(uint32_t node_id);
int ha_node_update_state(uint32_t node_id, node_state_t state);
cluster_node_t* ha_node_find(uint32_t node_id);
cluster_node_t* ha_node_find_by_hostname(const char* hostname);
int ha_node_set_primary(uint32_t node_id);
int ha_node_heartbeat(uint32_t node_id);

// Service management
cluster_service_t* ha_service_add(const char* name, const char* description);
int ha_service_remove(uint32_t service_id);
int ha_service_start(uint32_t service_id);
int ha_service_stop(uint32_t service_id);
int ha_service_restart(uint32_t service_id);
int ha_service_set_primary_node(uint32_t service_id, uint32_t node_id);
int ha_service_set_backup_node(uint32_t service_id, uint32_t node_id);
cluster_service_t* ha_service_find(uint32_t service_id);
cluster_service_t* ha_service_find_by_name(const char* name);

// Failover management
failover_group_t* ha_group_create(const char* name, const char* description);
int ha_group_destroy(uint32_t group_id);
int ha_group_add_service(uint32_t group_id, uint32_t service_id);
int ha_group_remove_service(uint32_t group_id, uint32_t service_id);
int ha_group_set_strategy(uint32_t group_id, failover_strategy_t strategy);
int ha_group_trigger_failover(uint32_t group_id);
failover_group_t* ha_group_find(uint32_t group_id);
failover_group_t* ha_group_find_by_name(const char* name);

// Load balancing
load_balancer_t* ha_lb_create(const char* name, uint32_t port);
int ha_lb_destroy(uint32_t lb_id);
int ha_lb_add_node(uint32_t lb_id, uint32_t node_id);
int ha_lb_remove_node(uint32_t lb_id, uint32_t node_id);
int ha_lb_set_algorithm(uint32_t lb_id, load_balancing_algorithm_t algorithm);
int ha_lb_set_max_connections(uint32_t lb_id, uint32_t max_conn);
load_balancer_t* ha_lb_find(uint32_t lb_id);
load_balancer_t* ha_lb_find_by_name(const char* name);

// Replication
replication_pair_t* ha_replication_create(const char* name, uint32_t source_node, uint32_t target_node);
int ha_replication_destroy(uint32_t pair_id);
int ha_replication_sync(uint32_t pair_id);
int ha_replication_set_bidirectional(uint32_t pair_id, bool bidirectional);
int ha_replication_set_real_time(uint32_t pair_id, bool real_time);
replication_pair_t* ha_replication_find(uint32_t pair_id);
replication_pair_t* ha_replication_find_by_name(const char* name);

// System configuration
int ha_set_heartbeat_interval(uint32_t interval);
int ha_set_failover_timeout(uint32_t timeout);
int ha_set_health_check_interval(uint32_t interval);
int ha_enable_auto_failover(bool enabled);
int ha_enable_load_balancing(bool enabled);
int ha_enable_replication(bool enabled);

// Monitoring and statistics
void ha_get_stats(ha_stats_t* stats);
void ha_reset_stats(void);
int ha_health_check(void);
int ha_perform_failover_test(void);

// High availability debugging
void ha_dump_nodes(void);
void ha_dump_services(void);
void ha_dump_groups(void);
void ha_dump_load_balancers(void);
void ha_dump_replication_pairs(void);
void ha_dump_stats(void);

#endif // HIGH_AVAILABILITY_H 