#include <kernel/drivers/ata.h>
#include <kernel/drivers/device_manager.h>
#include <kernel/memory/memory.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/core/kernel.h>
#include <kernel/libc/string.h>
#include <kernel/libc/stdio.h>

// ATA Controller I/O ports
#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_ERROR       0x1F1
#define ATA_PRIMARY_FEATURES    0x1F1
#define ATA_PRIMARY_SECTOR_COUNT 0x1F2
#define ATA_PRIMARY_LBA_LOW     0x1F3
#define ATA_PRIMARY_LBA_MID     0x1F4
#define ATA_PRIMARY_LBA_HIGH    0x1F5
#define ATA_PRIMARY_DRIVE_HEAD  0x1F6
#define ATA_PRIMARY_STATUS      0x1F7
#define ATA_PRIMARY_COMMAND     0x1F7

#define ATA_SECONDARY_DATA      0x170
#define ATA_SECONDARY_ERROR     0x171
#define ATA_SECONDARY_FEATURES  0x171
#define ATA_SECONDARY_SECTOR_COUNT 0x172
#define ATA_SECONDARY_LBA_LOW   0x173
#define ATA_SECONDARY_LBA_MID   0x174
#define ATA_SECONDARY_LBA_HIGH  0x175
#define ATA_SECONDARY_DRIVE_HEAD 0x176
#define ATA_SECONDARY_STATUS    0x177
#define ATA_SECONDARY_COMMAND   0x177

// ATA Commands
#define ATA_CMD_READ_SECTORS    0x20
#define ATA_CMD_WRITE_SECTORS   0x30
#define ATA_CMD_IDENTIFY        0xEC
#define ATA_CMD_FLUSH_CACHE     0xE7
#define ATA_CMD_PACKET          0xA0

// ATA Status bits
#define ATA_STATUS_ERR          0x01
#define ATA_STATUS_DRQ          0x08
#define ATA_STATUS_BSY          0x80
#define ATA_STATUS_RDY          0x40

// Drive selection
#define ATA_DRIVE_MASTER        0xA0
#define ATA_DRIVE_SLAVE         0xB0

// LBA addressing
#define ATA_LBA_ENABLE          0x40

// DMA support
#define ATA_DMA_ENABLE          0x01
#define ATA_DMA_READ            0x08
#define ATA_DMA_WRITE           0x00

// ATA Device structure
typedef struct {
    uint8_t controller;         // 0 = primary, 1 = secondary
    uint8_t drive;             // 0 = master, 1 = slave
    uint32_t lba_sectors;      // Total sectors
    uint32_t sector_size;      // Bytes per sector
    uint8_t dma_supported;     // DMA capability
    uint8_t lba_supported;     // LBA addressing support
    char model[41];           // Device model string
    char serial[21];          // Serial number
    uint16_t capabilities;     // Device capabilities
} ata_device_t;

// Global ATA devices
static ata_device_t ata_devices[4];  // 2 controllers * 2 drives
static int ata_device_count = 0;

// DMA buffer for ATA operations
static uint8_t* ata_dma_buffer = NULL;
static uint32_t ata_dma_buffer_size = 64 * 1024; // 64KB DMA buffer

// Function prototypes
static void ata_wait_ready(uint8_t controller);
static void ata_wait_drq(uint8_t controller);
static int ata_identify_device(uint8_t controller, uint8_t drive);
static int ata_read_sectors_lba(uint8_t controller, uint8_t drive, uint32_t lba, uint8_t count, void* buffer);
static int ata_write_sectors_lba(uint8_t controller, uint8_t drive, uint32_t lba, uint8_t count, const void* buffer);
static void ata_irq_handler(struct interrupt_frame* frame);

