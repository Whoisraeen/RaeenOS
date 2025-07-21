#include <kernel/drivers/nvme.h>
#include <kernel/drivers/device_manager.h>
#include <kernel/memory/memory.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/core/kernel.h>
#include <kernel/libc/string.h>
#include <kernel/libc/stdio.h>
#include <kernel/pci/pci.h>

// NVMe Register Definitions
#define NVME_REG_CAP        0x0000  // Controller Capabilities
#define NVME_REG_VS         0x0008  // Version
#define NVME_REG_INTMS      0x000C  // Interrupt Mask Set
#define NVME_REG_INTMC      0x0010  // Interrupt Mask Clear
#define NVME_REG_CC         0x0014  // Controller Configuration
#define NVME_REG_CSTS       0x001C  // Controller Status
#define NVME_REG_NSSR       0x0020  // NVM Subsystem Reset
#define NVME_REG_AQA        0x0024  // Admin Queue Attributes
#define NVME_REG_ASQ        0x0028  // Admin Submission Queue Base Address
#define NVME_REG_ACQ        0x0030  // Admin Completion Queue Base Address
#define NVME_REG_CMBLOC     0x0038  // Controller Memory Buffer Location
#define NVME_REG_CMBSZ      0x003C  // Controller Memory Buffer Size

// NVMe Command Opcodes
#define NVME_CMD_DELETE_IO_SQ    0x00
#define NVME_CMD_CREATE_IO_SQ    0x01
#define NVME_CMD_GET_LOG_PAGE    0x02
#define NVME_CMD_DELETE_IO_CQ    0x04
#define NVME_CMD_CREATE_IO_CQ    0x05
#define NVME_CMD_IDENTIFY        0x06
#define NVME_CMD_ABORT           0x08
#define NVME_CMD_SET_FEATURES    0x09
#define NVME_CMD_GET_FEATURES    0x0A
#define NVME_CMD_ASYNC_EVENT_REQ 0x0C
#define NVME_CMD_NAMESPACE_MGMT  0x0D
#define NVME_CMD_FIRMWARE_COMMIT 0x10
#define NVME_CMD_FIRMWARE_IMAGE_DOWNLOAD 0x11
#define NVME_CMD_DEVICE_SELF_TEST 0x14
#define NVME_CMD_NAMESPACE_ATTACHMENT 0x15
#define NVME_CMD_KEEP_ALIVE      0x18
#define NVME_CMD_DIRECTIVE_SEND  0x19
#define NVME_CMD_DIRECTIVE_RECEIVE 0x1A
#define NVME_CMD_VIRTUALIZATION_MANAGEMENT 0x1C
#define NVME_CMD_NVME_MI_SEND    0x1D
#define NVME_CMD_NVME_MI_RECEIVE 0x1E
#define NVME_CMD_DOORBELL_BUFFER_CONFIG 0x7C
#define NVME_CMD_FORMAT_NVM      0x80
#define NVME_CMD_SECURITY_SEND   0x81
#define NVME_CMD_SECURITY_RECEIVE 0x82
#define NVME_CMD_SANITIZE_NVM    0x84
#define NVME_CMD_GET_LBA_STATUS  0x86
#define NVME_CMD_READ            0x02
#define NVME_CMD_WRITE           0x01
#define NVME_CMD_FLUSH           0x00
#define NVME_CMD_WRITE_UNCORRECTABLE 0x04
#define NVME_CMD_COMPARE         0x05
#define NVME_CMD_WRITE_ZEROES    0x08
#define NVME_CMD_DATASET_MANAGEMENT 0x09
#define NVME_CMD_VERIFY          0x0C
#define NVME_CMD_RESERVATION_REGISTER 0x0D
#define NVME_CMD_RESERVATION_REPORT_ACQUIRED 0x0E
#define NVME_CMD_RESERVATION_REPORT_RELEASED 0x0F
#define NVME_CMD_RESERVATION_ACQUIRE 0x11
#define NVME_CMD_RESERVATION_RELEASE 0x15

// NVMe Command Structure
typedef struct {
    uint32_t cdw0;           // Command Dword 0
    uint32_t nsid;           // Namespace ID
    uint64_t rsvd2;          // Reserved
    uint64_t mptr;           // Metadata Pointer
    uint64_t prp1;           // PRP Entry 1
    uint64_t prp2;           // PRP Entry 2
    uint64_t cdw10;          // Command Dword 10
    uint64_t cdw11;          // Command Dword 11
    uint32_t cdw12;          // Command Dword 12
    uint32_t cdw13;          // Command Dword 13
    uint32_t cdw14;          // Command Dword 14
    uint32_t cdw15;          // Command Dword 15
} __attribute__((packed)) nvme_command_t;

