#ifndef NETWORK_H
#define NETWORK_H

#include "core/include/error.h"

error_t network_init(void);
void network_update(void);
void network_dump_interfaces(void);
uint32_t get_network_interface_count(void);

typedef struct {
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t bytes_sent;
    uint64_t bytes_received;
} network_stats_t;

void network_get_stats(network_stats_t* stats);

#endif