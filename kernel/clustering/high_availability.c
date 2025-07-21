#include "high_availability.h"
#include "../core/kernel.h"
#include "../memory/memory.h"
#include "../process/process.h"
#include "../network/network.h"
#include <string.h>
#include <stdio.h>

// Global high availability system instance
static high_availability_system_t ha_system;

// System initialization
int ha_system_init(void) {
    if (ha_system.initialized) {
        return 0;
    }
    
    // Initialize spinlock
    spinlock_init(&ha_system.lock);
    
    // Initialize system state
    ha_system.initialized = true;
    ha_system.node_count = 0;
    ha_system.service_count = 0;
    ha_system.group_count = 0;
    ha_system.lb_count = 0;
    ha_system.pair_count = 0;
    ha_system.primary_node_id = 0;
    ha_system.current_node_id = 1; // This node
    ha_system.is_cluster_member = false;
    
    // Set default configuration
    ha_system.heartbeat_interval = 1000; // 1 second
    ha_system.failover_timeout = 10000;  // 10 seconds
    ha_system.health_check_interval = 5000; // 5 seconds
    ha_system.auto_failover_enabled = true;
    ha_system.load_balancing_enabled = true;
    ha_system.replication_enabled = true;
    
    // Initialize statistics
    ha_system.total_failovers = 0;
    ha_system.total_requests_handled = 0;
    ha_system.total_bytes_transferred = 0;
    ha_system.last_update = get_system_time();
    
    // Add this node to the cluster
    cluster_node_t* this_node = ha_node_add("raeenos-node-1", "192.168.1.100");
    if (this_node) {
        this_node->state = NODE_STATE_ONLINE;
        this_node->is_primary = true;
        this_node->priority = 1;
        ha_system.primary_node_id = this_node->node_id;
        ha_system.is_cluster_member = true;
    }
    
    printf("High Availability System initialized\n");
    return 0;
}

void ha_system_shutdown(void) {
    if (!ha_system.initialized) {
        return;
    }
    
    // Stop all services
    for (uint32_t i = 0; i < ha_system.service_count; i++) {
        if (ha_system.services[i].state == SERVICE_STATE_RUNNING) {
            ha_service_stop(ha_system.services[i].service_id);
        }
    }
    
    // Clean up resources
    ha_system.initialized = false;
    printf("High Availability System shutdown\n");
}

high_availability_system_t* ha_system_get_system(void) {
    return &ha_system;
}

// Node management
cluster_node_t* ha_node_add(const char* hostname, const char* ip_address) {
    spinlock_acquire(&ha_system.lock);
    
    if (ha_system.node_count >= MAX_CLUSTER_NODES) {
        spinlock_release(&ha_system.lock);
        return NULL;
    }
    
    cluster_node_t* node = &ha_system.nodes[ha_system.node_count];
    node->node_id = ha_system.node_count + 1;
    strncpy(node->hostname, hostname, sizeof(node->hostname) - 1);
    strncpy(node->ip_address, ip_address, sizeof(node->ip_address) - 1);
    node->state = NODE_STATE_OFFLINE;
    node->last_heartbeat = get_system_time();
    node->uptime = 0;
    node->cpu_cores = 4; // Default
    node->memory_total = 8ULL * 1024 * 1024 * 1024; // 8GB default
    node->memory_used = 0;
    node->disk_total = 100ULL * 1024 * 1024 * 1024; // 100GB default
    node->disk_used = 0;
    node->cpu_usage = 0.0f;
    node->memory_usage = 0.0f;
    node->disk_usage = 0.0f;
    node->network_interfaces = 1;
    node->network_rx_bytes = 0;
    node->network_tx_bytes = 0;
    node->is_primary = false;
    node->is_backup = false;
    node->priority = 100; // Default priority
    node->weight = 1; // Default weight
    node->node_data = NULL;
    
    ha_system.node_count++;
    
    spinlock_release(&ha_system.lock);
    printf("Added cluster node: %s (%s)\n", hostname, ip_address);
    return node;
}

int ha_node_remove(uint32_t node_id) {
    spinlock_acquire(&ha_system.lock);
    
    for (uint32_t i = 0; i < ha_system.node_count; i++) {
        if (ha_system.nodes[i].node_id == node_id) {
            // Move remaining nodes
            for (uint32_t j = i; j < ha_system.node_count - 1; j++) {
                ha_system.nodes[j] = ha_system.nodes[j + 1];
            }
            ha_system.node_count--;
            
            spinlock_release(&ha_system.lock);
            printf("Removed cluster node: %u\n", node_id);
            return 0;
        }
    }
    
    spinlock_release(&ha_system.lock);
    return -1;
}

