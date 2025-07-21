#include "package_manager.h"
#include "filesystem/vfs.h"
#include "security/security.h"
#include "memory/memory.h"
#include <string.h>
#include <stdio.h>

// Package manager state
static bool package_manager_initialized = false;
static package_t* package_list = NULL;
static repository_t* repository_list = NULL;

// Package database
#define PACKAGE_DB_PATH "/var/lib/raeenos/packages.db"
#define PACKAGE_CACHE_PATH "/var/cache/raeenos/packages"

// Initialize package manager
error_t package_manager_init(void) {
    if (package_manager_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing package manager");
    
    // Create package directories
    vfs_mkdir("/var/lib/raeenos", 0755);
    vfs_mkdir("/var/cache/raeenos", 0755);
    vfs_mkdir("/var/cache/raeenos/packages", 0755);
    vfs_mkdir("/usr/local/bin", 0755);
    vfs_mkdir("/usr/local/lib", 0755);
    vfs_mkdir("/usr/local/share", 0755);
    
    // Load package database
    error_t result = package_load_database();
    if (result != SUCCESS) {
        KWARN("Failed to load package database, creating new one");
        result = package_create_database();
        if (result != SUCCESS) {
            KERROR("Failed to create package database");
            return result;
        }
    }
    
    // Initialize default repositories
    result = package_init_default_repositories();
    if (result != SUCCESS) {
        KERROR("Failed to initialize default repositories");
        return result;
    }
    
    package_manager_initialized = true;
    
    KINFO("Package manager initialized");
    return SUCCESS;
}

// Shutdown package manager
void package_manager_shutdown(void) {
    if (!package_manager_initialized) {
        return;
    }
    
    KINFO("Shutting down package manager");
    
    // Save package database
    package_save_database();
    
    // Free package list
    package_t* pkg = package_list;
    while (pkg) {
        package_t* next = pkg->next;
        package_free(pkg);
        pkg = next;
    }
    
    // Free repository list
    repository_t* repo = repository_list;
    while (repo) {
        repository_t* next = repo->next;
        repository_free(repo);
        repo = next;
    }
    
    package_manager_initialized = false;
    
    KINFO("Package manager shutdown complete");
}

// Install a package
error_t package_install(const char* package_name, const char* version) {
    if (!package_name) {
        return E_INVAL;
    }
    
    KINFO("Installing package: %s%s%s", package_name, 
          version ? " version " : "", version ? version : "");
    
    // Check if package is already installed
    package_t* existing = package_find_installed(package_name);
    if (existing) {
        KINFO("Package %s is already installed (version %s)", 
              package_name, existing->version);
        return SUCCESS;
    }
    
    // Find package in repositories
    package_t* pkg = package_find_in_repositories(package_name, version);
    if (!pkg) {
        KERROR("Package %s not found in repositories", package_name);
        return E_NOENT;
    }
    
    // Download package
    error_t result = package_download(pkg);
    if (result != SUCCESS) {
        KERROR("Failed to download package %s", package_name);
        return result;
    }
    
    // Verify package integrity
    result = package_verify(pkg);
    if (result != SUCCESS) {
        KERROR("Package %s failed integrity check", package_name);
        return result;
    }
    
    // Install package files
    result = package_install_files(pkg);
    if (result != SUCCESS) {
        KERROR("Failed to install package %s", package_name);
        return result;
    }
    
    // Update package database
    pkg->installed = true;
    pkg->install_time = hal_get_timestamp();
    package_add_to_database(pkg);
    
    KINFO("Successfully installed package %s version %s", 
          package_name, pkg->version);
    return SUCCESS;
}

// Remove a package
error_t package_remove(const char* package_name) {
    if (!package_name) {
        return E_INVAL;
    }
    
    KINFO("Removing package: %s", package_name);
    
    // Find installed package
    package_t* pkg = package_find_installed(package_name);
    if (!pkg) {
        KERROR("Package %s is not installed", package_name);
        return E_NOENT;
    }
    
    // Check for dependencies
    if (package_has_dependents(pkg)) {
        KERROR("Cannot remove package %s: other packages depend on it", package_name);
        return E_BUSY;
    }
    
    // Remove package files
    error_t result = package_remove_files(pkg);
    if (result != SUCCESS) {
        KERROR("Failed to remove package %s", package_name);
        return result;
    }
    
    // Remove from database
    package_remove_from_database(pkg);
    
    KINFO("Successfully removed package %s", package_name);
    return SUCCESS;
}

// Update a package
error_t package_update(const char* package_name) {
    if (!package_name) {
        return E_INVAL;
    }
    
    KINFO("Updating package: %s", package_name);
    
    // Find installed package
    package_t* installed = package_find_installed(package_name);
    if (!installed) {
        KERROR("Package %s is not installed", package_name);
        return E_NOENT;
    }
    
    // Find latest version in repositories
    package_t* latest = package_find_latest_version(package_name);
    if (!latest) {
        KERROR("No newer version found for package %s", package_name);
        return E_NOENT;
    }
    
    // Compare versions
    if (package_version_compare(installed->version, latest->version) >= 0) {
        KINFO("Package %s is already up to date", package_name);
        return SUCCESS;
    }
    
    // Download new version
    error_t result = package_download(latest);
    if (result != SUCCESS) {
        KERROR("Failed to download new version of package %s", package_name);
        return result;
    }
    
    // Verify package integrity
    result = package_verify(latest);
    if (result != SUCCESS) {
        KERROR("New version of package %s failed integrity check", package_name);
        return result;
    }
    
    // Remove old version
    result = package_remove_files(installed);
    if (result != SUCCESS) {
        KERROR("Failed to remove old version of package %s", package_name);
        return result;
    }
    
    // Install new version
    result = package_install_files(latest);
    if (result != SUCCESS) {
        KERROR("Failed to install new version of package %s", package_name);
        return result;
    }
    
    // Update database
    latest->installed = true;
    latest->install_time = hal_get_timestamp();
    package_update_in_database(installed, latest);
    
    KINFO("Successfully updated package %s from %s to %s", 
          package_name, installed->version, latest->version);
    return SUCCESS;
}

// List installed packages
error_t package_list_installed(package_t** packages, size_t* count) {
    if (!packages || !count) {
        return E_INVAL;
    }
    
    *count = 0;
    package_t* pkg = package_list;
    while (pkg) {
        if (pkg->installed) {
            (*count)++;
        }
        pkg = pkg->next;
    }
    
    if (*count == 0) {
        *packages = NULL;
        return SUCCESS;
    }
    
    *packages = (package_t*)memory_alloc(sizeof(package_t) * (*count));
    if (!*packages) {
        return E_NOMEM;
    }
    
    size_t index = 0;
    pkg = package_list;
    while (pkg && index < *count) {
        if (pkg->installed) {
            memcpy(&(*packages)[index], pkg, sizeof(package_t));
            index++;
        }
        pkg = pkg->next;
    }
    
    return SUCCESS;
}

// Search packages
error_t package_search(const char* query, package_t** packages, size_t* count) {
    if (!query || !packages || !count) {
        return E_INVAL;
    }
    
    *count = 0;
    package_t* pkg = package_list;
    while (pkg) {
        if (strstr(pkg->name, query) || strstr(pkg->description, query)) {
            (*count)++;
        }
        pkg = pkg->next;
    }
    
    if (*count == 0) {
        *packages = NULL;
        return SUCCESS;
    }
    
    *packages = (package_t*)memory_alloc(sizeof(package_t) * (*count));
    if (!*packages) {
        return E_NOMEM;
    }
    
    size_t index = 0;
    pkg = package_list;
    while (pkg && index < *count) {
        if (strstr(pkg->name, query) || strstr(pkg->description, query)) {
            memcpy(&(*packages)[index], pkg, sizeof(package_t));
            index++;
        }
        pkg = pkg->next;
    }
    
    return SUCCESS;
}

// Add repository
error_t package_add_repository(const char* name, const char* url, const char* key) {
    if (!name || !url) {
        return E_INVAL;
    }
    
    KINFO("Adding repository: %s (%s)", name, url);
    
    // Check if repository already exists
    repository_t* existing = repository_find(name);
    if (existing) {
        KERROR("Repository %s already exists", name);
        return E_EXISTS;
    }
    
    // Create repository
    repository_t* repo = (repository_t*)memory_alloc(sizeof(repository_t));
    if (!repo) {
        return E_NOMEM;
    }
    
    strncpy(repo->name, name, sizeof(repo->name) - 1);
    strncpy(repo->url, url, sizeof(repo->url) - 1);
    if (key) {
        strncpy(repo->key, key, sizeof(repo->key) - 1);
    }
    repo->enabled = true;
    
    // Add to repository list
    repo->next = repository_list;
    repository_list = repo;
    
    // Update package list from repository
    error_t result = repository_update_packages(repo);
    if (result != SUCCESS) {
        KWARN("Failed to update packages from repository %s", name);
    }
    
    KINFO("Successfully added repository %s", name);
    return SUCCESS;
}

// Remove repository
error_t package_remove_repository(const char* name) {
    if (!name) {
        return E_INVAL;
    }
    
    KINFO("Removing repository: %s", name);
    
    repository_t* repo = repository_find(name);
    if (!repo) {
        KERROR("Repository %s not found", name);
        return E_NOENT;
    }
    
    // Remove from list
    if (repo == repository_list) {
        repository_list = repo->next;
    } else {
        repository_t* prev = repository_list;
        while (prev && prev->next != repo) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = repo->next;
        }
    }
    
    // Remove packages from this repository
    package_t* pkg = package_list;
    while (pkg) {
        package_t* next = pkg->next;
        if (strcmp(pkg->repository, name) == 0) {
            package_remove_from_list(pkg);
            package_free(pkg);
        }
        pkg = next;
    }
    
    repository_free(repo);
    
    KINFO("Successfully removed repository %s", name);
    return SUCCESS;
}

