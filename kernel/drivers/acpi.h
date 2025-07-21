#ifndef RAEENOS_ACPI_H
#define RAEENOS_ACPI_H

#include "types.h"

// ACPI constants
#define MAX_ACPI_TABLES 64
#define ACPI_SIGNATURE_LENGTH 4

// ACPI table signatures
#define ACPI_SIGNATURE_RSDP "RSD PTR "
#define ACPI_SIGNATURE_RSDT "RSDT"
#define ACPI_SIGNATURE_XSDT "XSDT"
#define ACPI_SIGNATURE_FADT "FACP"
#define ACPI_SIGNATURE_MADT "APIC"
#define ACPI_SIGNATURE_HPET "HPET"
#define ACPI_SIGNATURE_MCFG "MCFG"
#define ACPI_SIGNATURE_DSDT "DSDT"
#define ACPI_SIGNATURE_SSDT "SSDT"
#define ACPI_SIGNATURE_FACS "FACS"
#define ACPI_SIGNATURE_SBST "SBST"
#define ACPI_SIGNATURE_ECDT "ECDT"
#define ACPI_SIGNATURE_SRAT "SRAT"
#define ACPI_SIGNATURE_SLIT "SLIT"
#define ACPI_SIGNATURE_BOOT "BOOT"
#define ACPI_SIGNATURE_CPEP "CPEP"
#define ACPI_SIGNATURE_DBGP "DBGP"
#define ACPI_SIGNATURE_DMAR "DMAR"
#define ACPI_SIGNATURE_DRTM "DRTM"
#define ACPI_SIGNATURE_ETDT "ETDT"
#define ACPI_SIGNATURE_HEST "HEST"
#define ACPI_SIGNATURE_MSCT "MSCT"
#define ACPI_SIGNATURE_MPST "MPST"
#define ACPI_SIGNATURE_OEMx "OEMx"
#define ACPI_SIGNATURE_PCCT "PCCT"
#define ACPI_SIGNATURE_PMTT "PMTT"
#define ACPI_SIGNATURE_RASF "RASF"
#define ACPI_SIGNATURE_RSDT "RSDT"
#define ACPI_SIGNATURE_S3PT "S3PT"
#define ACPI_SIGNATURE_SDEI "SDEI"
#define ACPI_SIGNATURE_SDEV "SDEV"
#define ACPI_SIGNATURE_SLIT "SLIT"
#define ACPI_SIGNATURE_SPCR "SPCR"
#define ACPI_SIGNATURE_SPMI "SPMI"
#define ACPI_SIGNATURE_STAO "STAO"
#define ACPI_SIGNATURE_TCPA "TCPA"
#define ACPI_SIGNATURE_TPM2 "TPM2"
#define ACPI_SIGNATURE_UEFI "UEFI"
#define ACPI_SIGNATURE_VRTC "VRTC"
#define ACPI_SIGNATURE_WAET "WAET"
#define ACPI_SIGNATURE_WDAT "WDAT"
#define ACPI_SIGNATURE_WDDT "WDDT"
#define ACPI_SIGNATURE_WDRT "WDRT"
#define ACPI_SIGNATURE_WSMT "WSMT"
#define ACPI_SIGNATURE_XENV "XENV"

// ACPI sleep states
#define ACPI_SLEEP_STATE_S0 0  // Working
#define ACPI_SLEEP_STATE_S1 1  // Power on suspend
#define ACPI_SLEEP_STATE_S2 2  // CPU off
#define ACPI_SLEEP_STATE_S3 3  // Suspend to RAM
#define ACPI_SLEEP_STATE_S4 4  // Suspend to disk
#define ACPI_SLEEP_STATE_S5 5  // Soft off

// ACPI header structure
typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    char creator_id[4];
    uint32_t creator_revision;
} __attribute__((packed)) acpi_header_t;

// RSDP (Root System Description Pointer) structure
typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) acpi_rsdp_t;

