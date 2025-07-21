/*
 * RaeenOS Hardware Emulator
 * Emulates hardware components for testing and development
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

#define MAX_DEVICES 32
#define MAX_MEMORY_SIZE (16 * 1024 * 1024) // 16MB
#define MAX_COMMAND_LEN 256

typedef enum {
    DEVICE_CPU,
    DEVICE_MEMORY,
    DEVICE_TIMER,
    DEVICE_KEYBOARD,
    DEVICE_VGA,
    DEVICE_DISK,
    DEVICE_NETWORK,
    DEVICE_USB,
    DEVICE_AUDIO,
    DEVICE_SERIAL
} device_type_t;

typedef struct {
    device_type_t type;
    char name[64];
    uint32_t base_address;
    uint32_t size;
    int enabled;
    void* device_data;
} device_t;

typedef struct {
    uint32_t frequency;
    uint32_t ticks;
    int running;
} timer_device_t;

typedef struct {
    uint8_t buffer[256];
    int head, tail;
    int interrupt_enabled;
} keyboard_device_t;

typedef struct {
    uint16_t width, height;
    uint8_t bpp;
    uint32_t framebuffer_addr;
    uint8_t* framebuffer;
} vga_device_t;

typedef struct {
    uint32_t sector_count;
    uint32_t sector_size;
    uint8_t* data;
    char filename[256];
} disk_device_t;

static device_t devices[MAX_DEVICES];
static int device_count = 0;
static uint8_t* memory = NULL;
static int emulator_running = 1;
static int verbose = 0;

void print_banner(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  RaeenOS Hardware Emulator                  ║\n");
    printf("║                         Version 1.0                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void print_help(void) {
    printf("Available Commands:\n");
    printf("  help                    - Show this help message\n");
    printf("  quit, exit, q           - Exit emulator\n");
    printf("  devices                 - List all devices\n");
    printf("  device <id>             - Show device details\n");
    printf("  enable <id>             - Enable device\n");
    printf("  disable <id>            - Disable device\n");
    printf("  memory <addr> [count]   - Dump memory at address\n");
    printf("  write <addr> <value>    - Write value to memory\n");
    printf("  load <file> <addr>      - Load file to memory address\n");
    printf("  save <addr> <size> <file> - Save memory to file\n");
    printf("  reset                   - Reset all devices\n");
    printf("  timer <freq>            - Set timer frequency\n");
    printf("  keyboard <text>         - Send keyboard input\n");
    printf("  disk <id> <file>        - Mount disk image\n");
    printf("  info                    - Show system information\n");
    printf("  verbose                 - Toggle verbose mode\n");
    printf("\n");
}

const char* device_type_name(device_type_t type) {
    switch (type) {
        case DEVICE_CPU: return "CPU";
        case DEVICE_MEMORY: return "Memory";
        case DEVICE_TIMER: return "Timer";
        case DEVICE_KEYBOARD: return "Keyboard";
        case DEVICE_VGA: return "VGA";
        case DEVICE_DISK: return "Disk";
        case DEVICE_NETWORK: return "Network";
        case DEVICE_USB: return "USB";
        case DEVICE_AUDIO: return "Audio";
        case DEVICE_SERIAL: return "Serial";
        default: return "Unknown";
    }
}

int add_device(device_type_t type, const char* name, uint32_t base_addr, uint32_t size) {
    if (device_count >= MAX_DEVICES) {
        printf("Error: Maximum number of devices reached\n");
        return -1;
    }
    
    device_t* dev = &devices[device_count];
    dev->type = type;
    strncpy(dev->name, name, sizeof(dev->name) - 1);
    dev->name[sizeof(dev->name) - 1] = '\0';
    dev->base_address = base_addr;
    dev->size = size;
    dev->enabled = 1;
    dev->device_data = NULL;
    
    // Initialize device-specific data
    switch (type) {
        case DEVICE_TIMER: {
            timer_device_t* timer = malloc(sizeof(timer_device_t));
            timer->frequency = 1000; // 1kHz default
            timer->ticks = 0;
            timer->running = 1;
            dev->device_data = timer;
            break;
        }
        case DEVICE_KEYBOARD: {
            keyboard_device_t* kbd = malloc(sizeof(keyboard_device_t));
            memset(kbd->buffer, 0, sizeof(kbd->buffer));
            kbd->head = kbd->tail = 0;
            kbd->interrupt_enabled = 1;
            dev->device_data = kbd;
            break;
        }
        case DEVICE_VGA: {
            vga_device_t* vga = malloc(sizeof(vga_device_t));
            vga->width = 640;
            vga->height = 480;
            vga->bpp = 8;
            vga->framebuffer_addr = 0xA0000;
            vga->framebuffer = malloc(vga->width * vga->height * (vga->bpp / 8));
            memset(vga->framebuffer, 0, vga->width * vga->height * (vga->bpp / 8));
            dev->device_data = vga;
            break;
        }
        case DEVICE_DISK: {
            disk_device_t* disk = malloc(sizeof(disk_device_t));
            disk->sector_count = 2048; // 1MB disk
            disk->sector_size = 512;
            disk->data = malloc(disk->sector_count * disk->sector_size);
            memset(disk->data, 0, disk->sector_count * disk->sector_size);
            strcpy(disk->filename, "");
            dev->device_data = disk;
            break;
        }
        default:
            break;
    }
    
    if (verbose) {
        printf("Added device: %s (%s) at 0x%08X, size %u bytes\n", 
               name, device_type_name(type), base_addr, size);
    }
    
    return device_count++;
}

void initialize_devices(void) {
    printf("Initializing hardware devices...\n");
    
    // Add standard PC devices
    add_device(DEVICE_CPU, "Intel x86_64", 0x00000000, 0);
    add_device(DEVICE_MEMORY, "System RAM", 0x00000000, MAX_MEMORY_SIZE);
    add_device(DEVICE_TIMER, "PIT Timer", 0x40, 4);
    add_device(DEVICE_KEYBOARD, "PS/2 Keyboard", 0x60, 2);
    add_device(DEVICE_VGA, "VGA Controller", 0xA0000, 0x20000);
    add_device(DEVICE_DISK, "IDE Disk 0", 0x1F0, 8);
    add_device(DEVICE_SERIAL, "COM1", 0x3F8, 8);
    add_device(DEVICE_NETWORK, "RTL8139", 0xC000, 256);
    add_device(DEVICE_USB, "USB Controller", 0xD000, 256);
    add_device(DEVICE_AUDIO, "Sound Blaster", 0x220, 16);
    
    printf("Initialized %d devices.\n", device_count);
}

void list_devices(void) {
    printf("\nDevice List:\n");
    printf("ID  Type     Name              Address    Size       Status\n");
    printf("--- -------- ----------------- ---------- ---------- --------\n");
    
    for (int i = 0; i < device_count; i++) {
        printf("%-3d %-8s %-17s 0x%08X %-10u %s\n",
               i,
               device_type_name(devices[i].type),
               devices[i].name,
               devices[i].base_address,
               devices[i].size,
               devices[i].enabled ? "Enabled" : "Disabled");
    }
}

void show_device_details(int id) {
    if (id < 0 || id >= device_count) {
        printf("Error: Invalid device ID %d\n", id);
        return;
    }
    
    device_t* dev = &devices[id];
    printf("\nDevice Details:\n");
    printf("  ID: %d\n", id);
    printf("  Type: %s\n", device_type_name(dev->type));
    printf("  Name: %s\n", dev->name);
    printf("  Base Address: 0x%08X\n", dev->base_address);
    printf("  Size: %u bytes\n", dev->size);
    printf("  Status: %s\n", dev->enabled ? "Enabled" : "Disabled");
    
    // Show device-specific information
    switch (dev->type) {
        case DEVICE_TIMER: {
            timer_device_t* timer = (timer_device_t*)dev->device_data;
            if (timer) {
                printf("  Frequency: %u Hz\n", timer->frequency);
                printf("  Ticks: %u\n", timer->ticks);
                printf("  Running: %s\n", timer->running ? "Yes" : "No");
            }
            break;
        }
        case DEVICE_KEYBOARD: {
            keyboard_device_t* kbd = (keyboard_device_t*)dev->device_data;
            if (kbd) {
                printf("  Buffer size: %d bytes\n", (kbd->tail - kbd->head + 256) % 256);
                printf("  Interrupts: %s\n", kbd->interrupt_enabled ? "Enabled" : "Disabled");
            }
            break;
        }
        case DEVICE_VGA: {
            vga_device_t* vga = (vga_device_t*)dev->device_data;
            if (vga) {
                printf("  Resolution: %dx%d\n", vga->width, vga->height);
                printf("  Color depth: %d bpp\n", vga->bpp);
                printf("  Framebuffer: 0x%08X\n", vga->framebuffer_addr);
            }
            break;
        }
        case DEVICE_DISK: {
            disk_device_t* disk = (disk_device_t*)dev->device_data;
            if (disk) {
                printf("  Sectors: %u\n", disk->sector_count);
                printf("  Sector size: %u bytes\n", disk->sector_size);
                printf("  Total size: %u KB\n", (disk->sector_count * disk->sector_size) / 1024);
                printf("  Image file: %s\n", strlen(disk->filename) > 0 ? disk->filename : "None");
            }
            break;
        }
        default:
            break;
    }
}

void toggle_device(int id, int enable) {
    if (id < 0 || id >= device_count) {
        printf("Error: Invalid device ID %d\n", id);
        return;
    }
    
    devices[id].enabled = enable;
    printf("Device %d (%s) %s\n", id, devices[id].name, enable ? "enabled" : "disabled");
}

void dump_memory(uint32_t address, int count) {
    if (!memory) {
        printf("Error: Memory not initialized\n");
        return;
    }
    
    printf("Memory dump at 0x%08X:\n", address);
    printf("Address    Hex                              ASCII\n");
    printf("---------- -------------------------------- ----------------\n");
    
    for (int i = 0; i < count; i++) {
        uint32_t addr = address + i * 16;
        if (addr >= MAX_MEMORY_SIZE) break;
        
        printf("0x%08X: ", addr);
        
        // Hex dump
        for (int j = 0; j < 16; j++) {
            if (addr + j < MAX_MEMORY_SIZE) {
                printf("%02X ", memory[addr + j]);
            } else {
                printf("   ");
            }
        }
        
        printf(" ");
        
        // ASCII dump
        for (int j = 0; j < 16; j++) {
            if (addr + j < MAX_MEMORY_SIZE) {
                uint8_t c = memory[addr + j];
                printf("%c", (c >= 32 && c <= 126) ? c : '.');
            } else {
                printf(" ");
            }
        }
        
        printf("\n");
    }
}

void write_memory(uint32_t address, uint8_t value) {
    if (!memory) {
        printf("Error: Memory not initialized\n");
        return;
    }
    
    if (address >= MAX_MEMORY_SIZE) {
        printf("Error: Address 0x%08X out of range\n", address);
        return;
    }
    
    memory[address] = value;
    if (verbose) {
        printf("Wrote 0x%02X to address 0x%08X\n", value, address);
    }
}

void set_timer_frequency(int device_id, uint32_t frequency) {
    if (device_id < 0 || device_id >= device_count) {
        printf("Error: Invalid device ID\n");
        return;
    }
    
    if (devices[device_id].type != DEVICE_TIMER) {
        printf("Error: Device %d is not a timer\n", device_id);
        return;
    }
    
    timer_device_t* timer = (timer_device_t*)devices[device_id].device_data;
    if (timer) {
        timer->frequency = frequency;
        printf("Timer frequency set to %u Hz\n", frequency);
    }
}

void send_keyboard_input(const char* text) {
    // Find keyboard device
    int kbd_id = -1;
    for (int i = 0; i < device_count; i++) {
        if (devices[i].type == DEVICE_KEYBOARD) {
            kbd_id = i;
            break;
        }
    }
    
    if (kbd_id == -1) {
        printf("Error: No keyboard device found\n");
        return;
    }
    
    keyboard_device_t* kbd = (keyboard_device_t*)devices[kbd_id].device_data;
    if (!kbd) {
        printf("Error: Keyboard device not initialized\n");
        return;
    }
    
    // Add text to keyboard buffer
    for (const char* p = text; *p; p++) {
        int next_tail = (kbd->tail + 1) % 256;
        if (next_tail != kbd->head) {
            kbd->buffer[kbd->tail] = *p;
            kbd->tail = next_tail;
        } else {
            printf("Warning: Keyboard buffer full\n");
            break;
        }
    }
    
    printf("Sent keyboard input: \"%s\"\n", text);
}

void mount_disk_image(int device_id, const char* filename) {
    if (device_id < 0 || device_id >= device_count) {
        printf("Error: Invalid device ID\n");
        return;
    }
    
    if (devices[device_id].type != DEVICE_DISK) {
        printf("Error: Device %d is not a disk\n", device_id);
        return;
    }
    
    disk_device_t* disk = (disk_device_t*)devices[device_id].device_data;
    if (!disk) {
        printf("Error: Disk device not initialized\n");
        return;
    }
    
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error: Cannot open file %s\n", filename);
        return;
    }
    
    // Read file into disk data
    size_t bytes_read = fread(disk->data, 1, disk->sector_count * disk->sector_size, fp);
    fclose(fp);
    
    strncpy(disk->filename, filename, sizeof(disk->filename) - 1);
    disk->filename[sizeof(disk->filename) - 1] = '\0';
    
    printf("Mounted disk image: %s (%zu bytes)\n", filename, bytes_read);
}

void reset_devices(void) {
    printf("Resetting all devices...\n");
    
    for (int i = 0; i < device_count; i++) {
        devices[i].enabled = 1;
        
        // Reset device-specific state
        switch (devices[i].type) {
            case DEVICE_TIMER: {
                timer_device_t* timer = (timer_device_t*)devices[i].device_data;
                if (timer) {
                    timer->ticks = 0;
                    timer->running = 1;
                }
                break;
            }
            case DEVICE_KEYBOARD: {
                keyboard_device_t* kbd = (keyboard_device_t*)devices[i].device_data;
                if (kbd) {
                    kbd->head = kbd->tail = 0;
                    memset(kbd->buffer, 0, sizeof(kbd->buffer));
                }
                break;
            }
            default:
                break;
        }
    }
    
    // Clear memory
    if (memory) {
        memset(memory, 0, MAX_MEMORY_SIZE);
    }
    
    printf("All devices reset.\n");
}

void show_system_info(void) {
    printf("\nSystem Information:\n");
    printf("  Emulator: RaeenOS Hardware Emulator v1.0\n");
    printf("  Target: RaeenOS\n");
    printf("  Architecture: x86_64\n");
    printf("  Memory size: %d MB\n", MAX_MEMORY_SIZE / (1024 * 1024));
    printf("  Devices: %d\n", device_count);
    printf("  Verbose mode: %s\n", verbose ? "On" : "Off");
    printf("  Status: %s\n", emulator_running ? "Running" : "Stopped");
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
    char arg3[MAX_COMMAND_LEN] = {0};
    
    sscanf(command, "%s %s %s %s", cmd, arg1, arg2, arg3);
    
    if (strcmp(cmd, "help") == 0) {
        print_help();
    } else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0 || strcmp(cmd, "q") == 0) {
        emulator_running = 0;
    } else if (strcmp(cmd, "devices") == 0) {
        list_devices();
    } else if (strcmp(cmd, "device") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: device <id>\n");
        } else {
            show_device_details(atoi(arg1));
        }
    } else if (strcmp(cmd, "enable") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: enable <device_id>\n");
        } else {
            toggle_device(atoi(arg1), 1);
        }
    } else if (strcmp(cmd, "disable") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: disable <device_id>\n");
        } else {
            toggle_device(atoi(arg1), 0);
        }
    } else if (strcmp(cmd, "memory") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: memory <address> [count]\n");
        } else {
            uint32_t addr = strtoul(arg1, NULL, 0);
            int count = strlen(arg2) > 0 ? atoi(arg2) : 4;
            dump_memory(addr, count);
        }
    } else if (strcmp(cmd, "write") == 0) {
        if (strlen(arg1) == 0 || strlen(arg2) == 0) {
            printf("Usage: write <address> <value>\n");
        } else {
            uint32_t addr = strtoul(arg1, NULL, 0);
            uint8_t value = strtoul(arg2, NULL, 0);
            write_memory(addr, value);
        }
    } else if (strcmp(cmd, "reset") == 0) {
        reset_devices();
    } else if (strcmp(cmd, "timer") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: timer <frequency>\n");
        } else {
            // Find timer device
            for (int i = 0; i < device_count; i++) {
                if (devices[i].type == DEVICE_TIMER) {
                    set_timer_frequency(i, atoi(arg1));
                    break;
                }
            }
        }
    } else if (strcmp(cmd, "keyboard") == 0) {
        if (strlen(arg1) == 0) {
            printf("Usage: keyboard <text>\n");
        } else {
            // Reconstruct the full text (including spaces)
            char text[MAX_COMMAND_LEN];
            const char* start = strstr(command, arg1);
            if (start) {
                strncpy(text, start, sizeof(text) - 1);
                text[sizeof(text) - 1] = '\0';
                send_keyboard_input(text);
            }
        }
    } else if (strcmp(cmd, "disk") == 0) {
        if (strlen(arg1) == 0 || strlen(arg2) == 0) {
            printf("Usage: disk <device_id> <filename>\n");
        } else {
            mount_disk_image(atoi(arg1), arg2);
        }
    } else if (strcmp(cmd, "info") == 0) {
        show_system_info();
    } else if (strcmp(cmd, "verbose") == 0) {
        verbose = !verbose;
        printf("Verbose mode: %s\n", verbose ? "On" : "Off");
    } else if (strlen(cmd) > 0) {
        printf("Unknown command: %s\n", cmd);
        printf("Type 'help' for available commands.\n");
    }
}

int main(int argc, char* argv[]) {
    char command[MAX_COMMAND_LEN];
    
    signal(SIGINT, signal_handler);
    
    print_banner();
    
    // Process command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [-v] [-h]\n", argv[0]);
            printf("  -v  Verbose output\n");
            printf("  -h  Show this help\n");
            return 0;
        }
    }
    
    // Initialize memory
    memory = malloc(MAX_MEMORY_SIZE);
    if (!memory) {
        printf("Error: Failed to allocate memory\n");
        return 1;
    }
    memset(memory, 0, MAX_MEMORY_SIZE);
    
    // Initialize devices
    initialize_devices();
    
    printf("Hardware emulator ready. Type 'help' for commands.\n");
    printf("(emulator) ");
    fflush(stdout);
    
    while (emulator_running && fgets(command, sizeof(command), stdin)) {
        // Remove newline
        command[strcspn(command, "\n")] = '\0';
        
        if (strlen(command) > 0) {
            process_command(command);
        }
        
        if (emulator_running) {
            printf("(emulator) ");
            fflush(stdout);
        }
    }
    
    printf("\nShutting down emulator...\n");
    
    // Cleanup
    for (int i = 0; i < device_count; i++) {
        if (devices[i].device_data) {
            if (devices[i].type == DEVICE_VGA) {
                vga_device_t* vga = (vga_device_t*)devices[i].device_data;
                if (vga->framebuffer) {
                    free(vga->framebuffer);
                }
            } else if (devices[i].type == DEVICE_DISK) {
                disk_device_t* disk = (disk_device_t*)devices[i].device_data;
                if (disk->data) {
                    free(disk->data);
                }
            }
            free(devices[i].device_data);
        }
    }
    
    if (memory) {
        free(memory);
    }
    
    printf("Goodbye!\n");
    return 0;
}