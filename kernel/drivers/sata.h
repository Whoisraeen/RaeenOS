#ifndef SATA_H
#define SATA_H

#include "../core/types.h"
#include "../core/error.h"
#include "driver_framework.h"

// SATA/AHCI constants
#define SATA_MAX_CONTROLLERS 8
#define SATA_MAX_PORTS 32
#define SATA_MAX_DEVICES 256
#define SATA_SECTOR_SIZE 512
#define SATA_MAX_SECTORS_PER_TRANSFER 65536

// AHCI Host Control Registers
#define AHCI_HBA_CAP     0x00  // Host Capabilities
#define AHCI_HBA_GHC     0x04  // Global Host Control
#define AHCI_HBA_IS      0x08  // Interrupt Status
#define AHCI_HBA_PI      0x0C  // Ports Implemented
#define AHCI_HBA_VS      0x10  // Version
#define AHCI_HBA_CCC_CTL 0x14  // Command Completion Coalescing Control
#define AHCI_HBA_CCC_PORTS 0x18 // Command Completion Coalescing Ports
#define AHCI_HBA_EM_LOC  0x1C  // Enclosure Management Location
#define AHCI_HBA_EM_CTL  0x20  // Enclosure Management Control
#define AHCI_HBA_CAP2    0x24  // Host Capabilities Extended
#define AHCI_HBA_BOHC    0x28  // BIOS/OS Handoff Control and Status

// AHCI Port Registers (offset from port base)
#define AHCI_PORT_CLB    0x00  // Command List Base Address
#define AHCI_PORT_CLBU   0x04  // Command List Base Address Upper
#define AHCI_PORT_FB     0x08  // FIS Base Address
#define AHCI_PORT_FBU    0x0C  // FIS Base Address Upper
#define AHCI_PORT_IS     0x10  // Interrupt Status
#define AHCI_PORT_IE     0x14  // Interrupt Enable
#define AHCI_PORT_CMD    0x18  // Command and Status
#define AHCI_PORT_TFD    0x20  // Task File Data
#define AHCI_PORT_SIG    0x24  // Signature
#define AHCI_PORT_SSTS   0x28  // Serial ATA Status
#define AHCI_PORT_SCTL   0x2C  // Serial ATA Control
#define AHCI_PORT_SERR   0x30  // Serial ATA Error
#define AHCI_PORT_SACT   0x34  // Serial ATA Active
#define AHCI_PORT_CI     0x38  // Command Issue
#define AHCI_PORT_SNTF   0x3C  // Serial ATA Notification

// SATA device types
typedef enum {
    SATA_DEVICE_TYPE_NONE = 0,
    SATA_DEVICE_TYPE_SATA,
    SATA_DEVICE_TYPE_SATAPI,
    SATA_DEVICE_TYPE_SEMB,
    SATA_DEVICE_TYPE_PM
} sata_device_type_t;

// SATA interface speeds
typedef enum {
    SATA_SPEED_GEN1 = 0,  // 1.5 Gbps
    SATA_SPEED_GEN2,      // 3.0 Gbps
    SATA_SPEED_GEN3,      // 6.0 Gbps
    SATA_SPEED_GEN4       // 12.0 Gbps
} sata_speed_t;

// SATA device states
typedef enum {
    SATA_DEVICE_STATE_UNKNOWN = 0,
    SATA_DEVICE_STATE_PRESENT,
    SATA_DEVICE_STATE_ACTIVE,
    SATA_DEVICE_STATE_OFFLINE,
    SATA_DEVICE_STATE_ERROR
} sata_device_state_t;

