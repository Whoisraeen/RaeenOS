#include "user_management.h"
#include "kernel.h"
#include "security.h"
#include "memory.h"
#include "filesystem.h"
#include <string.h>
#include <stdio.h>

// Global user management system
static user_management_system_t user_system;

// Default system users
static const char* DEFAULT_ADMIN_USER = "admin";
static const char* DEFAULT_ADMIN_PASSWORD = "raeenos2024";
static const char* DEFAULT_SYSTEM_USER = "system";
static const char* DEFAULT_GUEST_USER = "guest";

// Default groups
static const char* ADMIN_GROUP = "admin";
static const char* USER_GROUP = "users";
static const char* SYSTEM_GROUP = "system";
static const char* GUEST_GROUP = "guests";

// Password hashing (simple SHA-256 placeholder)
static void hash_password(const char* password, char* hash) {
    // In a real implementation, this would use proper cryptographic hashing
    // For now, we'll use a simple hash for demonstration
    uint32_t hash_value = 0;
    for (int i = 0; password[i] != '\0'; i++) {
        hash_value = hash_value * 31 + password[i];
    }
    snprintf(hash, 64, "%08x", hash_value);
}

// Password verification
static bool verify_password_hash(const char* password, const char* hash) {
    char computed_hash[64];
    hash_password(password, computed_hash);
    return strcmp(computed_hash, hash) == 0;
}

int user_management_init(void) {
    spinlock_init(&user_system.lock);
    user_system.initialized = true;
    user_system.user_count = 0;
    user_system.group_count = 0;
    user_system.session_count = 0;
    user_system.next_uid = 1000;  // Start UIDs at 1000
    user_system.next_gid = 1000;  // Start GIDs at 1000
    user_system.next_session_id = 1;
    
    // Initialize arrays
    memset(user_system.users, 0, sizeof(user_system.users));
    memset(user_system.groups, 0, sizeof(user_system.groups));
    memset(user_system.sessions, 0, sizeof(user_system.sessions));
    
    // Create default groups
    group_info_t* admin_group = group_create(ADMIN_GROUP, "Administrators");
    group_info_t* user_group = group_create(USER_GROUP, "Regular users");
    group_info_t* system_group = group_create(SYSTEM_GROUP, "System users");
    group_info_t* guest_group = group_create(GUEST_GROUP, "Guest users");
    
    if (!admin_group || !user_group || !system_group || !guest_group) {
        return -1;
    }
    
    // Create default users
    user_info_t* admin_user = user_create(DEFAULT_ADMIN_USER, DEFAULT_ADMIN_PASSWORD, USER_TYPE_ADMIN);
    user_info_t* system_user = user_create(DEFAULT_SYSTEM_USER, "", USER_TYPE_SYSTEM);
    user_info_t* guest_user = user_create(DEFAULT_GUEST_USER, "", USER_TYPE_GUEST);
    
    if (!admin_user || !system_user || !guest_user) {
        return -1;
    }
    
    // Add users to groups
    group_add_member(admin_group, admin_user);
    group_add_member(user_group, admin_user);
    group_add_member(system_group, system_user);
    group_add_member(guest_group, guest_user);
    
    // Set current user to system user initially
    user_system.current_user = system_user;
    
    printf("User management system initialized with %d users and %d groups\n", 
           user_system.user_count, user_system.group_count);
    
    return 0;
}

void user_management_shutdown(void) {
    spinlock_acquire(&user_system.lock);
    
    // Clean up sessions
    for (uint32_t i = 0; i < user_system.session_count; i++) {
        if (user_system.sessions[i]) {
            session_destroy(user_system.sessions[i]);
        }
    }
    
    // Clean up users
    for (uint32_t i = 0; i < user_system.user_count; i++) {
        if (user_system.users[i]) {
            kfree(user_system.users[i]);
        }
    }
    
    // Clean up groups
    for (uint32_t i = 0; i < user_system.group_count; i++) {
        if (user_system.groups[i]) {
            kfree(user_system.groups[i]);
        }
    }
    
    user_system.initialized = false;
    spinlock_release(&user_system.lock);
}

