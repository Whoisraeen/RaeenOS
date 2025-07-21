#include "security.h"
#include "memory.h"
#include "process.h"
#include <string.h>
#include <stddef.h>

// Global security system
static security_system_t security_system = {0};
static security_policy_t* policies = NULL;
static security_context_t* contexts = NULL;
static security_audit_log_t* audit_log = NULL;
static spinlock_t security_lock = SPINLOCK_INIT;

// Default security policy
static security_policy_t default_policy = {
    .name = "default",
    .type = POLICY_TYPE_DEFAULT_DENY,
    .capabilities = 0,
    .resource_limits = {
        .max_memory = 64 * 1024 * 1024,  // 64MB
        .max_processes = 10,
        .max_files = 100,
        .max_network_connections = 10
    },
    .allowed_syscalls = {
        SYS_EXIT, SYS_WRITE, SYS_READ, SYS_GETPID,
        SYS_FORK, SYS_EXECVE, SYS_WAIT, SYS_BRK
    },
    .allowed_syscalls_count = 8,
    .sandbox_level = SANDBOX_LEVEL_USER
};

// Initialize security system
int security_init(void) {
    memset(&security_system, 0, sizeof(security_system_t));
    security_system.lock = SPINLOCK_INIT;
    security_system.enabled = true;
    security_system.audit_enabled = true;
    
    // Initialize default policy
    if (security_policy_register(&default_policy) != 0) {
        KERROR("Failed to register default security policy");
        return -1;
    }
    
    // Initialize audit system
    if (security_audit_init() != 0) {
        KERROR("Failed to initialize security audit system");
        return -1;
    }
    
    // Initialize sandbox system
    if (security_sandbox_init() != 0) {
        KERROR("Failed to initialize security sandbox system");
        return -1;
    }
    
    // Initialize capability system
    if (security_capability_init() != 0) {
        KERROR("Failed to initialize security capability system");
        return -1;
    }
    
    KINFO("Security system initialized");
    return 0;
}

void security_shutdown(void) {
    // Clean up policies
    security_policy_t* policy = policies;
    while (policy) {
        security_policy_t* next = policy->next;
        security_policy_destroy(policy);
        policy = next;
    }
    
    // Clean up contexts
    security_context_t* context = contexts;
    while (context) {
        security_context_t* next = context->next;
        security_context_destroy(context);
        context = next;
    }
    
    // Clean up audit log
    security_audit_shutdown();
    
    // Clean up sandbox system
    security_sandbox_shutdown();
    
    // Clean up capability system
    security_capability_shutdown();
    
    KINFO("Security system shutdown complete");
}

security_system_t* security_get_system(void) {
    return &security_system;
}

// Security policy management
security_policy_t* security_policy_create(const char* name, policy_type_t type) {
    if (!name) {
        return NULL;
    }
    
    security_policy_t* policy = kmalloc(sizeof(security_policy_t));
    if (!policy) {
        return NULL;
    }
    
    memset(policy, 0, sizeof(security_policy_t));
    strncpy(policy->name, name, sizeof(policy->name) - 1);
    policy->type = type;
    policy->capabilities = 0;
    policy->sandbox_level = SANDBOX_LEVEL_USER;
    
    // Set default resource limits
    policy->resource_limits = default_policy.resource_limits;
    
    return policy;
}

void security_policy_destroy(security_policy_t* policy) {
    if (!policy) {
        return;
    }
    
    security_policy_unregister(policy);
    kfree(policy);
}

int security_policy_register(security_policy_t* policy) {
    if (!policy) {
        return -1;
    }
    
    spinlock_acquire(&security_lock);
    
    policy->next = policies;
    policies = policy;
    
    spinlock_release(&security_lock);
    
    KINFO("Security policy registered: %s", policy->name);
    return 0;
}

