/*
 * RaeenOS Kernel Debugger
 * Advanced debugging tool for RaeenOS kernel development
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>

#define MAX_BREAKPOINTS 64
#define MAX_COMMAND_LEN 256
#define MAX_SYMBOL_NAME 128

typedef struct {
    uint64_t address;
    int enabled;
    char symbol[MAX_SYMBOL_NAME];
} breakpoint_t;

typedef struct {
    char name[MAX_SYMBOL_NAME];
    uint64_t address;
    uint32_t size;
    char type; // 'F' = function, 'V' = variable, 'S' = section
} symbol_t;

static breakpoint_t breakpoints[MAX_BREAKPOINTS];
static int breakpoint_count = 0;
static symbol_t* symbols = NULL;
static int symbol_count = 0;
static int debugger_running = 1;

void print_banner(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    RaeenOS Kernel Debugger                  ║\n");
    printf("║                         Version 1.0                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\nType 'help' for available commands.\n\n");
}

void print_help(void) {
    printf("Available Commands:\n");
    printf("  help                    - Show this help message\n");
    printf("  quit, exit, q           - Exit debugger\n");
    printf("  break <addr|symbol>     - Set breakpoint at address or symbol\n");
    printf("  delete <id>             - Delete breakpoint by ID\n");
    printf("  list                    - List all breakpoints\n");
    printf("  enable <id>             - Enable breakpoint\n");
    printf("  disable <id>            - Disable breakpoint\n");
    printf("  symbols                 - List all symbols\n");
    printf("  symbol <name>           - Find symbol by name\n");
    printf("  disasm <addr> [count]   - Disassemble at address\n");
    printf("  memory <addr> [count]   - Dump memory at address\n");
    printf("  registers               - Show CPU registers\n");
    printf("  stack                   - Show stack trace\n");
    printf("  continue, c             - Continue execution\n");
    printf("  step, s                 - Single step\n");
    printf("  next, n                 - Step over function calls\n");
    printf("  info                    - Show system information\n");
    printf("  attach <pid>            - Attach to process\n");
    printf("  detach                  - Detach from process\n");
    printf("\n");
}

int add_breakpoint(uint64_t address, const char* symbol) {
    if (breakpoint_count >= MAX_BREAKPOINTS) {
        printf("Error: Maximum number of breakpoints reached (%d)\n", MAX_BREAKPOINTS);
        return -1;
    }
    
    breakpoints[breakpoint_count].address = address;
    breakpoints[breakpoint_count].enabled = 1;
    if (symbol) {
        strncpy(breakpoints[breakpoint_count].symbol, symbol, MAX_SYMBOL_NAME - 1);
        breakpoints[breakpoint_count].symbol[MAX_SYMBOL_NAME - 1] = '\0';
    } else {
        snprintf(breakpoints[breakpoint_count].symbol, MAX_SYMBOL_NAME, "0x%016llx", 
                (unsigned long long)address);
    }
    
    printf("Breakpoint %d set at %s (0x%016llx)\n", 
           breakpoint_count, breakpoints[breakpoint_count].symbol, 
           (unsigned long long)address);
    
    return breakpoint_count++;
}

void list_breakpoints(void) {
    if (breakpoint_count == 0) {
        printf("No breakpoints set.\n");
        return;
    }
    
    printf("Breakpoints:\n");
    printf("ID  Enabled  Address          Symbol\n");
    printf("--- ------- ---------------- --------------------------------\n");
    
    for (int i = 0; i < breakpoint_count; i++) {
        printf("%-3d %-7s 0x%016llx %s\n", 
               i, 
               breakpoints[i].enabled ? "Yes" : "No",
               (unsigned long long)breakpoints[i].address,
               breakpoints[i].symbol);
    }
}

void delete_breakpoint(int id) {
    if (id < 0 || id >= breakpoint_count) {
        printf("Error: Invalid breakpoint ID %d\n", id);
        return;
    }
    
    printf("Deleted breakpoint %d at %s\n", id, breakpoints[id].symbol);
    
    // Shift remaining breakpoints
    for (int i = id; i < breakpoint_count - 1; i++) {
        breakpoints[i] = breakpoints[i + 1];
    }
    breakpoint_count--;
}

void toggle_breakpoint(int id, int enable) {
    if (id < 0 || id >= breakpoint_count) {
        printf("Error: Invalid breakpoint ID %d\n", id);
        return;
    }
    
    breakpoints[id].enabled = enable;
    printf("Breakpoint %d %s\n", id, enable ? "enabled" : "disabled");
}

uint64_t parse_address(const char* str) {
    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {
        return strtoull(str, NULL, 16);
    } else {
        // Try to find symbol
        for (int i = 0; i < symbol_count; i++) {
            if (strcmp(symbols[i].name, str) == 0) {
                return symbols[i].address;
            }
        }
        // Try as decimal
        return strtoull(str, NULL, 10);
    }
}

void show_memory(uint64_t address, int count) {
    printf("Memory dump at 0x%016llx:\n", (unsigned long long)address);
    printf("Address          Hex                              ASCII\n");
    printf("---------------- -------------------------------- ----------------\n");
    
    // Simulate memory dump (in real implementation, would read from target)
    for (int i = 0; i < count; i++) {
        printf("0x%016llx: ", (unsigned long long)(address + i * 16));
        
        // Hex dump (simulated)
        for (int j = 0; j < 16; j++) {
            printf("%02x ", (unsigned char)((address + i * 16 + j) & 0xFF));
        }
        
        printf(" ");
        
        // ASCII dump (simulated)
        for (int j = 0; j < 16; j++) {
            unsigned char c = (address + i * 16 + j) & 0xFF;
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        
        printf("\n");
    }
}

void show_disassembly(uint64_t address, int count) {
    printf("Disassembly at 0x%016llx:\n", (unsigned long long)address);
    printf("Address          Bytes        Instruction\n");
    printf("---------------- ------------ --------------------------------\n");
    
    // Simulate disassembly (in real implementation, would use disassembler)
    const char* instructions[] = {
        "mov rax, rbx", "push rbp", "mov rbp, rsp", "sub rsp, 0x20",
        "call 0x401000", "test eax, eax", "jz 0x401050", "mov [rbp-8], rax",
        "add rsp, 0x20", "pop rbp", "ret", "nop"
    };
    
    for (int i = 0; i < count; i++) {
        printf("0x%016llx: %02x %02x %02x    %s\n", 
               (unsigned long long)(address + i * 3),
               (unsigned char)((address + i) & 0xFF),
               (unsigned char)((address + i + 1) & 0xFF),
               (unsigned char)((address + i + 2) & 0xFF),
               instructions[i % (sizeof(instructions) / sizeof(instructions[0]))]);
    }
}

void show_registers(void) {
    printf("CPU Registers (simulated):\n");
    printf("RAX: 0x0000000000401000  RBX: 0x0000000000000000\n");
    printf("RCX: 0x00007fff12345678  RDX: 0x0000000000000001\n");
    printf("RSI: 0x00007fff87654321  RDI: 0x0000000000000000\n");
    printf("RBP: 0x00007fff11111111  RSP: 0x00007fff22222222\n");
    printf("R8:  0x0000000000000000  R9:  0x0000000000000000\n");
    printf("R10: 0x0000000000000000  R11: 0x0000000000000000\n");
    printf("R12: 0x0000000000000000  R13: 0x0000000000000000\n");
    printf("R14: 0x0000000000000000  R15: 0x0000000000000000\n");
    printf("RIP: 0x0000000000401234  RFLAGS: 0x0000000000000246\n");
    printf("CS: 0x0008  DS: 0x0010  ES: 0x0010  FS: 0x0000  GS: 0x0000  SS: 0x0010\n");
}

void show_stack_trace(void) {
    printf("Stack Trace (simulated):\n");
    printf("#0  0x0000000000401234 in kernel_main() at kernel_main.c:123\n");
    printf("#1  0x0000000000401000 in _start() at boot.asm:45\n");
    printf("#2  0x0000000000400800 in multiboot_entry() at multiboot.c:67\n");
}

void show_system_info(void) {
    printf("System Information:\n");
    printf("Target: RaeenOS Kernel\n");
    printf("Architecture: x86_64\n");
    printf("Debugger: RaeenOS Kernel Debugger v1.0\n");
    printf("Symbols loaded: %d\n", symbol_count);
    printf("Breakpoints set: %d\n", breakpoint_count);
    printf("Status: %s\n", "Attached (simulated)");
}

void load_symbols(const char* filename) {
    // Simulate loading symbols
    printf("Loading symbols from %s...\n", filename ? filename : "kernel.elf");
    
    // Add some example symbols
    symbol_count = 5;
    symbols = malloc(symbol_count * sizeof(symbol_t));
    
    strcpy(symbols[0].name, "kernel_main");
    symbols[0].address = 0x401234;
    symbols[0].size = 256;
    symbols[0].type = 'F';
    
    strcpy(symbols[1].name, "_start");
    symbols[1].address = 0x401000;
    symbols[1].size = 64;
    symbols[1].type = 'F';
    
    strcpy(symbols[2].name, "multiboot_entry");
    symbols[2].address = 0x400800;
    symbols[2].size = 128;
    symbols[2].type = 'F';
    
    strcpy(symbols[3].name, "kernel_stack");
    symbols[3].address = 0x500000;
    symbols[3].size = 4096;
    symbols[3].type = 'V';
    
    strcpy(symbols[4].name, ".text");
    symbols[4].address = 0x400000;
    symbols[4].size = 65536;
    symbols[4].type = 'S';
    
    printf("Loaded %d symbols.\n", symbol_count);
}

void list_symbols(void) {
    if (symbol_count == 0) {
        printf("No symbols loaded. Use 'load' command to load symbols.\n");
        return;
    }
    
    printf("Symbols:\n");
    printf("Type Address          Size     Name\n");
    printf("---- ---------------- -------- --------------------------------\n");
    
    for (int i = 0; i < symbol_count; i++) {
        printf("%-4c 0x%016llx %-8u %s\n",
               symbols[i].type,
               (unsigned long long)symbols[i].address,
               symbols[i].size,
               symbols[i].name);
    }
}

void find_symbol(const char* name) {
    int found = 0;
    
    for (int i = 0; i < symbol_count; i++) {
        if (strstr(symbols[i].name, name)) {
            if (!found) {
                printf("Matching symbols:\n");
                printf("Type Address          Size     Name\n");
                printf("---- ---------------- -------- --------------------------------\n");
                found = 1;
            }
            printf("%-4c 0x%016llx %-8u %s\n",
                   symbols[i].type,
                   (unsigned long long)symbols[i].address,
                   symbols[i].size,
                   symbols[i].name);
        }
    }
    
    if (!found) {
        printf("No symbols matching '%s' found.\n", name);
    }
}

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nInterrupt received. Type 'quit' to exit.\n");
    }
}

void process_command(const char* command) {
    char cmd[MAX_COMMAND_LEN];
    char arg1[MAX_COMMAND_LEN] = {0};
    char arg2[MAX_COMMAND_LEN] = {0};
    
    sscanf(command, "%s %s %s", cmd, arg1, arg2);
    
    if (strcmp(cmd, "help") == 0) {
        print_help();
    } else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0 || strcmp(cmd, "q") == 0) {
        debugger_running = 0;
    } else if (strcmp(cmd, "break") == 0 || strcmp(cmd, "b") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: break <address|symbol>\n");
        } else {
            uint64_t addr = parse_address(arg1);
            add_breakpoint(addr, arg1);
        }
    } else if (strcmp(cmd, "delete") == 0 || strcmp(cmd, "d") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: delete <breakpoint_id>\n");
        } else {
            delete_breakpoint(atoi(arg1));
        }
    } else if (strcmp(cmd, "list") == 0 || strcmp(cmd, "l") == 0) {
        list_breakpoints();
    } else if (strcmp(cmd, "enable") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: enable <breakpoint_id>\n");
        } else {
            toggle_breakpoint(atoi(arg1), 1);
        }
    } else if (strcmp(cmd, "disable") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: disable <breakpoint_id>\n");
        } else {
            toggle_breakpoint(atoi(arg1), 0);
        }
    } else if (strcmp(cmd, "symbols") == 0) {
        list_symbols();
    } else if (strcmp(cmd, "symbol") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: symbol <name>\n");
        } else {
            find_symbol(arg1);
        }
    } else if (strcmp(cmd, "disasm") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: disasm <address> [count]\n");
        } else {
            uint64_t addr = parse_address(arg1);
            int count = strlen(arg2) > 0 ? atoi(arg2) : 10;
            show_disassembly(addr, count);
        }
    } else if (strcmp(cmd, "memory") == 0 || strcmp(cmd, "x") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: memory <address> [count]\n");
        } else {
            uint64_t addr = parse_address(arg1);
            int count = strlen(arg2) > 0 ? atoi(arg2) : 4;
            show_memory(addr, count);
        }
    } else if (strcmp(cmd, "registers") == 0 || strcmp(cmd, "r") == 0) {
        show_registers();
    } else if (strcmp(cmd, "stack") == 0 || strcmp(cmd, "bt") == 0) {
        show_stack_trace();
    } else if (strcmp(cmd, "continue") == 0 || strcmp(cmd, "c") == 0) {
        printf("Continuing execution...\n");
    } else if (strcmp(cmd, "step") == 0 || strcmp(cmd, "s") == 0) {
        printf("Single stepping...\n");
    } else if (strcmp(cmd, "next") == 0 || strcmp(cmd, "n") == 0) {
        printf("Stepping over...\n");
    } else if (strcmp(cmd, "info") == 0) {
        show_system_info();
    } else if (strcmp(cmd, "attach") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: attach <pid>\n");
        } else {
            printf("Attaching to process %s...\n", arg1);
        }
    } else if (strcmp(cmd, "detach") == 0) {
        printf("Detaching from target...\n");
    } else if (strlen(cmd) > 0) {
        printf("Unknown command: %s\n", cmd);
        printf("Type 'help' for available commands.\n");
    }
}

int main(int argc, char* argv[]) {
    char command[MAX_COMMAND_LEN];
    
    signal(SIGINT, signal_handler);
    
    print_banner();
    
    // Load default symbols
    load_symbols(NULL);
    
    // Process command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            load_symbols(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [-s symbol_file]\n", argv[0]);
            printf("  -s symbol_file  Load symbols from file\n");
            printf("  -h              Show this help\n");
            return 0;
        }
    }
    
    printf("(raeenos-gdb) ");
    fflush(stdout);
    
    while (debugger_running && fgets(command, sizeof(command), stdin)) {
        // Remove newline
        command[strcspn(command, "\n")] = '\0';
        
        if (strlen(command) > 0) {
            process_command(command);
        }
        
        if (debugger_running) {
            printf("(raeenos-gdb) ");
            fflush(stdout);
        }
    }
    
    printf("\nGoodbye!\n");
    
    if (symbols) {
        free(symbols);
    }
    
    return 0;
}