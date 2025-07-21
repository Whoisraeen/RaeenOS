#ifndef NVME_H
#define NVME_H

#include "../core/types.h"
#include "../core/error.h"
#include "driver_framework.h"

// NVMe constants
#define NVME_MAX_CONTROLLERS 16
#define NVME_MAX_NAMESPACES 1024
#define NVME_MAX_QUEUES 65536
#define NVME_MAX_QUEUE_ENTRIES 65536
#define NVME_SECTOR_SIZE 512
#define NVME_PAGE_SIZE 4096

// NVMe Controller Registers
#define NVME_REG_CAP     0x00  // Controller Capabilities
#define NVME_REG_VS      0x08  // Version
#define NVME_REG_INTMS   0x0C  // Interrupt Mask Set
#define NVME_REG_INTMC   0x10  // Interrupt Mask Clear
#define NVME_REG_CC      0x14  // Controller Configuration
#define NVME_REG_CSTS    0x1C  // Controller Status
#define NVME_REG_NSSR    0x20  // NVM Subsystem Reset
#define NVME_REG_AQA     0x24  // Admin Queue Attributes
#define NVME_REG_ASQ     0x28  // Admin Submission Queue Base Address
#define NVME_REG_ACQ     0x30  // Admin Completion Queue Base Address
#define NVME_REG_CMBLOC  0x38  // Controller Memory Buffer Location
#define NVME_REG_CMBSZ   0x3C  // Controller Memory Buffer Size

// NVMe Controller Capabilities (CAP)
#define NVME_CAP_MQES(cap)    ((cap) & 0xFFFF)
#define NVME_CAP_CQR(cap)     (((cap) >> 16) & 0x1)
#define NVME_CAP_AMS(cap)     (((cap) >> 17) & 0x3)
#define NVME_CAP_TO(cap)      (((cap) >> 24) & 0xFF)
#define NVME_CAP_DSTRD(cap)   (((cap) >> 32) & 0xF)
#define NVME_CAP_NSSRS(cap)   (((cap) >> 36) & 0x1)
#define NVME_CAP_CSS(cap)     (((cap) >> 37) & 0xFF)
#define NVME_CAP_BPS(cap)     (((cap) >> 45) & 0x1)
#define NVME_CAP_MPSMIN(cap)  (((cap) >> 48) & 0xF)
#define NVME_CAP_MPSMAX(cap)  (((cap) >> 52) & 0xF)

// NVMe Controller Configuration (CC)
#define NVME_CC_EN        (1 << 0)
#define NVME_CC_CSS_NVM   (0 << 4)
#define NVME_CC_MPS(x)    ((x) << 7)
#define NVME_CC_AMS_RR    (0 << 11)
#define NVME_CC_SHN_NONE  (0 << 14)
#define NVME_CC_SHN_NORMAL (1 << 14)
#define NVME_CC_SHN_ABRUPT (2 << 14)
#define NVME_CC_IOSQES(x) ((x) << 16)
#define NVME_CC_IOCQES(x) ((x) << 20)

// NVMe Controller Status (CSTS)
#define NVME_CSTS_RDY     (1 << 0)
#define NVME_CSTS_CFS     (1 << 1)
#define NVME_CSTS_SHST_NORMAL (0 << 2)
#define NVME_CSTS_SHST_OCCUR  (1 << 2)
#define NVME_CSTS_SHST_CMPLT  (2 << 2)
#define NVME_CSTS_NSSRO   (1 << 4)

