#ifndef ATA_H
#define ATA_H

#include "types.h"
#include "hal/hal.h"

// ATA I/O ports
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

// ATA commands
#define ATA_CMD_READ_SECTORS    0x20
#define ATA_CMD_READ_SECTORS_EXT 0x24
#define ATA_CMD_WRITE_SECTORS   0x30
#define ATA_CMD_WRITE_SECTORS_EXT 0x34
#define ATA_CMD_IDENTIFY       0xEC
#define ATA_CMD_SET_FEATURES   0xEF
#define ATA_CMD_FLUSH_CACHE    0xE7
#define ATA_CMD_FLUSH_CACHE_EXT 0xEA

// ATA status register bits
#define ATA_STATUS_ERR         0x01
#define ATA_STATUS_IDX         0x02
#define ATA_STATUS_CORR        0x04
#define ATA_STATUS_DRQ         0x08
#define ATA_STATUS_DSC         0x10
#define ATA_STATUS_DF          0x20
#define ATA_STATUS_DRDY        0x40
#define ATA_STATUS_BSY         0x80

// ATA drive/head register bits
#define ATA_DRIVE_HEAD_LBA     0x40
#define ATA_DRIVE_HEAD_MASTER  0xA0
#define ATA_DRIVE_HEAD_SLAVE   0xB0

// ATA drive types
typedef enum {
    ATA_DRIVE_NONE,
    ATA_DRIVE_PATA,
    ATA_DRIVE_SATA,
    ATA_DRIVE_ATAPI
} ata_drive_type_t;

// ATA drive information
typedef struct {
    u16 config;
    u16 cylinders;
    u16 reserved1;
    u16 heads;
    u16 track_bytes;
    u16 sector_bytes;
    u16 sectors_per_track;
    u16 vendor_specific[3];
    char serial[20];
    u16 controller_type;
    u16 buffer_size;
    u16 ecc_bytes;
    char firmware[8];
    char model[40];
    u8 max_multisector;
    u8 reserved2;
    u16 capabilities;
    u16 reserved3;
    u16 pio_modes;
    u16 dma_modes;
    u16 reserved4[7];
    u16 major_version;
    u16 minor_version;
    u16 command_sets[4];
    u16 reserved5[6];
    u16 ultra_dma_modes;
    u16 time_required;
    u16 time_current;
    u16 reserved6[6];
    u16 sector_size;
    u16 reserved7;
} __attribute__((packed)) ata_identify_data_t;

// ATA drive structure
typedef struct ata_drive {
    u8 controller;      // 0 = primary, 1 = secondary
    u8 drive;          // 0 = master, 1 = slave
    ata_drive_type_t type;
    bool present;
    bool lba_support;
    bool dma_support;
    u64 capacity;      // In sectors
    u32 sector_size;   // Usually 512 bytes
    ata_identify_data_t identify;
    
    // Device driver interface
    error_t (*read_sectors)(struct ata_drive* drive, u64 lba, u32 count, void* buffer);
    error_t (*write_sectors)(struct ata_drive* drive, u64 lba, u32 count, const void* buffer);
    error_t (*flush_cache)(struct ata_drive* drive);
    error_t (*identify)(struct ata_drive* drive);
    
    struct ata_drive* next;
} ata_drive_t;

// ATA controller structure
typedef struct {
    u16 base_port;
    u16 control_port;
    ata_drive_t drives[2];  // Master and slave
    bool initialized;
} ata_controller_t;

// ATA driver functions
error_t ata_init(void);
void ata_shutdown(void);

// Drive management
ata_drive_t* ata_get_drive(u8 controller, u8 drive);
ata_drive_t* ata_find_drive_by_capacity(u64 min_capacity);
ata_drive_t* ata_find_boot_drive(void);
u32 ata_get_drive_count(void);

// Drive operations
error_t ata_read_sectors(ata_drive_t* drive, u64 lba, u32 count, void* buffer);
error_t ata_write_sectors(ata_drive_t* drive, u64 lba, u32 count, const void* buffer);
error_t ata_flush_cache(ata_drive_t* drive);
error_t ata_identify_drive(ata_drive_t* drive);

// Low-level I/O functions
error_t ata_wait_ready(ata_drive_t* drive, u32 timeout_ms);
error_t ata_wait_drq(ata_drive_t* drive, u32 timeout_ms);
void ata_select_drive(ata_drive_t* drive);
error_t ata_send_command(ata_drive_t* drive, u8 command, u64 lba, u32 count);

// Utility functions
bool ata_is_drive_present(ata_drive_t* drive);
u64 ata_get_drive_capacity(ata_drive_t* drive);
const char* ata_get_drive_model(ata_drive_t* drive);
const char* ata_get_drive_serial(ata_drive_t* drive);
ata_drive_type_t ata_get_drive_type(ata_drive_t* drive);

// Error handling
const char* ata_get_error_string(error_t error);
void ata_reset_controller(u8 controller);
error_t ata_soft_reset(ata_drive_t* drive);

// Performance functions
error_t ata_set_pio_mode(ata_drive_t* drive, u8 mode);
error_t ata_set_dma_mode(ata_drive_t* drive, u8 mode);
error_t ata_enable_dma(ata_drive_t* drive);
error_t ata_disable_dma(ata_drive_t* drive);

// Power management
error_t ata_sleep(ata_drive_t* drive);
error_t ata_wake(ata_drive_t* drive);
error_t ata_standby(ata_drive_t* drive);
error_t ata_idle(ata_drive_t* drive);

// SMART functions
error_t ata_smart_enable(ata_drive_t* drive);
error_t ata_smart_disable(ata_drive_t* drive);
error_t ata_smart_read_data(ata_drive_t* drive, void* buffer);
error_t ata_smart_execute_offline(ata_drive_t* drive, u8 test_type);

// Security functions
error_t ata_set_password(ata_drive_t* drive, const char* password);
error_t ata_unlock_drive(ata_drive_t* drive, const char* password);
error_t ata_disable_security(ata_drive_t* drive, const char* password);
error_t ata_secure_erase(ata_drive_t* drive, const char* password);

// Debug functions
void ata_dump_drive_info(ata_drive_t* drive);
void ata_dump_identify_data(ata_drive_t* drive);
void ata_test_drive(ata_drive_t* drive);

#endif // ATA_H 