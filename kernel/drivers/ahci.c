#include <kernel/drivers/ahci.h>
#include <kernel/drivers/device_manager.h>
#include <kernel/memory/memory.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/core/kernel.h>
#include <kernel/libc/string.h>
#include <kernel/libc/stdio.h>
#include <kernel/pci/pci.h>

// AHCI Register Definitions
#define AHCI_HBA_CAP        0x00    // Host Capabilities
#define AHCI_HBA_GHC        0x04    // Global Host Control
#define AHCI_HBA_IS         0x08    // Interrupt Status
#define AHCI_HBA_PI         0x0C    // Ports Implemented
#define AHCI_HBA_VS         0x10    // Version
#define AHCI_HBA_CCC_CTL    0x14    // Command Completion Coalescing Control
#define AHCI_HBA_CCC_PORTS  0x18    // Command Completion Coalescing Ports
#define AHCI_HBA_EM_LOC     0x1C    // Enclosure Management Location
#define AHCI_HBA_EM_CTL     0x20    // Enclosure Management Control
#define AHCI_HBA_CAP2       0x24    // Host Capabilities Extended
#define AHCI_HBA_BOHC       0x28    // BIOS/OS Handoff Control and Status

// Port Registers (offset from port base)
#define AHCI_PORT_CLB       0x00    // Command List Base Address
#define AHCI_PORT_CLBU      0x04    // Command List Base Address Upper 32-bits
#define AHCI_PORT_FB        0x08    // FIS Base Address
#define AHCI_PORT_FBU       0x0C    // FIS Base Address Upper 32-bits
#define AHCI_PORT_IS        0x10    // Interrupt Status
#define AHCI_PORT_IE        0x14    // Interrupt Enable
#define AHCI_PORT_CMD       0x18    // Command and Status
#define AHCI_PORT_TFD       0x20    // Task File Data
#define AHCI_PORT_SIG       0x24    // Signature
#define AHCI_PORT_SSTS      0x28    // SATA Status (SCR0: SStatus)
#define AHCI_PORT_SCTL      0x2C    // SATA Control (SCR2: SControl)
#define AHCI_PORT_SERR      0x30    // SATA Error (SCR1: SError)
#define AHCI_PORT_SACT      0x34    // SATA Active (SCR3: SActive)
#define AHCI_PORT_CI        0x38    // Command Issue
#define AHCI_PORT_SNTF      0x3C    // SATA Notification (SCR4: SNotification)

// AHCI Command Header
typedef struct {
    uint8_t  command_fis_length:5;   // Multiple of 4 dwords
    uint8_t  atapi:1;                // ATAPI command
    uint8_t  write:1;                // Write command
    uint8_t  prefetchable:1;         // Prefetchable
    uint8_t  reset:1;                // Reset
    uint8_t  bist:1;                 // BIST
    uint8_t  clear_busy:1;           // Clear busy upon R_OK
    uint8_t  reserved0:1;            // Reserved
    uint8_t  port_multiplier:4;      // Port multiplier port
    uint16_t prdt_length;            // Physical region descriptor table length
    uint32_t prdb_count;             // Physical region descriptor byte count transferred
    uint32_t command_table_base_addr; // Command table descriptor base address
    uint32_t command_table_base_addr_upper; // Command table descriptor base address upper 32-bits
    uint32_t reserved1[4];           // Reserved
} __attribute__((packed)) ahci_command_header_t;

// AHCI Command Table
typedef struct {
    uint8_t  command_fis[64];        // Command FIS
    uint8_t  atapi_command[16];      // ATAPI command (12 or 16 bytes)
    uint8_t  reserved[48];           // Reserved
    ahci_prdt_entry_t prdt[8];       // Physical region descriptor table
} __attribute__((packed)) ahci_command_table_t;

