#include "shell/shell.h"
#include "kernel.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include <string.h>
#include <stdio.h>

// Shell constants
#define SHELL_MAX_LINE_LENGTH 256
#define SHELL_MAX_ARGS 16
#define SHELL_PROMPT "RaeenOS> "

// Shell state
static shell_state_t shell_state = {0};
static bool shell_initialized = false;
static char shell_input_buffer[SHELL_MAX_LINE_LENGTH];
static size_t shell_input_position = 0;

// Command history
static char shell_history[10][SHELL_MAX_LINE_LENGTH];
static int shell_history_count = 0;
static int shell_history_position = 0;

// Built-in commands
static int shell_cmd_help(int argc, char* argv[]);
static int shell_cmd_clear(int argc, char* argv[]);
static int shell_cmd_echo(int argc, char* argv[]);
static int shell_cmd_ls(int argc, char* argv[]);
static int shell_cmd_cd(int argc, char* argv[]);
static int shell_cmd_pwd(int argc, char* argv[]);
static int shell_cmd_cat(int argc, char* argv[]);
static int shell_cmd_mkdir(int argc, char* argv[]);
static int shell_cmd_rm(int argc, char* argv[]);
static int shell_cmd_mv(int argc, char* argv[]);
static int shell_cmd_cp(int argc, char* argv[]);
static int shell_cmd_ps(int argc, char* argv[]);
static int shell_cmd_kill(int argc, char* argv[]);
static int shell_cmd_mem(int argc, char* argv[]);
static int shell_cmd_cpu(int argc, char* argv[]);
static int shell_cmd_uptime(int argc, char* argv[]);
static int shell_cmd_reboot(int argc, char* argv[]);
static int shell_cmd_shutdown(int argc, char* argv[]);

// Command table
typedef struct {
    const char* name;
    const char* description;
    int (*handler)(int argc, char* argv[]);
} shell_command_t;

static shell_command_t shell_commands[] = {
    {"help", "Show this help message", shell_cmd_help},
    {"clear", "Clear the screen", shell_cmd_clear},
    {"echo", "Print arguments", shell_cmd_echo},
    {"ls", "List directory contents", shell_cmd_ls},
    {"cd", "Change directory", shell_cmd_cd},
    {"pwd", "Print working directory", shell_cmd_pwd},
    {"cat", "Display file contents", shell_cmd_cat},
    {"mkdir", "Create directory", shell_cmd_mkdir},
    {"rm", "Remove file or directory", shell_cmd_rm},
    {"mv", "Move or rename file", shell_cmd_mv},
    {"cp", "Copy file", shell_cmd_cp},
    {"ps", "List processes", shell_cmd_ps},
    {"kill", "Kill process", shell_cmd_kill},
    {"mem", "Show memory usage", shell_cmd_mem},
    {"cpu", "Show CPU information", shell_cmd_cpu},
    {"uptime", "Show system uptime", shell_cmd_uptime},
    {"reboot", "Reboot system", shell_cmd_reboot},
    {"shutdown", "Shutdown system", shell_cmd_shutdown},
    {NULL, NULL, NULL}
};

// Initialize shell
error_t shell_init(void) {
    if (shell_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing shell...");
    
    // Initialize shell state
    memset(&shell_state, 0, sizeof(shell_state));
    shell_state.running = true;
    shell_state.current_directory[0] = '/';
    shell_state.current_directory[1] = '\0';
    
    // Initialize input buffer
    memset(shell_input_buffer, 0, sizeof(shell_input_buffer));
    shell_input_position = 0;
    
    // Initialize command history
    memset(shell_history, 0, sizeof(shell_history));
    shell_history_count = 0;
    shell_history_position = 0;
    
    shell_initialized = true;
    KINFO("Shell initialized successfully");
    
    return SUCCESS;
}

// Main shell loop
void shell_main_loop(void) {
    if (!shell_initialized) {
        KERROR("Shell not initialized");
        return;
    }
    
    KINFO("Starting shell main loop");
    
    // Print welcome message
    vga_printf("\n=== RaeenOS Shell ===\n");
    vga_printf("Type 'help' for available commands.\n\n");
    
    while (shell_state.running) {
        // Print prompt
        vga_printf("%s", SHELL_PROMPT);
        
        // Read command line
        if (shell_read_line()) {
            // Execute command
            shell_execute_command(shell_input_buffer);
        }
        
        // Clear input buffer
        shell_clear_input();
    }
}

// Read command line from keyboard
bool shell_read_line(void) {
    shell_input_position = 0;
    
    while (shell_input_position < SHELL_MAX_LINE_LENGTH - 1) {
        int c = keyboard_read_char();
        if (c == -1) {
            // No input available, yield to other processes
            continue;
        }
        
        if (c == '\n' || c == '\r') {
            // End of line
            vga_printf("\n");
            shell_input_buffer[shell_input_position] = '\0';
            
            // Add to history if not empty
            if (shell_input_position > 0) {
                shell_add_to_history(shell_input_buffer);
            }
            
            return true;
        } else if (c == '\b' || c == 127) {
            // Backspace
            if (shell_input_position > 0) {
                shell_input_position--;
                vga_printf("\b \b");
            }
        } else if (c >= 32 && c <= 126) {
            // Printable character
            shell_input_buffer[shell_input_position++] = c;
            vga_putchar(c);
        }
    }
    
    return false;
}

// Clear input buffer
void shell_clear_input(void) {
    memset(shell_input_buffer, 0, sizeof(shell_input_buffer));
    shell_input_position = 0;
}

// Add command to history
void shell_add_to_history(const char* command) {
    if (shell_history_count < 10) {
        strncpy(shell_history[shell_history_count], command, SHELL_MAX_LINE_LENGTH - 1);
        shell_history_count++;
    } else {
        // Shift history down
        for (int i = 0; i < 9; i++) {
            strcpy(shell_history[i], shell_history[i + 1]);
        }
        strncpy(shell_history[9], command, SHELL_MAX_LINE_LENGTH - 1);
    }
    shell_history_position = shell_history_count;
}

// Execute command
void shell_execute_command(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }
    
    // Parse command and arguments
    char* args[SHELL_MAX_ARGS];
    int argc = shell_parse_command(command, args);
    
    if (argc == 0) {
        return;
    }
    
    // Find and execute command
    shell_command_t* cmd = shell_find_command(args[0]);
    if (cmd) {
        int result = cmd->handler(argc, args);
        if (result != 0) {
            vga_printf("Command failed with exit code %d\n", result);
        }
    } else {
        vga_printf("Unknown command: %s\n", args[0]);
        vga_printf("Type 'help' for available commands.\n");
    }
}