// ATA commands
typedef enum {
    ATA_CMD_READ_DMA = 0xC8,
    ATA_CMD_READ_DMA_EXT = 0x25,
    ATA_CMD_WRITE_DMA = 0xCA,
    ATA_CMD_WRITE_DMA_EXT = 0x35,
    ATA_CMD_IDENTIFY = 0xEC,
    ATA_CMD_IDENTIFY_PACKET = 0xA1,
    ATA_CMD_FLUSH_CACHE = 0xE7,
    ATA_CMD_FLUSH_CACHE_EXT = 0xEA,
    ATA_CMD_STANDBY_IMMEDIATE = 0xE0,
    ATA_CMD_IDLE_IMMEDIATE = 0xE1,
    ATA_CMD_CHECK_POWER_MODE = 0xE5,
    ATA_CMD_SLEEP = 0xE6,
    ATA_CMD_SET_FEATURES = 0xEF,
    ATA_CMD_SMART = 0xB0,
    ATA_CMD_SECURITY_SET_PASSWORD = 0xF1,
    ATA_CMD_SECURITY_UNLOCK = 0xF2,
    ATA_CMD_SECURITY_ERASE_PREPARE = 0xF3,
    ATA_CMD_SECURITY_ERASE_UNIT = 0xF4,
    ATA_CMD_SECURITY_FREEZE_LOCK = 0xF5,
    ATA_CMD_SECURITY_DISABLE_PASSWORD = 0xF6
} ata_command_t;

// FIS (Frame Information Structure) types
typedef enum {
    FIS_TYPE_REG_H2D = 0x27,    // Register FIS - host to device
    FIS_TYPE_REG_D2H = 0x34,    // Register FIS - device to host
    FIS_TYPE_DMA_ACT = 0x39,    // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP = 0x41,  // DMA setup FIS - bidirectional
    FIS_TYPE_DATA = 0x46,       // Data FIS - bidirectional
    FIS_TYPE_BIST = 0x58,       // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP = 0x5F,  // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS = 0xA1    // Set device bits FIS - device to host
} fis_type_t;

// Register FIS - Host to Device
typedef struct __attribute__((packed)) {
    uint8_t fis_type;      // FIS_TYPE_REG_H2D
    uint8_t pmport:4;      // Port multiplier
    uint8_t rsv0:3;        // Reserved
    uint8_t c:1;           // 1: Command, 0: Control
    uint8_t command;       // Command register
    uint8_t featurel;      // Feature register, 7:0
    
    uint8_t lba0;          // LBA low register, 7:0
    uint8_t lba1;          // LBA mid register, 15:8
    uint8_t lba2;          // LBA high register, 23:16
    uint8_t device;        // Device register
    
    uint8_t lba3;          // LBA register, 31:24
    uint8_t lba4;          // LBA register, 39:32
    uint8_t lba5;          // LBA register, 47:40
    uint8_t featureh;      // Feature register, 15:8
    
    uint8_t countl;        // Count register, 7:0
    uint8_t counth;        // Count register, 15:8
    uint8_t icc;           // Isochronous command completion
    uint8_t control;       // Control register
    
    uint8_t rsv1[4];       // Reserved
} fis_reg_h2d_t;

// Register FIS - Device to Host
typedef struct __attribute__((packed)) {
    uint8_t fis_type;      // FIS_TYPE_REG_D2H
    uint8_t pmport:4;      // Port multiplier
    uint8_t rsv0:2;        // Reserved
    uint8_t i:1;           // Interrupt bit
    uint8_t rsv1:1;        // Reserved
    uint8_t status;        // Status register
    uint8_t error;         // Error register
    
    uint8_t lba0;          // LBA low register, 7:0
    uint8_t lba1;          // LBA mid register, 15:8
    uint8_t lba2;          // LBA high register, 23:16
    uint8_t device;        // Device register
    
    uint8_t lba3;          // LBA register, 31:24
    uint8_t lba4;          // LBA register, 39:32
    uint8_t lba5;          // LBA register, 47:40
    uint8_t rsv2;          // Reserved
    
    uint8_t countl;        // Count register, 7:0
    uint8_t counth;        // Count register, 15:8
    uint8_t rsv3[2];       // Reserved
    
    uint8_t rsv4[4];       // Reserved
} fis_reg_d2h_t;

// Data FIS
typedef struct __attribute__((packed)) {
    uint8_t fis_type;      // FIS_TYPE_DATA
    uint8_t pmport:4;      // Port multiplier
    uint8_t rsv0:4;        // Reserved
    uint8_t rsv1[2];       // Reserved
    
    uint32_t data[1];      // Payload
} fis_data_t;