// Physical Region Descriptor Table Entry
typedef struct {
    uint32_t data_base_addr;         // Data base address
    uint32_t data_base_addr_upper;   // Data base address upper 32-bits
    uint32_t reserved0;              // Reserved
    uint32_t byte_count:22;          // Byte count
    uint32_t reserved1:9;            // Reserved
    uint32_t interrupt_on_completion:1; // Interrupt on completion
} __attribute__((packed)) ahci_prdt_entry_t;

// FIS Types
#define FIS_TYPE_REG_H2D    0x27    // Register FIS - Host to Device
#define FIS_TYPE_REG_D2H    0x34    // Register FIS - Device to Host
#define FIS_TYPE_DMA_ACTIVATE 0x39  // DMA Activate FIS - Device to Host
#define FIS_TYPE_DMA_SETUP  0x41    // DMA Setup FIS - Bidirectional
#define FIS_TYPE_DATA       0x46    // Data FIS - Bidirectional
#define FIS_TYPE_BIST       0x58    // BIST Activate FIS - Bidirectional
#define FIS_TYPE_PIO_SETUP  0x5F    // PIO Setup FIS - Device to Host
#define FIS_TYPE_SET_DEVICE_BITS 0xA1 // Set Device Bits FIS - Device to Host

// Register FIS - Host to Device
typedef struct {
    uint8_t  fis_type;              // FIS type
    uint8_t  pmport:4;              // Port multiplier
    uint8_t  reserved0:3;           // Reserved
    uint8_t  command_update:1;      // Command update
    uint8_t  command;               // Command register
    uint8_t  feature_low;           // Feature register, 7:0
    uint8_t  lba_low;               // LBA low register, 7:0
    uint8_t  lba_mid;               // LBA mid register, 15:8
    uint8_t  lba_high;              // LBA high register, 23:16
    uint8_t  device;                // Device register
    uint8_t  lba_low_exp;           // LBA low register, 31:24
    uint8_t  lba_mid_exp;           // LBA mid register, 39:32
    uint8_t  lba_high_exp;          // LBA high register, 47:40
    uint8_t  feature_high;          // Feature register, 15:8
    uint16_t count;                 // Count register
    uint8_t  icc;                   // Isochronous command completion
    uint8_t  control;               // Control register
    uint8_t  reserved1[4];          // Reserved
} __attribute__((packed)) ahci_reg_h2d_fis_t;

// Register FIS - Device to Host
typedef struct {
    uint8_t  fis_type;              // FIS type
    uint8_t  pmport:4;              // Port multiplier
    uint8_t  reserved0:2;           // Reserved
    uint8_t  interrupt:1;           // Interrupt
    uint8_t  reserved1:1;           // Reserved
    uint8_t  status;                // Status register
    uint8_t  error;                 // Error register
    uint8_t  lba_low;               // LBA low register, 7:0
    uint8_t  lba_mid;               // LBA mid register, 15:8
    uint8_t  lba_high;              // LBA high register, 23:16
    uint8_t  device;                // Device register
    uint8_t  lba_low_exp;           // LBA low register, 31:24
    uint8_t  lba_mid_exp;           // LBA mid register, 39:32
    uint8_t  lba_high_exp;          // LBA high register, 47:40
    uint8_t  reserved2;             // Reserved
    uint16_t count;                 // Count register
    uint8_t  reserved3[6];          // Reserved
} __attribute__((packed)) ahci_reg_d2h_fis_t;

// AHCI Port structure
typedef struct {
    uint32_t base_addr;             // Port base address
    uint32_t command_list_base;     // Command list base address
    uint32_t fis_base;              // FIS base address
    uint32_t command_table_base;    // Command table base address
    uint32_t signature;             // Device signature
    uint8_t  port_number;           // Port number
    uint8_t  state;                 // Port state
    uint8_t  sata_capable;          // SATA capable
    uint8_t  ncq_supported;         // NCQ supported
    uint64_t sectors;               // Total sectors
    uint32_t sector_size;           // Sector size
    char     model[41];             // Model string
    char     serial[21];            // Serial number
    char     firmware[9];           // Firmware revision
} ahci_port_t;

