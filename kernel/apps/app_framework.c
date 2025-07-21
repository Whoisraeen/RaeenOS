#include "app_framework.h"
#include "memory.h"
#include "process.h"
#include "filesystem.h"
#include "gui.h"
#include <string.h>
#include <stddef.h>

// Global application framework
static app_framework_t app_framework = {0};
static application_t* applications = NULL;
static app_runtime_t* runtimes = NULL;
static app_package_t* packages = NULL;
static spinlock_t app_lock = SPINLOCK_INIT;

// Application registry
#define MAX_APPLICATIONS 100
static application_registry_t app_registry = {0};

// Initialize application framework
int app_framework_init(void) {
    memset(&app_framework, 0, sizeof(app_framework_t));
    app_framework.lock = SPINLOCK_INIT;
    app_framework.enabled = true;
    app_framework.auto_update = true;
    app_framework.sandbox_enabled = true;
    
    // Initialize application registry
    app_registry.applications = NULL;
    app_registry.count = 0;
    app_registry.max_count = MAX_APPLICATIONS;
    
    // Initialize runtime system
    if (app_runtime_init() != 0) {
        KERROR("Failed to initialize application runtime system");
        return -1;
    }
    
    // Initialize package manager
    if (app_package_manager_init() != 0) {
        KERROR("Failed to initialize application package manager");
        return -1;
    }
    
    // Initialize application launcher
    if (app_launcher_init() != 0) {
        KERROR("Failed to initialize application launcher");
        return -1;
    }
    
    // Initialize development tools
    if (app_dev_tools_init() != 0) {
        KERROR("Failed to initialize application development tools");
        return -1;
    }
    
    KINFO("Application framework initialized");
    return 0;
}

void app_framework_shutdown(void) {
    // Clean up applications
    application_t* app = applications;
    while (app) {
        application_t* next = app->next;
        application_destroy(app);
        app = next;
    }
    
    // Clean up runtimes
    app_runtime_t* runtime = runtimes;
    while (runtime) {
        app_runtime_t* next = runtime->next;
        app_runtime_destroy(runtime);
        runtime = next;
    }
    
    // Clean up packages
    app_package_t* package = packages;
    while (package) {
        app_package_t* next = package->next;
        app_package_destroy(package);
        package = next;
    }
    
    // Clean up development tools
    app_dev_tools_shutdown();
    
    // Clean up launcher
    app_launcher_shutdown();
    
    // Clean up package manager
    app_package_manager_shutdown();
    
    // Clean up runtime system
    app_runtime_shutdown();
    
    KINFO("Application framework shutdown complete");
}

app_framework_t* app_framework_get_system(void) {
    return &app_framework;
}

// Application management
application_t* application_create(const char* name, const char* path, app_type_t type) {
    if (!name || !path) {
        return NULL;
    }
    
    application_t* app = kmalloc(sizeof(application_t));
    if (!app) {
        return NULL;
    }
    
    memset(app, 0, sizeof(application_t));
    strncpy(app->name, name, sizeof(app->name) - 1);
    strncpy(app->path, path, sizeof(app->path) - 1);
    app->type = type;
    app->state = APP_STATE_INSTALLED;
    app->version = 1;
    app->priority = APP_PRIORITY_NORMAL;
    app->sandbox_level = SANDBOX_LEVEL_APPLICATION;
    
    // Add to application list
    spinlock_acquire(&app_lock);
    app->next = applications;
    applications = app;
    spinlock_release(&app_lock);
    
    // Register in application registry
    app_registry_register(app);
    
    return app;
}

