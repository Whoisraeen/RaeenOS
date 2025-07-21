#ifndef AUTHENTICATION_SYSTEM_H
#define AUTHENTICATION_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memory.h>

// Authentication System - Advanced user authentication for RaeenOS
// Provides biometric authentication, enterprise security, and multi-factor authentication

// Authentication methods
typedef enum {
    AUTH_METHOD_PASSWORD = 0,        // Password authentication
    AUTH_METHOD_PIN,                 // PIN authentication
    AUTH_METHOD_FINGERPRINT,         // Fingerprint authentication
    AUTH_METHOD_FACE_RECOGNITION,    // Face recognition
    AUTH_METHOD_IRIS_SCAN,           // Iris scanning
    AUTH_METHOD_VOICE_RECOGNITION,   // Voice recognition
    AUTH_METHOD_RETINA_SCAN,         // Retina scanning
    AUTH_METHOD_PALM_PRINT,          // Palm print
    AUTH_METHOD_GESTURE,             // Gesture authentication
    AUTH_METHOD_SMART_CARD,          // Smart card
    AUTH_METHOD_TOKEN,               // Security token
    AUTH_METHOD_SMS,                 // SMS verification
    AUTH_METHOD_EMAIL,               // Email verification
    AUTH_METHOD_APP,                 // Mobile app verification
    AUTH_METHOD_BIOMETRIC,           // Generic biometric
    AUTH_METHOD_MULTI_FACTOR,        // Multi-factor authentication
} auth_method_t;

// Authentication factors
typedef enum {
    AUTH_FACTOR_KNOWLEDGE = 0,       // Something you know (password, PIN)
    AUTH_FACTOR_POSSESSION,          // Something you have (token, card)
    AUTH_FACTOR_INHERENCE,           // Something you are (biometric)
    AUTH_FACTOR_LOCATION,            // Somewhere you are (GPS)
    AUTH_FACTOR_TIME,                // Time-based factor
} auth_factor_t;

// Authentication levels
typedef enum {
    AUTH_LEVEL_NONE = 0,             // No authentication
    AUTH_LEVEL_LOW,                  // Low security (single factor)
    AUTH_LEVEL_MEDIUM,               // Medium security (two factors)
    AUTH_LEVEL_HIGH,                 // High security (multi-factor)
    AUTH_LEVEL_CRITICAL,             // Critical security (biometric + token)
} auth_level_t;

// User roles
typedef enum {
    USER_ROLE_GUEST = 0,             // Guest user
    USER_ROLE_USER,                  // Standard user
    USER_ROLE_POWER_USER,            // Power user
    USER_ROLE_ADMINISTRATOR,         // Administrator
    USER_ROLE_SYSTEM,                // System user
    USER_ROLE_ROOT,                  // Root user
} user_role_t;

// User account status
typedef enum {
    USER_STATUS_ACTIVE = 0,          // Active account
    USER_STATUS_INACTIVE,            // Inactive account
    USER_STATUS_LOCKED,              // Locked account
    USER_STATUS_SUSPENDED,           // Suspended account
    USER_STATUS_EXPIRED,             // Expired account
    USER_STATUS_DELETED,             // Deleted account
} user_status_t;

// Biometric types
typedef enum {
    BIOMETRIC_FINGERPRINT = 0,       // Fingerprint
    BIOMETRIC_FACE,                  // Face recognition
    BIOMETRIC_IRIS,                  // Iris scanning
    BIOMETRIC_RETINA,                // Retina scanning
    BIOMETRIC_VOICE,                 // Voice recognition
    BIOMETRIC_PALM_PRINT,            // Palm print
    BIOMETRIC_GAIT,                  // Gait analysis
    BIOMETRIC_HEART_RATE,            // Heart rate
    BIOMETRIC_BRAIN_WAVE,            // Brain wave pattern
} biometric_type_t;