// NVMe Admin Commands
#define NVME_ADMIN_DELETE_SQ    0x00
#define NVME_ADMIN_CREATE_SQ    0x01
#define NVME_ADMIN_GET_LOG_PAGE 0x02
#define NVME_ADMIN_DELETE_CQ    0x04
#define NVME_ADMIN_CREATE_CQ    0x05
#define NVME_ADMIN_IDENTIFY     0x06
#define NVME_ADMIN_ABORT        0x08
#define NVME_ADMIN_SET_FEATURES 0x09
#define NVME_ADMIN_GET_FEATURES 0x0A
#define NVME_ADMIN_ASYNC_EVENT  0x0C
#define NVME_ADMIN_NS_MGMT      0x0D
#define NVME_ADMIN_ACTIVATE_FW  0x10
#define NVME_ADMIN_DOWNLOAD_FW  0x11
#define NVME_ADMIN_DEV_SELF_TEST 0x14
#define NVME_ADMIN_NS_ATTACH    0x15
#define NVME_ADMIN_KEEP_ALIVE   0x18
#define NVME_ADMIN_DIRECTIVE_SEND 0x19
#define NVME_ADMIN_DIRECTIVE_RECV 0x1A
#define NVME_ADMIN_VIRT_MGMT    0x1C
#define NVME_ADMIN_NVME_MI_SEND 0x1D
#define NVME_ADMIN_NVME_MI_RECV 0x1E
#define NVME_ADMIN_DBBUF        0x7C
#define NVME_ADMIN_FORMAT_NVM   0x80
#define NVME_ADMIN_SECURITY_SEND 0x81
#define NVME_ADMIN_SECURITY_RECV 0x82
#define NVME_ADMIN_SANITIZE     0x84

// NVMe I/O Commands
#define NVME_CMD_FLUSH          0x00
#define NVME_CMD_WRITE          0x01
#define NVME_CMD_READ           0x02
#define NVME_CMD_WRITE_UNCOR    0x04
#define NVME_CMD_COMPARE        0x05
#define NVME_CMD_WRITE_ZEROES   0x08
#define NVME_CMD_DSM            0x09
#define NVME_CMD_VERIFY         0x0C
#define NVME_CMD_RESV_REGISTER  0x0D
#define NVME_CMD_RESV_REPORT    0x0E
#define NVME_CMD_RESV_ACQUIRE   0x11
#define NVME_CMD_RESV_RELEASE   0x15

// NVMe Status Codes
#define NVME_SC_SUCCESS         0x00
#define NVME_SC_INVALID_OPCODE  0x01
#define NVME_SC_INVALID_FIELD   0x02
#define NVME_SC_CMDID_CONFLICT  0x03
#define NVME_SC_DATA_XFER_ERROR 0x04
#define NVME_SC_POWER_LOSS      0x05
#define NVME_SC_INTERNAL        0x06
#define NVME_SC_ABORT_REQ       0x07
#define NVME_SC_ABORT_QUEUE     0x08
#define NVME_SC_FUSED_FAIL      0x09
#define NVME_SC_FUSED_MISSING   0x0A
#define NVME_SC_INVALID_NS      0x0B
#define NVME_SC_CMD_SEQ_ERROR   0x0C
#define NVME_SC_SGL_INVALID_LAST 0x0D
#define NVME_SC_SGL_INVALID_COUNT 0x0E
#define NVME_SC_SGL_INVALID_DATA 0x0F
#define NVME_SC_SGL_INVALID_METADATA 0x10
#define NVME_SC_SGL_INVALID_TYPE 0x11

// NVMe Command structure
typedef struct __attribute__((packed)) {
    uint8_t opcode;        // Command opcode
    uint8_t flags;         // Command flags
    uint16_t command_id;   // Command identifier
    uint32_t nsid;         // Namespace identifier
    uint64_t rsvd2;        // Reserved
    uint64_t metadata;     // Metadata pointer
    uint64_t prp1;         // PRP entry 1
    uint64_t prp2;         // PRP entry 2
    uint32_t cdw10;        // Command-specific
    uint32_t cdw11;        // Command-specific
    uint32_t cdw12;        // Command-specific
    uint32_t cdw13;        // Command-specific
    uint32_t cdw14;        // Command-specific
    uint32_t cdw15;        // Command-specific
} nvme_command_t;

// NVMe Completion structure
typedef struct __attribute__((packed)) {
    uint32_t result;       // Command-specific result
    uint32_t rsvd;         // Reserved
    uint16_t sq_head;      // Submission queue head pointer
    uint16_t sq_id;        // Submission queue identifier
    uint16_t command_id;   // Command identifier
    uint16_t status;       // Status field
} nvme_completion_t;

