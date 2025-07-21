#include "acpi.h"
#include "memory/memory.h"
#include <string.h>

// ACPI driver state
static bool acpi_initialized = false;
static acpi_rsdp_t* rsdp = NULL;
static acpi_rsdt_t* rsdt = NULL;
static acpi_xsdt_t* xsdt = NULL;
static acpi_fadt_t* fadt = NULL;
static acpi_madt_t* madt = NULL;
static acpi_hpet_t* hpet = NULL;
static acpi_mcfg_t* mcfg = NULL;

// ACPI table registry
static acpi_table_t* tables[MAX_ACPI_TABLES];
static int table_count = 0;

// Initialize ACPI subsystem
error_t acpi_init(void) {
    if (acpi_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing ACPI subsystem");
    
    // Clear arrays
    memset(tables, 0, sizeof(tables));
    table_count = 0;
    
    // Find RSDP (Root System Description Pointer)
    error_t result = acpi_find_rsdp();
    if (result != SUCCESS) {
        KERROR("Failed to find ACPI RSDP");
        return result;
    }
    
    // Parse RSDT/XSDT
    result = acpi_parse_rsdt();
    if (result != SUCCESS) {
        KERROR("Failed to parse ACPI RSDT/XSDT");
        return result;
    }
    
    // Parse important tables
    result = acpi_parse_tables();
    if (result != SUCCESS) {
        KERROR("Failed to parse ACPI tables");
        return result;
    }
    
    // Initialize power management
    result = acpi_power_init();
    if (result != SUCCESS) {
        KWARN("Failed to initialize ACPI power management");
    }
    
    acpi_initialized = true;
    
    KINFO("ACPI subsystem initialized with %d tables", table_count);
    return SUCCESS;
}

// Shutdown ACPI subsystem
void acpi_shutdown(void) {
    if (!acpi_initialized) {
        return;
    }
    
    KINFO("Shutting down ACPI subsystem");
    
    // Shutdown power management
    acpi_power_shutdown();
    
    // Free all tables
    for (int i = 0; i < table_count; i++) {
        if (tables[i]) {
            memory_free(tables[i]);
        }
    }
    
    acpi_initialized = false;
    
    KINFO("ACPI subsystem shutdown complete");
}

// Find RSDP (Root System Description Pointer)
error_t acpi_find_rsdp(void) {
    KDEBUG("Searching for ACPI RSDP");
    
    // Search in BIOS memory area (0xE0000-0x100000)
    for (uintptr_t addr = 0xE0000; addr < 0x100000; addr += 16) {
        acpi_rsdp_t* candidate = (acpi_rsdp_t*)addr;
        
        // Check signature
        if (memcmp(candidate->signature, "RSD PTR ", 8) == 0) {
            // Verify checksum
            uint8_t sum = 0;
            for (int i = 0; i < 20; i++) {
                sum += ((uint8_t*)candidate)[i];
            }
            
            if (sum == 0) {
                rsdp = candidate;
                KDEBUG("Found ACPI RSDP at 0x%lx", addr);
                return SUCCESS;
            }
        }
    }
    
    // Search in EBDA (Extended BIOS Data Area)
    uint16_t ebda_segment = *(uint16_t*)0x40E;
    if (ebda_segment != 0) {
        uintptr_t ebda_addr = ebda_segment << 4;
        
        for (uintptr_t addr = ebda_addr; addr < ebda_addr + 1024; addr += 16) {
            acpi_rsdp_t* candidate = (acpi_rsdp_t*)addr;
            
            // Check signature
            if (memcmp(candidate->signature, "RSD PTR ", 8) == 0) {
                // Verify checksum
                uint8_t sum = 0;
                for (int i = 0; i < 20; i++) {
                    sum += ((uint8_t*)candidate)[i];
                }
                
                if (sum == 0) {
                    rsdp = candidate;
                    KDEBUG("Found ACPI RSDP in EBDA at 0x%lx", addr);
                    return SUCCESS;
                }
            }
        }
    }
    
    return E_NOENT;
}

// Parse RSDT/XSDT
error_t acpi_parse_rsdt(void) {
    if (!rsdp) {
        return E_INVAL;
    }
    
    KDEBUG("Parsing ACPI RSDT/XSDT");
    
    // Check if XSDT is available (ACPI 2.0+)
    if (rsdp->revision >= 2 && rsdp->xsdt_address != 0) {
        xsdt = (acpi_xsdt_t*)rsdp->xsdt_address;
        
        // Verify XSDT signature and checksum
        if (memcmp(xsdt->header.signature, "XSDT", 4) == 0 && acpi_verify_checksum(xsdt, xsdt->header.length)) {
            KDEBUG("Using ACPI XSDT with %d entries", (xsdt->header.length - sizeof(acpi_header_t)) / 8);
            return SUCCESS;
        }
    }
    
    // Fall back to RSDT
    rsdt = (acpi_rsdt_t*)rsdp->rsdt_address;
    
    // Verify RSDT signature and checksum
    if (memcmp(rsdt->header.signature, "RSDT", 4) == 0 && acpi_verify_checksum(rsdt, rsdt->header.length)) {
        KDEBUG("Using ACPI RSDT with %d entries", (rsdt->header.length - sizeof(acpi_header_t)) / 4);
        return SUCCESS;
    }
    
    return E_INVAL;
}

// Parse ACPI tables
error_t acpi_parse_tables(void) {
    KDEBUG("Parsing ACPI tables");
    
    if (xsdt) {
        // Parse XSDT entries
        int entry_count = (xsdt->header.length - sizeof(acpi_header_t)) / 8;
        for (int i = 0; i < entry_count; i++) {
            uintptr_t table_addr = xsdt->table_pointers[i];
            acpi_parse_table(table_addr);
        }
    } else if (rsdt) {
        // Parse RSDT entries
        int entry_count = (rsdt->header.length - sizeof(acpi_header_t)) / 4;
        for (int i = 0; i < entry_count; i++) {
            uintptr_t table_addr = rsdt->table_pointers[i];
            acpi_parse_table(table_addr);
        }
    } else {
        return E_INVAL;
    }
    
    return SUCCESS;
}

// Parse individual ACPI table
error_t acpi_parse_table(uintptr_t table_addr) {
    if (!table_addr) {
        return E_INVAL;
    }
    
    acpi_header_t* header = (acpi_header_t*)table_addr;
    
    // Verify table signature and checksum
    if (!acpi_verify_checksum(header, header->length)) {
        KDEBUG("Invalid ACPI table checksum at 0x%lx", table_addr);
        return E_INVAL;
    }
    
    KDEBUG("Found ACPI table: %.4s, version %d, length %d", 
           header->signature, header->revision, header->length);
    
    // Parse specific table types
    if (memcmp(header->signature, "FACP", 4) == 0) {
        fadt = (acpi_fadt_t*)header;
        KDEBUG("Parsed FADT (Fixed ACPI Description Table)");
    } else if (memcmp(header->signature, "APIC", 4) == 0) {
        madt = (acpi_madt_t*)header;
        KDEBUG("Parsed MADT (Multiple APIC Description Table)");
    } else if (memcmp(header->signature, "HPET", 4) == 0) {
        hpet = (acpi_hpet_t*)header;
        KDEBUG("Parsed HPET (High Precision Event Timer)");
    } else if (memcmp(header->signature, "MCFG", 4) == 0) {
        mcfg = (acpi_mcfg_t*)header;
        KDEBUG("Parsed MCFG (Memory Mapped Configuration)");
    }
    
    // Store table
    if (table_count < MAX_ACPI_TABLES) {
        acpi_table_t* table = (acpi_table_t*)memory_alloc(sizeof(acpi_table_t));
        if (table) {
            table->header = header;
            table->address = table_addr;
            tables[table_count++] = table;
        }
    }
    
    return SUCCESS;
}

// Verify ACPI table checksum
bool acpi_verify_checksum(void* table, uint32_t length) {
    uint8_t sum = 0;
    uint8_t* data = (uint8_t*)table;
    
    for (uint32_t i = 0; i < length; i++) {
        sum += data[i];
    }
    
    return sum == 0;
}

// Initialize ACPI power management
error_t acpi_power_init(void) {
    KDEBUG("Initializing ACPI power management");
    
    if (!fadt) {
        KERROR("FADT not found, cannot initialize power management");
        return E_NOENT;
    }
    
    // Initialize power management registers
    acpi_pm1a_control = fadt->pm1a_control_block;
    acpi_pm1b_control = fadt->pm1b_control_block;
    acpi_pm1a_status = fadt->pm1a_status_block;
    acpi_pm1b_status = fadt->pm1b_status_block;
    acpi_pm2_control = fadt->pm2_control_block;
    acpi_pm_timer = fadt->pm_timer_block;
    acpi_gpe0_base = fadt->gpe0_block;
    acpi_gpe1_base = fadt->gpe1_block;
    
    KDEBUG("ACPI power management registers initialized");
    KDEBUG("  PM1A Control: 0x%04x", acpi_pm1a_control);
    KDEBUG("  PM1B Control: 0x%04x", acpi_pm1b_control);
    KDEBUG("  PM1A Status: 0x%04x", acpi_pm1a_status);
    KDEBUG("  PM1B Status: 0x%04x", acpi_pm1b_status);
    KDEBUG("  PM2 Control: 0x%04x", acpi_pm2_control);
    KDEBUG("  PM Timer: 0x%04x", acpi_pm_timer);
    KDEBUG("  GPE0 Base: 0x%04x", acpi_gpe0_base);
    KDEBUG("  GPE1 Base: 0x%04x", acpi_gpe1_base);
    
    return SUCCESS;
}

// Shutdown ACPI power management
void acpi_power_shutdown(void) {
    KDEBUG("Shutting down ACPI power management");
    
    // Nothing specific to do here
}

// Power management functions
error_t acpi_enter_sleep_state(uint8_t sleep_state) {
    if (!fadt) {
        return E_NOENT;
    }
    
    KDEBUG("Entering ACPI sleep state %d", sleep_state);
    
    // Validate sleep state
    if (sleep_state < 1 || sleep_state > 5) {
        return E_INVAL;
    }
    
    // Check if sleep state is supported
    uint32_t supported_states = fadt->supported_sleep_states;
    if (!(supported_states & (1 << sleep_state))) {
        KERROR("Sleep state %d not supported", sleep_state);
        return E_INVAL;
    }
    
    // Prepare for sleep
    acpi_prepare_sleep(sleep_state);
    
    // Enter sleep state
    acpi_execute_sleep(sleep_state);
    
    return SUCCESS;
}

// Prepare for sleep
void acpi_prepare_sleep(uint8_t sleep_state) {
    KDEBUG("Preparing for sleep state %d", sleep_state);
    
    // Save processor state
    // Disable interrupts
    // Flush caches
    // Set up wake vector
    
    // Set sleep type and enable bit
    uint16_t sleep_type = sleep_state << 10;
    uint16_t sleep_enable = 0x2000; // SLP_EN bit
    
    // Write to PM1A control register
    if (acpi_pm1a_control) {
        outw(acpi_pm1a_control, sleep_type | sleep_enable);
    }
    
    // Write to PM1B control register if available
    if (acpi_pm1b_control) {
        outw(acpi_pm1b_control, sleep_type | sleep_enable);
    }
}

// Execute sleep
void acpi_execute_sleep(uint8_t sleep_state) {
    KDEBUG("Executing sleep state %d", sleep_state);
    
    // This would typically involve:
    // 1. Writing to specific ACPI registers
    // 2. Executing the sleep instruction
    // 3. System enters sleep state
    
    // For now, just simulate sleep
    KDEBUG("System would enter sleep state %d", sleep_state);
}

// Wake from sleep
error_t acpi_wake_from_sleep(void) {
    KDEBUG("Waking from sleep");
    
    // Restore processor state
    // Re-enable interrupts
    // Restore system state
    
    return SUCCESS;
}

// Get power management information
error_t acpi_get_power_info(acpi_power_info_t* info) {
    if (!info || !fadt) {
        return E_INVAL;
    }
    
    memset(info, 0, sizeof(acpi_power_info_t));
    
    // Read power management registers
    if (acpi_pm1a_status) {
        info->pm1a_status = inw(acpi_pm1a_status);
    }
    
    if (acpi_pm1b_status) {
        info->pm1b_status = inw(acpi_pm1b_status);
    }
    
    if (acpi_pm2_control) {
        info->pm2_control = inw(acpi_pm2_control);
    }
    
    // Get supported sleep states
    info->supported_sleep_states = fadt->supported_sleep_states;
    
    // Get wake vector
    info->wake_vector = fadt->wake_vector;
    
    return SUCCESS;
}

// Get MADT (Multiple APIC Description Table)
acpi_madt_t* acpi_get_madt(void) {
    return madt;
}

// Get HPET (High Precision Event Timer)
acpi_hpet_t* acpi_get_hpet(void) {
    return hpet;
}

// Get MCFG (Memory Mapped Configuration)
acpi_mcfg_t* acpi_get_mcfg(void) {
    return mcfg;
}

// Get FADT (Fixed ACPI Description Table)
acpi_fadt_t* acpi_get_fadt(void) {
    return fadt;
}

// Find ACPI table by signature
acpi_header_t* acpi_find_table(const char* signature) {
    for (int i = 0; i < table_count; i++) {
        if (tables[i] && memcmp(tables[i]->header->signature, signature, 4) == 0) {
            return tables[i]->header;
        }
    }
    return NULL;
}

// Get table count
int acpi_get_table_count(void) {
    return table_count;
}

// Get all tables
error_t acpi_get_tables(acpi_table_t** table_list, int* count) {
    if (!table_list || !count) {
        return E_INVAL;
    }
    
    *table_list = tables;
    *count = table_count;
    
    return SUCCESS;
}

// Check if ACPI is initialized
bool acpi_is_initialized(void) {
    return acpi_initialized;
}

// Check if ACPI is available
bool acpi_is_available(void) {
    return rsdp != NULL;
}

// Global ACPI power management registers
uint16_t acpi_pm1a_control = 0;
uint16_t acpi_pm1b_control = 0;
uint16_t acpi_pm1a_status = 0;
uint16_t acpi_pm1b_status = 0;
uint16_t acpi_pm2_control = 0;
uint16_t acpi_pm_timer = 0;
uint16_t acpi_gpe0_base = 0;
uint16_t acpi_gpe1_base = 0;

// I/O port access functions
static inline void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    asm volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
} 