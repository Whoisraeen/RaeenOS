#include "sata.h"
#include "../core/kernel.h"
#include "../memory/memory.h"
#include "../interrupts/interrupts.h"
#include "../pci/pci.h"
#include "../libc/string.h"
#include "../libc/stdio.h"

// SATA subsystem instance
static sata_subsystem_t sata_subsystem = {0};

// Common SATA controller PCI IDs
static const struct {
    uint16_t vendor_id;
    uint16_t device_id;
    const char* name;
} sata_controllers[] = {
    // Intel SATA controllers
    {0x8086, 0x2922, "Intel ICH9 SATA"},
    {0x8086, 0x3A22, "Intel ICH10 SATA"},
    {0x8086, 0x1C02, "Intel 6 Series SATA"},
    {0x8086, 0x1C03, "Intel 6 Series SATA"},
    {0x8086, 0x1E02, "Intel 7 Series SATA"},
    {0x8086, 0x1E03, "Intel 7 Series SATA"},
    {0x8086, 0x8C02, "Intel 8 Series SATA"},
    {0x8086, 0x8C03, "Intel 8 Series SATA"},
    {0x8086, 0x9C02, "Intel 9 Series SATA"},
    {0x8086, 0x9C03, "Intel 9 Series SATA"},
    {0x8086, 0xA102, "Intel 100 Series SATA"},
    {0x8086, 0xA103, "Intel 100 Series SATA"},
    {0x8086, 0xA182, "Intel 200 Series SATA"},
    {0x8086, 0xA183, "Intel 200 Series SATA"},
    {0x8086, 0xA282, "Intel 300 Series SATA"},
    {0x8086, 0xA283, "Intel 300 Series SATA"},
    
    // AMD SATA controllers
    {0x1022, 0x7800, "AMD FCH SATA"},
    {0x1022, 0x7801, "AMD FCH SATA"},
    {0x1022, 0x7804, "AMD FCH SATA"},
    {0x1002, 0x4390, "AMD SB7x0 SATA"},
    {0x1002, 0x4391, "AMD SB7x0 SATA"},
    {0x1002, 0x4392, "AMD SB7x0 SATA"},
    {0x1002, 0x4393, "AMD SB7x0 SATA"},
    {0x1002, 0x4394, "AMD SB7x0 SATA"},
    {0x1002, 0x4395, "AMD SB7x0 SATA"},
    
    // VIA SATA controllers
    {0x1106, 0x3349, "VIA VT8251 SATA"},
    {0x1106, 0x5337, "VIA VT8237 SATA"},
    {0x1106, 0x0591, "VIA VT8237A SATA"},
    
    // NVIDIA SATA controllers
    {0x10DE, 0x044C, "NVIDIA MCP65 SATA"},
    {0x10DE, 0x044D, "NVIDIA MCP65 SATA"},
    {0x10DE, 0x044E, "NVIDIA MCP65 SATA"},
    {0x10DE, 0x044F, "NVIDIA MCP65 SATA"},
    
    // Marvell SATA controllers
    {0x11AB, 0x6121, "Marvell 88SE6121 SATA"},
    {0x11AB, 0x6141, "Marvell 88SE6141 SATA"},
    {0x11AB, 0x6145, "Marvell 88SE6145 SATA"},
    
    {0, 0, NULL} // End marker
};

// Forward declarations
static error_t sata_controller_init(sata_controller_t* controller);
static error_t sata_port_init(sata_controller_t* controller, uint8_t port);
static error_t sata_device_detect(sata_controller_t* controller, uint8_t port);
static error_t sata_send_command(sata_device_t* device, uint8_t command, 
                                uint64_t lba, uint16_t count, void* buffer, bool write);
static void sata_interrupt_handler(interrupt_frame_t* frame);

// Initialize SATA subsystem
error_t sata_init(void) {
    printf("SATA: Initializing SATA subsystem...\n");
    
    // Initialize subsystem structure
    memset(&sata_subsystem, 0, sizeof(sata_subsystem_t));
    
    // Initialize mutexes
    mutex_init(&sata_subsystem.subsystem_mutex);
    mutex_init(&sata_subsystem.device_mutex);
    
    // Set default configuration
    sata_subsystem.max_transfer_size = 65536; // 64KB
    sata_subsystem.command_timeout = 30000;   // 30 seconds
    sata_subsystem.ncq_enabled = true;
    
    // Detect SATA controllers
    error_t result = sata_detect_controllers();
    if (result != ERROR_SUCCESS) {
        printf("SATA: Failed to detect controllers\n");
        return result;
    }
    
    sata_subsystem.initialized = true;
    printf("SATA: Subsystem initialized with %u controllers, %u devices\n",
           sata_subsystem.num_controllers, sata_subsystem.num_devices);
    
    return ERROR_SUCCESS;
}

