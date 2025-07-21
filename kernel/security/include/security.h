#ifndef SECURITY_H
#define SECURITY_H

#include "types.h"
#include <stdbool.h>

// Security framework initialization
error_t security_init(void);
void security_shutdown(void);
bool security_is_initialized(void);

// Access Control Lists (ACL) and permissions
typedef enum {
    PERM_READ    = (1 << 0),
    PERM_WRITE   = (1 << 1),
    PERM_EXECUTE = (1 << 2),
    PERM_DELETE  = (1 << 3),
    PERM_MODIFY  = (1 << 4),
    PERM_ADMIN   = (1 << 5),
    PERM_ALL     = 0x3F
} permission_t;

typedef struct {
    uid_t uid;
    gid_t gid;
    permission_t permissions;
} acl_entry_t;

typedef struct {
    u32 entry_count;
    acl_entry_t* entries;
    permission_t default_permissions;
} acl_t;

// Security contexts
typedef enum {
    SEC_CONTEXT_KERNEL,
    SEC_CONTEXT_SYSTEM,
    SEC_CONTEXT_USER,
    SEC_CONTEXT_SANDBOX,
    SEC_CONTEXT_UNTRUSTED
} security_context_t;

typedef struct {
    security_context_t context;
    uid_t uid;
    gid_t gid;
    u32 capabilities;
    bool privileged;
    char label[64];
} security_token_t;

// Sandboxing
typedef struct sandbox sandbox_t;

typedef enum {
    SANDBOX_STRICT,    // Minimal permissions, isolated filesystem
    SANDBOX_NORMAL,    // Standard application permissions
    SANDBOX_RELAXED,   // Extended permissions for system apps
    SANDBOX_NONE       // No sandboxing (kernel/system only)
} sandbox_policy_t;

typedef struct {
    sandbox_policy_t policy;
    bool allow_network;
    bool allow_filesystem_write;
    bool allow_device_access;
    bool allow_process_spawn;
    bool allow_ipc;
    u32 memory_limit_mb;
    u32 cpu_time_limit_ms;
    char* allowed_paths[64];
    u32 allowed_path_count;
} sandbox_config_t;

// Mandatory Access Control (MAC)
typedef enum {
    MAC_LABEL_UNCLASSIFIED,
    MAC_LABEL_CONFIDENTIAL,
    MAC_LABEL_SECRET,
    MAC_LABEL_TOP_SECRET
} mac_label_t;

typedef struct {
    mac_label_t classification;
    u32 categories;
    char compartments[256];
} mac_security_label_t;

// Encryption and cryptography
typedef enum {
    CRYPTO_AES_128,
    CRYPTO_AES_256,
    CRYPTO_CHACHA20,
    CRYPTO_RSA_2048,
    CRYPTO_RSA_4096,
    CRYPTO_ECDSA_P256,
    CRYPTO_ECDSA_P384
} crypto_algorithm_t;

typedef struct {
    crypto_algorithm_t algorithm;
    u8* key_data;
    u32 key_size;
    u32 flags;
} crypto_key_t;

typedef struct {
    u8 hash[64];
    u32 hash_size;
    crypto_algorithm_t algorithm;
} crypto_hash_t;

// System integrity
typedef struct {
    u32 version;
    crypto_hash_t kernel_hash;
    crypto_hash_t bootloader_hash;
    crypto_hash_t system_hash;
    timestamp_t measurement_time;
    bool secure_boot_enabled;
    bool integrity_verified;
} integrity_manifest_t;

// Security policies
typedef enum {
    POLICY_DEFAULT_DENY,  // Deny all actions unless explicitly allowed
    POLICY_DEFAULT_ALLOW, // Allow all actions unless explicitly denied
    POLICY_CAPABILITY,    // Capability-based access control
    POLICY_MANDATORY      // Mandatory access control
} security_policy_type_t;

typedef struct {
    security_policy_type_t type;
    bool enforce_sandboxing;
    bool require_signatures;
    bool allow_untrusted_code;
    u32 max_privilege_level;
    char name[64];
} security_policy_t;

// Function declarations

// Access control
error_t security_check_permission(security_token_t* token, void* resource, permission_t perm);
error_t security_set_acl(void* resource, acl_t* acl);
error_t security_get_acl(void* resource, acl_t** acl);
error_t security_grant_permission(security_token_t* token, void* resource, permission_t perm);
error_t security_revoke_permission(security_token_t* token, void* resource, permission_t perm);

// Security tokens
security_token_t* security_create_token(uid_t uid, gid_t gid, security_context_t context);
void security_destroy_token(security_token_t* token);
error_t security_elevate_privileges(security_token_t* token, const char* reason);
error_t security_drop_privileges(security_token_t* token);
bool security_has_capability(security_token_t* token, u32 capability);