// PIO Setup FIS
typedef struct __attribute__((packed)) {
    uint8_t fis_type;      // FIS_TYPE_PIO_SETUP
    uint8_t pmport:4;      // Port multiplier
    uint8_t rsv0:1;        // Reserved
    uint8_t d:1;           // Data transfer direction, 1 - device to host
    uint8_t i:1;           // Interrupt bit
    uint8_t rsv1:1;        // Reserved
    uint8_t status;        // Status register
    uint8_t error;         // Error register
    
    uint8_t lba0;          // LBA low register, 7:0
    uint8_t lba1;          // LBA mid register, 15:8
    uint8_t lba2;          // LBA high register, 23:16
    uint8_t device;        // Device register
    
    uint8_t lba3;          // LBA register, 31:24
    uint8_t lba4;          // LBA register, 39:32
    uint8_t lba5;          // LBA register, 47:40
    uint8_t rsv2;          // Reserved
    
    uint8_t countl;        // Count register, 7:0
    uint8_t counth;        // Count register, 15:8
    uint8_t rsv3;          // Reserved
    uint8_t e_status;      // New value of status register
    
    uint16_t tc;           // Transfer count
    uint8_t rsv4[2];       // Reserved
} fis_pio_setup_t;

// DMA Setup FIS
typedef struct __attribute__((packed)) {
    uint8_t fis_type;      // FIS_TYPE_DMA_SETUP
    uint8_t pmport:4;      // Port multiplier
    uint8_t rsv0:1;        // Reserved
    uint8_t d:1;           // Data transfer direction, 1 - device to host
    uint8_t i:1;           // Interrupt bit
    uint8_t a:1;           // Auto-activate. Specifies if DMA Activate FIS is needed
    uint8_t rsv1[2];       // Reserved
    
    uint64_t dma_buffer_id; // DMA Buffer Identifier
    uint32_t rsv2;         // More reserved
    uint32_t dma_buf_offset; // Byte offset into buffer
    uint32_t transfer_count; // Number of bytes to transfer
    uint32_t rsv3;         // Reserved
} fis_dma_setup_t;

// AHCI Command Header
typedef struct __attribute__((packed)) {
    uint8_t cfl:5;         // Command FIS length in DWORDS, 2 ~ 16
    uint8_t a:1;           // ATAPI
    uint8_t w:1;           // Write, 1: H2D, 0: D2H
    uint8_t p:1;           // Prefetchable
    
    uint8_t r:1;           // Reset
    uint8_t b:1;           // BIST
    uint8_t c:1;           // Clear busy upon R_OK
    uint8_t rsv0:1;        // Reserved
    uint8_t pmp:4;         // Port multiplier port
    
    uint16_t prdtl;        // Physical region descriptor table length in entries
    
    volatile uint32_t prdbc; // Physical region descriptor byte count transferred
    
    uint32_t ctba;         // Command table descriptor base address
    uint32_t ctbau;        // Command table descriptor base address upper 32 bits
    
    uint32_t rsv1[4];      // Reserved
} ahci_cmd_header_t;

// AHCI Physical Region Descriptor Table Entry
typedef struct __attribute__((packed)) {
    uint32_t dba;          // Data base address
    uint32_t dbau;         // Data base address upper 32 bits
    uint32_t rsv0;         // Reserved
    
    uint32_t dbc:22;       // Byte count, 4M max
    uint32_t rsv1:9;       // Reserved
    uint32_t i:1;          // Interrupt on completion
} ahci_prdt_entry_t;

// AHCI Command Table
typedef struct __attribute__((packed)) {
    uint8_t cfis[64];      // Command FIS
    uint8_t acmd[16];      // ATAPI command, 12 or 16 bytes
    uint8_t rsv[48];       // Reserved
    ahci_prdt_entry_t prdt_entry[1]; // Physical region descriptor table entries
} ahci_cmd_table_t;