// Shutdown SATA subsystem
void sata_shutdown(void) {
    if (!sata_subsystem.initialized) {
        return;
    }
    
    printf("SATA: Shutting down SATA subsystem...\n");
    
    mutex_lock(&sata_subsystem.subsystem_mutex);
    
    // Shutdown all controllers
    sata_controller_t* controller = sata_subsystem.controller_list;
    while (controller) {
        sata_controller_t* next = controller->next;
        
        // Shutdown controller
        if (controller->initialized) {
            // Stop all devices
            for (uint8_t port = 0; port < controller->num_ports; port++) {
                if (controller->devices[port]) {
                    sata_device_t* device = controller->devices[port];
                    sata_standby_device(device);
                }
            }
            
            // Disable controller
            controller->enabled = false;
            controller->initialized = false;
        }
        
        // Free memory
        if (controller->command_list_base) {
            memory_free(controller->command_list_base);
        }
        if (controller->received_fis_base) {
            memory_free(controller->received_fis_base);
        }
        if (controller->command_table_base) {
            memory_free(controller->command_table_base);
        }
        
        memory_free(controller);
        controller = next;
    }
    
    sata_subsystem.controller_list = NULL;
    sata_subsystem.num_controllers = 0;
    sata_subsystem.device_list = NULL;
    sata_subsystem.num_devices = 0;
    sata_subsystem.initialized = false;
    
    mutex_unlock(&sata_subsystem.subsystem_mutex);
    
    printf("SATA: Subsystem shutdown complete\n");
}

// Detect SATA controllers
error_t sata_detect_controllers(void) {
    printf("SATA: Scanning for SATA controllers...\n");
    
    uint32_t controllers_found = 0;
    
    // Scan PCI bus for SATA controllers
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint16_t vendor_id = pci_read_config_word(bus, device, function, 0x00);
                if (vendor_id == 0xFFFF) {
                    continue; // No device
                }
                
                uint16_t device_id = pci_read_config_word(bus, device, function, 0x02);
                uint8_t class_code = pci_read_config_byte(bus, device, function, 0x0B);
                uint8_t subclass = pci_read_config_byte(bus, device, function, 0x0A);
                uint8_t prog_if = pci_read_config_byte(bus, device, function, 0x09);
                
                // Check if this is a SATA controller (Mass Storage Controller, SATA)
                if (class_code == 0x01 && subclass == 0x06) {
                    // Check if it's in our supported list
                    const char* controller_name = "Unknown SATA Controller";
                    for (int i = 0; sata_controllers[i].vendor_id != 0; i++) {
                        if (sata_controllers[i].vendor_id == vendor_id &&
                            sata_controllers[i].device_id == device_id) {
                            controller_name = sata_controllers[i].name;
                            break;
                        }
                    }
                    
                    printf("SATA: Found %s (VID: 0x%04X, DID: 0x%04X) at %02X:%02X.%X\n",
                           controller_name, vendor_id, device_id, bus, device, function);
                    
                    // Create controller structure
                    sata_controller_t* controller = memory_alloc(sizeof(sata_controller_t));
                    if (!controller) {
                        printf("SATA: Failed to allocate memory for controller\n");
                        continue;
                    }
                    
                    memset(controller, 0, sizeof(sata_controller_t));
                    
                    // Initialize controller
                    controller->vendor_id = vendor_id;
                    controller->device_id = device_id;
                    controller->base_address = pci_read_config_dword(bus, device, function, 0x10) & 0xFFFFFFF0;
                    controller->memory_base = pci_read_config_dword(bus, device, function, 0x24) & 0xFFFFFFF0;
                    controller->irq = pci_read_config_byte(bus, device, function, 0x3C);
                    
                    // Initialize the controller
                    error_t result = sata_controller_init(controller);
                    if (result == ERROR_SUCCESS) {
                        // Register controller
                        sata_register_controller(controller);
                        controllers_found++;
                    } else {
                        printf("SATA: Failed to initialize controller\n");
                        memory_free(controller);
                    }
                }
            }
        }
    }
    
    printf("SATA: Found %u SATA controllers\n", controllers_found);
    return ERROR_SUCCESS;
}