user_management_system_t* user_management_get_system(void) {
    return &user_system;
}

user_info_t* user_create(const char* username, const char* password, user_type_t type) {
    if (!user_system.initialized || !username) {
        return NULL;
    }
    
    spinlock_acquire(&user_system.lock);
    
    // Check if user already exists
    if (user_find_by_username(username)) {
        spinlock_release(&user_system.lock);
        return NULL;
    }
    
    // Allocate user structure
    user_info_t* user = kmalloc(sizeof(user_info_t));
    if (!user) {
        spinlock_release(&user_system.lock);
        return NULL;
    }
    
    // Initialize user
    memset(user, 0, sizeof(user_info_t));
    user->uid = user_system.next_uid++;
    strncpy(user->username, username, MAX_USERNAME_LENGTH - 1);
    user->type = type;
    user->state = USER_STATE_ACTIVE;
    user->primary_gid = user_system.next_gid++;  // Create user group
    user->group_count = 0;
    user->failed_logins = 0;
    user->created_time = get_system_time();
    user->last_login = 0;
    user->password_changed = user_system.next_uid;  // Use UID as timestamp for now
    
    // Set password if provided
    if (password && strlen(password) > 0) {
        hash_password(password, user->password_hash);
        user->auth_method = AUTH_METHOD_PASSWORD;
    } else {
        memset(user->password_hash, 0, sizeof(user->password_hash));
        user->auth_method = AUTH_METHOD_PASSWORD;
    }
    
    // Set home directory
    snprintf(user->home_dir, MAX_HOME_DIR_LENGTH, "/home/%s", username);
    
    // Set default shell
    if (type == USER_TYPE_SYSTEM) {
        strncpy(user->shell, "/bin/system", MAX_SHELL_LENGTH - 1);
    } else {
        strncpy(user->shell, "/bin/sh", MAX_SHELL_LENGTH - 1);
    }
    
    // Add to user list
    user_system.users[user_system.user_count++] = user;
    
    // Create user group
    char group_name[MAX_USERNAME_LENGTH];
    snprintf(group_name, MAX_USERNAME_LENGTH, "%s", username);
    group_info_t* user_group = group_create(group_name, "User's primary group");
    if (user_group) {
        user_group->gid = user->primary_gid;
        group_add_member(user_group, user);
    }
    
    spinlock_release(&user_system.lock);
    
    printf("Created user: %s (UID: %d, GID: %d)\n", username, user->uid, user->primary_gid);
    
    return user;
}

int user_delete(user_info_t* user) {
    if (!user || !user_system.initialized) {
        return -1;
    }
    
    spinlock_acquire(&user_system.lock);
    
    // Mark user as deleted
    user->state = USER_STATE_DELETED;
    
    // Remove from all groups
    for (uint32_t i = 0; i < user_system.group_count; i++) {
        if (user_system.groups[i]) {
            group_remove_member(user_system.groups[i], user);
        }
    }
    
    // Terminate all user sessions
    for (uint32_t i = 0; i < user_system.session_count; i++) {
        if (user_system.sessions[i] && user_system.sessions[i]->uid == user->uid) {
            session_destroy(user_system.sessions[i]);
            user_system.sessions[i] = NULL;
        }
    }
    
    spinlock_release(&user_system.lock);
    
    printf("Deleted user: %s (UID: %d)\n", user->username, user->uid);
    
    return 0;
}

user_info_t* user_find_by_uid(uint32_t uid) {
    if (!user_system.initialized) {
        return NULL;
    }
    
    spinlock_acquire(&user_system.lock);
    
    for (uint32_t i = 0; i < user_system.user_count; i++) {
        if (user_system.users[i] && user_system.users[i]->uid == uid && 
            user_system.users[i]->state != USER_STATE_DELETED) {
            spinlock_release(&user_system.lock);
            return user_system.users[i];
        }
    }
    
    spinlock_release(&user_system.lock);
    return NULL;
}