// ATA IDENTIFY data structure
typedef struct __attribute__((packed)) {
    uint16_t general_config;           // 0: General configuration
    uint16_t num_cylinders;            // 1: Number of cylinders
    uint16_t specific_config;          // 2: Specific configuration
    uint16_t num_heads;                // 3: Number of heads
    uint16_t unformatted_bytes_track;  // 4: Unformatted bytes per track
    uint16_t unformatted_bytes_sector; // 5: Unformatted bytes per sector
    uint16_t sectors_per_track;        // 6: Sectors per track
    uint16_t vendor_unique1[3];        // 7-9: Vendor unique
    char serial_number[20];            // 10-19: Serial number
    uint16_t buffer_type;              // 20: Buffer type
    uint16_t buffer_size;              // 21: Buffer size
    uint16_t ecc_bytes;                // 22: ECC bytes
    char firmware_revision[8];         // 23-26: Firmware revision
    char model_number[40];             // 27-46: Model number
    uint16_t max_sectors_per_interrupt; // 47: Maximum sectors per interrupt
    uint16_t dword_io;                 // 48: Dword I/O
    uint16_t capabilities;             // 49: Capabilities
    uint16_t capabilities2;            // 50: Capabilities
    uint16_t pio_mode;                 // 51: PIO mode
    uint16_t dma_mode;                 // 52: DMA mode
    uint16_t field_validity;           // 53: Field validity
    uint16_t current_cylinders;        // 54: Current cylinders
    uint16_t current_heads;            // 55: Current heads
    uint16_t current_sectors;          // 56: Current sectors
    uint32_t current_capacity;         // 57-58: Current capacity
    uint16_t multi_sector_setting;     // 59: Multi-sector setting
    uint32_t lba_capacity;             // 60-61: LBA capacity
    uint16_t single_word_dma;          // 62: Single word DMA
    uint16_t multi_word_dma;           // 63: Multi word DMA
    uint16_t pio_modes;                // 64: PIO modes
    uint16_t min_mw_xfer_cycle;        // 65: Minimum multiword transfer cycle
    uint16_t rec_mw_xfer_cycle;        // 66: Recommended multiword transfer cycle
    uint16_t min_pio_cycle;            // 67: Minimum PIO cycle
    uint16_t min_pio_cycle_iordy;      // 68: Minimum PIO cycle with IORDY
    uint16_t additional_supported;     // 69: Additional supported
    uint16_t reserved1;                // 70: Reserved
    uint16_t release_time_overlap;     // 71: Release time for overlapped commands
    uint16_t release_time_service;     // 72: Release time for service commands
    uint16_t major_revision;           // 73: Major revision number
    uint16_t minor_revision;           // 74: Minor revision number
    uint16_t command_set_support;      // 75: Command set support
    uint16_t command_set_support2;     // 76: Command set support
    uint16_t command_set_support_ext;  // 77: Command set support extension
    uint16_t command_set_enable;       // 78: Command set enable
    uint16_t command_set_enable2;      // 79: Command set enable
    uint16_t command_set_enable_ext;   // 80: Command set enable extension
    uint16_t udma_modes;               // 81: UDMA modes
    uint16_t erase_time;               // 82: Time for security erase
    uint16_t enhanced_erase_time;      // 83: Time for enhanced security erase
    uint16_t current_apm_level;        // 84: Current APM level
    uint16_t master_password_rev;      // 85: Master password revision
    uint16_t hw_reset_result;          // 86: Hardware reset result
    uint16_t acoustic_value;           // 87: Acoustic management value
    uint16_t stream_min_req_size;      // 88: Stream minimum request size
    uint16_t stream_transfer_time_dma; // 89: Stream transfer time DMA
    uint16_t stream_access_latency;    // 90: Stream access latency
    uint32_t stream_perf_granularity;  // 91-92: Stream performance granularity
    uint64_t lba48_capacity;           // 93-96: 48-bit LBA capacity
    uint16_t stream_transfer_time_pio; // 97: Stream transfer time PIO
    uint16_t max_lba_range_entries;    // 98: Max LBA range descriptor entries
    uint16_t phys_logical_sector_size; // 99: Physical/logical sector size
    uint16_t inter_seek_delay;         // 100: Inter-seek delay for acoustic
    uint16_t world_wide_name[4];       // 101-104: World wide name
    uint16_t reserved_wwn[4];          // 105-108: Reserved for WWN extension
    uint16_t logical_sector_size[2];   // 109-110: Logical sector size
    uint16_t commands_and_feature_sets; // 111: Commands and feature sets
    uint16_t commands_and_feature_sets2; // 112: Commands and feature sets
    uint16_t reserved2[13];            // 113-125: Reserved
    uint16_t atapi_byte_count_0;       // 126: ATAPI byte count 0
    uint16_t removable_status;         // 127: Removable media status
    uint16_t security_status;          // 128: Security status
    uint16_t vendor_specific[31];      // 129-159: Vendor specific
    uint16_t cfa_power_mode1;          // 160: CFA power mode 1
    uint16_t reserved_cfa[7];          // 161-167: Reserved for CFA
    uint16_t device_nominal_form_factor; // 168: Device nominal form factor
    uint16_t data_set_management;      // 169: Data set management
    uint16_t additional_product_id[4]; // 170-173: Additional product identifier
    uint16_t reserved3[2];             // 174-175: Reserved
    char current_media_serial[60];     // 176-205: Current media serial number
    uint16_t sct_command_transport;    // 206: SCT command transport
    uint16_t reserved4[2];             // 207-208: Reserved
    uint16_t logical_alignment;        // 209: Logical sector alignment
    uint32_t write_read_verify_count;  // 210-211: Write-read-verify sector count
    uint32_t verify_sector_count;      // 212-213: Verify sector count
    uint16_t nv_cache_capabilities;    // 214: NV cache capabilities
    uint16_t nv_cache_size[2];         // 215-216: NV cache size
    uint16_t nominal_media_rotation;   // 217: Nominal media rotation rate
    uint16_t reserved5;                // 218: Reserved
    uint16_t nv_cache_options;         // 219: NV cache options
    uint16_t write_read_verify_mode;   // 220: Write-read-verify feature set
    uint16_t reserved6;                // 221: Reserved
    uint16_t transport_major_version;  // 222: Transport major version
    uint16_t transport_minor_version;  // 223: Transport minor version
    uint16_t reserved7[31];            // 224-254: Reserved
    uint16_t integrity_word;           // 255: Integrity word
} ata_identify_t;