// Helper functions

// Load package database
static error_t package_load_database(void) {
    file_t* file = vfs_open(PACKAGE_DB_PATH, O_RDONLY, 0);
    if (!file) {
        return E_NOENT;
    }
    
    // TODO: Implement database loading
    vfs_close(file);
    return SUCCESS;
}

// Create package database
static error_t package_create_database(void) {
    file_t* file = vfs_open(PACKAGE_DB_PATH, O_WRONLY | O_CREAT, 0644);
    if (!file) {
        return E_IO;
    }
    
    // TODO: Implement database creation
    vfs_close(file);
    return SUCCESS;
}

// Save package database
static error_t package_save_database(void) {
    file_t* file = vfs_open(PACKAGE_DB_PATH, O_WRONLY | O_CREAT, 0644);
    if (!file) {
        return E_IO;
    }
    
    // TODO: Implement database saving
    vfs_close(file);
    return SUCCESS;
}

// Initialize default repositories
static error_t package_init_default_repositories(void) {
    // Add official RaeenOS repository
    error_t result = package_add_repository("raeenos-official", 
                                           "https://packages.raeenos.com/official", 
                                           NULL);
    if (result != SUCCESS) {
        return result;
    }
    
    // Add community repository
    result = package_add_repository("raeenos-community", 
                                   "https://packages.raeenos.com/community", 
                                   NULL);
    if (result != SUCCESS) {
        return result;
    }
    
    return SUCCESS;
}

