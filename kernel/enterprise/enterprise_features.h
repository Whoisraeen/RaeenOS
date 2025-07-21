#ifndef RAEENOS_ENTERPRISE_FEATURES_H
#define RAEENOS_ENTERPRISE_FEATURES_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// Enterprise constants
#define MAX_ENTERPRISE_POLICIES 256
#define MAX_ENTERPRISE_DEPLOYMENTS 128
#define MAX_DOMAIN_NAME_LENGTH 256
#define MAX_USERNAME_LENGTH 64
#define MAX_PASSWORD_LENGTH 128
#define MAX_POLICY_NAME_LENGTH 128
#define MAX_PACKAGE_PATH_LENGTH 512

// Enterprise policy types
typedef enum {
    ENTERPRISE_POLICY_SECURITY = 0,
    ENTERPRISE_POLICY_USER = 1,
    ENTERPRISE_POLICY_SYSTEM = 2,
    ENTERPRISE_POLICY_NETWORK = 3,
    ENTERPRISE_POLICY_APPLICATION = 4,
    ENTERPRISE_POLICY_DEVICE = 5
} enterprise_policy_type_t;

// Enterprise deployment status
typedef enum {
    ENTERPRISE_DEPLOYMENT_PENDING = 0,
    ENTERPRISE_DEPLOYMENT_RUNNING = 1,
    ENTERPRISE_DEPLOYMENT_COMPLETED = 2,
    ENTERPRISE_DEPLOYMENT_FAILED = 3,
    ENTERPRISE_DEPLOYMENT_CANCELLED = 4
} enterprise_deployment_status_t;