// NVMe Identify Controller structure
typedef struct __attribute__((packed)) {
    uint16_t vid;          // PCI Vendor ID
    uint16_t ssvid;        // PCI Subsystem Vendor ID
    char sn[20];           // Serial Number
    char mn[40];           // Model Number
    char fr[8];            // Firmware Revision
    uint8_t rab;           // Recommended Arbitration Burst
    uint8_t ieee[3];       // IEEE OUI Identifier
    uint8_t cmic;          // Controller Multi-Path I/O and Namespace Sharing Capabilities
    uint8_t mdts;          // Maximum Data Transfer Size
    uint16_t cntlid;       // Controller ID
    uint32_t ver;          // Version
    uint32_t rtd3r;        // RTD3 Resume Latency
    uint32_t rtd3e;        // RTD3 Entry Latency
    uint32_t oaes;         // Optional Asynchronous Events Supported
    uint32_t ctratt;       // Controller Attributes
    uint16_t rrls;         // Read Recovery Levels Supported
    uint8_t rsvd102[9];    // Reserved
    uint8_t cntrltype;     // Controller Type
    uint8_t fguid[16];     // FRU Globally Unique Identifier
    uint16_t crdt1;        // Command Retry Delay Time 1
    uint16_t crdt2;        // Command Retry Delay Time 2
    uint16_t crdt3;        // Command Retry Delay Time 3
    uint8_t rsvd134[122];  // Reserved
    uint16_t oacs;         // Optional Admin Command Support
    uint8_t acl;           // Abort Command Limit
    uint8_t aerl;          // Asynchronous Event Request Limit
    uint8_t frmw;          // Firmware Updates
    uint8_t lpa;           // Log Page Attributes
    uint8_t elpe;          // Error Log Page Entries
    uint8_t npss;          // Number of Power States Support
    uint8_t avscc;         // Admin Vendor Specific Command Configuration
    uint8_t apsta;         // Autonomous Power State Transition Attributes
    uint16_t wctemp;       // Warning Composite Temperature Threshold
    uint16_t cctemp;       // Critical Composite Temperature Threshold
    uint16_t mtfa;         // Maximum Time for Firmware Activation
    uint32_t hmpre;        // Host Memory Buffer Preferred Size
    uint32_t hmmin;        // Host Memory Buffer Minimum Size
    uint8_t tnvmcap[16];   // Total NVM Capacity
    uint8_t unvmcap[16];   // Unallocated NVM Capacity
    uint32_t rpmbs;        // Replay Protected Memory Block Support
    uint16_t edstt;        // Extended Device Self-test Time
    uint8_t dsto;          // Device Self-test Options
    uint8_t fwug;          // Firmware Update Granularity
    uint16_t kas;          // Keep Alive Support
    uint16_t hctma;        // Host Controlled Thermal Management Attributes
    uint16_t mntmt;        // Minimum Thermal Management Temperature
    uint16_t mxtmt;        // Maximum Thermal Management Temperature
    uint32_t sanicap;      // Sanitize Capabilities
    uint32_t hmminds;      // Host Memory Buffer Minimum Descriptor Entry Size
    uint16_t hmmaxd;       // Host Memory Buffer Maximum Descriptors Entries
    uint16_t nsetidmax;    // NVM Set Identifier Maximum
    uint16_t endgidmax;    // Endurance Group Identifier Maximum
    uint8_t anatt;         // ANA Transition Time
    uint8_t anacap;        // Asymmetric Namespace Access Capabilities
    uint32_t anagrpmax;    // ANA Group Identifier Maximum
    uint32_t nanagrpid;    // Number of ANA Group Identifiers
    uint32_t pels;         // Persistent Event Log Size
    uint8_t rsvd356[156];  // Reserved
    uint8_t sqes;          // Submission Queue Entry Size
    uint8_t cqes;          // Completion Queue Entry Size
    uint16_t maxcmd;       // Maximum Outstanding Commands
    uint32_t nn;           // Number of Namespaces
    uint16_t oncs;         // Optional NVM Command Support
    uint16_t fuses;        // Fused Operation Support
    uint8_t fna;           // Format NVM Attributes
    uint8_t vwc;           // Volatile Write Cache
    uint16_t awun;         // Atomic Write Unit Normal
    uint16_t awupf;        // Atomic Write Unit Power Fail
    uint8_t nvscc;         // NVM Vendor Specific Command Configuration
    uint8_t nwpc;          // Namespace Write Protection Capabilities
    uint16_t acwu;         // Atomic Compare & Write Unit
    uint16_t rsvd534;      // Reserved
    uint32_t sgls;         // SGL Support
    uint32_t mnan;         // Maximum Number of Allowed Namespaces
    uint8_t rsvd544[224];  // Reserved
    char subnqn[256];      // NVM Subsystem NVMe Qualified Name
    uint8_t rsvd1024[768]; // Reserved
    uint8_t nvmeof[256];   // NVMe over Fabrics
    uint8_t psds[1024];    // Power State Descriptors
    uint8_t vs[1024];      // Vendor Specific
} nvme_id_ctrl_t;