// Initialize SATA controller
static error_t sata_controller_init(sata_controller_t* controller) {
    printf("SATA: Initializing controller at 0x%08X\n", controller->memory_base);
    
    // Map AHCI memory
    if (!controller->memory_base) {
        printf("SATA: Invalid memory base address\n");
        return ERROR_INVALID_PARAMETER;
    }
    
    // Read capabilities
    uint32_t cap = *(volatile uint32_t*)(controller->memory_base + AHCI_HBA_CAP);
    controller->num_ports = (cap & 0x1F) + 1;
    controller->num_command_slots = ((cap >> 8) & 0x1F) + 1;
    controller->ncq_supported = (cap & (1 << 30)) != 0;
    controller->ahci_only = (cap & (1 << 18)) != 0;
    controller->interface_speed = (cap >> 20) & 0x0F;
    controller->port_multiplier_support = (cap & (1 << 17)) != 0;
    controller->64bit_addressing = (cap & (1 << 31)) != 0;
    
    printf("SATA: Controller capabilities: %u ports, %u slots, NCQ: %s, 64-bit: %s\n",
           controller->num_ports, controller->num_command_slots,
           controller->ncq_supported ? "Yes" : "No",
           controller->64bit_addressing ? "Yes" : "No");
    
    // Read implemented ports
    controller->ports_implemented = *(volatile uint32_t*)(controller->memory_base + AHCI_HBA_PI);
    
    // Enable AHCI mode
    uint32_t ghc = *(volatile uint32_t*)(controller->memory_base + AHCI_HBA_GHC);
    ghc |= (1 << 31); // AHCI Enable
    *(volatile uint32_t*)(controller->memory_base + AHCI_HBA_GHC) = ghc;
    
    // Allocate memory for command structures
    size_t cmd_list_size = controller->num_command_slots * sizeof(ahci_cmd_header_t);
    size_t fis_size = 256; // Standard FIS size
    size_t cmd_table_size = controller->num_command_slots * sizeof(ahci_cmd_table_t);
    
    controller->command_list_base = memory_alloc_aligned(cmd_list_size, 1024);
    controller->received_fis_base = memory_alloc_aligned(fis_size, 256);
    controller->command_table_base = memory_alloc_aligned(cmd_table_size, 128);
    
    if (!controller->command_list_base || !controller->received_fis_base || !controller->command_table_base) {
        printf("SATA: Failed to allocate command structures\n");
        return ERROR_OUT_OF_MEMORY;
    }
    
    memset(controller->command_list_base, 0, cmd_list_size);
    memset(controller->received_fis_base, 0, fis_size);
    memset(controller->command_table_base, 0, cmd_table_size);
    
    // Initialize ports
    for (uint8_t port = 0; port < controller->num_ports; port++) {
        if (controller->ports_implemented & (1 << port)) {
            sata_port_init(controller, port);
            sata_device_detect(controller, port);
        }
    }
    
    // Install interrupt handler
    interrupt_register_handler(controller->irq, sata_interrupt_handler);
    
    controller->initialized = true;
    controller->enabled = true;
    
    printf("SATA: Controller initialized successfully\n");
    return ERROR_SUCCESS;
}