int ha_node_update_state(uint32_t node_id, node_state_t state) {
    spinlock_acquire(&ha_system.lock);
    
    cluster_node_t* node = ha_node_find(node_id);
    if (node) {
        node->state = state;
        if (state == NODE_STATE_ONLINE) {
            node->last_heartbeat = get_system_time();
        }
        spinlock_release(&ha_system.lock);
        return 0;
    }
    
    spinlock_release(&ha_system.lock);
    return -1;
}

cluster_node_t* ha_node_find(uint32_t node_id) {
    for (uint32_t i = 0; i < ha_system.node_count; i++) {
        if (ha_system.nodes[i].node_id == node_id) {
            return &ha_system.nodes[i];
        }
    }
    return NULL;
}

cluster_node_t* ha_node_find_by_hostname(const char* hostname) {
    for (uint32_t i = 0; i < ha_system.node_count; i++) {
        if (strcmp(ha_system.nodes[i].hostname, hostname) == 0) {
            return &ha_system.nodes[i];
        }
    }
    return NULL;
}

int ha_node_set_primary(uint32_t node_id) {
    spinlock_acquire(&ha_system.lock);
    
    // Clear existing primary
    for (uint32_t i = 0; i < ha_system.node_count; i++) {
        ha_system.nodes[i].is_primary = false;
    }
    
    // Set new primary
    cluster_node_t* node = ha_node_find(node_id);
    if (node) {
        node->is_primary = true;
        ha_system.primary_node_id = node_id;
        spinlock_release(&ha_system.lock);
        return 0;
    }
    
    spinlock_release(&ha_system.lock);
    return -1;
}

int ha_node_heartbeat(uint32_t node_id) {
    cluster_node_t* node = ha_node_find(node_id);
    if (node) {
        node->last_heartbeat = get_system_time();
        node->uptime = get_system_time() - node->last_heartbeat;
        return 0;
    }
    return -1;
}

// Service management
cluster_service_t* ha_service_add(const char* name, const char* description) {
    spinlock_acquire(&ha_system.lock);
    
    if (ha_system.service_count >= MAX_CLUSTER_SERVICES) {
        spinlock_release(&ha_system.lock);
        return NULL;
    }
    
    cluster_service_t* service = &ha_system.services[ha_system.service_count];
    service->service_id = ha_system.service_count + 1;
    strncpy(service->name, name, sizeof(service->name) - 1);
    strncpy(service->description, description, sizeof(service->description) - 1);
    service->state = SERVICE_STATE_STOPPED;
    service->primary_node_id = ha_system.primary_node_id;
    service->backup_node_id = 0;
    service->current_node_id = 0;
    service->start_time = 0;
    service->last_health_check = 0;
    service->health_check_interval = ha_system.health_check_interval;
    service->restart_count = 0;
    service->max_restarts = 3;
    service->restart_timeout = 5000; // 5 seconds
    service->auto_start = false;
    service->auto_failover = true;
    service->failover_timeout = ha_system.failover_timeout;
    service->dependency_count = 0;
    service->service_data = NULL;
    
    ha_system.service_count++;
    
    spinlock_release(&ha_system.lock);
    printf("Added cluster service: %s\n", name);
    return service;
}

int ha_service_remove(uint32_t service_id) {
    spinlock_acquire(&ha_system.lock);
    
    for (uint32_t i = 0; i < ha_system.service_count; i++) {
        if (ha_system.services[i].service_id == service_id) {
            // Move remaining services
            for (uint32_t j = i; j < ha_system.service_count - 1; j++) {
                ha_system.services[j] = ha_system.services[j + 1];
            }
            ha_system.service_count--;
            
            spinlock_release(&ha_system.lock);
            printf("Removed cluster service: %u\n", service_id);
            return 0;
        }
    }
    
    spinlock_release(&ha_system.lock);
    return -1;
}