// NVMe Identify Namespace structure
typedef struct __attribute__((packed)) {
    uint64_t nsze;         // Namespace Size
    uint64_t ncap;         // Namespace Capacity
    uint64_t nuse;         // Namespace Utilization
    uint8_t nsfeat;        // Namespace Features
    uint8_t nlbaf;         // Number of LBA Formats
    uint8_t flbas;         // Formatted LBA Size
    uint8_t mc;            // Metadata Capabilities
    uint8_t dpc;           // End-to-end Data Protection Capabilities
    uint8_t dps;           // End-to-end Data Protection Type Settings
    uint8_t nmic;          // Namespace Multi-path I/O and Namespace Sharing Capabilities
    uint8_t rescap;        // Reservation Capabilities
    uint8_t fpi;           // Format Progress Indicator
    uint8_t dlfeat;        // Deallocate Logical Block Features
    uint16_t nawun;        // Namespace Atomic Write Unit Normal
    uint16_t nawupf;       // Namespace Atomic Write Unit Power Fail
    uint16_t nacwu;        // Namespace Atomic Compare & Write Unit
    uint16_t nabsn;        // Namespace Atomic Boundary Size Normal
    uint16_t nabo;         // Namespace Atomic Boundary Offset
    uint16_t nabspf;       // Namespace Atomic Boundary Size Power Fail
    uint16_t noiob;        // Namespace Optimal IO Boundary
    uint8_t nvmcap[16];    // NVM Capacity
    uint16_t npwg;         // Namespace Preferred Write Granularity
    uint16_t npwa;         // Namespace Preferred Write Alignment
    uint16_t npdg;         // Namespace Preferred Deallocate Granularity
    uint16_t npda;         // Namespace Preferred Deallocate Alignment
    uint16_t nows;         // Namespace Optimal Write Size
    uint8_t rsvd74[18];    // Reserved
    uint32_t anagrpid;     // ANA Group Identifier
    uint8_t rsvd96[3];     // Reserved
    uint8_t nsattr;        // Namespace Attributes
    uint16_t nvmsetid;     // NVM Set Identifier
    uint16_t endgid;       // Endurance Group Identifier
    uint8_t nguid[16];     // Namespace Globally Unique Identifier
    uint8_t eui64[8];      // IEEE Extended Unique Identifier
    struct {
        uint16_t ms;       // Metadata Size
        uint8_t lbads;     // LBA Data Size
        uint8_t rp;        // Relative Performance
    } lbaf[16];            // LBA Format Support
    uint8_t rsvd192[192];  // Reserved
    uint8_t vs[3712];      // Vendor Specific
} nvme_id_ns_t;

// NVMe Queue structure
typedef struct {
    void* queue_memory;        // Queue memory
    uint32_t queue_size;       // Queue size in entries
    uint32_t entry_size;       // Entry size in bytes
    uint16_t queue_id;         // Queue ID
    uint16_t head;             // Head pointer
    uint16_t tail;             // Tail pointer
    volatile uint32_t* doorbell; // Doorbell register
    bool is_completion_queue;  // Is completion queue
    uint8_t phase_bit;         // Phase bit for completion queues
    mutex_t queue_mutex;       // Queue access mutex
} nvme_queue_t;