// User account
typedef struct {
    uint32_t id;
    char username[64];               // Username
    char display_name[128];          // Display name
    char email[256];                 // Email address
    char password_hash[256];         // Password hash
    char salt[64];                   // Password salt
    user_role_t role;                // User role
    user_status_t status;            // Account status
    uint64_t created_time;           // Account creation time
    uint64_t last_login_time;        // Last login time
    uint64_t password_changed_time;  // Password change time
    uint64_t account_expires_time;   // Account expiration time
    uint32_t login_attempts;         // Failed login attempts
    uint32_t max_login_attempts;     // Maximum login attempts
    bool password_expires;           // Password expires
    uint32_t password_age_days;      // Password age in days
    uint32_t max_password_age_days;  // Maximum password age
    bool require_password_change;    // Require password change
    bool account_locked;             // Account locked
    uint64_t lockout_time;           // Lockout time
    uint32_t lockout_duration;       // Lockout duration in minutes
    void* user_data;                 // User-specific data
} user_account_t;

// Biometric data
typedef struct {
    uint32_t id;
    uint32_t user_id;                // Associated user
    biometric_type_t type;           // Biometric type
    char name[64];                   // Biometric name
    void* template_data;             // Biometric template
    uint32_t template_size;          // Template size
    uint32_t quality_score;          // Quality score (0-100)
    bool is_enrolled;                // Is enrolled
    uint64_t enrolled_time;          // Enrollment time
    uint64_t last_used_time;         // Last used time
    uint32_t usage_count;            // Usage count
    bool is_primary;                 // Is primary biometric
} biometric_data_t;

// Authentication session
typedef struct {
    uint32_t id;
    uint32_t user_id;                // Authenticated user
    char session_token[256];         // Session token
    uint64_t created_time;           // Session creation time
    uint64_t expires_time;           // Session expiration time
    uint64_t last_activity_time;     // Last activity time
    auth_level_t auth_level;         // Authentication level
    auth_method_t auth_method;       // Authentication method
    char client_ip[16];              // Client IP address
    char user_agent[256];            // User agent string
    bool is_active;                  // Is active session
    bool is_remembered;              // Is remembered session
    void* session_data;              // Session-specific data
} auth_session_t;

// Authentication policy
typedef struct {
    uint32_t id;
    char name[64];                   // Policy name
    char description[256];           // Policy description
    auth_level_t min_auth_level;     // Minimum authentication level
    uint32_t min_password_length;    // Minimum password length
    bool require_uppercase;          // Require uppercase letters
    bool require_lowercase;          // Require lowercase letters
    bool require_numbers;            // Require numbers
    bool require_symbols;            // Require symbols
    bool prevent_common_passwords;   // Prevent common passwords
    uint32_t password_history_count; // Password history count
    uint32_t max_password_age_days;  // Maximum password age
    uint32_t min_password_age_days;  // Minimum password age
    uint32_t max_login_attempts;     // Maximum login attempts
    uint32_t lockout_duration;       // Lockout duration in minutes
    bool require_mfa;                // Require multi-factor authentication
    auth_method_t* allowed_methods;  // Allowed authentication methods
    uint32_t allowed_method_count;   // Number of allowed methods
    bool enforce_session_timeout;    // Enforce session timeout
    uint32_t session_timeout_minutes; // Session timeout in minutes
    bool enforce_idle_timeout;       // Enforce idle timeout
    uint32_t idle_timeout_minutes;   // Idle timeout in minutes
    bool is_active;                  // Is policy active
} auth_policy_t;

// Multi-factor authentication
typedef struct {
    uint32_t id;
    uint32_t user_id;                // Associated user
    auth_method_t primary_method;    // Primary authentication method
    auth_method_t secondary_method;  // Secondary authentication method
    auth_method_t tertiary_method;   // Tertiary authentication method
    bool is_enabled;                 // Is MFA enabled
    bool require_backup_codes;       // Require backup codes
    char backup_codes[10][16];       // Backup codes
    uint32_t backup_codes_used;      // Number of backup codes used
    uint64_t last_used_time;         // Last MFA usage time
    uint32_t usage_count;            // MFA usage count
} mfa_config_t;

