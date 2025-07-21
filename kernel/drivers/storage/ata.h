#ifndef ATA_H
#define ATA_H

#include "types.h"
#include "drivers/include/driver_framework.h"

// ATA/SATA Driver Implementation
// Provides real disk I/O capabilities for persistent storage

// ATA Command Codes
#define ATA_CMD_READ_SECTORS    0x20
#define ATA_CMD_WRITE_SECTORS   0x30
#define ATA_CMD_IDENTIFY        0xEC
#define ATA_CMD_FLUSH           0xE7

// ATA Status Register Bits
#define ATA_STATUS_BSY          0x80    // Busy
#define ATA_STATUS_RDY          0x40    // Ready
#define ATA_STATUS_DRQ          0x08    // Data Request
#define ATA_STATUS_ERR          0x01    // Error

// ATA I/O Ports (Primary)
#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_FEATURES    0x1F1
#define ATA_PRIMARY_SECTOR_COUNT 0x1F2
#define ATA_PRIMARY_LBA_LOW     0x1F3
#define ATA_PRIMARY_LBA_MID     0x1F4
#define ATA_PRIMARY_LBA_HIGH    0x1F5
#define ATA_PRIMARY_DRIVE_HEAD  0x1F6
#define ATA_PRIMARY_STATUS      0x1F7
#define ATA_PRIMARY_COMMAND     0x1F7

// ATA I/O Ports (Secondary)
#define ATA_SECONDARY_DATA      0x170
#define ATA_SECONDARY_FEATURES  0x171
#define ATA_SECONDARY_SECTOR_COUNT 0x172
#define ATA_SECONDARY_LBA_LOW   0x173
#define ATA_SECONDARY_LBA_MID   0x174
#define ATA_SECONDARY_LBA_HIGH  0x175
#define ATA_SECONDARY_DRIVE_HEAD 0x176
#define ATA_SECONDARY_STATUS    0x177
#define ATA_SECONDARY_COMMAND   0x177

// ATA Drive Selection
#define ATA_DRIVE_MASTER        0xA0
#define ATA_DRIVE_SLAVE         0xB0

// ATA Device Structure
typedef struct ata_device {
    device_t base;
    
    // ATA-specific fields
    u16 io_base;
    u16 control_base;
    u8 drive_select;
    bool is_master;
    
    // Device identification
    u16 identify_data[256];
    u64 total_sectors;
    u32 sector_size;
    
    // Performance counters
    u64 read_operations;
    u64 write_operations;
    u64 bytes_read;
    u64 bytes_written;
    
    // Error tracking
    u32 error_count;
    u32 last_error;
} ata_device_t;

// Function prototypes
error_t ata_init(void);
void ata_shutdown(void);

// Device operations
error_t ata_probe(device_t* device);
error_t ata_init_device(device_t* device);
error_t ata_shutdown_device(device_t* device);

// I/O operations
ssize_t ata_read(device_t* device, void* buffer, size_t size, off_t offset);
ssize_t ata_write(device_t* device, const void* buffer, size_t size, off_t offset);
error_t ata_ioctl(device_t* device, u32 cmd, void* arg);

// Low-level operations
error_t ata_read_sectors(ata_device_t* dev, u64 lba, u32 count, void* buffer);
error_t ata_write_sectors(ata_device_t* dev, u64 lba, u32 count, const void* buffer);
error_t ata_identify(ata_device_t* dev);
error_t ata_flush(ata_device_t* dev);

// Utility functions
void ata_select_drive(ata_device_t* dev);
error_t ata_wait_ready(ata_device_t* dev);
error_t ata_wait_data(ata_device_t* dev);
bool ata_is_error(ata_device_t* dev);

// Device detection
void ata_detect_devices(void);
ata_device_t* ata_get_device(u32 index);
u32 ata_get_device_count(void);

// Driver structure
extern driver_t ata_driver;

#endif // ATA_H