// AHCI Controller structure
typedef struct {
    uint32_t base_addr;             // Controller base address
    uint32_t capabilities;          // Host capabilities
    uint32_t ports_implemented;     // Ports implemented
    uint8_t  num_ports;             // Number of ports
    ahci_port_t ports[32];          // Port array
    uint8_t  initialized;           // Initialization status
} ahci_controller_t;

// Global AHCI controller
static ahci_controller_t ahci_controller;

// Function prototypes
static int ahci_find_controller();
static int ahci_init_controller();
static int ahci_init_port(int port_num);
static int ahci_identify_device(int port_num);
static int ahci_read_sectors(int port_num, uint64_t lba, uint32_t count, void* buffer);
static int ahci_write_sectors(int port_num, uint64_t lba, uint32_t count, const void* buffer);
static void ahci_irq_handler(struct interrupt_frame* frame);
static void ahci_wait_for_ready(int port_num);
static void ahci_wait_for_drq(int port_num);

// Initialize AHCI subsystem
int ahci_init() {
    printf("AHCI: Initializing AHCI subsystem...\n");
    
    // Find AHCI controller
    if (!ahci_find_controller()) {
        printf("AHCI: No AHCI controller found\n");
        return -1;
    }
    
    // Initialize controller
    if (ahci_init_controller() != 0) {
        printf("AHCI: Failed to initialize controller\n");
        return -1;
    }
    
    // Initialize ports
    for (int i = 0; i < ahci_controller.num_ports; i++) {
        if (ahci_controller.ports_implemented & (1 << i)) {
            if (ahci_init_port(i) == 0) {
                if (ahci_identify_device(i) == 0) {
                    printf("AHCI: Port %d initialized with device\n", i);
                }
            }
        }
    }
    
    // Register IRQ handler
    register_irq_handler(11, ahci_irq_handler); // MSI or legacy IRQ
    
    printf("AHCI: Initialization complete\n");
    return 0;
}

// Find AHCI controller via PCI
static int ahci_find_controller() {
    // Search for AHCI controller in PCI space
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int function = 0; function < 8; function++) {
                uint32_t vendor_id = pci_read_config_dword(bus, device, function, 0);
                uint16_t vendor = vendor_id & 0xFFFF;
                uint16_t device_id = (vendor_id >> 16) & 0xFFFF;
                
                if (vendor == 0x8086) { // Intel
                    if (device_id == 0x2922 || device_id == 0x3A22 || 
                        device_id == 0x3B22 || device_id == 0x3B23) {
                        // Found Intel AHCI controller
                        ahci_controller.base_addr = pci_read_config_dword(bus, device, function, 0x24) & 0xFFFFFFF0;
                        ahci_controller.capabilities = mmio_read32(ahci_controller.base_addr + AHCI_HBA_CAP);
                        ahci_controller.ports_implemented = mmio_read32(ahci_controller.base_addr + AHCI_HBA_PI);
                        ahci_controller.num_ports = 32 - __builtin_clz(ahci_controller.ports_implemented);
                        
                        printf("AHCI: Found Intel controller at 0x%x, %d ports\n", 
                               ahci_controller.base_addr, ahci_controller.num_ports);
                        return 1;
                    }
                }
            }
        }
    }
    
    return 0;
}

// Initialize AHCI controller
static int ahci_init_controller() {
    // Enable AHCI mode
    uint32_t ghc = mmio_read32(ahci_controller.base_addr + AHCI_HBA_GHC);
    ghc |= 0x80000000; // Enable AHCI
    mmio_write32(ahci_controller.base_addr + AHCI_HBA_GHC, ghc);
    
    // Wait for AHCI to be enabled
    while (!(mmio_read32(ahci_controller.base_addr + AHCI_HBA_GHC) & 0x80000000)) {
        // Wait
    }
    
    // Clear interrupts
    mmio_write32(ahci_controller.base_addr + AHCI_HBA_IS, 0xFFFFFFFF);
    
    // Enable interrupts
    ghc = mmio_read32(ahci_controller.base_addr + AHCI_HBA_GHC);
    ghc |= 0x00000002; // Enable interrupts
    mmio_write32(ahci_controller.base_addr + AHCI_HBA_GHC, ghc);
    
    ahci_controller.initialized = 1;
    return 0;
}