// RSDT (Root System Description Table) structure
typedef struct {
    acpi_header_t header;
    uint32_t table_pointers[0];
} __attribute__((packed)) acpi_rsdt_t;

// XSDT (Extended System Description Table) structure
typedef struct {
    acpi_header_t header;
    uint64_t table_pointers[0];
} __attribute__((packed)) acpi_xsdt_t;

// FADT (Fixed ACPI Description Table) structure
typedef struct {
    acpi_header_t header;
    uint32_t firmware_control;
    uint32_t dsdt;
    uint8_t reserved1;
    uint8_t preferred_pm_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_length;
    uint8_t gpe1_length;
    uint8_t gpe1_base;
    uint8_t cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;
    uint16_t iapc_boot_arch;
    uint8_t reserved2;
    uint32_t flags;
    acpi_generic_address_t reset_register;
    uint8_t reset_value;
    uint8_t reserved3[3];
    uint64_t x_firmware_control;
    uint64_t x_dsdt;
    acpi_generic_address_t x_pm1a_event_block;
    acpi_generic_address_t x_pm1b_event_block;
    acpi_generic_address_t x_pm1a_control_block;
    acpi_generic_address_t x_pm1b_control_block;
    acpi_generic_address_t x_pm2_control_block;
    acpi_generic_address_t x_pm_timer_block;
    acpi_generic_address_t x_gpe0_block;
    acpi_generic_address_t x_gpe1_block;
    acpi_generic_address_t sleep_control_register;
    acpi_generic_address_t sleep_status_register;
    uint64_t hypervisor_vendor_id;
} __attribute__((packed)) acpi_fadt_t;

// Generic Address Structure
typedef struct {
    uint8_t address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t access_size;
    uint64_t address;
} __attribute__((packed)) acpi_generic_address_t;

// MADT (Multiple APIC Description Table) structure
typedef struct {
    acpi_header_t header;
    uint32_t local_apic_address;
    uint32_t flags;
    uint8_t entries[0];
} __attribute__((packed)) acpi_madt_t;

// HPET (High Precision Event Timer) structure
typedef struct {
    acpi_header_t header;
    uint8_t hardware_rev_id;
    uint8_t comparator_count : 5;
    uint8_t counter_size : 1;
    uint8_t reserved : 1;
    uint8_t legacy_route : 1;
    uint16_t pci_vendor_id;
    acpi_generic_address_t address;
    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;
} __attribute__((packed)) acpi_hpet_t;

// MCFG (Memory Mapped Configuration) structure
typedef struct {
    acpi_header_t header;
    uint64_t reserved;
    uint8_t entries[0];
} __attribute__((packed)) acpi_mcfg_t;

// MCFG entry structure
typedef struct {
    uint64_t base_address;
    uint16_t pci_segment_group;
    uint8_t start_bus_number;
    uint8_t end_bus_number;
    uint32_t reserved;
} __attribute__((packed)) acpi_mcfg_entry_t;

// ACPI table structure
typedef struct {
    acpi_header_t* header;
    uintptr_t address;
} acpi_table_t;

// ACPI power information structure
typedef struct {
    uint16_t pm1a_status;
    uint16_t pm1b_status;
    uint16_t pm2_control;
    uint32_t supported_sleep_states;
    uint32_t wake_vector;
} acpi_power_info_t;

// ACPI driver functions
error_t acpi_init(void);
void acpi_shutdown(void);
error_t acpi_find_rsdp(void);
error_t acpi_parse_rsdt(void);
error_t acpi_parse_tables(void);
error_t acpi_parse_table(uintptr_t table_addr);
bool acpi_verify_checksum(void* table, uint32_t length);

// Power management functions
error_t acpi_power_init(void);
void acpi_power_shutdown(void);
error_t acpi_enter_sleep_state(uint8_t sleep_state);
void acpi_prepare_sleep(uint8_t sleep_state);
void acpi_execute_sleep(uint8_t sleep_state);
error_t acpi_wake_from_sleep(void);
error_t acpi_get_power_info(acpi_power_info_t* info);

