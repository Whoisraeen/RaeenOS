#ifndef RAEENOS_PACKAGE_MANAGER_H
#define RAEENOS_PACKAGE_MANAGER_H

#include "types.h"

// Package structure
typedef struct package {
    char name[64];                      // Package name
    char version[32];                   // Package version
    char description[256];              // Package description
    char repository[64];                // Source repository
    char maintainer[64];                // Package maintainer
    char license[32];                   // License type
    
    // Package metadata
    size_t size;                        // Package size in bytes
    time_t build_time;                  // Build timestamp
    time_t install_time;                // Installation timestamp
    bool installed;                     // Installation status
    
    // Dependencies
    char** dependencies;                // Required packages
    size_t dependency_count;            // Number of dependencies
    char** conflicts;                   // Conflicting packages
    size_t conflict_count;              // Number of conflicts
    
    // Files
    char** files;                       // Package files
    size_t file_count;                  // Number of files
    
    // Checksums
    char md5sum[33];                    // MD5 checksum
    char sha256sum[65];                 // SHA256 checksum
    
    // Linked list
    struct package* next;               // Next package
    struct package* prev;               // Previous package
} package_t;

// Repository structure
typedef struct repository {
    char name[64];                      // Repository name
    char url[256];                      // Repository URL
    char key[256];                      // GPG key for verification
    bool enabled;                       // Repository status
    time_t last_update;                 // Last update time
    
    // Linked list
    struct repository* next;            // Next repository
} repository_t;

// Package manager functions
error_t package_manager_init(void);
void package_manager_shutdown(void);
error_t package_install(const char* package_name, const char* version);
error_t package_remove(const char* package_name);
error_t package_update(const char* package_name);
error_t package_list_installed(package_t** packages, size_t* count);
error_t package_search(const char* query, package_t** packages, size_t* count);
error_t package_add_repository(const char* name, const char* url, const char* key);
error_t package_remove_repository(const char* name);
bool package_manager_is_initialized(void);

// Package syscalls (to be implemented in syscall.c)
u64 sys_package_install(u64 name, u64 version, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
u64 sys_package_remove(u64 name, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
u64 sys_package_update(u64 name, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
u64 sys_package_list(u64 packages, u64 count, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
u64 sys_package_search(u64 query, u64 packages, u64 count, u64 arg4, u64 arg5, u64 arg6);

// Package manager constants
#define MAX_PACKAGE_NAME_LEN 64
#define MAX_PACKAGE_VERSION_LEN 32
#define MAX_PACKAGE_DESCRIPTION_LEN 256
#define MAX_REPOSITORY_NAME_LEN 64
#define MAX_REPOSITORY_URL_LEN 256
#define MAX_MAINTAINER_LEN 64
#define MAX_LICENSE_LEN 32

// Package states
#define PACKAGE_STATE_AVAILABLE 0
#define PACKAGE_STATE_INSTALLED 1
#define PACKAGE_STATE_UPGRADABLE 2
#define PACKAGE_STATE_BROKEN 3

// Repository states
#define REPOSITORY_STATE_DISABLED 0
#define REPOSITORY_STATE_ENABLED 1
#define REPOSITORY_STATE_ERROR 2

// Package manager error codes
#define E_PACKAGE_NOT_FOUND 1001
#define E_PACKAGE_ALREADY_INSTALLED 1002
#define E_PACKAGE_DEPENDENCY_FAILED 1003
#define E_PACKAGE_CONFLICT 1004
#define E_PACKAGE_DOWNLOAD_FAILED 1005
#define E_PACKAGE_VERIFICATION_FAILED 1006
#define E_PACKAGE_INSTALLATION_FAILED 1007
#define E_REPOSITORY_NOT_FOUND 1008
#define E_REPOSITORY_ALREADY_EXISTS 1009
#define E_REPOSITORY_UPDATE_FAILED 1010

#endif // RAEENOS_PACKAGE_MANAGER_H 