// Initialize SATA port
static error_t sata_port_init(sata_controller_t* controller, uint8_t port) {
    uint32_t port_base = controller->memory_base + 0x100 + (port * 0x80);
    
    // Stop command engine
    uint32_t cmd = *(volatile uint32_t*)(port_base + AHCI_PORT_CMD);
    cmd &= ~(1 << 0); // Clear ST (Start)
    *(volatile uint32_t*)(port_base + AHCI_PORT_CMD) = cmd;
    
    // Wait for command engine to stop
    while (*(volatile uint32_t*)(port_base + AHCI_PORT_CMD) & (1 << 15)) {
        // Wait for CR (Command Running) to clear
    }
    
    // Set command list base address
    uint64_t cmd_list_addr = (uint64_t)controller->command_list_base + (port * 1024);
    *(volatile uint32_t*)(port_base + AHCI_PORT_CLB) = (uint32_t)cmd_list_addr;
    if (controller->64bit_addressing) {
        *(volatile uint32_t*)(port_base + AHCI_PORT_CLBU) = (uint32_t)(cmd_list_addr >> 32);
    }
    
    // Set FIS base address
    uint64_t fis_addr = (uint64_t)controller->received_fis_base + (port * 256);
    *(volatile uint32_t*)(port_base + AHCI_PORT_FB) = (uint32_t)fis_addr;
    if (controller->64bit_addressing) {
        *(volatile uint32_t*)(port_base + AHCI_PORT_FBU) = (uint32_t)(fis_addr >> 32);
    }
    
    // Clear interrupt status
    *(volatile uint32_t*)(port_base + AHCI_PORT_IS) = 0xFFFFFFFF;
    
    // Enable interrupts
    *(volatile uint32_t*)(port_base + AHCI_PORT_IE) = 0x7FFFFFFF;
    
    // Start command engine
    cmd = *(volatile uint32_t*)(port_base + AHCI_PORT_CMD);
    cmd |= (1 << 4); // FRE (FIS Receive Enable)
    cmd |= (1 << 0); // ST (Start)
    *(volatile uint32_t*)(port_base + AHCI_PORT_CMD) = cmd;
    
    return ERROR_SUCCESS;
}

// Detect device on SATA port
static error_t sata_device_detect(sata_controller_t* controller, uint8_t port) {
    uint32_t port_base = controller->memory_base + 0x100 + (port * 0x80);
    
    // Check SATA status
    uint32_t ssts = *(volatile uint32_t*)(port_base + AHCI_PORT_SSTS);
    uint8_t det = ssts & 0x0F;
    uint8_t spd = (ssts >> 4) & 0x0F;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    
    if (det != 3 || ipm != 1) {
        // No device present or not active
        return ERROR_NOT_FOUND;
    }
    
    // Check device signature
    uint32_t sig = *(volatile uint32_t*)(port_base + AHCI_PORT_SIG);
    sata_device_type_t device_type;
    
    switch (sig) {
        case 0x00000101:
            device_type = SATA_DEVICE_TYPE_SATA;
            break;
        case 0xEB140101:
            device_type = SATA_DEVICE_TYPE_SATAPI;
            break;
        case 0xC33C0101:
            device_type = SATA_DEVICE_TYPE_SEMB;
            break;
        case 0x96690101:
            device_type = SATA_DEVICE_TYPE_PM;
            break;
        default:
            printf("SATA: Unknown device signature 0x%08X on port %u\n", sig, port);
            return ERROR_NOT_SUPPORTED;
    }
    
    printf("SATA: Device detected on port %u, type: %s, speed: Gen%u\n",
           port, sata_device_type_to_string(device_type), spd);
    
    // Create device structure
    sata_device_t* device = memory_alloc(sizeof(sata_device_t));
    if (!device) {
        printf("SATA: Failed to allocate memory for device\n");
        return ERROR_OUT_OF_MEMORY;
    }
    
    memset(device, 0, sizeof(sata_device_t));
    
    // Initialize device
    device->port = port;
    device->type = device_type;
    device->state = SATA_DEVICE_STATE_PRESENT;
    device->speed = (sata_speed_t)(spd - 1);
    device->controller = controller;
    device->sector_size = SATA_SECTOR_SIZE;
    
    // Identify device
    error_t result = sata_identify_device(device);
    if (result == ERROR_SUCCESS) {
        // Register device
        controller->devices[port] = device;
        controller->num_devices++;
        sata_register_device(device);
        
        device->state = SATA_DEVICE_STATE_ACTIVE;
        
        printf("SATA: Device %s (%s) registered on port %u\n",
               device->model, device->serial, port);
    } else {
        printf("SATA: Failed to identify device on port %u\n", port);
        memory_free(device);
    }
    
    return result;
}

// Register SATA controller
error_t sata_register_controller(sata_controller_t* controller) {
    if (!controller) {
        return ERROR_INVALID_PARAMETER;
    }
    
    mutex_lock(&sata_subsystem.subsystem_mutex);
    
    // Add to controller list
    controller->next = sata_subsystem.controller_list;
    sata_subsystem.controller_list = controller;
    sata_subsystem.num_controllers++;
    
    mutex_unlock(&sata_subsystem.subsystem_mutex);
    
    return ERROR_SUCCESS;
}

