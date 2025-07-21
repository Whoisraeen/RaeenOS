#include <stdint.h>

// Syscall numbers
#define SYS_EXIT    1
#define SYS_WRITE   2
#define SYS_GETPID  3

// File descriptors
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// Syscall function (x86-64 calling convention)
static inline int64_t syscall(uint64_t number, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    int64_t result;
    asm volatile(
        "syscall"
        : "=a" (result)
        : "a" (number), "D" (arg1), "S" (arg2), "d" (arg3)
        : "rcx", "r11", "memory"
    );
    return result;
}

// Write to stdout
static int write(const char* str, size_t len) {
    return syscall(SYS_WRITE, STDOUT_FILENO, (uint64_t)str, len);
}

// Get process ID
static int getpid(void) {
    return syscall(SYS_GETPID, 0, 0, 0);
}

// Exit process
static void exit(int code) {
    syscall(SYS_EXIT, code, 0, 0);
    // Should never return
    while(1);
}

// Simple string length function
static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Simple integer to string conversion
static void itoa(int value, char* str) {
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    int i = 0;
    int temp = value;
    
    // Count digits
    while (temp > 0) {
        temp /= 10;
        i++;
    }
    
    str[i] = '\0';
    i--;
    
    // Convert to string
    while (value > 0) {
        str[i] = '0' + (value % 10);
        value /= 10;
        i--;
    }
}

// Main function
void _start(void) {
    const char* message1 = "Hello from user space!\n";
    const char* message2 = "My process ID is: ";
    const char* message3 = "\n";
    
    // Write first message
    write(message1, strlen(message1));
    
    // Get and display process ID
    write(message2, strlen(message2));
    
    int pid = getpid();
    char pid_str[16];
    itoa(pid, pid_str);
    write(pid_str, strlen(pid_str));
    
    write(message3, strlen(message3));
    
    // Exit with success
    exit(0);
} 