// SATA device structure
typedef struct sata_device {
    device_t base;                     // Base device structure
    
    // Device identification
    uint8_t port;                      // SATA port number
    uint8_t device_id;                 // Device ID on port
    sata_device_type_t type;           // Device type
    sata_device_state_t state;         // Device state
    sata_speed_t speed;                // Interface speed
    
    // Device information
    char model[41];                    // Model number
    char serial[21];                   // Serial number
    char firmware[9];                  // Firmware revision
    uint64_t capacity;                 // Capacity in sectors
    uint32_t sector_size;              // Sector size in bytes
    bool lba48_supported;              // 48-bit LBA support
    bool dma_supported;                // DMA support
    bool ncq_supported;                // NCQ support
    uint8_t max_queue_depth;           // Maximum queue depth
    
    // Security features
    bool security_supported;           // Security feature set support
    bool security_enabled;             // Security enabled
    bool security_locked;              // Security locked
    bool security_frozen;              // Security frozen
    
    // Power management
    bool apm_supported;                // APM support
    uint8_t apm_level;                 // Current APM level
    bool standby_timer_supported;      // Standby timer support
    
    // SMART features
    bool smart_supported;              // SMART support
    bool smart_enabled;                // SMART enabled
    
    // Performance statistics
    uint64_t read_count;               // Read operation count
    uint64_t write_count;              // Write operation count
    uint64_t bytes_read;               // Bytes read
    uint64_t bytes_written;            // Bytes written
    uint64_t error_count;              // Error count
    
    // Controller reference
    struct sata_controller* controller; // Parent controller
    
    // List management
    struct sata_device* next;
} sata_device_t;