// Initialize AHCI port
static int ahci_init_port(int port_num) {
    ahci_port_t* port = &ahci_controller.ports[port_num];
    port->port_number = port_num;
    port->base_addr = ahci_controller.base_addr + 0x100 + (port_num * 0x80);
    
    // Stop command engine
    uint32_t cmd = mmio_read32(port->base_addr + AHCI_PORT_CMD);
    cmd &= ~0x00000001; // Stop command engine
    mmio_write32(port->base_addr + AHCI_PORT_CMD, cmd);
    
    // Wait for command engine to stop
    while (mmio_read32(port->base_addr + AHCI_PORT_CMD) & 0x00000001) {
        // Wait
    }
    
    // Allocate command list
    port->command_list_base = (uint32_t)kmalloc(1024);
    if (!port->command_list_base) {
        return -1;
    }
    
    // Allocate FIS
    port->fis_base = (uint32_t)kmalloc(256);
    if (!port->fis_base) {
        kfree((void*)port->command_list_base);
        return -1;
    }
    
    // Allocate command table
    port->command_table_base = (uint32_t)kmalloc(256);
    if (!port->command_table_base) {
        kfree((void*)port->command_list_base);
        kfree((void*)port->fis_base);
        return -1;
    }
    
    // Clear command list
    memset((void*)port->command_list_base, 0, 1024);
    
    // Clear FIS
    memset((void*)port->fis_base, 0, 256);
    
    // Clear command table
    memset((void*)port->command_table_base, 0, 256);
    
    // Set command list base
    mmio_write32(port->base_addr + AHCI_PORT_CLB, port->command_list_base);
    mmio_write32(port->base_addr + AHCI_PORT_CLBU, 0);
    
    // Set FIS base
    mmio_write32(port->base_addr + AHCI_PORT_FB, port->fis_base);
    mmio_write32(port->base_addr + AHCI_PORT_FBU, 0);
    
    // Clear interrupts
    mmio_write32(port->base_addr + AHCI_PORT_IS, 0xFFFFFFFF);
    
    // Enable interrupts
    mmio_write32(port->base_addr + AHCI_PORT_IE, 0x00000001); // D2H interrupt
    
    // Start command engine
    cmd = mmio_read32(port->base_addr + AHCI_PORT_CMD);
    cmd |= 0x00000001; // Start command engine
    mmio_write32(port->base_addr + AHCI_PORT_CMD, cmd);
    
    // Wait for command engine to start
    while (!(mmio_read32(port->base_addr + AHCI_PORT_CMD) & 0x00000001)) {
        // Wait
    }
    
    return 0;
}