// Register SATA device
error_t sata_register_device(sata_device_t* device) {
    if (!device) {
        return ERROR_INVALID_PARAMETER;
    }
    
    mutex_lock(&sata_subsystem.device_mutex);
    
    // Add to device list
    device->next = sata_subsystem.device_list;
    sata_subsystem.device_list = device;
    sata_subsystem.num_devices++;
    
    mutex_unlock(&sata_subsystem.device_mutex);
    
    return ERROR_SUCCESS;
}

// Identify SATA device
error_t sata_identify_device(sata_device_t* device) {
    if (!device) {
        return ERROR_INVALID_PARAMETER;
    }
    
    // Allocate buffer for IDENTIFY data
    ata_identify_t* identify_data = memory_alloc(sizeof(ata_identify_t));
    if (!identify_data) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    // Send IDENTIFY command
    uint8_t command = (device->type == SATA_DEVICE_TYPE_SATAPI) ? 
                      ATA_CMD_IDENTIFY_PACKET : ATA_CMD_IDENTIFY;
    
    error_t result = sata_send_command(device, command, 0, 1, identify_data, false);
    if (result != ERROR_SUCCESS) {
        memory_free(identify_data);
        return result;
    }
    
    // Parse IDENTIFY data
    // Model number (words 27-46)
    for (int i = 0; i < 20; i++) {
        uint16_t word = identify_data->model_number[i];
        device->model[i * 2] = (word >> 8) & 0xFF;
        device->model[i * 2 + 1] = word & 0xFF;
    }
    device->model[40] = '\0';
    
    // Serial number (words 10-19)
    for (int i = 0; i < 10; i++) {
        uint16_t word = identify_data->serial_number[i];
        device->serial[i * 2] = (word >> 8) & 0xFF;
        device->serial[i * 2 + 1] = word & 0xFF;
    }
    device->serial[20] = '\0';
    
    // Firmware revision (words 23-26)
    for (int i = 0; i < 4; i++) {
        uint16_t word = identify_data->firmware_revision[i];
        device->firmware[i * 2] = (word >> 8) & 0xFF;
        device->firmware[i * 2 + 1] = word & 0xFF;
    }
    device->firmware[8] = '\0';
    
    // Capacity
    if (identify_data->command_set_support & (1 << 10)) {
        // 48-bit LBA supported
        device->lba48_supported = true;
        device->capacity = identify_data->lba48_capacity;
    } else {
        device->lba48_supported = false;
        device->capacity = identify_data->lba_capacity;
    }
    
    // Features
    device->dma_supported = (identify_data->capabilities & (1 << 8)) != 0;
    device->smart_supported = (identify_data->command_set_support & (1 << 0)) != 0;
    device->security_supported = (identify_data->command_set_support & (1 << 1)) != 0;
    device->apm_supported = (identify_data->command_set_support & (1 << 3)) != 0;
    
    // NCQ support
    if (identify_data->field_validity & (1 << 2)) {
        device->ncq_supported = (identify_data->field_validity & (1 << 8)) != 0;
        if (device->ncq_supported) {
            device->max_queue_depth = (identify_data->field_validity & 0x1F) + 1;
        }
    }
    
    memory_free(identify_data);
    
    printf("SATA: Device identified - Model: %s, Serial: %s, Capacity: %llu sectors\n",
           device->model, device->serial, device->capacity);
    
    return ERROR_SUCCESS;
}

// Read sectors from SATA device
error_t sata_read_sectors(sata_device_t* device, uint64_t lba, uint32_t count, void* buffer) {
    if (!device || !buffer || count == 0) {
        return ERROR_INVALID_PARAMETER;
    }
    
    if (lba + count > device->capacity) {
        return ERROR_INVALID_PARAMETER;
    }
    
    uint8_t command = device->lba48_supported ? ATA_CMD_READ_DMA_EXT : ATA_CMD_READ_DMA;
    return sata_send_command(device, command, lba, count, buffer, false);
}

// Write sectors to SATA device
error_t sata_write_sectors(sata_device_t* device, uint64_t lba, uint32_t count, const void* buffer) {
    if (!device || !buffer || count == 0) {
        return ERROR_INVALID_PARAMETER;
    }
    
    if (lba + count > device->capacity) {
        return ERROR_INVALID_PARAMETER;
    }
    
    uint8_t command = device->lba48_supported ? ATA_CMD_WRITE_DMA_EXT : ATA_CMD_WRITE_DMA;
    return sata_send_command(device, command, lba, count, (void*)buffer, true);
}