// Enterprise domain structure
typedef struct {
    char name[MAX_DOMAIN_NAME_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    bool authenticated;
    uint64_t auth_timestamp;
    uint64_t last_sync_timestamp;
    void* private_data;
} enterprise_domain_t;

// Enterprise policy structure
typedef struct {
    char name[MAX_POLICY_NAME_LENGTH];
    enterprise_policy_type_t type;
    bool enabled;
    uint64_t created_timestamp;
    uint64_t modified_timestamp;
    enterprise_domain_t* domain;
    void* policy_data;
    void* private_data;
} enterprise_policy_t;

// Enterprise deployment structure
typedef struct {
    char name[MAX_POLICY_NAME_LENGTH];
    char package_path[MAX_PACKAGE_PATH_LENGTH];
    enterprise_deployment_status_t status;
    uint64_t created_timestamp;
    uint64_t started_timestamp;
    uint64_t completed_timestamp;
    uint64_t cancelled_timestamp;
    int progress_percentage;
    char error_message[256];
    void* private_data;
} enterprise_deployment_t;

// Enterprise features functions
error_t enterprise_init(void);
void enterprise_shutdown(void);

// Domain management functions
error_t enterprise_domain_init(void);
void enterprise_domain_shutdown(void);
error_t enterprise_join_domain(const char* domain_name, const char* username, const char* password);
error_t enterprise_leave_domain(void);
error_t enterprise_authenticate_domain(enterprise_domain_t* domain);
error_t enterprise_apply_domain_policies(enterprise_domain_t* domain);
void enterprise_remove_domain_policies(enterprise_domain_t* domain);

// Group Policy functions
error_t enterprise_policy_init(void);
void enterprise_policy_shutdown(void);
enterprise_policy_t* enterprise_policy_create(const char* name, enterprise_policy_type_t type);
error_t enterprise_policy_add(enterprise_policy_t* policy);
void enterprise_policy_remove(enterprise_policy_t* policy);
error_t enterprise_policy_apply(enterprise_policy_t* policy);
void enterprise_policy_unapply(enterprise_policy_t* policy);

// Centralized deployment functions
error_t enterprise_deployment_init(void);
void enterprise_deployment_shutdown(void);
enterprise_deployment_t* enterprise_deployment_create(const char* name, const char* package_path);
error_t enterprise_deployment_add(enterprise_deployment_t* deployment);
void enterprise_deployment_remove(enterprise_deployment_t* deployment);
error_t enterprise_deployment_execute(enterprise_deployment_t* deployment);
void enterprise_deployment_cancel(enterprise_deployment_t* deployment);

// Enterprise security functions
error_t enterprise_security_init(void);
void enterprise_security_shutdown(void);

// Utility functions
uint64_t enterprise_get_timestamp(void);
bool enterprise_is_initialized(void);
enterprise_domain_t* enterprise_get_current_domain(void);
int enterprise_get_policy_count(void);
int enterprise_get_deployment_count(void);

// Policy application functions
error_t enterprise_apply_security_policy(enterprise_policy_t* policy);
void enterprise_unapply_security_policy(enterprise_policy_t* policy);
error_t enterprise_apply_user_policy(enterprise_policy_t* policy);
void enterprise_unapply_user_policy(enterprise_policy_t* policy);
error_t enterprise_apply_system_policy(enterprise_policy_t* policy);
void enterprise_unapply_system_policy(enterprise_policy_t* policy);
error_t enterprise_apply_network_policy(enterprise_policy_t* policy);
void enterprise_unapply_network_policy(enterprise_policy_t* policy);

// Security component functions
void enterprise_encryption_init(void);
void enterprise_encryption_shutdown(void);
void enterprise_tpm_init(void);
void enterprise_tpm_shutdown(void);
void enterprise_biometric_init(void);
void enterprise_biometric_shutdown(void);
void enterprise_firewall_init(void);
void enterprise_firewall_shutdown(void);
void enterprise_antivirus_init(void);
void enterprise_antivirus_shutdown(void);

// Domain management helper functions
void enterprise_load_domain_config(void);
void enterprise_update_domain_config(enterprise_domain_t* domain);
void enterprise_clear_domain_config(void);
void enterprise_load_domain_policies(enterprise_domain_t* domain);
void enterprise_apply_security_policies(enterprise_domain_t* domain);
void enterprise_apply_user_policies(enterprise_domain_t* domain);

// Policy management helper functions
void enterprise_policy_registry_init(void);
void enterprise_load_local_policies(void);

// Deployment management helper functions
void enterprise_deployment_registry_init(void);
void enterprise_deployment_service_start(void);
void enterprise_deployment_service_stop(void);

// Enterprise security features
#define ENTERPRISE_SECURITY_BITLOCKER 0x00000001
#define ENTERPRISE_SECURITY_TPM 0x00000002
#define ENTERPRISE_SECURITY_BIOMETRIC 0x00000004
#define ENTERPRISE_SECURITY_FIREWALL 0x00000008
#define ENTERPRISE_SECURITY_ANTIVIRUS 0x00000010
#define ENTERPRISE_SECURITY_SECURE_BOOT 0x00000020
#define ENTERPRISE_SECURITY_CODE_SIGNING 0x00000040
#define ENTERPRISE_SECURITY_NETWORK_ISOLATION 0x00000080
#define ENTERPRISE_SECURITY_VPN 0x00000100
#define ENTERPRISE_SECURITY_AUDIT 0x00000200

// Enterprise policy flags
#define ENTERPRISE_POLICY_ENFORCED 0x00000001
#define ENTERPRISE_POLICY_OVERRIDABLE 0x00000002
#define ENTERPRISE_POLICY_TEMPORARY 0x00000004
#define ENTERPRISE_POLICY_USER_SPECIFIC 0x00000008
#define ENTERPRISE_POLICY_MACHINE_SPECIFIC 0x00000010
#define ENTERPRISE_POLICY_REQUIRES_RESTART 0x00000020
#define ENTERPRISE_POLICY_REQUIRES_LOGON 0x00000040

// Enterprise deployment flags
#define ENTERPRISE_DEPLOYMENT_FORCE 0x00000001
#define ENTERPRISE_DEPLOYMENT_ROLLBACK 0x00000002
#define ENTERPRISE_DEPLOYMENT_SILENT 0x00000004
#define ENTERPRISE_DEPLOYMENT_REBOOT 0x00000008
#define ENTERPRISE_DEPLOYMENT_VERIFY 0x00000010
#define ENTERPRISE_DEPLOYMENT_LOG 0x00000020

// Enterprise domain types
#define ENTERPRISE_DOMAIN_TYPE_ACTIVE_DIRECTORY 0x01
#define ENTERPRISE_DOMAIN_TYPE_LDAP 0x02
#define ENTERPRISE_DOMAIN_TYPE_KERBEROS 0x03
#define ENTERPRISE_DOMAIN_TYPE_SAML 0x04
#define ENTERPRISE_DOMAIN_TYPE_OAUTH 0x05

// Enterprise authentication methods
#define ENTERPRISE_AUTH_PASSWORD 0x01
#define ENTERPRISE_AUTH_CERTIFICATE 0x02
#define ENTERPRISE_AUTH_SMARTCARD 0x03
#define ENTERPRISE_AUTH_BIOMETRIC 0x04
#define ENTERPRISE_AUTH_SSO 0x05

// Enterprise policy scopes
#define ENTERPRISE_POLICY_SCOPE_LOCAL 0x01
#define ENTERPRISE_POLICY_SCOPE_SITE 0x02
#define ENTERPRISE_POLICY_SCOPE_DOMAIN 0x03
#define ENTERPRISE_POLICY_SCOPE_OU 0x04
#define ENTERPRISE_POLICY_SCOPE_USER 0x05

// Enterprise deployment types
#define ENTERPRISE_DEPLOYMENT_TYPE_APPLICATION 0x01
#define ENTERPRISE_DEPLOYMENT_TYPE_UPDATE 0x02
#define ENTERPRISE_DEPLOYMENT_TYPE_DRIVER 0x03
#define ENTERPRISE_DEPLOYMENT_TYPE_CONFIGURATION 0x04
#define ENTERPRISE_DEPLOYMENT_TYPE_SCRIPT 0x05

#endif // RAEENOS_ENTERPRISE_FEATURES_H 