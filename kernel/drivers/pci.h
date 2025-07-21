#ifndef RAEENOS_PCI_H
#define RAEENOS_PCI_H

#include "types.h"

// PCI constants
#define MAX_PCI_DEVICES 256
#define MAX_PCI_BUSES 256
#define MAX_DEVICES_PER_BUS 32
#define MAX_FUNCTIONS_PER_DEVICE 8

// PCI configuration space size
#define PCI_CONFIG_SPACE_SIZE 256
#define PCI_CONFIG_SPACE_DWORDS 64

// PCI header types
#define PCI_HEADER_TYPE_DEVICE 0x00
#define PCI_HEADER_TYPE_BRIDGE 0x01
#define PCI_HEADER_TYPE_CARDBUS 0x02

// PCI class codes
#define PCI_CLASS_UNCLASSIFIED 0x00
#define PCI_CLASS_MASS_STORAGE 0x01
#define PCI_CLASS_NETWORK 0x02
#define PCI_CLASS_DISPLAY 0x03
#define PCI_CLASS_MULTIMEDIA 0x04
#define PCI_CLASS_MEMORY 0x05
#define PCI_CLASS_BRIDGE 0x06
#define PCI_CLASS_SIMPLE_COMM 0x07
#define PCI_CLASS_BASE_PERIPHERAL 0x08
#define PCI_CLASS_INPUT_DEVICE 0x09
#define PCI_CLASS_DOCKING_STATION 0x0A
#define PCI_CLASS_PROCESSOR 0x0B
#define PCI_CLASS_SERIAL_BUS 0x0C
#define PCI_CLASS_WIRELESS 0x0D
#define PCI_CLASS_INTELLIGENT_IO 0x0E
#define PCI_CLASS_SATELLITE_COMM 0x0F
#define PCI_CLASS_ENCRYPTION 0x10
#define PCI_CLASS_SIGNAL_PROCESSING 0x11
#define PCI_CLASS_PROCESSING_ACCELERATOR 0x12
#define PCI_CLASS_NON_ESSENTIAL_INSTRUMENTATION 0x13
#define PCI_CLASS_COPROCESSOR 0x40
#define PCI_CLASS_UNASSIGNED 0xFF

// PCI bridge subclasses
#define PCI_SUBCLASS_BRIDGE_HOST 0x00
#define PCI_SUBCLASS_BRIDGE_ISA 0x01
#define PCI_SUBCLASS_BRIDGE_EISA 0x02
#define PCI_SUBCLASS_BRIDGE_MCA 0x03
#define PCI_SUBCLASS_BRIDGE_VGA 0x04
#define PCI_SUBCLASS_BRIDGE_PCI 0x04
#define PCI_SUBCLASS_BRIDGE_PCMCIA 0x05
#define PCI_SUBCLASS_BRIDGE_NUBUS 0x06
#define PCI_SUBCLASS_BRIDGE_CARDBUS 0x07
#define PCI_SUBCLASS_BRIDGE_RACEWAY 0x08
#define PCI_SUBCLASS_BRIDGE_ISA_POSITIVE_DECODE 0x80

// PCI serial bus subclasses
#define PCI_SUBCLASS_SERIAL_FIREWIRE 0x00
#define PCI_SUBCLASS_SERIAL_ACCESS 0x01
#define PCI_SUBCLASS_SERIAL_SSA 0x02
#define PCI_SUBCLASS_SERIAL_USB 0x03
#define PCI_SUBCLASS_SERIAL_FIBRE_CHANNEL 0x04
#define PCI_SUBCLASS_SERIAL_SMBUS 0x05
#define PCI_SUBCLASS_SERIAL_INFINIBAND 0x06
#define PCI_SUBCLASS_SERIAL_IPMI 0x07
#define PCI_SUBCLASS_SERIAL_SERCOS 0x08
#define PCI_SUBCLASS_SERIAL_CANBUS 0x09

// PCI device structure
typedef struct {
    int bus;
    int device;
    int function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t programming_interface;
    uint8_t header_type;
    uint32_t bars[6];           // Base Address Registers
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint32_t config_space[64];  // Configuration space
    void* driver_data;          // Driver-specific data
} pci_device_t;

// PCI bus structure
typedef struct {
    int bus_number;
    pci_device_t* devices[MAX_DEVICES_PER_BUS];
    int device_count;
} pci_bus_t;

// PCI driver functions
error_t pci_init(void);
void pci_shutdown(void);
error_t pci_scan_buses(void);
error_t pci_scan_bus(int bus);
pci_device_t* pci_device_create(int bus, int device, int function, uint16_t vendor_id, uint16_t device_id);
error_t pci_device_init(pci_device_t* device);
void pci_device_shutdown(pci_device_t* device);

// Configuration space access
error_t pci_read_config_dword(int bus, int device, int function, int offset, uint32_t* value);
error_t pci_write_config_dword(int bus, int device, int function, int offset, uint32_t value);
error_t pci_read_config_word(int bus, int device, int function, int offset, uint16_t* value);
error_t pci_write_config_word(int bus, int device, int function, int offset, uint16_t value);
error_t pci_read_config_byte(int bus, int device, int function, int offset, uint8_t* value);
error_t pci_write_config_byte(int bus, int device, int function, int offset, uint8_t value);