// NVMe Completion Structure
typedef struct {
    uint64_t cdw0;           // Command Dword 0
    uint32_t rsvd;           // Reserved
    uint16_t sq_head;        // Submission Queue Head Pointer
    uint16_t sq_id;          // Submission Queue Identifier
    uint16_t command_id;     // Command Identifier
    uint16_t status;         // Status Field
} __attribute__((packed)) nvme_completion_t;

// NVMe Queue Structure
typedef struct {
    uint32_t id;             // Queue ID
    uint32_t size;           // Queue size
    uint32_t head;           // Head pointer
    uint32_t tail;           // Tail pointer
    uint32_t phase;          // Phase bit
    nvme_command_t* commands; // Command array
    nvme_completion_t* completions; // Completion array
    uint32_t doorbell;       // Doorbell register offset
} nvme_queue_t;

// NVMe Namespace Structure
typedef struct {
    uint32_t id;             // Namespace ID
    uint64_t size;           // Size in blocks
    uint32_t block_size;     // Block size
    uint32_t max_io_size;    // Maximum I/O size
    uint8_t  format;         // Format
    char     nguid[16];      // Namespace Globally Unique Identifier
    char     eui64[8];       // IEEE Extended Unique Identifier
} nvme_namespace_t;

// NVMe Controller Structure
typedef struct {
    uint32_t base_addr;      // Controller base address
    uint32_t capabilities;   // Controller capabilities
    uint32_t version;        // Version
    uint32_t max_queues;     // Maximum number of queues
    uint32_t max_entries;    // Maximum queue entries
    uint32_t max_namespaces; // Maximum number of namespaces
    nvme_queue_t admin_sq;   // Admin submission queue
    nvme_queue_t admin_cq;   // Admin completion queue
    nvme_queue_t io_sq;      // I/O submission queue
    nvme_queue_t io_cq;      // I/O completion queue
    nvme_namespace_t namespaces[256]; // Namespace array
    uint32_t num_namespaces; // Number of namespaces
    uint8_t  initialized;    // Initialization status
} nvme_controller_t;

// Global NVMe controller
static nvme_controller_t nvme_controller;

// Function prototypes
static int nvme_find_controller();
static int nvme_init_controller();
static int nvme_create_admin_queues();
static int nvme_create_io_queues();
static int nvme_identify_controller();
static int nvme_identify_namespaces();
static int nvme_submit_admin_command(nvme_command_t* cmd, nvme_completion_t* completion);
static int nvme_submit_io_command(nvme_command_t* cmd, nvme_completion_t* completion);
static int nvme_read_sectors(uint32_t nsid, uint64_t lba, uint32_t count, void* buffer);
static int nvme_write_sectors(uint32_t nsid, uint64_t lba, uint32_t count, const void* buffer);
static void nvme_irq_handler(struct interrupt_frame* frame);

// Initialize NVMe subsystem
error_t nvme_init(void) {
    printf("NVMe: Initializing NVMe subsystem...\n");
    
    // Find NVMe controller
    if (!nvme_find_controller()) {
        printf("NVMe: No NVMe controller found\n");
        return E_NOT_FOUND;
    }
    
    // Initialize controller
    if (nvme_init_controller() != 0) {
        printf("NVMe: Failed to initialize controller\n");
        return E_IO;
    }
    
    // Create admin queues
    if (nvme_create_admin_queues() != 0) {
        printf("NVMe: Failed to create admin queues\n");
        return E_IO;
    }
    
    // Identify controller
    if (nvme_identify_controller() != 0) {
        printf("NVMe: Failed to identify controller\n");
        return E_IO;
    }
    
    // Create I/O queues
    if (nvme_create_io_queues() != 0) {
        printf("NVMe: Failed to create I/O queues\n");
        return E_IO;
    }
    
    // Identify namespaces
    if (nvme_identify_namespaces() != 0) {
        printf("NVMe: Failed to identify namespaces\n");
        return E_IO;
    }
    
    // Register IRQ handler
    register_irq_handler(11, nvme_irq_handler); // MSI or legacy IRQ
    
    printf("NVMe: Initialization complete, %d namespaces found\n", nvme_controller.num_namespaces);
    return SUCCESS;
}