// Identify SATA device
static int ahci_identify_device(int port_num) {
    ahci_port_t* port = &ahci_controller.ports[port_num];
    
    // Check if device is present
    uint32_t ssts = mmio_read32(port->base_addr + AHCI_PORT_SSTS);
    if ((ssts & 0x0F) != 0x03) { // Device not present or not ready
        return -1;
    }
    
    // Check signature
    uint32_t sig = mmio_read32(port->base_addr + AHCI_PORT_SIG);
    if (sig != 0x00000101) { // SATA signature
        return -1;
    }
    
    port->sata_capable = 1;
    
    // Send IDENTIFY command
    uint8_t identify_data[512];
    if (ahci_read_sectors(port_num, 0, 1, identify_data) != 1) {
        return -1;
    }
    
    // Parse device information
    // Model name
    for (int i = 0; i < 20; i++) {
        port->model[i*2] = identify_data[54+i*2+1];
        port->model[i*2+1] = identify_data[54+i*2];
    }
    port->model[40] = '\0';
    
    // Serial number
    for (int i = 0; i < 10; i++) {
        port->serial[i*2] = identify_data[20+i*2+1];
        port->serial[i*2+1] = identify_data[20+i*2];
    }
    port->serial[20] = '\0';
    
    // Firmware revision
    for (int i = 0; i < 4; i++) {
        port->firmware[i*2] = identify_data[46+i*2+1];
        port->firmware[i*2+1] = identify_data[46+i*2];
    }
    port->firmware[8] = '\0';
    
    // LBA sectors
    port->sectors = ((uint64_t)identify_data[103] << 48) |
                   ((uint64_t)identify_data[102] << 32) |
                   ((uint64_t)identify_data[101] << 16) |
                   identify_data[100];
    
    // Sector size
    port->sector_size = 512;
    
    // Check NCQ support
    uint16_t capabilities = identify_data[98] | (identify_data[99] << 8);
    port->ncq_supported = (capabilities & 0x6000) ? 1 : 0;
    
    printf("AHCI: Port %d device: %s (%s), %llu sectors, NCQ: %s\n",
           port_num, port->model, port->serial, port->sectors,
           port->ncq_supported ? "Yes" : "No");
    
    return 0;
}

// Read sectors using AHCI
static int ahci_read_sectors(int port_num, uint64_t lba, uint32_t count, void* buffer) {
    ahci_port_t* port = &ahci_controller.ports[port_num];
    
    // Wait for port to be ready
    ahci_wait_for_ready(port_num);
    
    // Setup command header
    ahci_command_header_t* cmd_header = (ahci_command_header_t*)port->command_list_base;
    memset(cmd_header, 0, sizeof(ahci_command_header_t));
    
    cmd_header->command_fis_length = 5; // 5 dwords
    cmd_header->write = 0; // Read command
    cmd_header->prdt_length = 1; // One PRDT entry
    cmd_header->command_table_base_addr = port->command_table_base;
    
    // Setup command table
    ahci_command_table_t* cmd_table = (ahci_command_table_t*)port->command_table_base;
    memset(cmd_table, 0, sizeof(ahci_command_table_t));
    
    // Setup FIS
    ahci_reg_h2d_fis_t* fis = (ahci_reg_h2d_fis_t*)cmd_table->command_fis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->command = 0x25; // READ DMA EXT
    fis->lba_low = lba & 0xFF;
    fis->lba_mid = (lba >> 8) & 0xFF;
    fis->lba_high = (lba >> 16) & 0xFF;
    fis->lba_low_exp = (lba >> 24) & 0xFF;
    fis->lba_mid_exp = (lba >> 32) & 0xFF;
    fis->lba_high_exp = (lba >> 40) & 0xFF;
    fis->count = count & 0xFFFF;
    fis->device = 0x40; // LBA mode
    
    // Setup PRDT
    cmd_table->prdt[0].data_base_addr = (uint32_t)buffer;
    cmd_table->prdt[0].data_base_addr_upper = 0;
    cmd_table->prdt[0].byte_count = count * 512 - 1;
    cmd_table->prdt[0].interrupt_on_completion = 1;
    
    // Issue command
    mmio_write32(port->base_addr + AHCI_PORT_CI, 1);
    
    // Wait for completion
    while (mmio_read32(port->base_addr + AHCI_PORT_CI) & 1) {
        // Wait
    }
    
    // Check for errors
    uint32_t tfd = mmio_read32(port->base_addr + AHCI_PORT_TFD);
    if (tfd & 0x00000001) { // Error bit
        return -1;
    }
    
    return count;
}

