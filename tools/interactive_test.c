#include <stdint.h>
#include <stddef.h>

// System call numbers
#define SYS_EXIT    1
#define SYS_WRITE   2
#define SYS_READ    3
#define SYS_OPEN    4
#define SYS_CLOSE   5
#define SYS_FORK    6
#define SYS_EXECVE  7
#define SYS_WAIT    8
#define SYS_GETPID  9

// File descriptors
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// Simple system call wrapper
static inline int64_t syscall(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    int64_t ret;
    asm volatile (
        "mov %1, %%rax\n"
        "mov %2, %%rdi\n"
        "mov %3, %%rsi\n"
        "mov %4, %%rdx\n"
        "mov %5, %%r10\n"
        "mov %6, %%r8\n"
        "mov %7, %%r9\n"
        "syscall\n"
        "mov %%rax, %0\n"
        : "=r" (ret)
        : "r" (num), "r" (arg1), "r" (arg2), "r" (arg3), "r" (arg4), "r" (arg5), "r" (arg6)
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "rcx", "r11"
    );
    return ret;
}

// Helper functions
void write_string(const char* str) {
    syscall(SYS_WRITE, STDOUT_FILENO, (uint64_t)str, 0, 0, 0, 0);
}

void write_char(char ch) {
    syscall(SYS_WRITE, STDOUT_FILENO, (uint64_t)&ch, 1, 0, 0, 0);
}

int read_char(char* ch) {
    return syscall(SYS_READ, STDIN_FILENO, (uint64_t)ch, 1, 0, 0, 0);
}

int read_string(char* buffer, int max_len) {
    return syscall(SYS_READ, STDIN_FILENO, (uint64_t)buffer, max_len, 0, 0, 0);
}

int get_pid(void) {
    return syscall(SYS_GETPID, 0, 0, 0, 0, 0, 0);
}

int fork(void) {
    return syscall(SYS_FORK, 0, 0, 0, 0, 0, 0);
}

int wait(int* status) {
    return syscall(SYS_WAIT, 0, (uint64_t)status, 0, 0, 0, 0);
}

void exit(int code) {
    syscall(SYS_EXIT, code, 0, 0, 0, 0, 0);
}

// Test functions
void test_keyboard_input(void) {
    write_string("=== Keyboard Input Test ===\n");
    write_string("Type some characters (press Enter to finish):\n");
    
    char buffer[256];
    int bytes_read = read_string(buffer, sizeof(buffer) - 1);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        write_string("You typed: ");
        write_string(buffer);
        write_string("\n");
    } else {
        write_string("No input received\n");
    }
}

void test_process_management(void) {
    write_string("=== Process Management Test ===\n");
    
    int pid = get_pid();
    write_string("Current PID: ");
    
    // Convert PID to string
    char pid_str[16];
    int i = 0;
    int temp = pid;
    do {
        pid_str[i++] = '0' + (temp % 10);
        temp /= 10;
    } while (temp > 0);
    
    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = pid_str[j];
        pid_str[j] = pid_str[i - 1 - j];
        pid_str[i - 1 - j] = temp;
    }
    pid_str[i] = '\0';
    
    write_string(pid_str);
    write_string("\n");
    
    write_string("Forking process...\n");
    int child_pid = fork();
    
    if (child_pid == 0) {
        // Child process
        write_string("Child process running (PID: ");
        int child_id = get_pid();
        // Convert child PID to string (simplified)
        char child_str[16];
        int j = 0;
        int temp2 = child_id;
        do {
            child_str[j++] = '0' + (temp2 % 10);
            temp2 /= 10;
        } while (temp2 > 0);
        
        // Reverse the string
        for (int k = 0; k < j / 2; k++) {
            char temp3 = child_str[k];
            child_str[k] = child_str[j - 1 - k];
            child_str[j - 1 - k] = temp3;
        }
        child_str[j] = '\0';
        
        write_string(child_str);
        write_string(")\n");
        write_string("Child process exiting...\n");
        exit(42);
    } else if (child_pid > 0) {
        // Parent process
        write_string("Parent waiting for child...\n");
        int status;
        int waited_pid = wait(&status);
        
        write_string("Child process ");
        
        // Convert waited PID to string
        char waited_str[16];
        int j = 0;
        int temp2 = waited_pid;
        do {
            waited_str[j++] = '0' + (temp2 % 10);
            temp2 /= 10;
        } while (temp2 > 0);
        
        // Reverse the string
        for (int k = 0; k < j / 2; k++) {
            char temp3 = waited_str[k];
            waited_str[k] = waited_str[j - 1 - k];
            waited_str[j - 1 - k] = temp3;
        }
        waited_str[j] = '\0';
        
        write_string(waited_str);
        write_string(" exited with status: ");
        
        // Convert status to string
        char status_str[16];
        j = 0;
        temp2 = status;
        do {
            status_str[j++] = '0' + (temp2 % 10);
            temp2 /= 10;
        } while (temp2 > 0);
        
        // Reverse the string
        for (int k = 0; k < j / 2; k++) {
            char temp3 = status_str[k];
            status_str[k] = status_str[j - 1 - k];
            status_str[j - 1 - k] = temp3;
        }
        status_str[j] = '\0';
        
        write_string(status_str);
        write_string("\n");
    } else {
        write_string("Fork failed!\n");
    }
}

void test_interactive_menu(void) {
    write_string("=== Interactive Test Menu ===\n");
    write_string("1. Test keyboard input\n");
    write_string("2. Test process management\n");
    write_string("3. Exit\n");
    write_string("Enter your choice: ");
    
    char choice;
    if (read_char(&choice) > 0) {
        write_string("\n");
        
        switch (choice) {
            case '1':
                test_keyboard_input();
                break;
            case '2':
                test_process_management();
                break;
            case '3':
                write_string("Exiting...\n");
                exit(0);
                break;
            default:
                write_string("Invalid choice!\n");
                break;
        }
    }
}

// Main function
void _start(void) {
    write_string("RaeenOS Interactive Test Program\n");
    write_string("================================\n\n");
    
    while (1) {
        test_interactive_menu();
        write_string("\nPress Enter to continue...\n");
        
        char ch;
        while (read_char(&ch) > 0 && ch != '\n') {
            // Wait for Enter
        }
        
        write_string("\n");
    }
} 