// Initialize ATA subsystem
int ata_init() {
    printf("ATA: Initializing ATA subsystem...\n");
    
    // Allocate DMA buffer
    ata_dma_buffer = kmalloc(ata_dma_buffer_size);
    if (!ata_dma_buffer) {
        printf("ATA: Failed to allocate DMA buffer\n");
        return -1;
    }
    
    // Register IRQ handlers
    register_irq_handler(14, ata_irq_handler); // Primary ATA
    register_irq_handler(15, ata_irq_handler); // Secondary ATA
    
    // Initialize devices
    ata_device_count = 0;
    
    // Probe primary controller
    if (ata_identify_device(0, 0)) { // Master
        ata_devices[ata_device_count].controller = 0;
        ata_devices[ata_device_count].drive = 0;
        ata_device_count++;
        printf("ATA: Found primary master drive: %s\n", ata_devices[ata_device_count-1].model);
    }
    
    if (ata_identify_device(0, 1)) { // Slave
        ata_devices[ata_device_count].controller = 0;
        ata_devices[ata_device_count].drive = 1;
        ata_device_count++;
        printf("ATA: Found primary slave drive: %s\n", ata_devices[ata_device_count-1].model);
    }
    
    // Probe secondary controller
    if (ata_identify_device(1, 0)) { // Master
        ata_devices[ata_device_count].controller = 1;
        ata_devices[ata_device_count].drive = 0;
        ata_device_count++;
        printf("ATA: Found secondary master drive: %s\n", ata_devices[ata_device_count-1].model);
    }
    
    if (ata_identify_device(1, 1)) { // Slave
        ata_devices[ata_device_count].controller = 1;
        ata_devices[ata_device_count].drive = 1;
        ata_device_count++;
        printf("ATA: Found secondary slave drive: %s\n", ata_devices[ata_device_count-1].model);
    }
    
    printf("ATA: Found %d devices\n", ata_device_count);
    
    // Register block devices
    for (int i = 0; i < ata_device_count; i++) {
        char device_name[32];
        snprintf(device_name, sizeof(device_name), "hd%c", 'a' + i);
        
        block_device_t* block_dev = kmalloc(sizeof(block_device_t));
        if (block_dev) {
            block_dev->name = strdup(device_name);
            block_dev->sector_size = ata_devices[i].sector_size;
            block_dev->total_sectors = ata_devices[i].lba_sectors;
            block_dev->read_sectors = ata_read_sectors;
            block_dev->write_sectors = ata_write_sectors;
            block_dev->private_data = &ata_devices[i];
            
            register_block_device(block_dev);
            printf("ATA: Registered block device %s\n", device_name);
        }
    }
    
    return ata_device_count;
}

// Wait for device to be ready
static void ata_wait_ready(uint8_t controller) {
    uint16_t status_port = (controller == 0) ? ATA_PRIMARY_STATUS : ATA_SECONDARY_STATUS;
    
    // Wait for BSY to clear
    while (inb(status_port) & ATA_STATUS_BSY) {
        // Small delay
        for (volatile int i = 0; i < 1000; i++);
    }
}

// Wait for DRQ (data request)
static void ata_wait_drq(uint8_t controller) {
    uint16_t status_port = (controller == 0) ? ATA_PRIMARY_STATUS : ATA_SECONDARY_STATUS;
    
    while (!(inb(status_port) & ATA_STATUS_DRQ)) {
        // Small delay
        for (volatile int i = 0; i < 1000; i++);
    }
}