// Sandboxing
sandbox_t* security_create_sandbox(sandbox_config_t* config);
void security_destroy_sandbox(sandbox_t* sandbox);
error_t security_enter_sandbox(sandbox_t* sandbox, pid_t process_id);
error_t security_exit_sandbox(sandbox_t* sandbox, pid_t process_id);
bool security_is_sandboxed(pid_t process_id);
error_t security_sandbox_check_access(sandbox_t* sandbox, const char* path, permission_t perm);

// Mandatory Access Control
error_t security_set_mac_label(void* object, mac_security_label_t* label);
error_t security_get_mac_label(void* object, mac_security_label_t** label);
bool security_mac_can_access(mac_security_label_t* subject, mac_security_label_t* object, permission_t perm);
error_t security_mac_check_flow(mac_security_label_t* from, mac_security_label_t* to);

// Cryptography
error_t security_crypto_init(void);
crypto_key_t* security_generate_key(crypto_algorithm_t algorithm, u32 key_size);
void security_destroy_key(crypto_key_t* key);
error_t security_encrypt(crypto_key_t* key, const u8* plaintext, u32 plaintext_size, 
                        u8** ciphertext, u32* ciphertext_size);
error_t security_decrypt(crypto_key_t* key, const u8* ciphertext, u32 ciphertext_size,
                        u8** plaintext, u32* plaintext_size);
error_t security_hash(const u8* data, u32 data_size, crypto_hash_t* hash);
error_t security_verify_signature(crypto_key_t* public_key, const u8* data, u32 data_size,
                                 const u8* signature, u32 signature_size);

// System integrity
error_t security_measure_integrity(integrity_manifest_t* manifest);
error_t security_verify_integrity(integrity_manifest_t* manifest);
bool security_is_secure_boot_enabled(void);
error_t security_enable_secure_boot(crypto_key_t* signing_key);
error_t security_verify_code_signature(const u8* code, u32 code_size, const u8* signature);

// Security policies
error_t security_set_policy(security_policy_t* policy);
security_policy_t* security_get_current_policy(void);
error_t security_enforce_policy(security_token_t* token, const char* operation, void* context);

// Security audit and logging
typedef enum {
    AUDIT_ACCESS_DENIED,
    AUDIT_PRIVILEGE_ESCALATION,
    AUDIT_SANDBOX_VIOLATION,
    AUDIT_INTEGRITY_FAILURE,
    AUDIT_CRYPTO_ERROR,
    AUDIT_POLICY_VIOLATION
} audit_event_type_t;

typedef struct {
    audit_event_type_t type;
    timestamp_t timestamp;
    uid_t uid;
    pid_t pid;
    char description[256];
    void* context_data;
} audit_event_t;

error_t security_audit_log(audit_event_t* event);
error_t security_audit_query(audit_event_type_t type, timestamp_t start_time, 
                            timestamp_t end_time, audit_event_t** events, u32* count);

// Security capabilities (bit flags)
#define CAP_ADMIN               (1 << 0)
#define CAP_NETWORK_ADMIN       (1 << 1)
#define CAP_FILE_ADMIN          (1 << 2)
#define CAP_DEVICE_ADMIN        (1 << 3)
#define CAP_PROCESS_ADMIN       (1 << 4)
#define CAP_MEMORY_ADMIN        (1 << 5)
#define CAP_CRYPTO_ADMIN        (1 << 6)
#define CAP_SECURITY_ADMIN      (1 << 7)
#define CAP_DEBUG               (1 << 8)
#define CAP_RAW_NETWORK         (1 << 9)
#define CAP_RAW_DEVICE          (1 << 10)
#define CAP_OVERRIDE_SANDBOX    (1 << 11)
#define CAP_MODIFY_POLICY       (1 << 12)
#define CAP_VIEW_AUDIT_LOG      (1 << 13)
#define CAP_CLEAR_AUDIT_LOG     (1 << 14)
#define CAP_KERNEL_MODULE       (1 << 15)

// Security levels
#define SECURITY_LEVEL_NONE     0
#define SECURITY_LEVEL_BASIC    1
#define SECURITY_LEVEL_STANDARD 2
#define SECURITY_LEVEL_HIGH     3
#define SECURITY_LEVEL_MAXIMUM  4

// Error codes specific to security
#define E_SECURITY_ACCESS_DENIED    -1000
#define E_SECURITY_INSUFFICIENT_PRIV -1001
#define E_SECURITY_SANDBOX_VIOLATION -1002
#define E_SECURITY_INTEGRITY_FAILURE -1003
#define E_SECURITY_CRYPTO_ERROR     -1004
#define E_SECURITY_POLICY_VIOLATION -1005
#define E_SECURITY_AUDIT_FAILURE    -1006
#define E_SECURITY_KEY_ERROR        -1007
#define E_SECURITY_SIGNATURE_INVALID -1008
#define E_SECURITY_UNTRUSTED        -1009

#endif // SECURITY_H