// Send command to SATA device
static error_t sata_send_command(sata_device_t* device, uint8_t command, 
                                uint64_t lba, uint16_t count, void* buffer, bool write) {
    if (!device || !device->controller) {
        return ERROR_INVALID_PARAMETER;
    }
    
    sata_controller_t* controller = device->controller;
    uint8_t port = device->port;
    uint32_t port_base = controller->memory_base + 0x100 + (port * 0x80);
    
    // Find free command slot
    uint32_t slots = *(volatile uint32_t*)(port_base + AHCI_PORT_SACT) |
                     *(volatile uint32_t*)(port_base + AHCI_PORT_CI);
    
    int slot = -1;
    for (int i = 0; i < controller->num_command_slots; i++) {
        if (!(slots & (1 << i))) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        return ERROR_BUSY;
    }
    
    // Get command header
    ahci_cmd_header_t* cmd_header = (ahci_cmd_header_t*)controller->command_list_base + slot;
    memset(cmd_header, 0, sizeof(ahci_cmd_header_t));
    
    // Set command header
    cmd_header->cfl = sizeof(fis_reg_h2d_t) / 4; // Command FIS length
    cmd_header->w = write ? 1 : 0;               // Write direction
    cmd_header->prdtl = 1;                       // One PRD entry
    
    // Get command table
    ahci_cmd_table_t* cmd_table = (ahci_cmd_table_t*)
        ((uint8_t*)controller->command_table_base + (slot * sizeof(ahci_cmd_table_t)));
    memset(cmd_table, 0, sizeof(ahci_cmd_table_t));
    
    // Set command table address
    cmd_header->ctba = (uint32_t)(uintptr_t)cmd_table;
    if (controller->64bit_addressing) {
        cmd_header->ctbau = (uint32_t)((uintptr_t)cmd_table >> 32);
    }
    
    // Build command FIS
    fis_reg_h2d_t* fis = (fis_reg_h2d_t*)cmd_table->cfis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1; // Command
    fis->command = command;
    
    // Set LBA and count
    if (device->lba48_supported && (lba > 0xFFFFFFF || count > 256)) {
        // 48-bit LBA
        fis->lba0 = lba & 0xFF;
        fis->lba1 = (lba >> 8) & 0xFF;
        fis->lba2 = (lba >> 16) & 0xFF;
        fis->lba3 = (lba >> 24) & 0xFF;
        fis->lba4 = (lba >> 32) & 0xFF;
        fis->lba5 = (lba >> 40) & 0xFF;
        fis->device = 1 << 6; // LBA mode
        fis->countl = count & 0xFF;
        fis->counth = (count >> 8) & 0xFF;
    } else {
        // 28-bit LBA
        fis->lba0 = lba & 0xFF;
        fis->lba1 = (lba >> 8) & 0xFF;
        fis->lba2 = (lba >> 16) & 0xFF;
        fis->device = ((lba >> 24) & 0x0F) | (1 << 6); // LBA mode
        fis->countl = count & 0xFF;
    }
    
    // Set up PRD entry
    if (buffer && count > 0) {
        cmd_table->prdt_entry[0].dba = (uint32_t)(uintptr_t)buffer;
        if (controller->64bit_addressing) {
            cmd_table->prdt_entry[0].dbau = (uint32_t)((uintptr_t)buffer >> 32);
        }
        cmd_table->prdt_entry[0].dbc = (count * device->sector_size) - 1;
        cmd_table->prdt_entry[0].i = 1; // Interrupt on completion
    }
    
    // Issue command
    *(volatile uint32_t*)(port_base + AHCI_PORT_CI) = 1 << slot;
    
    // Wait for completion (simplified - should use interrupts)
    uint32_t timeout = sata_subsystem.command_timeout;
    while (timeout-- > 0) {
        if (!(*(volatile uint32_t*)(port_base + AHCI_PORT_CI) & (1 << slot))) {
            break; // Command completed
        }
        // Small delay
        for (volatile int i = 0; i < 1000; i++);
    }
    
    if (timeout == 0) {
        printf("SATA: Command timeout on port %u\n", port);
        return ERROR_TIMEOUT;
    }
    
    // Check for errors
    uint32_t is = *(volatile uint32_t*)(port_base + AHCI_PORT_IS);
    if (is & 0x7D800000) { // Error bits
        printf("SATA: Command error on port %u, IS=0x%08X\n", port, is);
        return ERROR_IO_ERROR;
    }
    
    // Update statistics
    device->read_count++;
    device->bytes_read += count * device->sector_size;
    
    return ERROR_SUCCESS;
}