void application_destroy(application_t* app) {
    if (!app) {
        return;
    }
    
    // Remove from application list
    spinlock_acquire(&app_lock);
    application_t* current = applications;
    application_t* prev = NULL;
    
    while (current) {
        if (current == app) {
            if (prev) {
                prev->next = current->next;
            } else {
                applications = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }
    spinlock_release(&app_lock);
    
    // Unregister from application registry
    app_registry_unregister(app);
    
    // Clean up resources
    if (app->icon) {
        kfree(app->icon);
    }
    
    if (app->description) {
        kfree(app->description);
    }
    
    if (app->metadata) {
        kfree(app->metadata);
    }
    
    kfree(app);
}

int application_install(application_t* app) {
    if (!app) {
        return -1;
    }
    
    // Check if application file exists
    if (!vfs_file_exists(app->path)) {
        KERROR("Application file not found: %s", app->path);
        return -1;
    }
    
    // Create application directory
    char app_dir[256];
    snprintf(app_dir, sizeof(app_dir), "/apps/%s", app->name);
    if (vfs_mkdir(app_dir, 0755) != 0) {
        KERROR("Failed to create application directory: %s", app_dir);
        return -1;
    }
    
    // Copy application files
    if (vfs_copy_file(app->path, app_dir) != 0) {
        KERROR("Failed to copy application files");
        return -1;
    }
    
    app->state = APP_STATE_INSTALLED;
    KINFO("Application installed: %s", app->name);
    
    return 0;
}

int application_uninstall(application_t* app) {
    if (!app) {
        return -1;
    }
    
    // Stop application if running
    if (app->state == APP_STATE_RUNNING) {
        application_stop(app);
    }
    
    // Remove application files
    char app_dir[256];
    snprintf(app_dir, sizeof(app_dir), "/apps/%s", app->name);
    if (vfs_remove_directory(app_dir) != 0) {
        KERROR("Failed to remove application directory: %s", app_dir);
        return -1;
    }
    
    app->state = APP_STATE_UNINSTALLED;
    KINFO("Application uninstalled: %s", app->name);
    
    return 0;
}

int application_start(application_t* app) {
    if (!app) {
        return -1;
    }
    
    if (app->state != APP_STATE_INSTALLED) {
        KERROR("Application not installed: %s", app->name);
        return -1;
    }
    
    // Create process for application
    process_t* process = process_create(app->name, app->path);
    if (!process) {
        KERROR("Failed to create process for application: %s", app->name);
        return -1;
    }
    
    // Set up security context
    security_policy_t* policy = security_policy_find("application");
    if (policy) {
        security_context_t* context = security_context_create(process, policy);
        if (context) {
            context->sandbox->level = app->sandbox_level;
        }
    }
    
    // Set up GUI window if needed
    if (app->type == APP_TYPE_GUI) {
        window_t* window = window_create(app->name, (rect_t){100, 100, 800, 600}, 
                                       WINDOW_TYPE_NORMAL, 0);
        if (window) {
            app->window = window;
        }
    }
    
    app->process = process;
    app->state = APP_STATE_RUNNING;
    app->start_time = hal_get_timestamp();
    
    KINFO("Application started: %s (PID: %d)", app->name, process->pid);
    
    return 0;
}

int application_stop(application_t* app) {
    if (!app) {
        return -1;
    }
    
    if (app->state != APP_STATE_RUNNING) {
        KERROR("Application not running: %s", app->name);
        return -1;
    }
    
    // Terminate process
    if (app->process) {
        process_terminate(app->process);
        app->process = NULL;
    }
    
    // Close GUI window
    if (app->window) {
        window_close(app->window);
        app->window = NULL;
    }
    
    app->state = APP_STATE_INSTALLED;
    app->end_time = hal_get_timestamp();
    
    KINFO("Application stopped: %s", app->name);
    
    return 0;
}

int application_pause(application_t* app) {
    if (!app || app->state != APP_STATE_RUNNING) {
        return -1;
    }
    
    if (app->process) {
        process_suspend(app->process);
    }
    
    app->state = APP_STATE_PAUSED;
    
    return 0;
}

int application_resume(application_t* app) {
    if (!app || app->state != APP_STATE_PAUSED) {
        return -1;
    }
    
    if (app->process) {
        process_resume(app->process);
    }
    
    app->state = APP_STATE_RUNNING;
    
    return 0;
}

application_t* application_find(const char* name) {
    if (!name) {
        return NULL;
    }
    
    application_t* app = applications;
    while (app) {
        if (strcmp(app->name, name) == 0) {
            return app;
        }
        app = app->next;
    }
    
    return NULL;
}

application_t* application_find_by_process(process_t* process) {
    if (!process) {
        return NULL;
    }
    
    application_t* app = applications;
    while (app) {
        if (app->process == process) {
            return app;
        }
        app = app->next;
    }
    
    return NULL;
}

// Application registry
int app_registry_register(application_t* app) {
    if (!app || app_registry.count >= app_registry.max_count) {
        return -1;
    }
    
    // Check if already registered
    for (int i = 0; i < app_registry.count; i++) {
        if (app_registry.applications[i] == app) {
            return 0; // Already registered
        }
    }
    
    app_registry.applications[app_registry.count++] = app;
    
    return 0;
}

int app_registry_unregister(application_t* app) {
    if (!app) {
        return -1;
    }
    
    for (int i = 0; i < app_registry.count; i++) {
        if (app_registry.applications[i] == app) {
            // Remove by shifting remaining elements
            for (int j = i; j < app_registry.count - 1; j++) {
                app_registry.applications[j] = app_registry.applications[j + 1];
            }
            app_registry.count--;
            return 0;
        }
    }
    
    return -1;
}

application_t* app_registry_find(const char* name) {
    if (!name) {
        return NULL;
    }
    
    for (int i = 0; i < app_registry.count; i++) {
        if (strcmp(app_registry.applications[i]->name, name) == 0) {
            return app_registry.applications[i];
        }
    }
    
    return NULL;
}

int app_registry_get_all(application_t** apps, int max_count) {
    if (!apps || max_count <= 0) {
        return -1;
    }
    
    int count = app_registry.count < max_count ? app_registry.count : max_count;
    for (int i = 0; i < count; i++) {
        apps[i] = app_registry.applications[i];
    }
    
    return count;
}

// Application runtime
app_runtime_t* app_runtime_create(const char* name, runtime_type_t type) {
    if (!name) {
        return NULL;
    }
    
    app_runtime_t* runtime = kmalloc(sizeof(app_runtime_t));
    if (!runtime) {
        return NULL;
    }
    
    memset(runtime, 0, sizeof(app_runtime_t));
    strncpy(runtime->name, name, sizeof(runtime->name) - 1);
    runtime->type = type;
    runtime->state = RUNTIME_STATE_READY;
    runtime->version = 1;
    
    // Add to runtime list
    spinlock_acquire(&app_lock);
    runtime->next = runtimes;
    runtimes = runtime;
    spinlock_release(&app_lock);
    
    return runtime;
}

void app_runtime_destroy(app_runtime_t* runtime) {
    if (!runtime) {
        return;
    }
    
    // Remove from runtime list
    spinlock_acquire(&app_lock);
    app_runtime_t* current = runtimes;
    app_runtime_t* prev = NULL;
    
    while (current) {
        if (current == runtime) {
            if (prev) {
                prev->next = current->next;
            } else {
                runtimes = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }
    spinlock_release(&app_lock);
    
    kfree(runtime);
}

int app_runtime_load(app_runtime_t* runtime, const char* path) {
    if (!runtime || !path) {
        return -1;
    }
    
    // Load runtime from file
    if (vfs_file_exists(path)) {
        runtime->path = kmalloc(strlen(path) + 1);
        if (runtime->path) {
            strcpy(runtime->path, path);
            runtime->state = RUNTIME_STATE_LOADED;
            return 0;
        }
    }
    
    return -1;
}

int app_runtime_execute(app_runtime_t* runtime, const char* script) {
    if (!runtime || !script) {
        return -1;
    }
    
    if (runtime->state != RUNTIME_STATE_LOADED) {
        return -1;
    }
    
    // Execute script in runtime
    runtime->state = RUNTIME_STATE_EXECUTING;
    
    // This would execute the script in the appropriate runtime
    // (e.g., Python, JavaScript, etc.)
    
    return 0;
}

app_runtime_t* app_runtime_find(const char* name) {
    if (!name) {
        return NULL;
    }
    
    app_runtime_t* runtime = runtimes;
    while (runtime) {
        if (strcmp(runtime->name, name) == 0) {
            return runtime;
        }
        runtime = runtime->next;
    }
    
    return NULL;
}

int app_runtime_init(void) {
    // Create default runtimes
    app_runtime_create("python", RUNTIME_TYPE_PYTHON);
    app_runtime_create("javascript", RUNTIME_TYPE_JAVASCRIPT);
    app_runtime_create("lua", RUNTIME_TYPE_LUA);
    app_runtime_create("native", RUNTIME_TYPE_NATIVE);
    
    KINFO("Application runtime system initialized");
    return 0;
}

void app_runtime_shutdown(void) {
    KINFO("Application runtime system shutdown");
}

// Application package management
app_package_t* app_package_create(const char* name, const char* version) {
    if (!name || !version) {
        return NULL;
    }
    
    app_package_t* package = kmalloc(sizeof(app_package_t));
    if (!package) {
        return NULL;
    }
    
    memset(package, 0, sizeof(app_package_t));
    strncpy(package->name, name, sizeof(package->name) - 1);
    strncpy(package->version, version, sizeof(package->version) - 1);
    package->state = PACKAGE_STATE_AVAILABLE;
    package->size = 0;
    package->dependencies = NULL;
    
    // Add to package list
    spinlock_acquire(&app_lock);
    package->next = packages;
    packages = package;
    spinlock_release(&app_lock);
    
    return package;
}

void app_package_destroy(app_package_t* package) {
    if (!package) {
        return;
    }
    
    // Remove from package list
    spinlock_acquire(&app_lock);
    app_package_t* current = packages;
    app_package_t* prev = NULL;
    
    while (current) {
        if (current == package) {
            if (prev) {
                prev->next = current->next;
            } else {
                packages = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }
    spinlock_release(&app_lock);
    
    // Clean up dependencies
    package_dependency_t* dep = package->dependencies;
    while (dep) {
        package_dependency_t* next = dep->next;
        kfree(dep);
        dep = next;
    }
    
    kfree(package);
}

int app_package_install(app_package_t* package) {
    if (!package) {
        return -1;
    }
    
    // Check dependencies
    if (!app_package_check_dependencies(package)) {
        KERROR("Package dependencies not satisfied: %s", package->name);
        return -1;
    }
    
    // Download package if needed
    if (package->state == PACKAGE_STATE_AVAILABLE) {
        if (app_package_download(package) != 0) {
            KERROR("Failed to download package: %s", package->name);
            return -1;
        }
    }
    
    // Install package
    if (app_package_extract(package) != 0) {
        KERROR("Failed to extract package: %s", package->name);
        return -1;
    }
    
    package->state = PACKAGE_STATE_INSTALLED;
    KINFO("Package installed: %s", package->name);
    
    return 0;
}

int app_package_uninstall(app_package_t* package) {
    if (!package) {
        return -1;
    }
    
    // Remove package files
    char package_dir[256];
    snprintf(package_dir, sizeof(package_dir), "/packages/%s", package->name);
    if (vfs_remove_directory(package_dir) != 0) {
        KERROR("Failed to remove package directory: %s", package_dir);
        return -1;
    }
    
    package->state = PACKAGE_STATE_AVAILABLE;
    KINFO("Package uninstalled: %s", package->name);
    
    return 0;
}

bool app_package_check_dependencies(app_package_t* package) {
    if (!package) {
        return false;
    }
    
    package_dependency_t* dep = package->dependencies;
    while (dep) {
        app_package_t* required = app_package_find(dep->name);
        if (!required || required->state != PACKAGE_STATE_INSTALLED) {
            return false;
        }
        dep = dep->next;
    }
    
    return true;
}

int app_package_download(app_package_t* package) {
    if (!package) {
        return -1;
    }
    
    // Download package from repository
    // This would implement actual download logic
    
    package->state = PACKAGE_STATE_DOWNLOADED;
    return 0;
}

int app_package_extract(app_package_t* package) {
    if (!package) {
        return -1;
    }
    
    // Extract package to filesystem
    // This would implement actual extraction logic
    
    return 0;
}

app_package_t* app_package_find(const char* name) {
    if (!name) {
        return NULL;
    }
    
    app_package_t* package = packages;
    while (package) {
        if (strcmp(package->name, name) == 0) {
            return package;
        }
        package = package->next;
    }
    
    return NULL;
}

int app_package_manager_init(void) {
    KINFO("Application package manager initialized");
    return 0;
}

void app_package_manager_shutdown(void) {
    KINFO("Application package manager shutdown");
}

// Application launcher
int app_launcher_init(void) {
    KINFO("Application launcher initialized");
    return 0;
}

void app_launcher_shutdown(void) {
    KINFO("Application launcher shutdown");
}

int app_launcher_start_application(const char* name) {
    if (!name) {
        return -1;
    }
    
    application_t* app = application_find(name);
    if (!app) {
        KERROR("Application not found: %s", name);
        return -1;
    }
    
    return application_start(app);
}

int app_launcher_stop_application(const char* name) {
    if (!name) {
        return -1;
    }
    
    application_t* app = application_find(name);
    if (!app) {
        KERROR("Application not found: %s", name);
        return -1;
    }
    
    return application_stop(app);
}

int app_launcher_list_applications(application_t** apps, int max_count) {
    return app_registry_get_all(apps, max_count);
}

// Application development tools
int app_dev_tools_init(void) {
    KINFO("Application development tools initialized");
    return 0;
}

void app_dev_tools_shutdown(void) {
    KINFO("Application development tools shutdown");
}

int app_dev_tools_compile(const char* source_path, const char* output_path) {
    if (!source_path || !output_path) {
        return -1;
    }
    
    // Compile source code
    // This would implement actual compilation logic
    
    return 0;
}

int app_dev_tools_debug(application_t* app) {
    if (!app) {
        return -1;
    }
    
    // Start debugging session
    // This would implement actual debugging logic
    
    return 0;
}

int app_dev_tools_profile(application_t* app) {
    if (!app) {
        return -1;
    }
    
    // Start profiling session
    // This would implement actual profiling logic
    
    return 0;
}

// Application utilities
bool app_framework_is_enabled(void) {
    return app_framework.enabled;
}

int app_framework_set_enabled(bool enabled) {
    app_framework.enabled = enabled;
    return 0;
}

bool app_framework_is_auto_update_enabled(void) {
    return app_framework.auto_update;
}

int app_framework_set_auto_update(bool enabled) {
    app_framework.auto_update = enabled;
    return 0;
}

bool app_framework_is_sandbox_enabled(void) {
    return app_framework.sandbox_enabled;
}

int app_framework_set_sandbox_enabled(bool enabled) {
    app_framework.sandbox_enabled = enabled;
    return 0;
}

// Application debugging
void app_framework_dump_applications(void) {
    KINFO("Applications:");
    application_t* app = applications;
    while (app) {
        KINFO("  %s: type=%d state=%d version=%d", 
              app->name, app->type, app->state, app->version);
        app = app->next;
    }
}

void app_framework_dump_runtimes(void) {
    KINFO("Application Runtimes:");
    app_runtime_t* runtime = runtimes;
    while (runtime) {
        KINFO("  %s: type=%d state=%d version=%d", 
              runtime->name, runtime->type, runtime->state, runtime->version);
        runtime = runtime->next;
    }
}

void app_framework_dump_packages(void) {
    KINFO("Application Packages:");
    app_package_t* package = packages;
    while (package) {
        KINFO("  %s: version=%s state=%d size=%zu", 
              package->name, package->version, package->state, package->size);
        package = package->next;
    }
}

// Application statistics
void app_framework_get_stats(app_framework_stats_t* stats) {
    if (!stats) {
        return;
    }
    
    memset(stats, 0, sizeof(app_framework_stats_t));
    
    // Count applications
    application_t* app = applications;
    while (app) {
        stats->app_count++;
        if (app->state == APP_STATE_RUNNING) {
            stats->running_app_count++;
        }
        app = app->next;
    }
    
    // Count runtimes
    app_runtime_t* runtime = runtimes;
    while (runtime) {
        stats->runtime_count++;
        runtime = runtime->next;
    }
    
    // Count packages
    app_package_t* package = packages;
    while (package) {
        stats->package_count++;
        if (package->state == PACKAGE_STATE_INSTALLED) {
            stats->installed_package_count++;
        }
        package = package->next;
    }
    
    // Registry statistics
    stats->registry_count = app_registry.count;
    stats->registry_max_count = app_registry.max_count;
}

void app_framework_reset_stats(void) {
    // Reset application framework statistics
    KINFO("Application framework statistics reset");
} 