// NVMe Namespace structure
typedef struct nvme_namespace {
    uint32_t nsid;             // Namespace ID
    uint64_t size;             // Size in logical blocks
    uint64_t capacity;         // Capacity in logical blocks
    uint64_t utilization;      // Utilization in logical blocks
    uint32_t block_size;       // Logical block size
    uint32_t metadata_size;    // Metadata size
    uint8_t lba_format;        // Current LBA format
    bool formatted;            // Namespace is formatted
    bool active;               // Namespace is active
    
    // Features
    bool thin_provisioning;    // Thin provisioning support
    bool deallocate_support;   // Deallocate support
    bool write_zeroes_support; // Write zeroes support
    bool atomic_write_support; // Atomic write support
    
    // Performance characteristics
    uint16_t optimal_io_boundary; // Optimal I/O boundary
    uint16_t preferred_write_granularity; // Preferred write granularity
    uint16_t preferred_write_alignment;   // Preferred write alignment
    
    // Statistics
    uint64_t read_count;       // Read operation count
    uint64_t write_count;      // Write operation count
    uint64_t bytes_read;       // Bytes read
    uint64_t bytes_written;    // Bytes written
    uint64_t error_count;      // Error count
    
    // Controller reference
    struct nvme_controller* controller; // Parent controller
    
    // List management
    struct nvme_namespace* next;
} nvme_namespace_t;

// NVMe Controller structure
typedef struct nvme_controller {
    device_t base;             // Base device structure
    
    // Hardware information
    uint32_t base_address;     // Base I/O address
    uint64_t memory_base;      // Memory-mapped base address
    uint8_t irq;               // IRQ number
    uint16_t vendor_id;        // PCI vendor ID
    uint16_t device_id;        // PCI device ID
    
    // Controller identification
    char serial_number[21];    // Serial number
    char model_number[41];     // Model number
    char firmware_revision[9]; // Firmware revision
    uint32_t controller_id;    // Controller ID
    uint32_t version;          // NVMe version
    
    // Controller capabilities
    uint16_t max_queue_entries; // Maximum queue entries
    bool contiguous_queues_required; // Contiguous queues required
    uint8_t arbitration_mechanism; // Arbitration mechanism
    uint8_t timeout;           // Timeout value
    uint8_t doorbell_stride;   // Doorbell stride
    bool nvm_subsystem_reset_supported; // NVM subsystem reset support
    uint8_t command_sets_supported; // Command sets supported
    bool boot_partition_support; // Boot partition support
    uint8_t memory_page_size_min; // Minimum memory page size
    uint8_t memory_page_size_max; // Maximum memory page size
    
    // Controller state
    bool initialized;          // Controller initialized
    bool enabled;              // Controller enabled
    uint32_t page_size;        // Memory page size
    uint32_t max_transfer_size; // Maximum transfer size
    
    // Queues
    nvme_queue_t admin_sq;     // Admin submission queue
    nvme_queue_t admin_cq;     // Admin completion queue
    nvme_queue_t* io_sq;       // I/O submission queues
    nvme_queue_t* io_cq;       // I/O completion queues
    uint16_t num_io_queues;    // Number of I/O queues
    uint16_t max_io_queues;    // Maximum I/O queues
    
    // Namespaces
    nvme_namespace_t* namespaces[NVME_MAX_NAMESPACES]; // Namespaces
    uint32_t num_namespaces;   // Number of namespaces
    uint32_t max_namespaces;   // Maximum namespaces
    
    // Command tracking
    uint16_t next_command_id;  // Next command ID
    mutex_t command_mutex;     // Command mutex
    
    // Performance statistics
    uint64_t total_commands;   // Total commands processed
    uint64_t successful_commands; // Successful commands
    uint64_t failed_commands;  // Failed commands
    uint64_t bytes_transferred; // Total bytes transferred
    
    // List management
    struct nvme_controller* next;
} nvme_controller_t;