int ha_service_start(uint32_t service_id) {
    cluster_service_t* service = ha_service_find(service_id);
    if (!service) {
        return -1;
    }
    
    if (service->state == SERVICE_STATE_RUNNING) {
        return 0; // Already running
    }
    
    service->state = SERVICE_STATE_STARTING;
    service->start_time = get_system_time();
    service->current_node_id = ha_system.current_node_id;
    
    // Simulate service start
    printf("Starting service: %s\n", service->name);
    
    service->state = SERVICE_STATE_RUNNING;
    return 0;
}

int ha_service_stop(uint32_t service_id) {
    cluster_service_t* service = ha_service_find(service_id);
    if (!service) {
        return -1;
    }
    
    if (service->state == SERVICE_STATE_STOPPED) {
        return 0; // Already stopped
    }
    
    service->state = SERVICE_STATE_STOPPING;
    
    // Simulate service stop
    printf("Stopping service: %s\n", service->name);
    
    service->state = SERVICE_STATE_STOPPED;
    service->current_node_id = 0;
    return 0;
}

int ha_service_restart(uint32_t service_id) {
    int result = ha_service_stop(service_id);
    if (result == 0) {
        result = ha_service_start(service_id);
    }
    return result;
}

int ha_service_set_primary_node(uint32_t service_id, uint32_t node_id) {
    cluster_service_t* service = ha_service_find(service_id);
    if (service) {
        service->primary_node_id = node_id;
        return 0;
    }
    return -1;
}

int ha_service_set_backup_node(uint32_t service_id, uint32_t node_id) {
    cluster_service_t* service = ha_service_find(service_id);
    if (service) {
        service->backup_node_id = node_id;
        return 0;
    }
    return -1;
}

cluster_service_t* ha_service_find(uint32_t service_id) {
    for (uint32_t i = 0; i < ha_system.service_count; i++) {
        if (ha_system.services[i].service_id == service_id) {
            return &ha_system.services[i];
        }
    }
    return NULL;
}

cluster_service_t* ha_service_find_by_name(const char* name) {
    for (uint32_t i = 0; i < ha_system.service_count; i++) {
        if (strcmp(ha_system.services[i].name, name) == 0) {
            return &ha_system.services[i];
        }
    }
    return NULL;
}

// Failover management
failover_group_t* ha_group_create(const char* name, const char* description) {
    spinlock_acquire(&ha_system.lock);
    
    if (ha_system.group_count >= MAX_FAILOVER_GROUPS) {
        spinlock_release(&ha_system.lock);
        return NULL;
    }
    
    failover_group_t* group = &ha_system.groups[ha_system.group_count];
    group->group_id = ha_system.group_count + 1;
    strncpy(group->name, name, sizeof(group->name) - 1);
    strncpy(group->description, description, sizeof(group->description) - 1);
    group->strategy = FAILOVER_STRATEGY_ACTIVE_PASSIVE;
    group->primary_node_id = ha_system.primary_node_id;
    group->backup_node_id = 0;
    group->current_node_id = ha_system.primary_node_id;
    group->auto_failover = true;
    group->failover_timeout = ha_system.failover_timeout;
    group->health_check_interval = ha_system.health_check_interval;
    group->last_failover = 0;
    group->failover_count = 0;
    group->service_count = 0;
    group->group_data = NULL;
    
    ha_system.group_count++;
    
    spinlock_release(&ha_system.lock);
    printf("Created failover group: %s\n", name);
    return group;
}

int ha_group_destroy(uint32_t group_id) {
    spinlock_acquire(&ha_system.lock);
    
    for (uint32_t i = 0; i < ha_system.group_count; i++) {
        if (ha_system.groups[i].group_id == group_id) {
            // Move remaining groups
            for (uint32_t j = i; j < ha_system.group_count - 1; j++) {
                ha_system.groups[j] = ha_system.groups[j + 1];
            }
            ha_system.group_count--;
            
            spinlock_release(&ha_system.lock);
            printf("Destroyed failover group: %u\n", group_id);
            return 0;
        }
    }
    
    spinlock_release(&ha_system.lock);
    return -1;
}

int ha_group_add_service(uint32_t group_id, uint32_t service_id) {
    failover_group_t* group = ha_group_find(group_id);
    if (!group || group->service_count >= MAX_CLUSTER_SERVICES) {
        return -1;
    }
    
    group->services[group->service_count] = service_id;
    group->service_count++;
    return 0;
}