// Find NVMe controller via PCI
static int nvme_find_controller() {
    // Search for NVMe controller in PCI space
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int function = 0; function < 8; function++) {
                uint32_t vendor_id = pci_read_config_dword(bus, device, function, 0);
                uint16_t vendor = vendor_id & 0xFFFF;
                uint16_t device_id = (vendor_id >> 16) & 0xFFFF;
                
                if (vendor == 0x8086) { // Intel
                    if (device_id == 0x0953 || device_id == 0x0A53 || 
                        device_id == 0x0A54 || device_id == 0x0A55) {
                        // Found Intel NVMe controller
                        nvme_controller.base_addr = pci_read_config_dword(bus, device, function, 0x10) & 0xFFFFFFF0;
                        nvme_controller.capabilities = mmio_read32(nvme_controller.base_addr + NVME_REG_CAP);
                        nvme_controller.version = mmio_read32(nvme_controller.base_addr + NVME_REG_VS);
                        
                        printf("NVMe: Found Intel controller at 0x%x, version %d.%d.%d\n", 
                               nvme_controller.base_addr,
                               (nvme_controller.version >> 16) & 0xFFFF,
                               (nvme_controller.version >> 8) & 0xFF,
                               nvme_controller.version & 0xFF);
                        return 1;
                    }
                }
            }
        }
    }
    
    return 0;
}

// Initialize NVMe controller
static int nvme_init_controller() {
    // Reset controller
    uint32_t cc = mmio_read32(nvme_controller.base_addr + NVME_REG_CC);
    cc &= ~0x00000001; // Disable controller
    mmio_write32(nvme_controller.base_addr + NVME_REG_CC, cc);
    
    // Wait for controller to be disabled
    while (mmio_read32(nvme_controller.base_addr + NVME_REG_CSTS) & 0x00000001) {
        // Wait
    }
    
    // Clear interrupts
    mmio_write32(nvme_controller.base_addr + NVME_REG_INTMS, 0xFFFFFFFF);
    
    nvme_controller.initialized = 1;
    return 0;
}

// Create admin queues
static int nvme_create_admin_queues() {
    // Allocate admin submission queue
    nvme_controller.admin_sq.size = 64;
    nvme_controller.admin_sq.commands = kmalloc(sizeof(nvme_command_t) * nvme_controller.admin_sq.size);
    if (!nvme_controller.admin_sq.commands) {
        return -1;
    }
    
    // Allocate admin completion queue
    nvme_controller.admin_cq.size = 64;
    nvme_controller.admin_cq.completions = kmalloc(sizeof(nvme_completion_t) * nvme_controller.admin_cq.size);
    if (!nvme_controller.admin_cq.completions) {
        kfree(nvme_controller.admin_sq.commands);
        return -1;
    }
    
    // Clear queues
    memset(nvme_controller.admin_sq.commands, 0, sizeof(nvme_command_t) * nvme_controller.admin_sq.size);
    memset(nvme_controller.admin_cq.completions, 0, sizeof(nvme_completion_t) * nvme_controller.admin_cq.size);
    
    // Set queue attributes
    mmio_write32(nvme_controller.base_addr + NVME_REG_AQA, 
                 (nvme_controller.admin_sq.size - 1) | ((nvme_controller.admin_cq.size - 1) << 16));
    
    // Set queue base addresses
    mmio_write32(nvme_controller.base_addr + NVME_REG_ASQ, (uint32_t)nvme_controller.admin_sq.commands);
    mmio_write32(nvme_controller.base_addr + NVME_REG_ACQ, (uint32_t)nvme_controller.admin_cq.completions);
    
    // Enable controller
    uint32_t cc = mmio_read32(nvme_controller.base_addr + NVME_REG_CC);
    cc |= 0x00000001; // Enable controller
    mmio_write32(nvme_controller.base_addr + NVME_REG_CC, cc);
    
    // Wait for controller to be ready
    while (!(mmio_read32(nvme_controller.base_addr + NVME_REG_CSTS) & 0x00000001)) {
        // Wait
    }
    
    return 0;
}