// SATA interrupt handler
static void sata_interrupt_handler(interrupt_frame_t* frame) {
    // Handle SATA interrupts
    // This is a simplified implementation
    
    sata_controller_t* controller = sata_subsystem.controller_list;
    while (controller) {
        if (controller->initialized) {
            // Check global interrupt status
            uint32_t is = *(volatile uint32_t*)(controller->memory_base + AHCI_HBA_IS);
            if (is) {
                // Clear global interrupt
                *(volatile uint32_t*)(controller->memory_base + AHCI_HBA_IS) = is;
                
                // Handle port interrupts
                for (uint8_t port = 0; port < controller->num_ports; port++) {
                    if (is & (1 << port)) {
                        uint32_t port_base = controller->memory_base + 0x100 + (port * 0x80);
                        uint32_t port_is = *(volatile uint32_t*)(port_base + AHCI_PORT_IS);
                        
                        // Clear port interrupt
                        *(volatile uint32_t*)(port_base + AHCI_PORT_IS) = port_is;
                        
                        // Handle specific interrupt types
                        if (port_is & 0x7D800000) {
                            // Error occurred
                            if (controller->devices[port]) {
                                controller->devices[port]->error_count++;
                            }
                        }
                    }
                }
            }
        }
        controller = controller->next;
    }
}

// Utility functions
const char* sata_device_type_to_string(sata_device_type_t type) {
    switch (type) {
        case SATA_DEVICE_TYPE_NONE: return "None";
        case SATA_DEVICE_TYPE_SATA: return "SATA";
        case SATA_DEVICE_TYPE_SATAPI: return "SATAPI";
        case SATA_DEVICE_TYPE_SEMB: return "SEMB";
        case SATA_DEVICE_TYPE_PM: return "Port Multiplier";
        default: return "Unknown";
    }
}

const char* sata_device_state_to_string(sata_device_state_t state) {
    switch (state) {
        case SATA_DEVICE_STATE_UNKNOWN: return "Unknown";
        case SATA_DEVICE_STATE_PRESENT: return "Present";
        case SATA_DEVICE_STATE_ACTIVE: return "Active";
        case SATA_DEVICE_STATE_OFFLINE: return "Offline";
        case SATA_DEVICE_STATE_ERROR: return "Error";
        default: return "Invalid";
    }
}

const char* sata_speed_to_string(sata_speed_t speed) {
    switch (speed) {
        case SATA_SPEED_GEN1: return "Gen1 (1.5 Gbps)";
        case SATA_SPEED_GEN2: return "Gen2 (3.0 Gbps)";
        case SATA_SPEED_GEN3: return "Gen3 (6.0 Gbps)";
        case SATA_SPEED_GEN4: return "Gen4 (12.0 Gbps)";
        default: return "Unknown";
    }
}

uint32_t sata_speed_to_mbps(sata_speed_t speed) {
    switch (speed) {
        case SATA_SPEED_GEN1: return 1500;
        case SATA_SPEED_GEN2: return 3000;
        case SATA_SPEED_GEN3: return 6000;
        case SATA_SPEED_GEN4: return 12000;
        default: return 0;
    }
}

// Power management functions
error_t sata_standby_device(sata_device_t* device) {
    if (!device) {
        return ERROR_INVALID_PARAMETER;
    }
    
    return sata_send_command(device, ATA_CMD_STANDBY_IMMEDIATE, 0, 0, NULL, false);
}

error_t sata_idle_device(sata_device_t* device) {
    if (!device) {
        return ERROR_INVALID_PARAMETER;
    }
    
    return sata_send_command(device, ATA_CMD_IDLE_IMMEDIATE, 0, 0, NULL, false);
}

error_t sata_flush_cache(sata_device_t* device) {
    if (!device) {
        return ERROR_INVALID_PARAMETER;
    }
    
    uint8_t command = device->lba48_supported ? ATA_CMD_FLUSH_CACHE_EXT : ATA_CMD_FLUSH_CACHE;
    return sata_send_command(device, command, 0, 0, NULL, false);
}

// Get device list
sata_device_t* sata_get_device_list(void) {
    return sata_subsystem.device_list;
}