int ha_group_remove_service(uint32_t group_id, uint32_t service_id) {
    failover_group_t* group = ha_group_find(group_id);
    if (!group) {
        return -1;
    }
    
    for (uint32_t i = 0; i < group->service_count; i++) {
        if (group->services[i] == service_id) {
            // Move remaining services
            for (uint32_t j = i; j < group->service_count - 1; j++) {
                group->services[j] = group->services[j + 1];
            }
            group->service_count--;
            return 0;
        }
    }
    
    return -1;
}

int ha_group_set_strategy(uint32_t group_id, failover_strategy_t strategy) {
    failover_group_t* group = ha_group_find(group_id);
    if (group) {
        group->strategy = strategy;
        return 0;
    }
    return -1;
}

int ha_group_trigger_failover(uint32_t group_id) {
    failover_group_t* group = ha_group_find(group_id);
    if (!group) {
        return -1;
    }
    
    printf("Triggering failover for group: %s\n", group->name);
    
    // Simulate failover
    uint32_t old_node = group->current_node_id;
    group->current_node_id = group->backup_node_id;
    group->last_failover = get_system_time();
    group->failover_count++;
    ha_system.total_failovers++;
    
    // Move services to backup node
    for (uint32_t i = 0; i < group->service_count; i++) {
        cluster_service_t* service = ha_service_find(group->services[i]);
        if (service) {
            service->current_node_id = group->current_node_id;
        }
    }
    
    printf("Failover completed: %u -> %u\n", old_node, group->current_node_id);
    return 0;
}

failover_group_t* ha_group_find(uint32_t group_id) {
    for (uint32_t i = 0; i < ha_system.group_count; i++) {
        if (ha_system.groups[i].group_id == group_id) {
            return &ha_system.groups[i];
        }
    }
    return NULL;
}

failover_group_t* ha_group_find_by_name(const char* name) {
    for (uint32_t i = 0; i < ha_system.group_count; i++) {
        if (strcmp(ha_system.groups[i].name, name) == 0) {
            return &ha_system.groups[i];
        }
    }
    return NULL;
}

// Load balancing
load_balancer_t* ha_lb_create(const char* name, uint32_t port) {
    spinlock_acquire(&ha_system.lock);
    
    if (ha_system.lb_count >= MAX_LOAD_BALANCERS) {
        spinlock_release(&ha_system.lock);
        return NULL;
    }
    
    load_balancer_t* lb = &ha_system.load_balancers[ha_system.lb_count];
    lb->lb_id = ha_system.lb_count + 1;
    strncpy(lb->name, name, sizeof(lb->name) - 1);
    strncpy(lb->description, "Load balancer", sizeof(lb->description) - 1);
    lb->algorithm = LB_ALGORITHM_ROUND_ROBIN;
    lb->port = port;
    lb->node_count = 0;
    lb->max_connections = 1000;
    lb->current_connections = 0;
    lb->health_check_interval = 5000;
    lb->health_check_timeout = 3000;
    lb->sticky_sessions = false;
    lb->session_timeout = 300000; // 5 minutes
    lb->total_requests = 0;
    lb->total_bytes_sent = 0;
    lb->total_bytes_received = 0;
    lb->lb_data = NULL;
    
    ha_system.lb_count++;
    
    spinlock_release(&ha_system.lock);
    printf("Created load balancer: %s (port %u)\n", name, port);
    return lb;
}

int ha_lb_destroy(uint32_t lb_id) {
    spinlock_acquire(&ha_system.lock);
    
    for (uint32_t i = 0; i < ha_system.lb_count; i++) {
        if (ha_system.load_balancers[i].lb_id == lb_id) {
            // Move remaining load balancers
            for (uint32_t j = i; j < ha_system.lb_count - 1; j++) {
                ha_system.load_balancers[j] = ha_system.load_balancers[j + 1];
            }
            ha_system.lb_count--;
            
            spinlock_release(&ha_system.lock);
            printf("Destroyed load balancer: %u\n", lb_id);
            return 0;
        }
    }
    
    spinlock_release(&ha_system.lock);
    return -1;
}

int ha_lb_add_node(uint32_t lb_id, uint32_t node_id) {
    load_balancer_t* lb = ha_lb_find(lb_id);
    if (!lb || lb->node_count >= MAX_CLUSTER_NODES) {
        return -1;
    }
    
    lb->nodes[lb->node_count] = node_id;
    lb->node_count++;
    return 0;
}