// Write sectors using AHCI
static int ahci_write_sectors(int port_num, uint64_t lba, uint32_t count, const void* buffer) {
    ahci_port_t* port = &ahci_controller.ports[port_num];
    
    // Wait for port to be ready
    ahci_wait_for_ready(port_num);
    
    // Setup command header
    ahci_command_header_t* cmd_header = (ahci_command_header_t*)port->command_list_base;
    memset(cmd_header, 0, sizeof(ahci_command_header_t));
    
    cmd_header->command_fis_length = 5; // 5 dwords
    cmd_header->write = 1; // Write command
    cmd_header->prdt_length = 1; // One PRDT entry
    cmd_header->command_table_base_addr = port->command_table_base;
    
    // Setup command table
    ahci_command_table_t* cmd_table = (ahci_command_table_t*)port->command_table_base;
    memset(cmd_table, 0, sizeof(ahci_command_table_t));
    
    // Setup FIS
    ahci_reg_h2d_fis_t* fis = (ahci_reg_h2d_fis_t*)cmd_table->command_fis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->command = 0x35; // WRITE DMA EXT
    fis->lba_low = lba & 0xFF;
    fis->lba_mid = (lba >> 8) & 0xFF;
    fis->lba_high = (lba >> 16) & 0xFF;
    fis->lba_low_exp = (lba >> 24) & 0xFF;
    fis->lba_mid_exp = (lba >> 32) & 0xFF;
    fis->lba_high_exp = (lba >> 40) & 0xFF;
    fis->count = count & 0xFFFF;
    fis->device = 0x40; // LBA mode
    
    // Setup PRDT
    cmd_table->prdt[0].data_base_addr = (uint32_t)buffer;
    cmd_table->prdt[0].data_base_addr_upper = 0;
    cmd_table->prdt[0].byte_count = count * 512 - 1;
    cmd_table->prdt[0].interrupt_on_completion = 1;
    
    // Issue command
    mmio_write32(port->base_addr + AHCI_PORT_CI, 1);
    
    // Wait for completion
    while (mmio_read32(port->base_addr + AHCI_PORT_CI) & 1) {
        // Wait
    }
    
    // Check for errors
    uint32_t tfd = mmio_read32(port->base_addr + AHCI_PORT_TFD);
    if (tfd & 0x00000001) { // Error bit
        return -1;
    }
    
    return count;
}

// Wait for port to be ready
static void ahci_wait_for_ready(int port_num) {
    ahci_port_t* port = &ahci_controller.ports[port_num];
    
    // Wait for command engine to be ready
    while (!(mmio_read32(port->base_addr + AHCI_PORT_CMD) & 0x00000001)) {
        // Wait
    }
    
    // Wait for device to be ready
    while (mmio_read32(port->base_addr + AHCI_PORT_TFD) & 0x00000080) {
        // Wait
    }
}

// Wait for DRQ
static void ahci_wait_for_drq(int port_num) {
    ahci_port_t* port = &ahci_controller.ports[port_num];
    
    while (!(mmio_read32(port->base_addr + AHCI_PORT_TFD) & 0x00000008)) {
        // Wait
    }
}

// AHCI interrupt handler
static void ahci_irq_handler(struct interrupt_frame* frame) {
    // Check which port generated the interrupt
    for (int i = 0; i < ahci_controller.num_ports; i++) {
        if (ahci_controller.ports_implemented & (1 << i)) {
            ahci_port_t* port = &ahci_controller.ports[i];
            uint32_t is = mmio_read32(port->base_addr + AHCI_PORT_IS);
            
            if (is) {
                // Clear interrupt
                mmio_write32(port->base_addr + AHCI_PORT_IS, is);
                
                // Handle interrupt
                if (is & 0x00000001) { // D2H interrupt
                    // Command completed
                }
            }
        }
    }
    
    // Acknowledge interrupt
    outb(0x20, 0x20);
}

// Get AHCI port information
ahci_port_t* ahci_get_port(int port_num) {
    if (port_num >= 0 && port_num < ahci_controller.num_ports) {
        return &ahci_controller.ports[port_num];
    }
    return NULL;
}

// Get number of AHCI ports
int ahci_get_port_count() {
    return ahci_controller.num_ports;
} 