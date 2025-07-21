#include <stdint.h>

// Syscall numbers
#define SYS_EXIT    1
#define SYS_WRITE   2
#define SYS_GETPID  3
#define SYS_READ    4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_FORK    7
#define SYS_EXEC    8
#define SYS_WAIT    9

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

// Read from stdin
static int read(char* buf, size_t len) {
    return syscall(SYS_READ, STDIN_FILENO, (uint64_t)buf, len);
}

// Get process ID
static int getpid(void) {
    return syscall(SYS_GETPID, 0, 0, 0);
}

// Fork process
static int fork(void) {
    return syscall(SYS_FORK, 0, 0, 0);
}

// Execute program
static int exec(const char* pathname) {
    return syscall(SYS_EXEC, (uint64_t)pathname, 0, 0);
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

// Simple string copy function
static void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

// Simple string comparison function
static int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) {
            return *s1 - *s2;
        }
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

// Simple shell main function
void _start(void) {
    const char* prompt = "RaeenOS Shell> ";
    const char* welcome = "Welcome to RaeenOS Simple Shell!\n";
    const char* help_msg = "Available commands:\n  help - Show this help\n  exit - Exit shell\n  test - Run test program\n";
    const char* unknown_cmd = "Unknown command. Type 'help' for available commands.\n";
    
    char input[256];
    char command[64];
    
    // Show welcome message
    write(welcome, strlen(welcome));
    write(help_msg, strlen(help_msg));
    
    while (1) {
        // Show prompt
        write(prompt, strlen(prompt));
        
        // Read input
        int bytes_read = read(input, sizeof(input) - 1);
        if (bytes_read <= 0) {
            continue;
        }
        input[bytes_read] = '\0';
        
        // Parse command (simple - just get first word)
        int i = 0;
        while (input[i] == ' ' || input[i] == '\t') i++; // Skip whitespace
        
        int j = 0;
        while (input[i] && input[i] != ' ' && input[i] != '\t' && input[i] != '\n') {
            command[j++] = input[i++];
        }
        command[j] = '\0';
        
        // Handle commands
        if (strcmp(command, "help") == 0) {
            write(help_msg, strlen(help_msg));
        } else if (strcmp(command, "exit") == 0) {
            write("Goodbye!\n", 9);
            exit(0);
        } else if (strcmp(command, "test") == 0) {
            // Fork and exec test program
            int pid = fork();
            if (pid == 0) {
                // Child process
                write("Child process executing test program...\n", 40);
                exec("test_program");
                // If exec returns, it failed
                write("Failed to execute test_program\n", 30);
                exit(1);
            } else if (pid > 0) {
                // Parent process
                write("Parent process waiting for child...\n", 36);
                // Wait for child (simplified - just yield)
                while (1) {
                    // In a real implementation, we'd use sys_wait here
                    // For now, just continue
                    break;
                }
                write("Child process completed\n", 23);
            } else {
                // Fork failed
                write("Failed to fork process\n", 22);
            }
        } else if (strlen(command) > 0) {
            write(unknown_cmd, strlen(unknown_cmd));
        }
    }
} 