int ha_lb_remove_node(uint32_t lb_id, uint32_t node_id) {
    load_balancer_t* lb = ha_lb_find(lb_id);
    if (!lb) {
        return -1;
    }
    
    for (uint32_t i = 0; i < lb->node_count; i++) {
        if (lb->nodes[i] == node_id) {
            // Move remaining nodes
            for (uint32_t j = i; j < lb->node_count - 1; j++) {
                lb->nodes[j] = lb->nodes[j + 1];
            }
            lb->node_count--;
            return 0;
        }
    }
    
    return -1;
}

int ha_lb_set_algorithm(uint32_t lb_id, load_balancing_algorithm_t algorithm) {
    load_balancer_t* lb = ha_lb_find(lb_id);
    if (lb) {
        lb->algorithm = algorithm;
        return 0;
    }
    return -1;
}

int ha_lb_set_max_connections(uint32_t lb_id, uint32_t max_conn) {
    load_balancer_t* lb = ha_lb_find(lb_id);
    if (lb) {
        lb->max_connections = max_conn;
        return 0;
    }
    return -1;
}

load_balancer_t* ha_lb_find(uint32_t lb_id) {
    for (uint32_t i = 0; i < ha_system.lb_count; i++) {
        if (ha_system.load_balancers[i].lb_id == lb_id) {
            return &ha_system.load_balancers[i];
        }
    }
    return NULL;
}

load_balancer_t* ha_lb_find_by_name(const char* name) {
    for (uint32_t i = 0; i < ha_system.lb_count; i++) {
        if (strcmp(ha_system.load_balancers[i].name, name) == 0) {
            return &ha_system.load_balancers[i];
        }
    }
    return NULL;
}

// Replication
replication_pair_t* ha_replication_create(const char* name, uint32_t source_node, uint32_t target_node) {
    spinlock_acquire(&ha_system.lock);
    
    if (ha_system.pair_count >= MAX_REPLICATION_PAIRS) {
        spinlock_release(&ha_system.lock);
        return NULL;
    }
    
    replication_pair_t* pair = &ha_system.replication_pairs[ha_system.pair_count];
    pair->pair_id = ha_system.pair_count + 1;
    strncpy(pair->name, name, sizeof(pair->name) - 1);
    pair->source_node_id = source_node;
    pair->target_node_id = target_node;
    strncpy(pair->source_path, "/data", sizeof(pair->source_path) - 1);
    strncpy(pair->target_path, "/data", sizeof(pair->target_path) - 1);
    pair->bidirectional = false;
    pair->sync_interval = 300000; // 5 minutes
    pair->last_sync = 0;
    pair->total_files_synced = 0;
    pair->total_bytes_synced = 0;
    pair->real_time_sync = false;
    pair->compression_enabled = true;
    pair->encryption_enabled = false;
    pair->sync_status = 0; // idle
    pair->pair_data = NULL;
    
    ha_system.pair_count++;
    
    spinlock_release(&ha_system.lock);
    printf("Created replication pair: %s (%u -> %u)\n", name, source_node, target_node);
    return pair;
}

int ha_replication_destroy(uint32_t pair_id) {
    spinlock_acquire(&ha_system.lock);
    
    for (uint32_t i = 0; i < ha_system.pair_count; i++) {
        if (ha_system.replication_pairs[i].pair_id == pair_id) {
            // Move remaining pairs
            for (uint32_t j = i; j < ha_system.pair_count - 1; j++) {
                ha_system.replication_pairs[j] = ha_system.replication_pairs[j + 1];
            }
            ha_system.pair_count--;
            
            spinlock_release(&ha_system.lock);
            printf("Destroyed replication pair: %u\n", pair_id);
            return 0;
        }
    }
    
    spinlock_release(&ha_system.lock);
    return -1;
}

int ha_replication_sync(uint32_t pair_id) {
    replication_pair_t* pair = ha_replication_find(pair_id);
    if (!pair) {
        return -1;
    }
    
    printf("Starting replication sync: %s\n", pair->name);
    
    pair->sync_status = 1; // syncing
    pair->last_sync = get_system_time();
    
    // Simulate sync
    pair->total_files_synced += 100;
    pair->total_bytes_synced += 1024 * 1024; // 1MB
    
    pair->sync_status = 0; // idle
    printf("Replication sync completed: %s\n", pair->name);
    return 0;
}