user_info_t* user_find_by_username(const char* username) {
    if (!user_system.initialized || !username) {
        return NULL;
    }
    
    spinlock_acquire(&user_system.lock);
    
    for (uint32_t i = 0; i < user_system.user_count; i++) {
        if (user_system.users[i] && strcmp(user_system.users[i]->username, username) == 0 && 
            user_system.users[i]->state != USER_STATE_DELETED) {
            spinlock_release(&user_system.lock);
            return user_system.users[i];
        }
    }
    
    spinlock_release(&user_system.lock);
    return NULL;
}

int user_authenticate(const char* username, const char* password, auth_method_t method) {
    if (!username || !password) {
        return -1;
    }
    
    user_info_t* user = user_find_by_username(username);
    if (!user) {
        return -1;
    }
    
    // Check if account is locked
    if (user->state == USER_STATE_LOCKED) {
        return -1;
    }
    
    // Verify password
    if (method == AUTH_METHOD_PASSWORD) {
        if (verify_password_hash(password, user->password_hash)) {
            // Reset failed login count
            user->failed_logins = 0;
            user->last_login = get_system_time();
            return 0;
        } else {
            // Increment failed login count
            user->failed_logins++;
            
            // Lock account after 5 failed attempts
            if (user->failed_logins >= 5) {
                user->state = USER_STATE_LOCKED;
                user->lock_time = get_system_time();
                printf("Account locked for user: %s after %d failed attempts\n", 
                       username, user->failed_logins);
            }
            
            return -1;
        }
    }
    
    return -1;
}

group_info_t* group_create(const char* name, const char* description) {
    if (!user_system.initialized || !name) {
        return NULL;
    }
    
    spinlock_acquire(&user_system.lock);
    
    // Check if group already exists
    if (group_find_by_name(name)) {
        spinlock_release(&user_system.lock);
        return NULL;
    }
    
    // Allocate group structure
    group_info_t* group = kmalloc(sizeof(group_info_t));
    if (!group) {
        spinlock_release(&user_system.lock);
        return NULL;
    }
    
    // Initialize group
    memset(group, 0, sizeof(group_info_t));
    group->gid = user_system.next_gid++;
    strncpy(group->name, name, MAX_USERNAME_LENGTH - 1);
    if (description) {
        strncpy(group->description, description, 255);
    }
    group->member_count = 0;
    group->created_time = get_system_time();
    
    // Add to group list
    user_system.groups[user_system.group_count++] = group;
    
    spinlock_release(&user_system.lock);
    
    printf("Created group: %s (GID: %d)\n", name, group->gid);
    
    return group;
}

int group_add_member(group_info_t* group, user_info_t* user) {
    if (!group || !user || !user_system.initialized) {
        return -1;
    }
    
    spinlock_acquire(&user_system.lock);
    
    // Check if user is already a member
    for (uint32_t i = 0; i < group->member_count; i++) {
        if (group->members[i] == user->uid) {
            spinlock_release(&user_system.lock);
            return 0;  // Already a member
        }
    }
    
    // Add user to group
    if (group->member_count < MAX_USERS) {
        group->members[group->member_count++] = user->uid;
        
        // Add group to user's group list
        if (user->group_count < MAX_GROUPS) {
            user->groups[user->group_count++] = group->gid;
        }
        
        spinlock_release(&user_system.lock);
        return 0;
    }
    
    spinlock_release(&user_system.lock);
    return -1;
}

group_info_t* group_find_by_name(const char* name) {
    if (!user_system.initialized || !name) {
        return NULL;
    }
    
    spinlock_acquire(&user_system.lock);
    
    for (uint32_t i = 0; i < user_system.group_count; i++) {
        if (user_system.groups[i] && strcmp(user_system.groups[i]->name, name) == 0) {
            spinlock_release(&user_system.lock);
            return user_system.groups[i];
        }
    }
    
    spinlock_release(&user_system.lock);
    return NULL;
}