// Create I/O queues
static int nvme_create_io_queues() {
    // Allocate I/O submission queue
    nvme_controller.io_sq.size = 256;
    nvme_controller.io_sq.commands = kmalloc(sizeof(nvme_command_t) * nvme_controller.io_sq.size);
    if (!nvme_controller.io_sq.commands) {
        return -1;
    }
    
    // Allocate I/O completion queue
    nvme_controller.io_cq.size = 256;
    nvme_controller.io_cq.completions = kmalloc(sizeof(nvme_completion_t) * nvme_controller.io_cq.size);
    if (!nvme_controller.io_cq.completions) {
        kfree(nvme_controller.io_sq.commands);
        return -1;
    }
    
    // Clear queues
    memset(nvme_controller.io_sq.commands, 0, sizeof(nvme_command_t) * nvme_controller.io_sq.size);
    memset(nvme_controller.io_cq.completions, 0, sizeof(nvme_completion_t) * nvme_controller.io_cq.size);
    
    // Create I/O completion queue
    nvme_command_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.cdw0 = NVME_CMD_CREATE_IO_CQ;
    cmd.cdw10 = 1; // Queue ID
    cmd.cdw11 = (nvme_controller.io_cq.size - 1) << 16; // Queue size
    cmd.cdw12 = (uint32_t)nvme_controller.io_cq.completions; // Base address
    cmd.cdw13 = 0; // Upper base address
    cmd.cdw14 = 0; // Interrupt vector
    cmd.cdw15 = 0x00000001; // Physically contiguous, interrupt enabled
    
    nvme_completion_t completion;
    if (nvme_submit_admin_command(&cmd, &completion) != 0) {
        kfree(nvme_controller.io_sq.commands);
        kfree(nvme_controller.io_cq.completions);
        return -1;
    }
    
    // Create I/O submission queue
    memset(&cmd, 0, sizeof(cmd));
    cmd.cdw0 = NVME_CMD_CREATE_IO_SQ;
    cmd.cdw10 = 1; // Queue ID
    cmd.cdw11 = (nvme_controller.io_sq.size - 1) << 16; // Queue size
    cmd.cdw12 = (uint32_t)nvme_controller.io_sq.commands; // Base address
    cmd.cdw13 = 0; // Upper base address
    cmd.cdw14 = 1; // Completion queue ID
    cmd.cdw15 = 0x00000001; // Physically contiguous
    
    if (nvme_submit_admin_command(&cmd, &completion) != 0) {
        kfree(nvme_controller.io_sq.commands);
        kfree(nvme_controller.io_cq.completions);
        return -1;
    }
    
    return 0;
}

// Identify controller
static int nvme_identify_controller() {
    uint8_t identify_data[4096];
    
    nvme_command_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.cdw0 = NVME_CMD_IDENTIFY;
    cmd.cdw10 = 1; // Identify controller data
    
    nvme_completion_t completion;
    if (nvme_submit_admin_command(&cmd, &completion) != 0) {
        return -1;
    }
    
    // Parse controller information
    nvme_controller.max_queues = (identify_data[516] | (identify_data[517] << 8)) + 1;
    nvme_controller.max_entries = (identify_data[518] | (identify_data[519] << 8)) + 1;
    nvme_controller.max_namespaces = identify_data[520] | (identify_data[521] << 8);
    
    printf("NVMe: Controller supports %d queues, %d entries, %d namespaces\n",
           nvme_controller.max_queues, nvme_controller.max_entries, nvme_controller.max_namespaces);
    
    return 0;
}

// Identify namespaces
static int nvme_identify_namespaces() {
    nvme_controller.num_namespaces = 0;
    
    for (uint32_t nsid = 1; nsid <= 256; nsid++) {
        uint8_t identify_data[4096];
        
        nvme_command_t cmd;
        memset(&cmd, 0, sizeof(cmd));
        cmd.cdw0 = NVME_CMD_IDENTIFY;
        cmd.nsid = nsid;
        cmd.cdw10 = 0; // Identify namespace data
        
        nvme_completion_t completion;
        if (nvme_submit_admin_command(&cmd, &completion) == 0) {
            // Parse namespace information
            nvme_namespace_t* ns = &nvme_controller.namespaces[nvme_controller.num_namespaces];
            ns->id = nsid;
            ns->size = *(uint64_t*)&identify_data[0];
            ns->block_size = 1 << identify_data[128];
            ns->max_io_size = (identify_data[130] | (identify_data[131] << 8)) + 1;
            ns->format = identify_data[132];
            
            // Copy GUID and EUI
            memcpy(ns->nguid, &identify_data[104], 16);
            memcpy(ns->eui64, &identify_data[120], 8);
            
            printf("NVMe: Namespace %d: %llu blocks, %d bytes per block\n",
                   nsid, ns->size, ns->block_size);
            
            nvme_controller.num_namespaces++;
        }
    }
    
    return 0;
}

