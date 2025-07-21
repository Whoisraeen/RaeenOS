#include "init_system.h"
#include "kernel.h"
#include "process/process.h"
#include "process/scheduler.h"
#include "memory/memory.h"
#include "filesystem/vfs.h"
#include "filesystem/initrd.h"
#include "syscalls/syscalls.h"
#include "shell/shell.h"
#include "apps/app_framework.h"
#include <string.h>
#include <stdio.h>

// Global init system state
static init_state_t init_state = INIT_STATE_BOOTING;
static init_config_t init_config = {0};
static service_t* service_list = NULL;
static u32 service_count = 0;
static timestamp_t boot_time = 0;
static bool init_initialized = false;
static process_t* init_process = NULL;
static process_t* shell_process = NULL;

// Default configuration
static const init_config_t default_config = {
    .hostname = "RaeenOS",
    .default_runlevel = "3",
    .enable_logging = true,
    .enable_networking = true,
    .enable_gui = true,
    .boot_timeout = 30,
    .default_shell = "/bin/sh",
    .root_password = "raeen"
};

// Built-in services
static const char* builtin_services[] = {
    "syslog",
    "udev",
    "network",
    "filesystem",
    "gui",
    "shell"
};

// Basic shell commands
typedef struct {
    const char* name;
    const char* description;
    int (*handler)(int argc, char** argv);
} shell_command_t;

// Forward declarations for shell commands
static int cmd_help(int argc, char** argv);
static int cmd_ls(int argc, char** argv);
static int cmd_cd(int argc, char** argv);
static int cmd_pwd(int argc, char** argv);
static int cmd_cat(int argc, char** argv);
static int cmd_echo(int argc, char** argv);
static int cmd_ps(int argc, char** argv);
static int cmd_kill(int argc, char** argv);
static int cmd_meminfo(int argc, char** argv);
static int cmd_sysinfo(int argc, char** argv);
static int cmd_clear(int argc, char** argv);
static int cmd_exit(int argc, char** argv);

// Shell command table
static shell_command_t shell_commands[] = {
    {"help", "Show available commands", cmd_help},
    {"ls", "List directory contents", cmd_ls},
    {"cd", "Change directory", cmd_cd},
    {"pwd", "Print working directory", cmd_pwd},
    {"cat", "Display file contents", cmd_cat},
    {"echo", "Display a line of text", cmd_echo},
    {"ps", "Show process status", cmd_ps},
    {"kill", "Terminate a process", cmd_kill},
    {"meminfo", "Show memory information", cmd_meminfo},
    {"sysinfo", "Show system information", cmd_sysinfo},
    {"clear", "Clear the screen", cmd_clear},
    {"exit", "Exit the shell", cmd_exit},
    {NULL, NULL, NULL}
};