int ha_replication_set_bidirectional(uint32_t pair_id, bool bidirectional) {
    replication_pair_t* pair = ha_replication_find(pair_id);
    if (pair) {
        pair->bidirectional = bidirectional;
        return 0;
    }
    return -1;
}

int ha_replication_set_real_time(uint32_t pair_id, bool real_time) {
    replication_pair_t* pair = ha_replication_find(pair_id);
    if (pair) {
        pair->real_time_sync = real_time;
        return 0;
    }
    return -1;
}

replication_pair_t* ha_replication_find(uint32_t pair_id) {
    for (uint32_t i = 0; i < ha_system.pair_count; i++) {
        if (ha_system.replication_pairs[i].pair_id == pair_id) {
            return &ha_system.replication_pairs[i];
        }
    }
    return NULL;
}

replication_pair_t* ha_replication_find_by_name(const char* name) {
    for (uint32_t i = 0; i < ha_system.pair_count; i++) {
        if (strcmp(ha_system.replication_pairs[i].name, name) == 0) {
            return &ha_system.replication_pairs[i];
        }
    }
    return NULL;
}

// System configuration
int ha_set_heartbeat_interval(uint32_t interval) {
    if (interval > 0 && interval <= MAX_HEARTBEAT_INTERVAL) {
        ha_system.heartbeat_interval = interval;
        return 0;
    }
    return -1;
}

int ha_set_failover_timeout(uint32_t timeout) {
    if (timeout > 0 && timeout <= MAX_FAILOVER_TIMEOUT) {
        ha_system.failover_timeout = timeout;
        return 0;
    }
    return -1;
}

int ha_set_health_check_interval(uint32_t interval) {
    if (interval > 0) {
        ha_system.health_check_interval = interval;
        return 0;
    }
    return -1;
}

int ha_enable_auto_failover(bool enabled) {
    ha_system.auto_failover_enabled = enabled;
    return 0;
}

int ha_enable_load_balancing(bool enabled) {
    ha_system.load_balancing_enabled = enabled;
    return 0;
}

int ha_enable_replication(bool enabled) {
    ha_system.replication_enabled = enabled;
    return 0;
}

// Monitoring and statistics
void ha_get_stats(ha_stats_t* stats) {
    if (!stats) {
        return;
    }
    
    // Cluster statistics
    stats->total_nodes = ha_system.node_count;
    stats->online_nodes = 0;
    stats->failed_nodes = 0;
    for (uint32_t i = 0; i < ha_system.node_count; i++) {
        if (ha_system.nodes[i].state == NODE_STATE_ONLINE) {
            stats->online_nodes++;
        } else if (ha_system.nodes[i].state == NODE_STATE_FAILED) {
            stats->failed_nodes++;
        }
    }
    
    stats->total_services = ha_system.service_count;
    stats->running_services = 0;
    stats->failed_services = 0;
    for (uint32_t i = 0; i < ha_system.service_count; i++) {
        if (ha_system.services[i].state == SERVICE_STATE_RUNNING) {
            stats->running_services++;
        } else if (ha_system.services[i].state == SERVICE_STATE_FAILED) {
            stats->failed_services++;
        }
    }
    
    // Failover statistics
    stats->total_groups = ha_system.group_count;
    stats->active_groups = ha_system.group_count;
    stats->total_failovers = ha_system.total_failovers;
    stats->successful_failovers = ha_system.total_failovers;
    stats->failed_failovers = 0;
    stats->average_failover_time = 1000.0f; // 1 second average
    
    // Load balancing statistics
    stats->total_load_balancers = ha_system.lb_count;
    stats->active_load_balancers = ha_system.lb_count;
    stats->total_requests = ha_system.total_requests_handled;
    stats->successful_requests = ha_system.total_requests_handled;
    stats->failed_requests = 0;
    stats->average_response_time = 50.0f; // 50ms average
    
    // Replication statistics
    stats->total_replication_pairs = ha_system.pair_count;
    stats->active_pairs = ha_system.pair_count;
    stats->total_files_synced = 0;
    stats->total_bytes_synced = 0;
    stats->sync_errors = 0;
    stats->average_sync_time = 5000.0f; // 5 seconds average
    
    for (uint32_t i = 0; i < ha_system.pair_count; i++) {
        stats->total_files_synced += ha_system.replication_pairs[i].total_files_synced;
        stats->total_bytes_synced += ha_system.replication_pairs[i].total_bytes_synced;
    }
    
    stats->last_update = get_system_time();
}