// Parse command into arguments
int shell_parse_command(const char* command, char* args[]) {
    int argc = 0;
    char* cmd_copy = strdup(command);
    char* token = strtok(cmd_copy, " \t\n\r");
    
    while (token && argc < SHELL_MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n\r");
    }
    args[argc] = NULL;
    
    return argc;
}

// Find command in command table
shell_command_t* shell_find_command(const char* name) {
    for (int i = 0; shell_commands[i].name != NULL; i++) {
        if (strcmp(shell_commands[i].name, name) == 0) {
            return &shell_commands[i];
        }
    }
    return NULL;
}

// Built-in command implementations
static int shell_cmd_help(int argc, char* argv[]) {
    vga_printf("Available commands:\n");
    for (int i = 0; shell_commands[i].name != NULL; i++) {
        vga_printf("  %-10s - %s\n", shell_commands[i].name, shell_commands[i].description);
    }
    return 0;
}

static int shell_cmd_clear(int argc, char* argv[]) {
    vga_clear();
    return 0;
}

static int shell_cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        vga_printf("%s", argv[i]);
        if (i < argc - 1) {
            vga_printf(" ");
        }
    }
    vga_printf("\n");
    return 0;
}

static int shell_cmd_ls(int argc, char* argv[]) {
    vga_printf("Directory listing not implemented yet.\n");
    return 0;
}

static int shell_cmd_cd(int argc, char* argv[]) {
    if (argc < 2) {
        vga_printf("Usage: cd <directory>\n");
        return 1;
    }
    vga_printf("Change directory not implemented yet.\n");
    return 0;
}

static int shell_cmd_pwd(int argc, char* argv[]) {
    vga_printf("%s\n", shell_state.current_directory);
    return 0;
}

static int shell_cmd_cat(int argc, char* argv[]) {
    if (argc < 2) {
        vga_printf("Usage: cat <file>\n");
        return 1;
    }
    vga_printf("File display not implemented yet.\n");
    return 0;
}

static int shell_cmd_mkdir(int argc, char* argv[]) {
    if (argc < 2) {
        vga_printf("Usage: mkdir <directory>\n");
        return 1;
    }
    vga_printf("Directory creation not implemented yet.\n");
    return 0;
}

static int shell_cmd_rm(int argc, char* argv[]) {
    if (argc < 2) {
        vga_printf("Usage: rm <file>\n");
        return 1;
    }
    vga_printf("File removal not implemented yet.\n");
    return 0;
}

static int shell_cmd_mv(int argc, char* argv[]) {
    if (argc < 3) {
        vga_printf("Usage: mv <source> <destination>\n");
        return 1;
    }
    vga_printf("File move not implemented yet.\n");
    return 0;
}

static int shell_cmd_cp(int argc, char* argv[]) {
    if (argc < 3) {
        vga_printf("Usage: cp <source> <destination>\n");
        return 1;
    }
    vga_printf("File copy not implemented yet.\n");
    return 0;
}

static int shell_cmd_ps(int argc, char* argv[]) {
    vga_printf("Process listing not implemented yet.\n");
    return 0;
}

static int shell_cmd_kill(int argc, char* argv[]) {
    if (argc < 2) {
        vga_printf("Usage: kill <pid>\n");
        return 1;
    }
    vga_printf("Process termination not implemented yet.\n");
    return 0;
}

static int shell_cmd_mem(int argc, char* argv[]) {
    vga_printf("Memory usage not implemented yet.\n");
    return 0;
}

static int shell_cmd_cpu(int argc, char* argv[]) {
    vga_printf("CPU information not implemented yet.\n");
    return 0;
}

static int shell_cmd_uptime(int argc, char* argv[]) {
    vga_printf("System uptime not implemented yet.\n");
    return 0;
}

static int shell_cmd_reboot(int argc, char* argv[]) {
    vga_printf("Rebooting system...\n");
    // TODO: Implement system reboot
    return 0;
}

static int shell_cmd_shutdown(int argc, char* argv[]) {
    vga_printf("Shutting down system...\n");
    shell_state.running = false;
    return 0;
}

// Get shell state
shell_state_t* shell_get_state(void) {
    return &shell_state;
}

// Check if shell is initialized
bool shell_is_initialized(void) {
    return shell_initialized;
} 