user_session_t* session_create(user_info_t* user, const char* terminal) {
    if (!user || !user_system.initialized) {
        return NULL;
    }
    
    spinlock_acquire(&user_system.lock);
    
    // Allocate session structure
    user_session_t* session = kmalloc(sizeof(user_session_t));
    if (!session) {
        spinlock_release(&user_system.lock);
        return NULL;
    }
    
    // Initialize session
    memset(session, 0, sizeof(user_session_t));
    session->session_id = user_system.next_session_id++;
    session->uid = user->uid;
    session->login_time = get_system_time();
    session->last_activity = session->login_time;
    
    if (terminal) {
        strncpy(session->terminal, terminal, 63);
    } else {
        strncpy(session->terminal, "console", 63);
    }
    
    // Create security context for session
    session->security_context = security_context_create(user->uid, user->primary_gid);
    
    // Add to session list
    user_system.sessions[user_system.session_count++] = session;
    
    spinlock_release(&user_system.lock);
    
    printf("Created session: %d for user %s on %s\n", 
           session->session_id, user->username, session->terminal);
    
    return session;
}

int session_destroy(user_session_t* session) {
    if (!session || !user_system.initialized) {
        return -1;
    }
    
    spinlock_acquire(&user_system.lock);
    
    // Remove from session list
    for (uint32_t i = 0; i < user_system.session_count; i++) {
        if (user_system.sessions[i] == session) {
            // Shift remaining sessions
            for (uint32_t j = i; j < user_system.session_count - 1; j++) {
                user_system.sessions[j] = user_system.sessions[j + 1];
            }
            user_system.session_count--;
            break;
        }
    }
    
    // Clean up security context
    if (session->security_context) {
        security_context_destroy(session->security_context);
    }
    
    // Free session
    kfree(session);
    
    spinlock_release(&user_system.lock);
    
    return 0;
}

user_info_t* user_get_current(void) {
    return user_system.current_user;
}

uint32_t user_get_current_uid(void) {
    return user_system.current_user ? user_system.current_user->uid : 0;
}

uint32_t user_get_current_gid(void) {
    return user_system.current_user ? user_system.current_user->primary_gid : 0;
}

bool user_is_admin(void) {
    return user_system.current_user && user_system.current_user->type == USER_TYPE_ADMIN;
}

bool user_is_system(void) {
    return user_system.current_user && user_system.current_user->type == USER_TYPE_SYSTEM;
}

void user_dump_all(void) {
    if (!user_system.initialized) {
        printf("User management system not initialized\n");
        return;
    }
    
    printf("\n=== User Management System ===\n");
    printf("Total users: %d\n", user_system.user_count);
    printf("Total groups: %d\n", user_system.group_count);
    printf("Active sessions: %d\n", user_system.session_count);
    printf("Current user: %s (UID: %d)\n", 
           user_system.current_user ? user_system.current_user->username : "none",
           user_get_current_uid());
    
    printf("\n--- Users ---\n");
    for (uint32_t i = 0; i < user_system.user_count; i++) {
        if (user_system.users[i] && user_system.users[i]->state != USER_STATE_DELETED) {
            user_info_t* user = user_system.users[i];
            printf("  %s (UID: %d, GID: %d, Type: %d, State: %d)\n",
                   user->username, user->uid, user->primary_gid, user->type, user->state);
        }
    }
    
    printf("\n--- Groups ---\n");
    for (uint32_t i = 0; i < user_system.group_count; i++) {
        if (user_system.groups[i]) {
            group_info_t* group = user_system.groups[i];
            printf("  %s (GID: %d, Members: %d)\n",
                   group->name, group->gid, group->member_count);
        }
    }
    
    printf("\n--- Sessions ---\n");
    for (uint32_t i = 0; i < user_system.session_count; i++) {
        if (user_system.sessions[i]) {
            user_session_t* session = user_system.sessions[i];
            user_info_t* user = user_find_by_uid(session->uid);
            printf("  Session %d: %s on %s (UID: %d)\n",
                   session->session_id, 
                   user ? user->username : "unknown",
                   session->terminal, session->uid);
        }
    }
    printf("=============================\n\n");
} 