// Authentication system configuration
typedef struct {
    bool enable_password_auth;       // Enable password authentication
    bool enable_biometric_auth;      // Enable biometric authentication
    bool enable_mfa;                 // Enable multi-factor authentication
    bool enable_smart_card_auth;     // Enable smart card authentication
    bool enable_token_auth;          // Enable token authentication
    bool enable_ldap_auth;           // Enable LDAP authentication
    bool enable_kerberos_auth;       // Enable Kerberos authentication
    bool enable_sso;                 // Enable single sign-on
    uint32_t max_users;              // Maximum users
    uint32_t max_sessions;           // Maximum sessions
    uint32_t max_biometric_templates; // Maximum biometric templates
    uint32_t session_timeout_minutes; // Default session timeout
    uint32_t idle_timeout_minutes;   // Default idle timeout
    uint32_t password_min_length;    // Minimum password length
    uint32_t password_max_age_days;  // Maximum password age
    uint32_t max_login_attempts;     // Maximum login attempts
    uint32_t lockout_duration;       // Lockout duration
    bool enable_audit_logging;       // Enable audit logging
    bool enable_password_history;    // Enable password history
    bool enable_account_lockout;     // Enable account lockout
} auth_system_config_t;

// Authentication system context
typedef struct {
    auth_system_config_t config;
    user_account_t* users;
    uint32_t user_count;
    uint32_t max_users;
    biometric_data_t* biometrics;
    uint32_t biometric_count;
    uint32_t max_biometrics;
    auth_session_t* sessions;
    uint32_t session_count;
    uint32_t max_sessions;
    auth_policy_t* policies;
    uint32_t policy_count;
    uint32_t max_policies;
    mfa_config_t* mfa_configs;
    uint32_t mfa_count;
    uint32_t max_mfa_configs;
    user_account_t* current_user;
    auth_session_t* current_session;
    bool initialized;
    uint32_t next_user_id;
    uint32_t next_biometric_id;
    uint32_t next_session_id;
    uint32_t next_policy_id;
    uint32_t next_mfa_id;
    uint64_t last_cleanup_time;
} auth_system_t;

// Function prototypes

// Initialization and shutdown
auth_system_t* auth_system_init(auth_system_config_t* config);
void auth_system_shutdown(auth_system_t* auth);
bool auth_system_is_initialized(auth_system_t* auth);

// User account management
uint32_t auth_system_create_user(auth_system_t* auth, const char* username, const char* display_name, const char* email, const char* password);
void auth_system_delete_user(auth_system_t* auth, uint32_t user_id);
user_account_t* auth_system_get_user(auth_system_t* auth, uint32_t user_id);
user_account_t* auth_system_get_user_by_username(auth_system_t* auth, const char* username);
user_account_t* auth_system_get_user_by_email(auth_system_t* auth, const char* email);
user_account_t* auth_system_get_users(auth_system_t* auth, uint32_t* count);
uint32_t auth_system_get_user_count(auth_system_t* auth);
void auth_system_update_user(auth_system_t* auth, uint32_t user_id, user_account_t* user_data);
void auth_system_set_user_role(auth_system_t* auth, uint32_t user_id, user_role_t role);
user_role_t auth_system_get_user_role(auth_system_t* auth, uint32_t user_id);
void auth_system_set_user_status(auth_system_t* auth, uint32_t user_id, user_status_t status);
user_status_t auth_system_get_user_status(auth_system_t* auth, uint32_t user_id);

// Password management
bool auth_system_set_password(auth_system_t* auth, uint32_t user_id, const char* password);
bool auth_system_verify_password(auth_system_t* auth, uint32_t user_id, const char* password);
bool auth_system_change_password(auth_system_t* auth, uint32_t user_id, const char* old_password, const char* new_password);
bool auth_system_reset_password(auth_system_t* auth, uint32_t user_id, const char* new_password);
bool auth_system_is_password_expired(auth_system_t* auth, uint32_t user_id);
uint32_t auth_system_get_password_age_days(auth_system_t* auth, uint32_t user_id);
bool auth_system_require_password_change(auth_system_t* auth, uint32_t user_id, bool require);