// NVMe subsystem structure
typedef struct {
    bool initialized;
    nvme_controller_t* controller_list; // Controller list
    uint32_t num_controllers;  // Number of controllers
    nvme_namespace_t* namespace_list; // All namespaces
    uint32_t num_namespaces;   // Total number of namespaces
    
    // Configuration
    uint32_t max_transfer_size; // Maximum transfer size
    uint32_t command_timeout;  // Command timeout
    uint16_t max_queues_per_controller; // Max queues per controller
    
    // Thread synchronization
    mutex_t subsystem_mutex;
    mutex_t namespace_mutex;
} nvme_subsystem_t;

// Function prototypes
error_t nvme_init(void);
void nvme_shutdown(void);

// Controller management
error_t nvme_register_controller(nvme_controller_t* controller);
error_t nvme_unregister_controller(nvme_controller_t* controller);
nvme_controller_t* nvme_find_controller(uint64_t base_address);
error_t nvme_detect_controllers(void);

// Namespace management
error_t nvme_register_namespace(nvme_namespace_t* namespace);
error_t nvme_unregister_namespace(nvme_namespace_t* namespace);
nvme_namespace_t* nvme_find_namespace(uint32_t controller_id, uint32_t nsid);
nvme_namespace_t* nvme_get_namespace_list(void);

// Device operations
error_t nvme_identify_controller(nvme_controller_t* controller);
error_t nvme_identify_namespace(nvme_controller_t* controller, uint32_t nsid);
error_t nvme_read_blocks(nvme_namespace_t* namespace, uint64_t lba, uint32_t count, void* buffer);
error_t nvme_write_blocks(nvme_namespace_t* namespace, uint64_t lba, uint32_t count, const void* buffer);
error_t nvme_flush(nvme_namespace_t* namespace);
error_t nvme_trim(nvme_namespace_t* namespace, uint64_t lba, uint32_t count);
error_t nvme_write_zeroes(nvme_namespace_t* namespace, uint64_t lba, uint32_t count);

// Queue management
error_t nvme_create_io_queue_pair(nvme_controller_t* controller, uint16_t queue_id, uint16_t queue_size);
error_t nvme_delete_io_queue_pair(nvme_controller_t* controller, uint16_t queue_id);

// Command operations
error_t nvme_submit_admin_command(nvme_controller_t* controller, nvme_command_t* cmd, void* buffer, size_t buffer_size);
error_t nvme_submit_io_command(nvme_controller_t* controller, uint16_t queue_id, nvme_command_t* cmd, void* buffer, size_t buffer_size);

// Feature management
error_t nvme_set_features(nvme_controller_t* controller, uint8_t feature_id, uint32_t value);
error_t nvme_get_features(nvme_controller_t* controller, uint8_t feature_id, uint32_t* value);

// Power management
error_t nvme_set_power_state(nvme_controller_t* controller, uint8_t power_state);
error_t nvme_get_power_state(nvme_controller_t* controller, uint8_t* power_state);

// Security features
error_t nvme_security_send(nvme_controller_t* controller, uint8_t protocol, uint16_t com_id, void* buffer, size_t buffer_size);
error_t nvme_security_receive(nvme_controller_t* controller, uint8_t protocol, uint16_t com_id, void* buffer, size_t buffer_size);

// Format operations
error_t nvme_format_namespace(nvme_namespace_t* namespace, uint8_t lba_format, bool secure_erase);
error_t nvme_sanitize_namespace(nvme_namespace_t* namespace, uint8_t sanitize_action);

// Utility functions
const char* nvme_status_to_string(uint16_t status);
uint32_t nvme_lba_size_from_format(uint8_t lbads);
bool nvme_is_controller_ready(nvme_controller_t* controller);

// Statistics and monitoring
error_t nvme_get_namespace_statistics(nvme_namespace_t* namespace, void* stats);
error_t nvme_get_controller_statistics(nvme_controller_t* controller, void* stats);
error_t nvme_get_smart_log(nvme_controller_t* controller, uint32_t nsid, void* log);

// Advanced features
error_t nvme_enable_host_memory_buffer(nvme_controller_t* controller, void* buffer, size_t size);
error_t nvme_disable_host_memory_buffer(nvme_controller_t* controller);
error_t nvme_set_interrupt_coalescing(nvme_controller_t* controller, uint8_t threshold, uint8_t time);

#endif // NVME_H