// SATA controller structure
typedef struct sata_controller {
    device_t base;                     // Base device structure
    
    // Hardware information
    uint32_t base_address;             // Base I/O address
    uint32_t memory_base;              // Memory-mapped base address
    uint8_t irq;                       // IRQ number
    uint16_t vendor_id;                // PCI vendor ID
    uint16_t device_id;                // PCI device ID
    
    // Controller capabilities
    uint8_t num_ports;                 // Number of ports
    uint8_t num_command_slots;         // Number of command slots
    bool ncq_supported;                // NCQ support
    bool 64bit_addressing;             // 64-bit addressing support
    bool port_multiplier_support;      // Port multiplier support
    bool ahci_only;                    // AHCI only mode
    uint32_t interface_speed;          // Interface speed support
    
    // Controller state
    bool initialized;                  // Controller initialized
    bool enabled;                      // Controller enabled
    uint32_t ports_implemented;        // Implemented ports bitmask
    
    // Memory regions
    void* command_list_base;           // Command list base
    void* received_fis_base;           // Received FIS base
    void* command_table_base;          // Command table base
    
    // Device management
    sata_device_t* devices[SATA_MAX_PORTS]; // Connected devices
    uint32_t num_devices;              // Number of devices
    
    // Performance statistics
    uint64_t total_commands;           // Total commands processed
    uint64_t successful_commands;      // Successful commands
    uint64_t failed_commands;          // Failed commands
    uint64_t bytes_transferred;        // Total bytes transferred
    
    // List management
    struct sata_controller* next;
} sata_controller_t;

// SATA subsystem structure
typedef struct {
    bool initialized;
    sata_controller_t* controller_list; // Controller list
    uint32_t num_controllers;          // Number of controllers
    sata_device_t* device_list;        // All devices
    uint32_t num_devices;              // Total number of devices
    
    // Configuration
    uint32_t max_transfer_size;        // Maximum transfer size
    uint32_t command_timeout;          // Command timeout
    bool ncq_enabled;                  // NCQ globally enabled
    
    // Thread synchronization
    mutex_t subsystem_mutex;
    mutex_t device_mutex;
} sata_subsystem_t;

// Function prototypes
error_t sata_init(void);
void sata_shutdown(void);

// Controller management
error_t sata_register_controller(sata_controller_t* controller);
error_t sata_unregister_controller(sata_controller_t* controller);
sata_controller_t* sata_find_controller(uint32_t base_address);
error_t sata_detect_controllers(void);

// Device management
error_t sata_register_device(sata_device_t* device);
error_t sata_unregister_device(sata_device_t* device);
sata_device_t* sata_find_device(uint8_t controller_id, uint8_t port);
sata_device_t* sata_get_device_list(void);

// Device operations
error_t sata_identify_device(sata_device_t* device);
error_t sata_read_sectors(sata_device_t* device, uint64_t lba, uint32_t count, void* buffer);
error_t sata_write_sectors(sata_device_t* device, uint64_t lba, uint32_t count, const void* buffer);
error_t sata_flush_cache(sata_device_t* device);
error_t sata_trim_sectors(sata_device_t* device, uint64_t lba, uint32_t count);

// Power management
error_t sata_standby_device(sata_device_t* device);
error_t sata_idle_device(sata_device_t* device);
error_t sata_sleep_device(sata_device_t* device);
error_t sata_check_power_mode(sata_device_t* device, uint8_t* mode);

// Security features
error_t sata_security_set_password(sata_device_t* device, const char* password, bool master);
error_t sata_security_unlock(sata_device_t* device, const char* password, bool master);
error_t sata_security_erase(sata_device_t* device, const char* password, bool master, bool enhanced);
error_t sata_security_freeze(sata_device_t* device);
error_t sata_security_disable(sata_device_t* device, const char* password, bool master);

// SMART features
error_t sata_smart_enable(sata_device_t* device);
error_t sata_smart_disable(sata_device_t* device);
error_t sata_smart_read_data(sata_device_t* device, void* buffer);
error_t sata_smart_read_threshold(sata_device_t* device, void* buffer);
error_t sata_smart_execute_offline_test(sata_device_t* device, uint8_t test_type);

// Utility functions
const char* sata_device_type_to_string(sata_device_type_t type);
const char* sata_device_state_to_string(sata_device_state_t state);
const char* sata_speed_to_string(sata_speed_t speed);
uint32_t sata_speed_to_mbps(sata_speed_t speed);

// Statistics and monitoring
error_t sata_get_device_statistics(sata_device_t* device, void* stats);
error_t sata_get_controller_statistics(sata_controller_t* controller, void* stats);

#endif // SATA_H