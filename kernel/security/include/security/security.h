#ifndef SECURITY_H
#define SECURITY_H

#include "../../core/include/types.h"
#include "../../core/include/error.h"
#include "../../process/process.h"

// Security levels
typedef enum {
    SECURITY_LEVEL_NONE = 0,
    SECURITY_LEVEL_USER = 1,
    SECURITY_LEVEL_SYSTEM = 2,
    SECURITY_LEVEL_KERNEL = 3
} security_level_t;

// Security initialization
error_t security_init(void);
void security_shutdown(void);

// Access control
int security_check_syscall(process_t* process, int syscall_num);
error_t security_check_access(process_t* process, const char* resource, int permissions);

// Security statistics
typedef struct {
    uint64_t access_checks;
    uint64_t access_denied;
    uint64_t security_violations;
} security_stats_t;

void security_get_stats(security_stats_t* stats);
void security_dump_policies(void);

// Enterprise security functions (stubs)
error_t enterprise_security_init(void);
void enterprise_security_shutdown(void);
void enterprise_security_perform_compliance_audit(void);

#endif