// Submit admin command
static int nvme_submit_admin_command(nvme_command_t* cmd, nvme_completion_t* completion) {
    // Add command to submission queue
    uint32_t tail = nvme_controller.admin_sq.tail;
    nvme_controller.admin_sq.commands[tail] = *cmd;
    nvme_controller.admin_sq.tail = (tail + 1) % nvme_controller.admin_sq.size;
    
    // Ring doorbell
    mmio_write32(nvme_controller.base_addr + 0x1000, tail);
    
    // Wait for completion
    uint32_t head = nvme_controller.admin_cq.head;
    while (head == nvme_controller.admin_cq.head) {
        // Wait
    }
    
    // Copy completion
    *completion = nvme_controller.admin_cq.completions[head];
    nvme_controller.admin_cq.head = (head + 1) % nvme_controller.admin_cq.size;
    
    // Update doorbell
    mmio_write32(nvme_controller.base_addr + 0x1004, head);
    
    return (completion->status & 0xFFFE) == 0 ? 0 : -1;
}

// Submit I/O command
static int nvme_submit_io_command(nvme_command_t* cmd, nvme_completion_t* completion) {
    // Add command to submission queue
    uint32_t tail = nvme_controller.io_sq.tail;
    nvme_controller.io_sq.commands[tail] = *cmd;
    nvme_controller.io_sq.tail = (tail + 1) % nvme_controller.io_sq.size;
    
    // Ring doorbell
    mmio_write32(nvme_controller.base_addr + 0x1008, tail);
    
    // Wait for completion
    uint32_t head = nvme_controller.io_cq.head;
    while (head == nvme_controller.io_cq.head) {
        // Wait
    }
    
    // Copy completion
    *completion = nvme_controller.io_cq.completions[head];
    nvme_controller.io_cq.head = (head + 1) % nvme_controller.io_cq.size;
    
    // Update doorbell
    mmio_write32(nvme_controller.base_addr + 0x100C, head);
    
    return (completion->status & 0xFFFE) == 0 ? 0 : -1;
}

// Read sectors using NVMe
static int nvme_read_sectors(uint32_t nsid, uint64_t lba, uint32_t count, void* buffer) {
    nvme_command_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.cdw0 = NVME_CMD_READ;
    cmd.nsid = nsid;
    cmd.prp1 = (uint64_t)buffer;
    cmd.cdw10 = lba & 0xFFFFFFFF;
    cmd.cdw11 = (lba >> 32) & 0xFFFFFFFF;
    cmd.cdw12 = (count - 1) & 0xFFFF;
    
    nvme_completion_t completion;
    return nvme_submit_io_command(&cmd, &completion);
}

// Write sectors using NVMe
static int nvme_write_sectors(uint32_t nsid, uint64_t lba, uint32_t count, const void* buffer) {
    nvme_command_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.cdw0 = NVME_CMD_WRITE;
    cmd.nsid = nsid;
    cmd.prp1 = (uint64_t)buffer;
    cmd.cdw10 = lba & 0xFFFFFFFF;
    cmd.cdw11 = (lba >> 32) & 0xFFFFFFFF;
    cmd.cdw12 = (count - 1) & 0xFFFF;
    
    nvme_completion_t completion;
    return nvme_submit_io_command(&cmd, &completion);
}

// NVMe interrupt handler
static void nvme_irq_handler(struct interrupt_frame* frame) {
    // Check for completion queue entries
    uint32_t head = nvme_controller.io_cq.head;
    uint32_t phase = nvme_controller.io_cq.phase;
    
    while (1) {
        nvme_completion_t* completion = &nvme_controller.io_cq.completions[head];
        
        if ((completion->status >> 15) != phase) {
            break; // No more completions
        }
        
        // Process completion
        head = (head + 1) % nvme_controller.io_cq.size;
        
        if (head == 0) {
            phase = !phase; // Toggle phase bit
        }
    }
    
    nvme_controller.io_cq.head = head;
    nvme_controller.io_cq.phase = phase;
    
    // Update doorbell
    mmio_write32(nvme_controller.base_addr + 0x100C, head);
    
    // Acknowledge interrupt
    outb(0x20, 0x20);
}

// Get NVMe namespace information
nvme_namespace_t* nvme_get_namespace(uint32_t nsid) {
    for (int i = 0; i < nvme_controller.num_namespaces; i++) {
        if (nvme_controller.namespaces[i].id == nsid) {
            return &nvme_controller.namespaces[i];
        }
    }
    return NULL;
}

// Get number of NVMe namespaces
int nvme_get_namespace_count() {
    return nvme_controller.num_namespaces;
}