// Table access functions
acpi_madt_t* acpi_get_madt(void);
acpi_hpet_t* acpi_get_hpet(void);
acpi_mcfg_t* acpi_get_mcfg(void);
acpi_fadt_t* acpi_get_fadt(void);
acpi_header_t* acpi_find_table(const char* signature);
int acpi_get_table_count(void);
error_t acpi_get_tables(acpi_table_t** table_list, int* count);

// Utility functions
bool acpi_is_initialized(void);
bool acpi_is_available(void);

// Global ACPI power management registers
extern uint16_t acpi_pm1a_control;
extern uint16_t acpi_pm1b_control;
extern uint16_t acpi_pm1a_status;
extern uint16_t acpi_pm1b_status;
extern uint16_t acpi_pm2_control;
extern uint16_t acpi_pm_timer;
extern uint16_t acpi_gpe0_base;
extern uint16_t acpi_gpe1_base;

// ACPI address space IDs
#define ACPI_ADDRESS_SPACE_MEMORY 0
#define ACPI_ADDRESS_SPACE_IO 1
#define ACPI_ADDRESS_SPACE_PCI_CONFIG 2
#define ACPI_ADDRESS_SPACE_EMBEDDED_CONTROLLER 3
#define ACPI_ADDRESS_SPACE_SMBUS 4
#define ACPI_ADDRESS_SPACE_SYSTEM_CMOS 5
#define ACPI_ADDRESS_SPACE_PCI_BAR_TARGET 6
#define ACPI_ADDRESS_SPACE_IPMI 7
#define ACPI_ADDRESS_SPACE_GPIO 8
#define ACPI_ADDRESS_SPACE_GSERI 9
#define ACPI_ADDRESS_SPACE_PLATFORM_COMM 10
#define ACPI_ADDRESS_SPACE_FUNCTIONAL_FIXED_HARDWARE 0x7F

// ACPI access sizes
#define ACPI_ACCESS_SIZE_UNDEFINED 0
#define ACPI_ACCESS_SIZE_BYTE 1
#define ACPI_ACCESS_SIZE_WORD 2
#define ACPI_ACCESS_SIZE_DWORD 3
#define ACPI_ACCESS_SIZE_QWORD 4

// ACPI FADT flags
#define ACPI_FADT_WBINVD 0x00000001
#define ACPI_FADT_WBINVD_FLUSH 0x00000002
#define ACPI_FADT_PROC_C1 0x00000004
#define ACPI_FADT_P_LVL2_UP 0x00000008
#define ACPI_FADT_PWR_BUTTON 0x00000010
#define ACPI_FADT_SLP_BUTTON 0x00000020
#define ACPI_FADT_FIX_RTC 0x00000040
#define ACPI_FADT_RTC_S4 0x00000080
#define ACPI_FADT_TMR_VAL_EXT 0x00000100
#define ACPI_FADT_DCK_CAP 0x00000200
#define ACPI_FADT_RESET_REG_SUP 0x00000400
#define ACPI_FADT_SEALED_CASE 0x00000800
#define ACPI_FADT_HEADLESS 0x00001000
#define ACPI_FADT_CPU_SW_SLP 0x00002000
#define ACPI_FADT_PCI_EXP_WAK 0x00004000
#define ACPI_FADT_USE_PLATFORM_CLOCK 0x00008000
#define ACPI_FADT_S4_RTC_STS_VALID 0x00010000
#define ACPI_FADT_REMOTE_POWER_ON_CAPABLE 0x00020000
#define ACPI_FADT_FORCE_APIC_CLUSTER_MODEL 0x00040000
#define ACPI_FADT_FORCE_APIC_PHYSICAL_DESTINATION_MODE 0x00080000
#define ACPI_FADT_HW_REDUCED_ACPI 0x00100000
#define ACPI_FADT_LOW_POWER_S0_IDLE_CAPABLE 0x00200000

#endif // RAEENOS_ACPI_H 