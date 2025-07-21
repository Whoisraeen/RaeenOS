#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H

#include "kernel.h"
#include "security.h"
#include <stdint.h>
#include <stdbool.h>

// User management constants
#define MAX_USERS 1000
#define MAX_GROUPS 100
#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 64
#define MAX_HOME_DIR_LENGTH 256
#define MAX_SHELL_LENGTH 64
#define MAX_EMAIL_LENGTH 128

// User types
typedef enum {
    USER_TYPE_REGULAR = 0,
    USER_TYPE_ADMIN = 1,
    USER_TYPE_SYSTEM = 2,
    USER_TYPE_GUEST = 3
} user_type_t;

// User states
typedef enum {
    USER_STATE_ACTIVE = 0,
    USER_STATE_INACTIVE = 1,
    USER_STATE_LOCKED = 2,
    USER_STATE_DELETED = 3
} user_state_t;

// Authentication methods
typedef enum {
    AUTH_METHOD_PASSWORD = 0,
    AUTH_METHOD_PUBLIC_KEY = 1,
    AUTH_METHOD_BIOMETRIC = 2,
    AUTH_METHOD_TOKEN = 3
} auth_method_t;

// User information
typedef struct user_info {
    uint32_t uid;                           // User ID
    char username[MAX_USERNAME_LENGTH];     // Username
    char password_hash[64];                 // Password hash (SHA-256)
    char email[MAX_EMAIL_LENGTH];           // Email address
    char home_dir[MAX_HOME_DIR_LENGTH];     // Home directory
    char shell[MAX_SHELL_LENGTH];           // Default shell
    user_type_t type;                       // User type
    user_state_t state;                     // User state
    uint32_t primary_gid;                   // Primary group ID
    uint32_t groups[MAX_GROUPS];            // Group memberships
    uint32_t group_count;                   // Number of groups
    uint64_t created_time;                  // Account creation time
    uint64_t last_login;                    // Last login time
    uint64_t password_changed;              // Password change time
    uint32_t failed_logins;                 // Failed login attempts
    uint64_t lock_time;                     // Account lock time
    auth_method_t auth_method;              // Authentication method
    security_policy_t* security_policy;     // User-specific security policy
    void* auth_data;                        // Authentication-specific data
} user_info_t;

// Group information
typedef struct group_info {
    uint32_t gid;                           // Group ID
    char name[MAX_USERNAME_LENGTH];         // Group name
    char description[256];                  // Group description
    uint32_t members[MAX_USERS];            // Group members
    uint32_t member_count;                  // Number of members
    uint64_t created_time;                  // Group creation time
    security_policy_t* security_policy;     // Group-specific security policy
} group_info_t;

// Session information
typedef struct user_session {
    uint32_t session_id;                    // Session ID
    uint32_t uid;                           // User ID
    uint64_t login_time;                    // Login time
    uint64_t last_activity;                 // Last activity time
    char terminal[64];                      // Terminal/display
    char remote_host[256];                  // Remote host (if applicable)
    process_t* login_process;               // Login process
    security_context_t* security_context;   // Session security context
    void* session_data;                     // Session-specific data
} user_session_t;

// User management system
typedef struct user_management_system {
    spinlock_t lock;                        // System lock
    bool initialized;                       // Initialization flag
    user_info_t* users[MAX_USERS];          // User list
    uint32_t user_count;                    // Number of users
    group_info_t* groups[MAX_GROUPS];       // Group list
    uint32_t group_count;                   // Number of groups
    user_session_t* sessions[MAX_USERS];    // Active sessions
    uint32_t session_count;                 // Number of active sessions
    user_info_t* current_user;              // Current user (for kernel)
    user_session_t* current_session;        // Current session
    uint32_t next_uid;                      // Next available UID
    uint32_t next_gid;                      // Next available GID
    uint32_t next_session_id;               // Next session ID
} user_management_system_t;

// User statistics
typedef struct user_stats {
    uint32_t total_users;                   // Total number of users
    uint32_t active_users;                  // Active users
    uint32_t online_users;                  // Currently online users
    uint32_t total_groups;                  // Total number of groups
    uint32_t total_sessions;                // Total sessions created
    uint32_t current_sessions;              // Current active sessions
    uint64_t total_logins;                  // Total login attempts
    uint64_t failed_logins;                 // Failed login attempts
    uint64_t last_update;                   // Last statistics update
} user_stats_t;

// Function declarations

// System initialization
int user_management_init(void);
void user_management_shutdown(void);
user_management_system_t* user_management_get_system(void);

// User management
user_info_t* user_create(const char* username, const char* password, user_type_t type);
int user_delete(user_info_t* user);
int user_modify(user_info_t* user);
user_info_t* user_find_by_uid(uint32_t uid);
user_info_t* user_find_by_username(const char* username);
int user_set_password(user_info_t* user, const char* password);
int user_verify_password(user_info_t* user, const char* password);
int user_lock(user_info_t* user);
int user_unlock(user_info_t* user);
int user_disable(user_info_t* user);
int user_enable(user_info_t* user);

// Group management
group_info_t* group_create(const char* name, const char* description);
int group_delete(group_info_t* group);
int group_add_member(group_info_t* group, user_info_t* user);
int group_remove_member(group_info_t* group, user_info_t* user);
group_info_t* group_find_by_gid(uint32_t gid);
group_info_t* group_find_by_name(const char* name);
bool user_in_group(user_info_t* user, group_info_t* group);

// Session management
user_session_t* session_create(user_info_t* user, const char* terminal);
int session_destroy(user_session_t* session);
int session_update_activity(user_session_t* session);
user_session_t* session_find_by_id(uint32_t session_id);
user_session_t* session_find_by_uid(uint32_t uid);
int session_switch(user_session_t* session);

// Authentication
int user_authenticate(const char* username, const char* password, auth_method_t method);
int user_authenticate_public_key(const char* username, const void* public_key, size_t key_size);
int user_authenticate_biometric(const char* username, const void* biometric_data, size_t data_size);
int user_authenticate_token(const char* username, const char* token);
bool user_has_permission(user_info_t* user, const char* permission);
bool user_has_capability(user_info_t* user, capability_t capability);

// Current user operations
user_info_t* user_get_current(void);
user_session_t* session_get_current(void);
uint32_t user_get_current_uid(void);
uint32_t user_get_current_gid(void);
bool user_is_admin(void);
bool user_is_system(void);

// User utilities
uint32_t user_get_next_uid(void);
uint32_t group_get_next_gid(void);
uint32_t session_get_next_id(void);
char* user_get_home_dir(user_info_t* user);
char* user_get_shell(user_info_t* user);
int user_create_home_dir(user_info_t* user);
int user_setup_environment(user_info_t* user);

// User statistics
void user_get_stats(user_stats_t* stats);
void user_reset_stats(void);

// User debugging
void user_dump_all(void);
void user_dump_groups(void);
void user_dump_sessions(void);
void user_dump_stats(void);

#endif // USER_MANAGEMENT_H 