// Find installed package
static package_t* package_find_installed(const char* name) {
    package_t* pkg = package_list;
    while (pkg) {
        if (pkg->installed && strcmp(pkg->name, name) == 0) {
            return pkg;
        }
        pkg = pkg->next;
    }
    return NULL;
}

// Find package in repositories
static package_t* package_find_in_repositories(const char* name, const char* version) {
    package_t* pkg = package_list;
    while (pkg) {
        if (strcmp(pkg->name, name) == 0) {
            if (!version || strcmp(pkg->version, version) == 0) {
                return pkg;
            }
        }
        pkg = pkg->next;
    }
    return NULL;
}

// Find latest version
static package_t* package_find_latest_version(const char* name) {
    package_t* latest = NULL;
    package_t* pkg = package_list;
    
    while (pkg) {
        if (strcmp(pkg->name, name) == 0) {
            if (!latest || package_version_compare(pkg->version, latest->version) > 0) {
                latest = pkg;
            }
        }
        pkg = pkg->next;
    }
    
    return latest;
}

// Download package
static error_t package_download(package_t* pkg) {
    if (!pkg) {
        return E_INVAL;
    }
    
    KDEBUG("Downloading package %s version %s", pkg->name, pkg->version);
    
    // TODO: Implement actual download
    // For now, just simulate download
    
    return SUCCESS;
}