// Device-specific initialization
error_t pci_storage_init(pci_device_t* device);
void pci_storage_shutdown(pci_device_t* device);
error_t pci_network_init(pci_device_t* device);
void pci_network_shutdown(pci_device_t* device);
error_t pci_display_init(pci_device_t* device);
void pci_display_shutdown(pci_device_t* device);
error_t pci_serial_init(pci_device_t* device);
void pci_serial_shutdown(pci_device_t* device);
error_t pci_bridge_init(pci_device_t* device);
void pci_bridge_shutdown(pci_device_t* device);

// Device management
error_t pci_add_device(pci_device_t* device);
error_t pci_remove_device(uint16_t vendor_id, uint16_t device_id);
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id);
error_t pci_get_devices(pci_device_t** device_list, int* count);

// BAR (Base Address Register) functions
uintptr_t pci_get_bar_address(pci_device_t* device, int bar_index);
size_t pci_get_bar_size(pci_device_t* device, int bar_index);

// Utility functions
bool pci_is_initialized(void);
int pci_get_device_count(void);
int pci_get_bus_count(void);

// Common vendor IDs
#define PCI_VENDOR_INTEL 0x8086
#define PCI_VENDOR_AMD 0x1022
#define PCI_VENDOR_NVIDIA 0x10DE
#define PCI_VENDOR_REALTEK 0x10EC
#define PCI_VENDOR_BROADCOM 0x14E4
#define PCI_VENDOR_ATHEROS 0x1969
#define PCI_VENDOR_MARVELL 0x11AB
#define PCI_VENDOR_SAMSUNG 0x144D
#define PCI_VENDOR_SANDISK 0x15B7
#define PCI_VENDOR_MICRON 0x1344

// Common device IDs (Intel examples)
#define PCI_DEVICE_INTEL_82540EM 0x100E
#define PCI_DEVICE_INTEL_82574L 0x10D3
#define PCI_DEVICE_INTEL_82579LM 0x1502
#define PCI_DEVICE_INTEL_82579V 0x1503
#define PCI_DEVICE_INTEL_82599ES 0x10FB
#define PCI_DEVICE_INTEL_I210 0x1533
#define PCI_DEVICE_INTEL_I211 0x1539
#define PCI_DEVICE_INTEL_I350 0x1521
#define PCI_DEVICE_INTEL_X540 0x1528
#define PCI_DEVICE_INTEL_X550 0x1563

// PCI capability IDs
#define PCI_CAPABILITY_POWER_MANAGEMENT 0x01
#define PCI_CAPABILITY_AGP 0x02
#define PCI_CAPABILITY_VPD 0x03
#define PCI_CAPABILITY_SLOT_ID 0x04
#define PCI_CAPABILITY_MSI 0x05
#define PCI_CAPABILITY_COMPACT_PCI_HOT_SWAP 0x06
#define PCI_CAPABILITY_PCI_X 0x07
#define PCI_CAPABILITY_HYPER_TRANSPORT 0x08
#define PCI_CAPABILITY_VENDOR_SPECIFIC 0x09
#define PCI_CAPABILITY_DEBUG_PORT 0x0A
#define PCI_CAPABILITY_COMPACT_PCI_CENTRAL_RESOURCE 0x0B
#define PCI_CAPABILITY_PCI_HOT_PLUG 0x0C
#define PCI_CAPABILITY_PCI_BRIDGE_SUBSYSTEM_VENDOR_ID 0x0D
#define PCI_CAPABILITY_AGP_8X 0x0E
#define PCI_CAPABILITY_SECURE_DEVICE 0x0F
#define PCI_CAPABILITY_PCI_EXPRESS 0x10
#define PCI_CAPABILITY_MSI_X 0x11
#define PCI_CAPABILITY_SATA_DATA_NDX_CONFIG 0x12
#define PCI_CAPABILITY_ADVANCED_FEATURES 0x13
#define PCI_CAPABILITY_ENHANCED_ALLOCATION 0x14
#define PCI_CAPABILITY_FLATTENING_PORTAL_BRIDGE 0x15

// PCI Express capability structure
typedef struct {
    uint16_t capability_id;
    uint16_t next_capability;
    uint16_t pcie_capability_version;
    uint32_t device_capabilities;
    uint32_t device_control;
    uint16_t device_status;
    uint32_t link_capabilities;
    uint32_t link_control;
    uint16_t link_status;
} pcie_capability_t;

// PCI MSI capability structure
typedef struct {
    uint16_t capability_id;
    uint16_t next_capability;
    uint16_t message_control;
    uint32_t message_address;
    uint16_t message_data;
    uint16_t mask_bits;
    uint32_t pending_bits;
} msi_capability_t;

// PCI MSI-X capability structure
typedef struct {
    uint16_t capability_id;
    uint16_t next_capability;
    uint32_t message_control;
    uint32_t table_offset;
    uint32_t pending_bit_array_offset;
} msix_capability_t;

#endif // RAEENOS_PCI_H 