// Identify ATA device
static int ata_identify_device(uint8_t controller, uint8_t drive) {
    uint16_t data_port = (controller == 0) ? ATA_PRIMARY_DATA : ATA_SECONDARY_DATA;
    uint16_t drive_head_port = (controller == 0) ? ATA_PRIMARY_DRIVE_HEAD : ATA_SECONDARY_DRIVE_HEAD;
    uint16_t command_port = (controller == 0) ? ATA_PRIMARY_COMMAND : ATA_SECONDARY_COMMAND;
    uint16_t status_port = (controller == 0) ? ATA_PRIMARY_STATUS : ATA_SECONDARY_STATUS;
    
    // Select drive
    uint8_t drive_select = (drive == 0) ? ATA_DRIVE_MASTER : ATA_DRIVE_SLAVE;
    outb(drive_head_port, drive_select);
    
    // Wait for device to be ready
    ata_wait_ready(controller);
    
    // Send IDENTIFY command
    outb(command_port, ATA_CMD_IDENTIFY);
    
    // Wait for response
    ata_wait_ready(controller);
    
    // Check if device responded
    uint8_t status = inb(status_port);
    if (status == 0) {
        return 0; // No device
    }
    
    // Wait for DRQ
    ata_wait_drq(controller);
    
    // Read identify data
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(data_port);
    }
    
    // Parse device information
    ata_device_t* device = &ata_devices[ata_device_count];
    
    // Model name
    for (int i = 0; i < 20; i++) {
        device->model[i*2] = (identify_data[27+i] >> 8) & 0xFF;
        device->model[i*2+1] = identify_data[27+i] & 0xFF;
    }
    device->model[40] = '\0';
    
    // Serial number
    for (int i = 0; i < 10; i++) {
        device->serial[i*2] = (identify_data[10+i] >> 8) & 0xFF;
        device->serial[i*2+1] = identify_data[10+i] & 0xFF;
    }
    device->serial[20] = '\0';
    
    // Capabilities
    device->capabilities = identify_data[49];
    device->lba_supported = (device->capabilities & 0x200) ? 1 : 0;
    device->dma_supported = (device->capabilities & 0x100) ? 1 : 0;
    
    // LBA sectors
    if (device->lba_supported) {
        device->lba_sectors = ((uint32_t)identify_data[61] << 16) | identify_data[60];
    } else {
        // CHS sectors
        uint16_t cylinders = identify_data[1];
        uint16_t heads = identify_data[3];
        uint16_t sectors = identify_data[6];
        device->lba_sectors = cylinders * heads * sectors;
    }
    
    // Sector size (usually 512 bytes)
    device->sector_size = 512;
    
    return 1;
}

// Read sectors using LBA addressing
static int ata_read_sectors_lba(uint8_t controller, uint8_t drive, uint32_t lba, uint8_t count, void* buffer) {
    uint16_t data_port = (controller == 0) ? ATA_PRIMARY_DATA : ATA_SECONDARY_DATA;
    uint16_t sector_count_port = (controller == 0) ? ATA_PRIMARY_SECTOR_COUNT : ATA_SECONDARY_SECTOR_COUNT;
    uint16_t lba_low_port = (controller == 0) ? ATA_PRIMARY_LBA_LOW : ATA_SECONDARY_LBA_LOW;
    uint16_t lba_mid_port = (controller == 0) ? ATA_PRIMARY_LBA_MID : ATA_SECONDARY_LBA_MID;
    uint16_t lba_high_port = (controller == 0) ? ATA_PRIMARY_LBA_HIGH : ATA_SECONDARY_LBA_HIGH;
    uint16_t drive_head_port = (controller == 0) ? ATA_PRIMARY_DRIVE_HEAD : ATA_SECONDARY_DRIVE_HEAD;
    uint16_t command_port = (controller == 0) ? ATA_PRIMARY_COMMAND : ATA_SECONDARY_COMMAND;
    
    // Select drive
    uint8_t drive_select = (drive == 0) ? ATA_DRIVE_MASTER : ATA_DRIVE_SLAVE;
    outb(drive_head_port, drive_select | ATA_LBA_ENABLE | ((lba >> 24) & 0x0F));
    
    // Set sector count
    outb(sector_count_port, count);
    
    // Set LBA
    outb(lba_low_port, lba & 0xFF);
    outb(lba_mid_port, (lba >> 8) & 0xFF);
    outb(lba_high_port, (lba >> 16) & 0xFF);
    
    // Send read command
    outb(command_port, ATA_CMD_READ_SECTORS);
    
    // Wait for DRQ
    ata_wait_drq(controller);
    
    // Read data
    uint16_t* data_buffer = (uint16_t*)buffer;
    for (int sector = 0; sector < count; sector++) {
        for (int word = 0; word < 256; word++) {
            data_buffer[sector * 256 + word] = inw(data_port);
        }
    }
    
    return count;
}

