#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Kernel version information
#define KERNEL_VERSION_MAJOR 0
#define KERNEL_VERSION_MINOR 1
#define KERNEL_VERSION_PATCH 0
#define KERNEL_NAME "RaeenOS"

// Memory management
#define PAGE_SIZE 4096
#define KERNEL_HEAP_SIZE (1024 * 1024 * 16)  // 16MB

// System limits
#define MAX_PROCESSES 1024
#define MAX_THREADS_PER_PROCESS 256
#define MAX_OPEN_FILES 1024

// Kernel subsystem initialization flags
#define INIT_MEMORY     (1 << 0)
#define INIT_PROCESSES  (1 << 1)
#define INIT_FILESYSTEM (1 << 2)
#define INIT_NETWORK    (1 << 3)
#define INIT_GRAPHICS   (1 << 4)
#define INIT_SECURITY   (1 << 5)
#define INIT_HAL        (1 << 6)
#define INIT_ALL        0xFF

// Kernel panic and debugging
#define KERNEL_PANIC(msg) kernel_panic(__FILE__, __LINE__, msg)

// Forward declarations
struct multiboot_info;
struct boot_params;

// Core kernel functions
void kernel_main(struct multiboot_info* mbi);
void kernel_init(uint32_t init_flags);
void kernel_panic(const char* file, int line, const char* message) __attribute__((noreturn));
void kernel_halt(void) __attribute__((noreturn));
void kernel_main_loop(void);

// Subsystem initialization functions
int memory_init(void);
int process_init(void);
int filesystem_init(void);
int network_init(void);
int graphics_init(void);
int security_init(void);
int hal_init(void);

// Utility functions
const char* kernel_get_version_string(void);
uint64_t kernel_get_uptime(void);
void kernel_log(const char* level, const char* format, ...);

// Logging levels
#define LOG_DEBUG   "DEBUG"
#define LOG_INFO    "INFO"
#define LOG_WARN    "WARN"
#define LOG_ERROR   "ERROR"
#define LOG_FATAL   "FATAL"

// Convenience macros for logging
#define KDEBUG(fmt, ...) kernel_log(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define KINFO(fmt, ...)  kernel_log(LOG_INFO, fmt, ##__VA_ARGS__)
#define KWARN(fmt, ...)  kernel_log(LOG_WARN, fmt, ##__VA_ARGS__)
#define KERROR(fmt, ...) kernel_log(LOG_ERROR, fmt, ##__VA_ARGS__)
#define KFATAL(fmt, ...) kernel_log(LOG_FATAL, fmt, ##__VA_ARGS__)

#endif // KERNEL_H