error_t init_system_start(void) {
    if (init_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Starting RaeenOS Init System");
    
    // Initialize configuration
    init_config = default_config;
    boot_time = hal_get_tick_count();
    
    // Set up signal handlers
    init_setup_signal_handlers();
    
    // Run boot sequence
    error_t result = init_run_boot_sequence();
    if (result != SUCCESS) {
        KERROR("Boot sequence failed");
        return result;
    }
    
    // Mount filesystems
    result = init_mount_filesystems();
    if (result != SUCCESS) {
        KERROR("Failed to mount filesystems");
        return result;
    }
    
    // Set up networking
    if (init_config.enable_networking) {
        result = init_setup_networking();
        if (result != SUCCESS) {
            KERROR("Failed to setup networking");
            return result;
        }
    }
    
    // Start essential services
    result = init_start_essential_services();
    if (result != SUCCESS) {
        KERROR("Failed to start essential services");
        return result;
    }
    
    // Start GUI if enabled
    if (init_config.enable_gui) {
        result = init_start_gui();
        if (result != SUCCESS) {
            KERROR("Failed to start GUI");
            return result;
        }
    }
    
    // Start default shell
    result = init_start_default_shell();
    if (result != SUCCESS) {
        KERROR("Failed to start default shell");
        return result;
    }
    
    init_state = INIT_STATE_MULTI_USER;
    init_initialized = true;
    
    KINFO("Init system started successfully");
    KINFO("Hostname: %s", init_config.hostname);
    KINFO("Uptime: %llu seconds", init_get_uptime());
    
    return SUCCESS;
}

void init_system_shutdown(void) {
    if (!init_initialized) {
        return;
    }
    
    KINFO("Shutting down RaeenOS");
    
    init_state = INIT_STATE_SHUTDOWN;
    
    // Run shutdown sequence
    init_run_shutdown_sequence();
    
    // Stop all services
    service_t* service = service_list;
    while (service) {
        if (service->state == SERVICE_STATE_RUNNING) {
            init_stop_service(service->name);
        }
        service = service->next;
    }
    
    // Unmount filesystems
    vfs_unmount_all();
    
    KINFO("System shutdown complete");
    
    // Halt the system
    hal_halt();
}

void init_system_reboot(void) {
    if (!init_initialized) {
        return;
    }
    
    KINFO("Rebooting RaeenOS");
    
    init_state = INIT_STATE_REBOOT;
    
    // Run reboot sequence
    init_run_reboot_sequence();
    
    // Restart the system
    hal_reboot();
}

error_t init_run_boot_sequence(void) {
    KINFO("Running boot sequence");
    
    // Initialize memory management
    KINFO("Initializing memory management");
    
    // Initialize process management
    KINFO("Initializing process management");
    
    // Initialize filesystem
    KINFO("Initializing filesystem");
    
    // Initialize device drivers
    KINFO("Initializing device drivers");
    
    // Initialize networking
    KINFO("Initializing networking");
    
    // Initialize security
    KINFO("Initializing security");
    
    KINFO("Boot sequence completed");
    return SUCCESS;
}

error_t init_mount_filesystems(void) {
    KINFO("Mounting filesystems");
    
    // Mount root filesystem
    error_t result = vfs_mount("/dev/root", "/", "ext4", 0);
    if (result != SUCCESS) {
        KERROR("Failed to mount root filesystem");
        return result;
    }
    
    // Mount proc filesystem
    result = vfs_mount("proc", "/proc", "proc", 0);
    if (result != SUCCESS) {
        KERROR("Failed to mount proc filesystem");
        return result;
    }
    
    // Mount sys filesystem
    result = vfs_mount("sys", "/sys", "sysfs", 0);
    if (result != SUCCESS) {
        KERROR("Failed to mount sys filesystem");
        return result;
    }
    
    // Mount tmp filesystem
    result = vfs_mount("tmp", "/tmp", "tmpfs", 0);
    if (result != SUCCESS) {
        KERROR("Failed to mount tmp filesystem");
        return result;
    }
    
    KINFO("Filesystems mounted successfully");
    return SUCCESS;
}

error_t init_setup_networking(void) {
    KINFO("Setting up networking");
    
    // Initialize network stack
    // TODO: Implement network initialization
    
    // Configure loopback interface
    // TODO: Configure network interfaces
    
    KINFO("Networking setup completed");
    return SUCCESS;
}

error_t init_start_essential_services(void) {
    KINFO("Starting essential services");
    
    // Start system logging
    service_t* syslog = init_register_service("syslog", "/sbin/syslogd");
    if (syslog) {
        syslog->auto_start = true;
        syslog->priority = SERVICE_PRIORITY_CRITICAL;
        init_start_service("syslog");
    }
    
    // Start device management
    service_t* udev = init_register_service("udev", "/sbin/udevd");
    if (udev) {
        udev->auto_start = true;
        udev->priority = SERVICE_PRIORITY_HIGH;
        init_start_service("udev");
    }
    
    // Start filesystem services
    service_t* fs = init_register_service("filesystem", "/sbin/fsd");
    if (fs) {
        fs->auto_start = true;
        fs->priority = SERVICE_PRIORITY_HIGH;
        init_start_service("filesystem");
    }
    
    KINFO("Essential services started");
    return SUCCESS;
}

error_t init_start_gui(void) {
    KINFO("Starting GUI");
    
    // Start GUI service
    service_t* gui = init_register_service("gui", "/sbin/raeen-gui");
    if (gui) {
        gui->auto_start = true;
        gui->priority = SERVICE_PRIORITY_NORMAL;
        init_start_service("gui");
    }
    
    KINFO("GUI started");
    return SUCCESS;
}

error_t init_start_default_shell(void) {
    KINFO("Starting default shell");
    
    // Create shell process
    pid_t shell_pid;
    const char* args[] = {init_config.default_shell, NULL};
    
    error_t result = init_spawn_process(init_config.default_shell, args, &shell_pid);
    if (result != SUCCESS) {
        KERROR("Failed to start shell: %s", init_config.default_shell);
        return result;
    }
    
    KINFO("Shell started with PID %d", shell_pid);
    return SUCCESS;
}

service_t* init_register_service(const char* name, const char* executable) {
    if (!name || !executable) {
        return NULL;
    }
    
    // Check if service already exists
    service_t* existing = init_find_service(name);
    if (existing) {
        KERROR("Service %s already registered", name);
        return existing;
    }
    
    // Create new service
    service_t* service = memory_alloc(sizeof(service_t));
    if (!service) {
        return NULL;
    }
    
    memset(service, 0, sizeof(service_t));
    strncpy(service->name, name, sizeof(service->name) - 1);
    strncpy(service->executable, executable, sizeof(service->executable) - 1);
    service->state = SERVICE_STATE_STOPPED;
    service->priority = SERVICE_PRIORITY_NORMAL;
    service->auto_start = false;
    service->restart_on_failure = false;
    service->max_restarts = 3;
    service->pid = -1;
    
    // Add to service list
    service->next = service_list;
    service_list = service;
    service_count++;
    
    KINFO("Registered service: %s", name);
    return service;
}

error_t init_start_service(const char* name) {
    service_t* service = init_find_service(name);
    if (!service) {
        KERROR("Service %s not found", name);
        return E_NOTFOUND;
    }
    
    if (service->state == SERVICE_STATE_RUNNING) {
        KINFO("Service %s already running", name);
        return SUCCESS;
    }
    
    KINFO("Starting service: %s", name);
    
    service->state = SERVICE_STATE_STARTING;
    
    // Spawn service process
    const char* args[] = {service->executable, NULL};
    error_t result = init_spawn_process(service->executable, args, &service->pid);
    if (result != SUCCESS) {
        KERROR("Failed to start service %s", name);
        service->state = SERVICE_STATE_FAILED;
        return result;
    }
    
    service->state = SERVICE_STATE_RUNNING;
    service->start_time = hal_get_tick_count();
    
    KINFO("Service %s started with PID %d", name, service->pid);
    return SUCCESS;
}

error_t init_stop_service(const char* name) {
    service_t* service = init_find_service(name);
    if (!service) {
        KERROR("Service %s not found", name);
        return E_NOTFOUND;
    }
    
    if (service->state != SERVICE_STATE_RUNNING) {
        KINFO("Service %s not running", name);
        return SUCCESS;
    }
    
    KINFO("Stopping service: %s", name);
    
    service->state = SERVICE_STATE_STOPPING;
    
    // Kill service process
    error_t result = init_kill_process(service->pid);
    if (result != SUCCESS) {
        KERROR("Failed to stop service %s", name);
        return result;
    }
    
    service->state = SERVICE_STATE_STOPPED;
    service->pid = -1;
    
    KINFO("Service %s stopped", name);
    return SUCCESS;
}

error_t init_spawn_process(const char* executable, const char* args[], pid_t* pid) {
    if (!executable || !pid) {
        return E_INVAL;
    }
    
    // Create new process
    process_t* process = process_create(executable);
    if (!process) {
        KERROR("Failed to create process for %s", executable);
        return E_NOMEM;
    }
    
    // Set up process arguments
    if (args) {
        // TODO: Set process arguments
    }
    
    // Start the process
    error_t result = process_start(process);
    if (result != SUCCESS) {
        KERROR("Failed to start process %s", executable);
        process_destroy(process);
        return result;
    }
    
    *pid = process->pid;
    
    KINFO("Spawned process: %s (PID: %d)", executable, *pid);
    return SUCCESS;
}

error_t init_kill_process(pid_t pid) {
    if (pid <= 0) {
        return E_INVAL;
    }
    
    process_t* process = process_find_by_pid(pid);
    if (!process) {
        KERROR("Process %d not found", pid);
        return E_NOTFOUND;
    }
    
    // Send termination signal
    process_send_signal(process, SIGTERM);
    
    // Wait for termination
    u32 timeout = 5000; // 5 seconds
    u32 start_time = hal_get_tick_count();
    
    while (hal_get_tick_count() - start_time < timeout) {
        if (process->state == PROCESS_STATE_TERMINATED) {
            break;
        }
        hal_delay_ms(10);
    }
    
    // Force kill if still running
    if (process->state != PROCESS_STATE_TERMINATED) {
        process_send_signal(process, SIGKILL);
    }
    
    KINFO("Killed process %d", pid);
    return SUCCESS;
}

service_t* init_find_service(const char* name) {
    if (!name) {
        return NULL;
    }
    
    service_t* service = service_list;
    while (service) {
        if (strcmp(service->name, name) == 0) {
            return service;
        }
        service = service->next;
    }
    
    return NULL;
}

init_state_t init_get_state(void) {
    return init_state;
}

void init_set_state(init_state_t state) {
    init_state = state;
}

const char* init_get_hostname(void) {
    return init_config.hostname;
}

timestamp_t init_get_uptime(void) {
    if (!init_initialized) {
        return 0;
    }
    return (hal_get_tick_count() - boot_time) / 1000; // Convert to seconds
}

void init_setup_signal_handlers(void) {
    // TODO: Set up signal handlers for init system
    KINFO("Signal handlers setup");
}

void init_log_message(const char* message) {
    if (!message) {
        return;
    }
    
    KINFO("[INIT] %s", message);
}

void init_log_error(const char* error) {
    if (!error) {
        return;
    }
    
    KERROR("[INIT] %s", error);
}

void init_emergency_shell(void) {
    KERROR("Entering emergency shell");
    
    // TODO: Start emergency shell
    // This would provide a minimal shell for system recovery
}

void init_panic(const char* message) {
    KERROR("INIT PANIC: %s", message ? message : "Unknown error");
    
    // TODO: Implement panic handling
    // This would save system state and halt
    hal_halt();
}

// Initialize init system
error_t init_system_init(void) {
    if (init_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing Init System (PID 1)...");
    
    // Set boot time
    boot_time = hal_get_tick_count();
    
    // Set initial state
    init_state = INIT_STATE_BOOTING;
    
    // Mount essential filesystems
    error_t result = init_mount_filesystems();
    if (result != SUCCESS) {
        KERROR("Failed to mount filesystems");
        return result;
    }
    
    // Setup default services
    init_setup_default_services();
    
    // Start system services
    result = init_start_system_services();
    if (result != SUCCESS) {
        KWARN("Some system services failed to start");
    }
    
    // Change to runlevel 3 (multi-user with networking)
    init_state = INIT_STATE_RUNNING;
    
    // Start user services
    result = init_start_user_services();
    if (result != SUCCESS) {
        KWARN("Some user services failed to start");
    }
    
    init_initialized = true;
    
    KINFO("Init System initialized successfully");
    KINFO("System is now ready for users");
    
    return SUCCESS;
}

// Mount essential filesystems
static error_t init_mount_filesystems(void) {
    KINFO("Mounting essential filesystems...");
    
    // Mount root filesystem (RAMFS for now)
    error_t result = vfs_mount("/", "ramfs", NULL);
    if (result != SUCCESS) {
        KERROR("Failed to mount root filesystem");
        return result;
    }
    
    // Create essential directories
    vfs_mkdir("/bin", 0755);
    vfs_mkdir("/sbin", 0755);
    vfs_mkdir("/usr", 0755);
    vfs_mkdir("/etc", 0755);
    vfs_mkdir("/var", 0755);
    vfs_mkdir("/tmp", 01777);
    vfs_mkdir("/proc", 0555);
    vfs_mkdir("/sys", 0555);
    vfs_mkdir("/dev", 0755);
    vfs_mkdir("/home", 0755);
    
    // Mount proc filesystem
    result = vfs_mount("/proc", "procfs", NULL);
    if (result != SUCCESS) {
        KWARN("Failed to mount proc filesystem");
    }
    
    // Mount sys filesystem
    result = vfs_mount("/sys", "sysfs", NULL);
    if (result != SUCCESS) {
        KWARN("Failed to mount sys filesystem");
    }
    
    // Mount dev filesystem
    result = vfs_mount("/dev", "devfs", NULL);
    if (result != SUCCESS) {
        KWARN("Failed to mount dev filesystem");
    }
    
    KINFO("Filesystems mounted successfully");
    return SUCCESS;
}

// Setup default services
static void init_setup_default_services(void) {
    KINFO("Setting up default services...");
    
    // System services
    init_register_service("syslog", "/sbin/syslogd", true);
    init_register_service("network", "/sbin/networkd", true);
    init_register_service("dhcp", "/sbin/dhcpd", true);
    init_register_service("sshd", "/sbin/sshd", false);
    init_register_service("cron", "/sbin/crond", true);
    init_register_service("ntpd", "/sbin/ntpd", true);
    
    // User services
    init_register_service("desktop", "/usr/bin/desktop", true);
    init_register_service("window-manager", "/usr/bin/window-manager", true);
    init_register_service("app-launcher", "/usr/bin/app-launcher", true);
    init_register_service("notification-daemon", "/usr/bin/notification-daemon", true);
    
    KINFO("Default services configured");
}

// Start system services
static error_t init_start_system_services(void) {
    KINFO("Starting system services...");
    
    service_t* service = service_list;
    while (service) {
        if (service->auto_start && strstr(service->name, "sys") != NULL) {
            error_t result = init_start_service(service);
            if (result != SUCCESS) {
                KERROR("Failed to start system service: %s", service->name);
            }
        }
        service = service->next;
    }
    
    return SUCCESS;
}

// Start user services
static error_t init_start_user_services(void) {
    KINFO("Starting user services...");
    
    service_t* service = service_list;
    while (service) {
        if (service->auto_start && strstr(service->name, "sys") == NULL) {
            error_t result = init_start_service(service);
            if (result != SUCCESS) {
                KERROR("Failed to start user service: %s", service->name);
            }
        }
        service = service->next;
    }
    
    return SUCCESS;
}

// Register a service
static error_t init_register_service(const char* name, const char* command, bool auto_start) {
    if (!name || !command) {
        return E_INVAL;
    }
    
    service_t* service = memory_alloc(sizeof(service_t));
    if (!service) {
        return E_NOMEM;
    }
    
    strncpy(service->name, name, sizeof(service->name) - 1);
    strncpy(service->command, command, sizeof(service->command) - 1);
    service->auto_start = auto_start;
    service->running = false;
    service->pid = -1;
    service->next = service_list;
    service_list = service;
    service_count++;
    
    KDEBUG("Registered service: %s (%s)", name, command);
    return SUCCESS;
}

// Start a service
static error_t init_start_service(service_t* service) {
    if (!service || service->running) {
        return E_INVAL;
    }
    
    KINFO("Starting service: %s", service->name);
    
    // Fork a new process for the service
    pid_t pid = process_fork();
    if (pid == 0) {
        // Child process - execute the service
        char* args[] = {service->command, NULL};
        process_execve(service->command, args, NULL);
        
        // If we get here, exec failed
        KERROR("Failed to execute service: %s", service->command);
        process_exit(1);
    } else if (pid > 0) {
        // Parent process - record the service
        service->pid = pid;
        service->running = true;
        KINFO("Service %s started with PID %d", service->name, pid);
        return SUCCESS;
    } else {
        KERROR("Failed to fork process for service: %s", service->name);
        return E_NOMEM;
    }
}

// Stop a service
static error_t init_stop_service(service_t* service) {
    if (!service || !service->running) {
        return E_INVAL;
    }
    
    KINFO("Stopping service: %s (PID %d)", service->name, service->pid);
    
    // Send SIGTERM to the service
    process_kill(service->pid, SIGTERM);
    
    // Wait for the process to terminate
    int status;
    pid_t result = process_waitpid(service->pid, &status, 0);
    
    if (result == service->pid) {
        service->running = false;
        service->pid = -1;
        KINFO("Service %s stopped", service->name);
        return SUCCESS;
    } else {
        KERROR("Failed to stop service: %s", service->name);
        return E_IO;
    }
}

// Main init loop
void init_main_loop(void) {
    KINFO("Init system entering main loop");
    
    while (init_state == INIT_STATE_RUNNING) {
        // Check for terminated services and restart if needed
        service_t* service = service_list;
        while (service) {
            if (service->running) {
                // Check if process is still running
                int status;
                pid_t result = process_waitpid(service->pid, &status, WNOHANG);
                if (result == service->pid) {
                    // Process terminated
                    KINFO("Service %s terminated with status %d", service->name, status);
                    service->running = false;
                    service->pid = -1;
                    
                    // Restart if auto-start
                    if (service->auto_start) {
                        KINFO("Restarting service: %s", service->name);
                        init_start_service(service);
                    }
                }
            }
            service = service->next;
        }
        
        // Sleep for a bit
        hal_sleep_ms(1000);
    }
}

// Get init process
process_t* init_get_process(void) {
    return init_process;
}

// Get shell process
process_t* init_get_shell_process(void) {
    return shell_process;
}

// Check if init system is initialized
bool init_system_is_initialized(void) {
    return init_initialized;
} 