// Write sectors using LBA addressing
static int ata_write_sectors_lba(uint8_t controller, uint8_t drive, uint32_t lba, uint8_t count, const void* buffer) {
    uint16_t data_port = (controller == 0) ? ATA_PRIMARY_DATA : ATA_SECONDARY_DATA;
    uint16_t sector_count_port = (controller == 0) ? ATA_PRIMARY_SECTOR_COUNT : ATA_SECONDARY_SECTOR_COUNT;
    uint16_t lba_low_port = (controller == 0) ? ATA_PRIMARY_LBA_LOW : ATA_SECONDARY_LBA_LOW;
    uint16_t lba_mid_port = (controller == 0) ? ATA_PRIMARY_LBA_MID : ATA_SECONDARY_LBA_MID;
    uint16_t lba_high_port = (controller == 0) ? ATA_PRIMARY_LBA_HIGH : ATA_SECONDARY_LBA_HIGH;
    uint16_t drive_head_port = (controller == 0) ? ATA_PRIMARY_DRIVE_HEAD : ATA_SECONDARY_DRIVE_HEAD;
    uint16_t command_port = (controller == 0) ? ATA_PRIMARY_COMMAND : ATA_SECONDARY_COMMAND;
    
    // Select drive
    uint8_t drive_select = (drive == 0) ? ATA_DRIVE_MASTER : ATA_DRIVE_SLAVE;
    outb(drive_head_port, drive_select | ATA_LBA_ENABLE | ((lba >> 24) & 0x0F));
    
    // Set sector count
    outb(sector_count_port, count);
    
    // Set LBA
    outb(lba_low_port, lba & 0xFF);
    outb(lba_mid_port, (lba >> 8) & 0xFF);
    outb(lba_high_port, (lba >> 16) & 0xFF);
    
    // Send write command
    outb(command_port, ATA_CMD_WRITE_SECTORS);
    
    // Wait for DRQ
    ata_wait_drq(controller);
    
    // Write data
    const uint16_t* data_buffer = (const uint16_t*)buffer;
    for (int sector = 0; sector < count; sector++) {
        for (int word = 0; word < 256; word++) {
            outw(data_port, data_buffer[sector * 256 + word]);
        }
    }
    
    // Flush cache
    outb(command_port, ATA_CMD_FLUSH_CACHE);
    ata_wait_ready(controller);
    
    return count;
}

// Block device read interface
int ata_read_sectors(block_device_t* device, uint32_t sector, uint32_t count, void* buffer) {
    ata_device_t* ata_dev = (ata_device_t*)device->private_data;
    
    if (!ata_dev) {
        return -1;
    }
    
    // Check bounds
    if (sector + count > ata_dev->lba_sectors) {
        return -1;
    }
    
    return ata_read_sectors_lba(ata_dev->controller, ata_dev->drive, sector, count, buffer);
}

// Block device write interface
int ata_write_sectors(block_device_t* device, uint32_t sector, uint32_t count, const void* buffer) {
    ata_device_t* ata_dev = (ata_device_t*)device->private_data;
    
    if (!ata_dev) {
        return -1;
    }
    
    // Check bounds
    if (sector + count > ata_dev->lba_sectors) {
        return -1;
    }
    
    return ata_write_sectors_lba(ata_dev->controller, ata_dev->drive, sector, count, buffer);
}

// ATA interrupt handler
static void ata_irq_handler(struct interrupt_frame* frame) {
    // Acknowledge interrupt
    // For now, just acknowledge the PIC
    if (frame->int_no == 14) {
        outb(0x20, 0x20); // Primary PIC
    } else if (frame->int_no == 15) {
        outb(0xA0, 0x20); // Secondary PIC
        outb(0x20, 0x20); // Primary PIC
    }
}

// Get ATA device information
ata_device_t* ata_get_device(int index) {
    if (index >= 0 && index < ata_device_count) {
        return &ata_devices[index];
    }
    return NULL;
}

// Get number of ATA devices
int ata_get_device_count() {
    return ata_device_count;
} 