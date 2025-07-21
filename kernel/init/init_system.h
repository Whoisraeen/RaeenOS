#ifndef INIT_SYSTEM_H
#define INIT_SYSTEM_H

#include "kernel.h"
#include <stdint.h>
#include <stdbool.h>

// Init system states
typedef enum {
    INIT_STATE_BOOTING,
    INIT_STATE_RUNNING,
    INIT_STATE_SHUTDOWN,
    INIT_STATE_PANIC
} init_state_t;

// Init configuration
typedef struct {
    char hostname[64];
    int runlevel;
    bool auto_login;
    char default_shell[256];
    bool system_services;
} init_config_t;

// Service structure
typedef struct service {
    char name[64];
    char command[256];
    bool auto_start;
    bool running;
    pid_t pid;
    struct service* next;
} service_t;

// Init system functions
error_t init_system_init(void);
void init_main_loop(void);
init_state_t init_get_state(void);
void init_set_state(init_state_t state);
const char* init_get_hostname(void);
timestamp_t init_get_uptime(void);

// Service management
error_t init_start_service(service_t* service);
error_t init_stop_service(service_t* service);
error_t init_register_service(const char* name, const char* command, bool auto_start);
service_t* init_find_service(const char* name);

// Signal handling
void init_setup_signal_handlers(void);

// Logging
void init_log_message(const char* message);
void init_log_error(const char* error);

// Emergency functions
void init_emergency_shell(void);
void init_panic(const char* message);

#endif // INIT_SYSTEM_H 