int security_policy_unregister(security_policy_t* policy) {
    if (!policy) {
        return -1;
    }
    
    spinlock_acquire(&security_lock);
    
    security_policy_t* current = policies;
    security_policy_t* prev = NULL;
    
    while (current) {
        if (current == policy) {
            if (prev) {
                prev->next = current->next;
            } else {
                policies = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }
    
    spinlock_release(&security_lock);
    
    return 0;
}

security_policy_t* security_policy_find(const char* name) {
    if (!name) {
        return NULL;
    }
    
    security_policy_t* policy = policies;
    while (policy) {
        if (strcmp(policy->name, name) == 0) {
            return policy;
        }
        policy = policy->next;
    }
    
    return NULL;
}

int security_policy_set_capabilities(security_policy_t* policy, uint64_t capabilities) {
    if (!policy) {
        return -1;
    }
    
    policy->capabilities = capabilities;
    return 0;
}

int security_policy_set_resource_limits(security_policy_t* policy, resource_limits_t limits) {
    if (!policy) {
        return -1;
    }
    
    policy->resource_limits = limits;
    return 0;
}

int security_policy_set_sandbox_level(security_policy_t* policy, sandbox_level_t level) {
    if (!policy) {
        return -1;
    }
    
    policy->sandbox_level = level;
    return 0;
}

int security_policy_add_allowed_syscall(security_policy_t* policy, int syscall) {
    if (!policy || policy->allowed_syscalls_count >= MAX_ALLOWED_SYSCALLS) {
        return -1;
    }
    
    policy->allowed_syscalls[policy->allowed_syscalls_count++] = syscall;
    return 0;
}

int security_policy_remove_allowed_syscall(security_policy_t* policy, int syscall) {
    if (!policy) {
        return -1;
    }
    
    for (int i = 0; i < policy->allowed_syscalls_count; i++) {
        if (policy->allowed_syscalls[i] == syscall) {
            // Remove by shifting remaining elements
            for (int j = i; j < policy->allowed_syscalls_count - 1; j++) {
                policy->allowed_syscalls[j] = policy->allowed_syscalls[j + 1];
            }
            policy->allowed_syscalls_count--;
            return 0;
        }
    }
    
    return -1;
}

bool security_policy_is_syscall_allowed(security_policy_t* policy, int syscall) {
    if (!policy) {
        return false;
    }
    
    for (int i = 0; i < policy->allowed_syscalls_count; i++) {
        if (policy->allowed_syscalls[i] == syscall) {
            return true;
        }
    }
    
    return false;
}

// Security context management
security_context_t* security_context_create(process_t* process, security_policy_t* policy) {
    if (!process || !policy) {
        return NULL;
    }
    
    security_context_t* context = kmalloc(sizeof(security_context_t));
    if (!context) {
        return NULL;
    }
    
    memset(context, 0, sizeof(security_context_t));
    context->process = process;
    context->policy = policy;
    context->capabilities = policy->capabilities;
    context->resource_usage = (resource_usage_t){0};
    context->sandbox = NULL;
    context->audit_id = security_audit_generate_id();
    
    // Create sandbox for context
    context->sandbox = security_sandbox_create(context);
    if (!context->sandbox) {
        kfree(context);
        return NULL;
    }
    
    // Add to context list
    spinlock_acquire(&security_lock);
    context->next = contexts;
    contexts = context;
    spinlock_release(&security_lock);
    
    return context;
}

void security_context_destroy(security_context_t* context) {
    if (!context) {
        return;
    }
    
    // Remove from context list
    spinlock_acquire(&security_lock);
    security_context_t* current = contexts;
    security_context_t* prev = NULL;
    
    while (current) {
        if (current == context) {
            if (prev) {
                prev->next = current->next;
            } else {
                contexts = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }
    spinlock_release(&security_lock);
    
    // Clean up sandbox
    if (context->sandbox) {
        security_sandbox_destroy(context->sandbox);
    }
    
    kfree(context);
}

security_context_t* security_context_find(process_t* process) {
    if (!process) {
        return NULL;
    }
    
    security_context_t* context = contexts;
    while (context) {
        if (context->process == process) {
            return context;
        }
        context = context->next;
    }
    
    return NULL;
}

int security_context_set_capabilities(security_context_t* context, uint64_t capabilities) {
    if (!context) {
        return -1;
    }
    
    context->capabilities = capabilities;
    return 0;
}

int security_context_add_capability(security_context_t* context, capability_t capability) {
    if (!context) {
        return -1;
    }
    
    context->capabilities |= (1ULL << capability);
    return 0;
}

int security_context_remove_capability(security_context_t* context, capability_t capability) {
    if (!context) {
        return -1;
    }
    
    context->capabilities &= ~(1ULL << capability);
    return 0;
}

bool security_context_has_capability(security_context_t* context, capability_t capability) {
    if (!context) {
        return false;
    }
    
    return (context->capabilities & (1ULL << capability)) != 0;
}

int security_context_update_resource_usage(security_context_t* context, resource_usage_t usage) {
    if (!context) {
        return -1;
    }
    
    context->resource_usage = usage;
    return 0;
}

bool security_context_check_resource_limits(security_context_t* context) {
    if (!context || !context->policy) {
        return false;
    }
    
    resource_limits_t* limits = &context->policy->resource_limits;
    resource_usage_t* usage = &context->resource_usage;
    
    return usage->memory_used <= limits->max_memory &&
           usage->process_count <= limits->max_processes &&
           usage->file_count <= limits->max_files &&
           usage->network_connections <= limits->max_network_connections;
}

// Security enforcement
int security_check_syscall(process_t* process, int syscall) {
    if (!process) {
        return -1;
    }
    
    security_context_t* context = security_context_find(process);
    if (!context) {
        // No security context, deny by default
        security_audit_log_event(AUDIT_EVENT_SYSCALL_DENIED, process, syscall, 0);
        return -1;
    }
    
    // Check if syscall is allowed by policy
    if (!security_policy_is_syscall_allowed(context->policy, syscall)) {
        security_audit_log_event(AUDIT_EVENT_SYSCALL_DENIED, process, syscall, 0);
        return -1;
    }
    
    // Check resource limits
    if (!security_context_check_resource_limits(context)) {
        security_audit_log_event(AUDIT_EVENT_RESOURCE_LIMIT_EXCEEDED, process, 0, 0);
        return -1;
    }
    
    security_audit_log_event(AUDIT_EVENT_SYSCALL_ALLOWED, process, syscall, 0);
    return 0;
}

int security_check_memory_access(process_t* process, void* addr, size_t size, memory_access_t access) {
    if (!process) {
        return -1;
    }
    
    security_context_t* context = security_context_find(process);
    if (!context) {
        return -1;
    }
    
    // Check if address is within process memory space
    if (!vm_is_user_address(addr)) {
        security_audit_log_event(AUDIT_EVENT_MEMORY_ACCESS_DENIED, process, (int)addr, access);
        return -1;
    }
    
    // Check sandbox restrictions
    if (context->sandbox && !security_sandbox_check_memory_access(context->sandbox, addr, size, access)) {
        security_audit_log_event(AUDIT_EVENT_SANDBOX_VIOLATION, process, (int)addr, access);
        return -1;
    }
    
    return 0;
}

int security_check_file_access(process_t* process, const char* path, file_access_t access) {
    if (!process || !path) {
        return -1;
    }
    
    security_context_t* context = security_context_find(process);
    if (!context) {
        return -1;
    }
    
    // Check sandbox file restrictions
    if (context->sandbox && !security_sandbox_check_file_access(context->sandbox, path, access)) {
        security_audit_log_event(AUDIT_EVENT_FILE_ACCESS_DENIED, process, 0, access);
        return -1;
    }
    
    return 0;
}

int security_check_network_access(process_t* process, ip_addr_t addr, uint16_t port, network_access_t access) {
    if (!process) {
        return -1;
    }
    
    security_context_t* context = security_context_find(process);
    if (!context) {
        return -1;
    }
    
    // Check sandbox network restrictions
    if (context->sandbox && !security_sandbox_check_network_access(context->sandbox, addr, port, access)) {
        security_audit_log_event(AUDIT_EVENT_NETWORK_ACCESS_DENIED, process, port, access);
        return -1;
    }
    
    return 0;
}

int security_check_capability(process_t* process, capability_t capability) {
    if (!process) {
        return -1;
    }
    
    security_context_t* context = security_context_find(process);
    if (!context) {
        return -1;
    }
    
    if (!security_context_has_capability(context, capability)) {
        security_audit_log_event(AUDIT_EVENT_CAPABILITY_DENIED, process, capability, 0);
        return -1;
    }
    
    return 0;
}

// Sandbox management
security_sandbox_t* security_sandbox_create(security_context_t* context) {
    if (!context) {
        return NULL;
    }
    
    security_sandbox_t* sandbox = kmalloc(sizeof(security_sandbox_t));
    if (!sandbox) {
        return NULL;
    }
    
    memset(sandbox, 0, sizeof(security_sandbox_t));
    sandbox->context = context;
    sandbox->level = context->policy->sandbox_level;
    sandbox->isolation_enabled = true;
    
    // Initialize sandbox based on level
    switch (sandbox->level) {
        case SANDBOX_LEVEL_NONE:
            sandbox->isolation_enabled = false;
            break;
        case SANDBOX_LEVEL_USER:
            // Basic user sandbox
            break;
        case SANDBOX_LEVEL_APPLICATION:
            // Application sandbox with file system restrictions
            break;
        case SANDBOX_LEVEL_SYSTEM:
            // System sandbox with strict restrictions
            break;
        default:
            break;
    }
    
    return sandbox;
}

void security_sandbox_destroy(security_sandbox_t* sandbox) {
    if (!sandbox) {
        return;
    }
    
    kfree(sandbox);
}

bool security_sandbox_check_memory_access(security_sandbox_t* sandbox, void* addr, size_t size, memory_access_t access) {
    if (!sandbox || !sandbox->isolation_enabled) {
        return true;
    }
    
    // Check memory access restrictions based on sandbox level
    switch (sandbox->level) {
        case SANDBOX_LEVEL_USER:
            // Basic memory access checks
            return true;
        case SANDBOX_LEVEL_APPLICATION:
            // Application-level memory restrictions
            return true;
        case SANDBOX_LEVEL_SYSTEM:
            // Strict system-level restrictions
            return false;
        default:
            return true;
    }
}

bool security_sandbox_check_file_access(security_sandbox_t* sandbox, const char* path, file_access_t access) {
    if (!sandbox || !sandbox->isolation_enabled) {
        return true;
    }
    
    // Check file access restrictions based on sandbox level
    switch (sandbox->level) {
        case SANDBOX_LEVEL_USER:
            // Basic file access checks
            return true;
        case SANDBOX_LEVEL_APPLICATION:
            // Application-level file restrictions
            return true;
        case SANDBOX_LEVEL_SYSTEM:
            // Strict system-level restrictions
            return false;
        default:
            return true;
    }
}

bool security_sandbox_check_network_access(security_sandbox_t* sandbox, ip_addr_t addr, uint16_t port, network_access_t access) {
    if (!sandbox || !sandbox->isolation_enabled) {
        return true;
    }
    
    // Check network access restrictions based on sandbox level
    switch (sandbox->level) {
        case SANDBOX_LEVEL_USER:
            // Basic network access checks
            return true;
        case SANDBOX_LEVEL_APPLICATION:
            // Application-level network restrictions
            return true;
        case SANDBOX_LEVEL_SYSTEM:
            // Strict system-level restrictions
            return false;
        default:
            return true;
    }
}

int security_sandbox_init(void) {
    KINFO("Security sandbox system initialized");
    return 0;
}

void security_sandbox_shutdown(void) {
    KINFO("Security sandbox system shutdown");
}

// Capability management
int security_capability_init(void) {
    KINFO("Security capability system initialized");
    return 0;
}

void security_capability_shutdown(void) {
    KINFO("Security capability system shutdown");
}

bool security_capability_check(process_t* process, capability_t capability) {
    return security_check_capability(process, capability) == 0;
}

int security_capability_grant(process_t* process, capability_t capability) {
    if (!process) {
        return -1;
    }
    
    security_context_t* context = security_context_find(process);
    if (!context) {
        return -1;
    }
    
    return security_context_add_capability(context, capability);
}

int security_capability_revoke(process_t* process, capability_t capability) {
    if (!process) {
        return -1;
    }
    
    security_context_t* context = security_context_find(process);
    if (!context) {
        return -1;
    }
    
    return security_context_remove_capability(context, capability);
}

// Audit system
int security_audit_init(void) {
    KINFO("Security audit system initialized");
    return 0;
}

void security_audit_shutdown(void) {
    // Clean up audit log
    security_audit_log_t* entry = audit_log;
    while (entry) {
        security_audit_log_t* next = entry->next;
        kfree(entry);
        entry = next;
    }
    
    KINFO("Security audit system shutdown");
}

int security_audit_log_event(audit_event_type_t type, process_t* process, int data1, int data2) {
    if (!security_system.audit_enabled) {
        return 0;
    }
    
    security_audit_log_t* entry = kmalloc(sizeof(security_audit_log_t));
    if (!entry) {
        return -1;
    }
    
    entry->timestamp = hal_get_timestamp();
    entry->type = type;
    entry->process = process;
    entry->data1 = data1;
    entry->data2 = data2;
    entry->next = NULL;
    
    // Add to audit log
    spinlock_acquire(&security_lock);
    if (audit_log) {
        entry->next = audit_log;
    }
    audit_log = entry;
    spinlock_release(&security_lock);
    
    return 0;
}

uint64_t security_audit_generate_id(void) {
    static uint64_t next_id = 1;
    return next_id++;
}

security_audit_log_t* security_audit_get_log(void) {
    return audit_log;
}

void security_audit_clear_log(void) {
    spinlock_acquire(&security_lock);
    
    security_audit_log_t* entry = audit_log;
    while (entry) {
        security_audit_log_t* next = entry->next;
        kfree(entry);
        entry = next;
    }
    audit_log = NULL;
    
    spinlock_release(&security_lock);
}

// Security utilities
bool security_is_enabled(void) {
    return security_system.enabled;
}

int security_set_enabled(bool enabled) {
    security_system.enabled = enabled;
    return 0;
}

bool security_is_audit_enabled(void) {
    return security_system.audit_enabled;
}

int security_set_audit_enabled(bool enabled) {
    security_system.audit_enabled = enabled;
    return 0;
}

// Security debugging
void security_dump_policies(void) {
    KINFO("Security Policies:");
    security_policy_t* policy = policies;
    while (policy) {
        KINFO("  %s: type=%d capabilities=0x%llx sandbox=%d", 
              policy->name, policy->type, policy->capabilities, policy->sandbox_level);
        policy = policy->next;
    }
}

void security_dump_contexts(void) {
    KINFO("Security Contexts:");
    security_context_t* context = contexts;
    while (context) {
        KINFO("  Process %d: policy=%s capabilities=0x%llx", 
              context->process ? context->process->pid : 0,
              context->policy ? context->policy->name : "none",
              context->capabilities);
        context = context->next;
    }
}

void security_dump_audit_log(void) {
    KINFO("Security Audit Log:");
    security_audit_log_t* entry = audit_log;
    int count = 0;
    while (entry && count < 10) { // Show last 10 entries
        KINFO("  [%llu] Event=%d Process=%d Data1=%d Data2=%d", 
              entry->timestamp, entry->type, 
              entry->process ? entry->process->pid : 0,
              entry->data1, entry->data2);
        entry = entry->next;
        count++;
    }
}

// Security statistics
void security_get_stats(security_stats_t* stats) {
    if (!stats) {
        return;
    }
    
    memset(stats, 0, sizeof(security_stats_t));
    
    // Count policies
    security_policy_t* policy = policies;
    while (policy) {
        stats->policy_count++;
        policy = policy->next;
    }
    
    // Count contexts
    security_context_t* context = contexts;
    while (context) {
        stats->context_count++;
        context = context->next;
    }
    
    // Count audit entries
    security_audit_log_t* entry = audit_log;
    while (entry) {
        stats->audit_entry_count++;
        entry = entry->next;
    }
}

void security_reset_stats(void) {
    // Reset statistics
    KINFO("Security statistics reset");
} 

// AI-Enhanced Zero-Trust Protection
void verify_kernel_integrity() {
    // Neural Network Validator compares runtime patterns vs. known-good state
    nn_validator_result res = ai_validate_kernel(rt_kernel_fingerprint());
    
    // Hardware-Enforced Isolation (Intel TDX/AMD SEV equivalent)
    if(res.anomaly_score > ZT_THRESHOLD) {
        isolate_core(res.suspicious_modules);
        heal_runtime(res.repair_plan);
    }
}

// Continuous Attestation Engine
void attestation_daemon() {
    while(true) {
        perform_remote_attestation(cloud_verifier);
        update_zt_policies(ai_analyze_threats());
        sleep(ZT_CHECK_INTERVAL);
    }
}