// Authentication
bool auth_system_authenticate_password(auth_system_t* auth, const char* username, const char* password);
bool auth_system_authenticate_pin(auth_system_t* auth, uint32_t user_id, const char* pin);
bool auth_system_authenticate_biometric(auth_system_t* auth, uint32_t user_id, biometric_type_t type, void* biometric_data);
bool auth_system_authenticate_smart_card(auth_system_t* auth, uint32_t user_id, const char* card_data);
bool auth_system_authenticate_token(auth_system_t* auth, uint32_t user_id, const char* token_code);
bool auth_system_authenticate_mfa(auth_system_t* auth, uint32_t user_id, auth_method_t method, const char* code);
auth_session_t* auth_system_create_session(auth_system_t* auth, uint32_t user_id, auth_level_t level, auth_method_t method);
void auth_system_destroy_session(auth_system_t* auth, uint32_t session_id);

// Session management
auth_session_t* auth_system_get_session(auth_system_t* auth, uint32_t session_id);
auth_session_t* auth_system_get_session_by_token(auth_system_t* auth, const char* token);
auth_session_t* auth_system_get_sessions(auth_system_t* auth, uint32_t* count);
uint32_t auth_system_get_session_count(auth_system_t* auth);
bool auth_system_validate_session(auth_system_t* auth, uint32_t session_id);
bool auth_system_is_session_expired(auth_system_t* auth, uint32_t session_id);
void auth_system_extend_session(auth_system_t* auth, uint32_t session_id, uint32_t minutes);
void auth_system_update_session_activity(auth_system_t* auth, uint32_t session_id);
user_account_t* auth_system_get_current_user(auth_system_t* auth);
auth_session_t* auth_system_get_current_session(auth_system_t* auth);

// Biometric management
uint32_t auth_system_enroll_biometric(auth_system_t* auth, uint32_t user_id, biometric_type_t type, const char* name, void* template_data, uint32_t template_size);
void auth_system_remove_biometric(auth_system_t* auth, uint32_t biometric_id);
biometric_data_t* auth_system_get_biometric(auth_system_t* auth, uint32_t biometric_id);
biometric_data_t* auth_system_get_user_biometrics(auth_system_t* auth, uint32_t user_id, uint32_t* count);
uint32_t auth_system_get_biometric_count(auth_system_t* auth, uint32_t user_id);
bool auth_system_verify_biometric(auth_system_t* auth, uint32_t biometric_id, void* biometric_data);
void auth_system_set_primary_biometric(auth_system_t* auth, uint32_t biometric_id, bool is_primary);
bool auth_system_is_biometric_enrolled(auth_system_t* auth, uint32_t user_id, biometric_type_t type);

// Multi-factor authentication
uint32_t auth_system_enable_mfa(auth_system_t* auth, uint32_t user_id, auth_method_t primary, auth_method_t secondary);
void auth_system_disable_mfa(auth_system_t* auth, uint32_t user_id);
mfa_config_t* auth_system_get_mfa_config(auth_system_t* auth, uint32_t user_id);
bool auth_system_is_mfa_enabled(auth_system_t* auth, uint32_t user_id);
bool auth_system_verify_mfa(auth_system_t* auth, uint32_t user_id, auth_method_t method, const char* code);
void auth_system_generate_backup_codes(auth_system_t* auth, uint32_t user_id);
bool auth_system_verify_backup_code(auth_system_t* auth, uint32_t user_id, const char* code);

// Policy management
uint32_t auth_system_create_policy(auth_system_t* auth, auth_policy_t* policy);
void auth_system_delete_policy(auth_system_t* auth, uint32_t policy_id);
auth_policy_t* auth_system_get_policy(auth_system_t* auth, uint32_t policy_id);
auth_policy_t* auth_system_get_policies(auth_system_t* auth, uint32_t* count);
uint32_t auth_system_get_policy_count(auth_system_t* auth);
void auth_system_apply_policy(auth_system_t* auth, uint32_t user_id, uint32_t policy_id);
bool auth_system_validate_password_policy(auth_system_t* auth, uint32_t policy_id, const char* password);
bool auth_system_check_password_history(auth_system_t* auth, uint32_t user_id, const char* password);

// Account security
void auth_system_lock_account(auth_system_t* auth, uint32_t user_id);
void auth_system_unlock_account(auth_system_t* auth, uint32_t user_id);
bool auth_system_is_account_locked(auth_system_t* auth, uint32_t user_id);
void auth_system_increment_login_attempts(auth_system_t* auth, uint32_t user_id);
void auth_system_reset_login_attempts(auth_system_t* auth, uint32_t user_id);
uint32_t auth_system_get_login_attempts(auth_system_t* auth, uint32_t user_id);
bool auth_system_is_account_locked_out(auth_system_t* auth, uint32_t user_id);
uint64_t auth_system_get_lockout_time(auth_system_t* auth, uint32_t user_id);