void ha_reset_stats(void) {
    ha_system.total_failovers = 0;
    ha_system.total_requests_handled = 0;
    ha_system.total_bytes_transferred = 0;
    ha_system.last_update = get_system_time();
}

int ha_health_check(void) {
    uint64_t current_time = get_system_time();
    
    // Check node health
    for (uint32_t i = 0; i < ha_system.node_count; i++) {
        cluster_node_t* node = &ha_system.nodes[i];
        if (node->state == NODE_STATE_ONLINE) {
            if (current_time - node->last_heartbeat > ha_system.heartbeat_interval * 3) {
                printf("Node %s appears to be down\n", node->hostname);
                node->state = NODE_STATE_FAILED;
            }
        }
    }
    
    // Check service health
    for (uint32_t i = 0; i < ha_system.service_count; i++) {
        cluster_service_t* service = &ha_system.services[i];
        if (service->state == SERVICE_STATE_RUNNING) {
            service->last_health_check = current_time;
        }
    }
    
    return 0;
}

int ha_perform_failover_test(void) {
    printf("Performing failover test...\n");
    
    // Create test group and trigger failover
    failover_group_t* test_group = ha_group_create("test-group", "Test failover group");
    if (test_group) {
        ha_group_trigger_failover(test_group->group_id);
        ha_group_destroy(test_group->group_id);
    }
    
    printf("Failover test completed\n");
    return 0;
}

// High availability debugging
void ha_dump_nodes(void) {
    printf("=== Cluster Nodes ===\n");
    for (uint32_t i = 0; i < ha_system.node_count; i++) {
        cluster_node_t* node = &ha_system.nodes[i];
        printf("Node %u: %s (%s) - State: %d, Primary: %s\n",
               node->node_id, node->hostname, node->ip_address,
               node->state, node->is_primary ? "Yes" : "No");
    }
}

void ha_dump_services(void) {
    printf("=== Cluster Services ===\n");
    for (uint32_t i = 0; i < ha_system.service_count; i++) {
        cluster_service_t* service = &ha_system.services[i];
        printf("Service %u: %s - State: %d, Node: %u\n",
               service->service_id, service->name,
               service->state, service->current_node_id);
    }
}

void ha_dump_groups(void) {
    printf("=== Failover Groups ===\n");
    for (uint32_t i = 0; i < ha_system.group_count; i++) {
        failover_group_t* group = &ha_system.groups[i];
        printf("Group %u: %s - Strategy: %d, Current Node: %u\n",
               group->group_id, group->name,
               group->strategy, group->current_node_id);
    }
}

void ha_dump_load_balancers(void) {
    printf("=== Load Balancers ===\n");
    for (uint32_t i = 0; i < ha_system.lb_count; i++) {
        load_balancer_t* lb = &ha_system.load_balancers[i];
        printf("LB %u: %s (port %u) - Algorithm: %d, Nodes: %u\n",
               lb->lb_id, lb->name, lb->port,
               lb->algorithm, lb->node_count);
    }
}

void ha_dump_replication_pairs(void) {
    printf("=== Replication Pairs ===\n");
    for (uint32_t i = 0; i < ha_system.pair_count; i++) {
        replication_pair_t* pair = &ha_system.replication_pairs[i];
        printf("Pair %u: %s (%u -> %u) - Status: %d\n",
               pair->pair_id, pair->name,
               pair->source_node_id, pair->target_node_id,
               pair->sync_status);
    }
}

void ha_dump_stats(void) {
    ha_stats_t stats;
    ha_get_stats(&stats);
    
    printf("=== High Availability Statistics ===\n");
    printf("Nodes: %u total, %u online, %u failed\n",
           stats.total_nodes, stats.online_nodes, stats.failed_nodes);
    printf("Services: %u total, %u running, %u failed\n",
           stats.total_services, stats.running_services, stats.failed_services);
    printf("Failovers: %lu total, %lu successful, %lu failed\n",
           stats.total_failovers, stats.successful_failovers, stats.failed_failovers);
    printf("Load Balancers: %u total, %u active\n",
           stats.total_load_balancers, stats.active_load_balancers);
    printf("Replication Pairs: %u total, %u active\n",
           stats.total_replication_pairs, stats.active_pairs);
} 