// Verify package integrity
static error_t package_verify(package_t* pkg) {
    if (!pkg) {
        return E_INVAL;
    }
    
    KDEBUG("Verifying package %s", pkg->name);
    
    // TODO: Implement integrity verification
    // For now, just return success
    
    return SUCCESS;
}

// Install package files
static error_t package_install_files(package_t* pkg) {
    if (!pkg) {
        return E_INVAL;
    }
    
    KDEBUG("Installing files for package %s", pkg->name);
    
    // TODO: Implement file installation
    // For now, just simulate installation
    
    return SUCCESS;
}

// Remove package files
static error_t package_remove_files(package_t* pkg) {
    if (!pkg) {
        return E_INVAL;
    }
    
    KDEBUG("Removing files for package %s", pkg->name);
    
    // TODO: Implement file removal
    // For now, just simulate removal
    
    return SUCCESS;
}

// Add package to database
static void package_add_to_database(package_t* pkg) {
    if (!pkg) {
        return;
    }
    
    // TODO: Implement database addition
}

// Remove package from database
static void package_remove_from_database(package_t* pkg) {
    if (!pkg) {
        return;
    }
    
    pkg->installed = false;
    // TODO: Implement database removal
}

// Update package in database
static void package_update_in_database(package_t* old_pkg, package_t* new_pkg) {
    if (!old_pkg || !new_pkg) {
        return;
    }
    
    // TODO: Implement database update
}

// Check if package has dependents
static bool package_has_dependents(package_t* pkg) {
    if (!pkg) {
        return false;
    }
    
    // TODO: Implement dependency checking
    return false;
}

// Version comparison
static int package_version_compare(const char* version1, const char* version2) {
    if (!version1 || !version2) {
        return 0;
    }
    
    // TODO: Implement proper version comparison
    return strcmp(version1, version2);
}

// Find repository
static repository_t* repository_find(const char* name) {
    repository_t* repo = repository_list;
    while (repo) {
        if (strcmp(repo->name, name) == 0) {
            return repo;
        }
        repo = repo->next;
    }
    return NULL;
}

// Update packages from repository
static error_t repository_update_packages(repository_t* repo) {
    if (!repo) {
        return E_INVAL;
    }
    
    KDEBUG("Updating packages from repository %s", repo->name);
    
    // TODO: Implement repository update
    return SUCCESS;
}

// Remove package from list
static void package_remove_from_list(package_t* pkg) {
    if (!pkg) {
        return;
    }
    
    if (pkg->prev) {
        pkg->prev->next = pkg->next;
    } else {
        package_list = pkg->next;
    }
    
    if (pkg->next) {
        pkg->next->prev = pkg->prev;
    }
}

// Free package
static void package_free(package_t* pkg) {
    if (!pkg) {
        return;
    }
    
    memory_free(pkg);
}

// Free repository
static void repository_free(repository_t* repo) {
    if (!repo) {
        return;
    }
    
    memory_free(repo);
}

// Check if package manager is initialized
bool package_manager_is_initialized(void) {
    return package_manager_initialized;
} 