// Audit logging
void auth_system_log_login_attempt(auth_system_t* auth, uint32_t user_id, bool success, const char* ip_address, const char* user_agent);
void auth_system_log_password_change(auth_system_t* auth, uint32_t user_id, bool success);
void auth_system_log_account_lockout(auth_system_t* auth, uint32_t user_id, const char* reason);
void auth_system_log_session_creation(auth_system_t* auth, uint32_t user_id, uint32_t session_id);
void auth_system_log_session_destruction(auth_system_t* auth, uint32_t user_id, uint32_t session_id);
void auth_system_log_privilege_escalation(auth_system_t* auth, uint32_t user_id, user_role_t old_role, user_role_t new_role);

// Information
bool auth_system_is_user_authenticated(auth_system_t* auth, uint32_t user_id);
bool auth_system_has_permission(auth_system_t* auth, uint32_t user_id, const char* permission);
bool auth_system_is_admin(auth_system_t* auth, uint32_t user_id);
bool auth_system_is_root(auth_system_t* auth, uint32_t user_id);
uint64_t auth_system_get_last_login_time(auth_system_t* auth, uint32_t user_id);
uint32_t auth_system_get_active_session_count(auth_system_t* auth);
uint32_t auth_system_get_locked_account_count(auth_system_t* auth);

// Utility functions
char* auth_system_hash_password(const char* password, const char* salt);
char* auth_system_generate_salt(void);
char* auth_system_generate_session_token(void);
char* auth_system_generate_backup_code(void);
bool auth_system_validate_username(const char* username);
bool auth_system_validate_email(const char* email);
bool auth_system_validate_password_strength(const char* password);
uint32_t auth_system_calculate_password_strength(const char* password);
char* auth_system_get_role_name(user_role_t role);
char* auth_system_get_status_name(user_status_t status);
char* auth_system_get_method_name(auth_method_t method);
char* auth_system_get_biometric_name(biometric_type_t type);

// Callbacks
typedef void (*auth_event_callback_t)(auth_system_t* auth, uint32_t user_id, const char* event_type, void* event_data, void* user_data);
typedef void (*session_event_callback_t)(auth_system_t* auth, uint32_t session_id, const char* event_type, void* event_data, void* user_data);

void auth_system_set_auth_event_callback(auth_system_t* auth, auth_event_callback_t callback, void* user_data);
void auth_system_set_session_event_callback(auth_system_t* auth, session_event_callback_t callback, void* user_data);

// Preset configurations
auth_system_config_t auth_system_preset_normal_style(void);
auth_system_config_t auth_system_preset_secure_style(void);
auth_system_config_t auth_system_preset_enterprise_style(void);
auth_system_config_t auth_system_preset_government_style(void);

// Error handling
typedef enum {
    AUTH_SYSTEM_SUCCESS = 0,
    AUTH_SYSTEM_ERROR_INVALID_CONTEXT,
    AUTH_SYSTEM_ERROR_INVALID_USER,
    AUTH_SYSTEM_ERROR_INVALID_SESSION,
    AUTH_SYSTEM_ERROR_INVALID_PASSWORD,
    AUTH_SYSTEM_ERROR_INVALID_BIOMETRIC,
    AUTH_SYSTEM_ERROR_AUTHENTICATION_FAILED,
    AUTH_SYSTEM_ERROR_ACCOUNT_LOCKED,
    AUTH_SYSTEM_ERROR_SESSION_EXPIRED,
    AUTH_SYSTEM_ERROR_INSUFFICIENT_PRIVILEGES,
    AUTH_SYSTEM_ERROR_OUT_OF_MEMORY,
    AUTH_SYSTEM_ERROR_POLICY_VIOLATION,
} auth_system_error_t;

auth_system_error_t auth_system_get_last_error(void);
const char* auth_system_get_error_string(auth_system_error_t error);

#endif // AUTHENTICATION_SYSTEM_H 