#include "bluetooth.h"
#include "../core/memory.h"
#include "../core/string.h"
#include "../core/debug.h"
#include "../hal/usb.h"
#include "../hal/interrupts.h"
#include "../core/timer.h"
#include <stddef.h>

// Global Bluetooth subsystem
static bt_subsystem_t g_bt_subsystem = {0};

// Common Bluetooth USB vendor/product IDs
static const struct {
    uint16_t vendor_id;
    uint16_t product_id;
    const char* name;
} bt_devices[] = {
    // Intel Bluetooth devices
    {0x8087, 0x0A2A, "Intel Bluetooth 9460/9560"},
    {0x8087, 0x0A2B, "Intel Bluetooth 9460/9560"},
    {0x8087, 0x0AA7, "Intel Bluetooth AX200"},
    {0x8087, 0x0AAA, "Intel Bluetooth AX201"},
    {0x8087, 0x0025, "Intel Bluetooth AX210"},
    {0x8087, 0x0026, "Intel Bluetooth AX211"},
    {0x8087, 0x0029, "Intel Bluetooth AX1650"},
    {0x8087, 0x002A, "Intel Bluetooth AX1675"},
    {0x8087, 0x0032, "Intel Bluetooth 7260"},
    {0x8087, 0x07DA, "Intel Bluetooth 7265"},
    {0x8087, 0x07DC, "Intel Bluetooth 3168"},
    {0x8087, 0x0A2A, "Intel Bluetooth 8265"},
    {0x8087, 0x0A2B, "Intel Bluetooth 8265"},
    
    // Broadcom Bluetooth devices
    {0x0A5C, 0x21E1, "Broadcom BCM20702A0"},
    {0x0A5C, 0x21E3, "Broadcom BCM20702A0"},
    {0x0A5C, 0x21E6, "Broadcom BCM20702A0"},
    {0x0A5C, 0x21E8, "Broadcom BCM20702A1"},
    {0x0A5C, 0x21EC, "Broadcom BCM20702A1"},
    {0x0A5C, 0x21F1, "Broadcom BCM20702A1"},
    {0x0A5C, 0x21F3, "Broadcom BCM20702A1"},
    {0x0A5C, 0x21F4, "Broadcom BCM20702A1"},
    {0x0A5C, 0x21FB, "Broadcom BCM20702A1"},
    {0x0A5C, 0x21FD, "Broadcom BCM20702A1"},
    {0x0A5C, 0x640B, "Broadcom BCM20703A1"},
    {0x0A5C, 0x6410, "Broadcom BCM20703A1"},
    {0x0A5C, 0x6412, "Broadcom BCM4377"},
    {0x0A5C, 0x6414, "Broadcom BCM4377"},
    
    // Realtek Bluetooth devices
    {0x0BDA, 0x8723, "Realtek RTL8723AE"},
    {0x0BDA, 0x8761, "Realtek RTL8761A"},
    {0x0BDA, 0x8771, "Realtek RTL8723B"},
    {0x0BDA, 0x8821, "Realtek RTL8821A"},
    {0x0BDA, 0x8822, "Realtek RTL8822B"},
    {0x0BDA, 0x B82C, "Realtek RTL8822C"},
    {0x0BDA, 0xC822, "Realtek RTL8822C"},
    {0x0BDA, 0xC82C, "Realtek RTL8822C"},
    {0x0BDA, 0x8852, "Realtek RTL8852A"},
    {0x0BDA, 0xC852, "Realtek RTL8852A"},
    
    // Qualcomm Atheros Bluetooth devices
    {0x0CF3, 0x3000, "Atheros AR3011"},
    {0x0CF3, 0x3002, "Atheros AR3011"},
    {0x0CF3, 0x3004, "Atheros AR3012"},
    {0x0CF3, 0x3005, "Atheros AR3012"},
    {0x0CF3, 0x3008, "Atheros AR3012"},
    {0x0CF3, 0x311D, "Atheros AR3012"},
    {0x0CF3, 0x311E, "Atheros AR3012"},
    {0x0CF3, 0x311F, "Atheros AR3012"},
    {0x0CF3, 0x3121, "Atheros AR3012"},
    {0x0CF3, 0xE003, "Atheros AR3012"},
    {0x0CF3, 0xE004, "Atheros AR3012"},
    {0x0CF3, 0xE005, "Atheros AR3012"},
    
    // Cambridge Silicon Radio (CSR)
    {0x0A12, 0x0001, "CSR BlueCore"},
    {0x0A12, 0x0002, "CSR BlueCore"},
    {0x0A12, 0x0003, "CSR BlueCore"},
    {0x0A12, 0x0004, "CSR BlueCore"},
    {0x0A12, 0x0005, "CSR BlueCore"},
    {0x0A12, 0x1000, "CSR Bluetooth"},
    {0x0A12, 0x1001, "CSR Bluetooth"},
    {0x0A12, 0x1002, "CSR Bluetooth"},
    
    {0, 0, NULL} // End marker
};

// Event callback list
static bt_event_callback_t event_callbacks[16];
static uint32_t num_event_callbacks = 0;

// Forward declarations
static error_t bt_detect_adapters(void);
static bt_adapter_t* bt_create_adapter(uint16_t vendor_id, uint16_t product_id);
static void bt_destroy_adapter(bt_adapter_t* adapter);
static const char* bt_get_device_name(uint16_t vendor_id, uint16_t product_id);

// Initialize Bluetooth subsystem
error_t bluetooth_init(void) {
    debug_print("Bluetooth: Initializing Bluetooth subsystem\n");
    
    if (g_bt_subsystem.initialized) {
        return ERROR_ALREADY_INITIALIZED;
    }
    
    // Initialize subsystem structure
    memset(&g_bt_subsystem, 0, sizeof(bt_subsystem_t));
    
    // Initialize mutexes
    mutex_init(&g_bt_subsystem.adapter_mutex);
    mutex_init(&g_bt_subsystem.discovery_mutex);
    
    // Set default configuration
    g_bt_subsystem.auto_power_on = true;
    g_bt_subsystem.scan_timeout = 10000;     // 10 seconds
    g_bt_subsystem.connect_timeout = 30000;  // 30 seconds
    g_bt_subsystem.pair_timeout = 60000;     // 60 seconds
    
    // Detect Bluetooth adapters
    error_t result = bt_detect_adapters();
    if (result != ERROR_SUCCESS) {
        debug_print("Bluetooth: Failed to detect adapters: %d\n", result);
        return result;
    }
    
    g_bt_subsystem.initialized = true;
    debug_print("Bluetooth: Subsystem initialized successfully\n");
    debug_print("Bluetooth: Found %u Bluetooth adapter(s)\n", g_bt_subsystem.num_adapters);
    
    return ERROR_SUCCESS;
}

// Shutdown Bluetooth subsystem
void bluetooth_shutdown(void) {
    if (!g_bt_subsystem.initialized) {
        return;
    }
    
    debug_print("Bluetooth: Shutting down Bluetooth subsystem\n");
    
    // Power off all adapters and clean up
    mutex_lock(&g_bt_subsystem.adapter_mutex);
    
    bt_adapter_t* adapter = g_bt_subsystem.adapter_list;
    while (adapter) {
        bt_adapter_t* next = adapter->next;
        
        // Stop discovery if active
        if (adapter->state == BT_STATE_SCANNING) {
            bt_stop_discovery(adapter);
        }
        
        // Power off adapter
        if (adapter->powered) {
            bt_power_off(adapter);
        }
        
        bt_destroy_adapter(adapter);
        adapter = next;
    }
    
    g_bt_subsystem.adapter_list = NULL;
    g_bt_subsystem.num_adapters = 0;
    g_bt_subsystem.default_adapter = NULL;
    
    mutex_unlock(&g_bt_subsystem.adapter_mutex);
    
    // Clear event callbacks
    num_event_callbacks = 0;
    
    g_bt_subsystem.initialized = false;
    debug_print("Bluetooth: Subsystem shutdown complete\n");
}

// Detect Bluetooth adapters via USB scan
static error_t bt_detect_adapters(void) {
    debug_print("Bluetooth: Scanning for Bluetooth adapters\n");
    
    // Get USB device list
    usb_device_t* usb_devices = usb_get_device_list();
    usb_device_t* current = usb_devices;
    
    while (current) {
        // Check if this is a Bluetooth device (class 0xE0, subclass 0x01, protocol 0x01)
        if (current->device_class == 0xE0 && 
            current->device_subclass == 0x01 && 
            current->device_protocol == 0x01) {
            
            // Check if it's a known Bluetooth device
            const char* device_name = bt_get_device_name(current->vendor_id, current->product_id);
            if (device_name) {
                debug_print("Bluetooth: Found %s (VID:PID %04x:%04x)\n", 
                          device_name, current->vendor_id, current->product_id);
                
                bt_adapter_t* adapter = bt_create_adapter(current->vendor_id, current->product_id);
                if (adapter) {
                    bt_register_adapter(adapter);
                }
            }
        }
        current = current->next;
    }
    
    return ERROR_SUCCESS;
}

// Get device name from vendor/product ID
static const char* bt_get_device_name(uint16_t vendor_id, uint16_t product_id) {
    for (int i = 0; bt_devices[i].name != NULL; i++) {
        if (bt_devices[i].vendor_id == vendor_id && 
            bt_devices[i].product_id == product_id) {
            return bt_devices[i].name;
        }
    }
    return NULL;
}

// Create Bluetooth adapter structure
static bt_adapter_t* bt_create_adapter(uint16_t vendor_id, uint16_t product_id) {
    bt_adapter_t* adapter = (bt_adapter_t*)kmalloc(sizeof(bt_adapter_t));
    if (!adapter) {
        return NULL;
    }
    
    memset(adapter, 0, sizeof(bt_adapter_t));
    
    // Initialize base device
    adapter->base.type = DEVICE_TYPE_COMMUNICATION;
    adapter->base.subtype = 0x01; // Bluetooth
    adapter->base.vendor_id = vendor_id;
    adapter->base.device_id = product_id;
    
    // Set adapter information
    const char* device_name = bt_get_device_name(vendor_id, product_id);
    if (device_name) {
        strncpy(adapter->name, device_name, sizeof(adapter->name) - 1);
    } else {
        snprintf(adapter->name, sizeof(adapter->name), "Bluetooth Adapter %04x:%04x", 
                vendor_id, product_id);
    }
    
    // Generate a Bluetooth address (in real implementation, read from hardware)
    adapter->address.addr[0] = 0x00;
    adapter->address.addr[1] = 0x1A;
    adapter->address.addr[2] = 0x7D;
    adapter->address.addr[3] = (vendor_id >> 8) & 0xFF;
    adapter->address.addr[4] = vendor_id & 0xFF;
    adapter->address.addr[5] = product_id & 0xFF;
    
    // Set friendly name
    strcpy(adapter->friendly_name, "RaeenOS Bluetooth");
    
    // Initialize state
    adapter->state = BT_STATE_POWERED_OFF;
    adapter->powered = false;
    adapter->discoverable = false;
    adapter->pairable = true;
    adapter->discoverable_timeout = 0; // No timeout
    adapter->pairable_timeout = 0;     // No timeout
    
    // Set hardware info
    adapter->vendor_id = vendor_id;
    adapter->product_id = product_id;
    
    // Initialize capabilities (generic defaults)
    adapter->capabilities.version = BT_VERSION_5_0;
    adapter->capabilities.supports_le = true;
    adapter->capabilities.supports_edr = true;
    adapter->capabilities.supports_hs = false;
    adapter->capabilities.supports_sco = true;
    adapter->capabilities.supports_esco = true;
    adapter->capabilities.max_connections = 7;
    adapter->capabilities.supported_profiles = 
        (1 << 0) |  // A2DP
        (1 << 1) |  // AVRCP
        (1 << 2) |  // HFP
        (1 << 3) |  // HSP
        (1 << 4) |  // HID
        (1 << 5) |  // SPP
        (1 << 6) |  // OPP
        (1 << 7);   // PAN
    adapter->capabilities.max_tx_power = 10; // 10 dBm
    adapter->capabilities.min_tx_power = -20; // -20 dBm
    
    debug_print("Bluetooth: Created adapter %s (Address: %02x:%02x:%02x:%02x:%02x:%02x)\n",
              adapter->name,
              adapter->address.addr[0], adapter->address.addr[1], adapter->address.addr[2],
              adapter->address.addr[3], adapter->address.addr[4], adapter->address.addr[5]);
    
    return adapter;
}

// Destroy Bluetooth adapter
static void bt_destroy_adapter(bt_adapter_t* adapter) {
    if (!adapter) {
        return;
    }
    
    // Free device lists
    if (adapter->paired_devices) {
        kfree(adapter->paired_devices);
    }
    if (adapter->discovered_devices) {
        kfree(adapter->discovered_devices);
    }
    
    kfree(adapter);
}

// Register Bluetooth adapter
error_t bt_register_adapter(bt_adapter_t* adapter) {
    if (!adapter) {
        return ERROR_INVALID_PARAMETER;
    }
    
    mutex_lock(&g_bt_subsystem.adapter_mutex);
    
    // Add to adapter list
    adapter->next = g_bt_subsystem.adapter_list;
    g_bt_subsystem.adapter_list = adapter;
    g_bt_subsystem.num_adapters++;
    
    // Set as default if first adapter
    if (!g_bt_subsystem.default_adapter) {
        g_bt_subsystem.default_adapter = adapter;
    }
    
    mutex_unlock(&g_bt_subsystem.adapter_mutex);
    
    // Auto power on if enabled
    if (g_bt_subsystem.auto_power_on) {
        bt_power_on(adapter);
    }
    
    // Notify event
    bt_event_t event = {
        .type = BT_EVENT_ADAPTER_ADDED,
        .adapter = adapter,
        .device = NULL,
        .data = NULL,
        .data_size = 0,
        .timestamp = timer_get_ticks()
    };
    bt_notify_event(&event);
    
    debug_print("Bluetooth: Registered adapter %s\n", adapter->name);
    return ERROR_SUCCESS;
}

// Unregister Bluetooth adapter
error_t bt_unregister_adapter(bt_adapter_t* adapter) {
    if (!adapter) {
        return ERROR_INVALID_PARAMETER;
    }
    
    mutex_lock(&g_bt_subsystem.adapter_mutex);
    
    // Remove from adapter list
    if (g_bt_subsystem.adapter_list == adapter) {
        g_bt_subsystem.adapter_list = adapter->next;
    } else {
        bt_adapter_t* current = g_bt_subsystem.adapter_list;
        while (current && current->next != adapter) {
            current = current->next;
        }
        if (current) {
            current->next = adapter->next;
        }
    }
    
    g_bt_subsystem.num_adapters--;
    
    // Update default adapter if necessary
    if (g_bt_subsystem.default_adapter == adapter) {
        g_bt_subsystem.default_adapter = g_bt_subsystem.adapter_list;
    }
    
    mutex_unlock(&g_bt_subsystem.adapter_mutex);
    
    // Notify event
    bt_event_t event = {
        .type = BT_EVENT_ADAPTER_REMOVED,
        .adapter = adapter,
        .device = NULL,
        .data = NULL,
        .data_size = 0,
        .timestamp = timer_get_ticks()
    };
    bt_notify_event(&event);
    
    debug_print("Bluetooth: Unregistered adapter %s\n", adapter->name);
    return ERROR_SUCCESS;
}

// Find Bluetooth adapter by name
bt_adapter_t* bt_find_adapter(const char* name) {
    if (!name) {
        return NULL;
    }
    
    mutex_lock(&g_bt_subsystem.adapter_mutex);
    
    bt_adapter_t* adapter = g_bt_subsystem.adapter_list;
    while (adapter) {
        if (strcmp(adapter->name, name) == 0) {
            mutex_unlock(&g_bt_subsystem.adapter_mutex);
            return adapter;
        }
        adapter = adapter->next;
    }
    
    mutex_unlock(&g_bt_subsystem.adapter_mutex);
    return NULL;
}

// Get default Bluetooth adapter
bt_adapter_t* bt_get_default_adapter(void) {
    return g_bt_subsystem.default_adapter;
}

// Set default Bluetooth adapter
error_t bt_set_default_adapter(bt_adapter_t* adapter) {
    if (!adapter) {
        return ERROR_INVALID_PARAMETER;
    }
    
    mutex_lock(&g_bt_subsystem.adapter_mutex);
    g_bt_subsystem.default_adapter = adapter;
    mutex_unlock(&g_bt_subsystem.adapter_mutex);
    
    debug_print("Bluetooth: Set default adapter to %s\n", adapter->name);
    return ERROR_SUCCESS;
}

// Power on Bluetooth adapter
error_t bt_power_on(bt_adapter_t* adapter) {
    if (!adapter) {
        return ERROR_INVALID_PARAMETER;
    }
    
    if (adapter->powered) {
        return ERROR_SUCCESS;
    }
    
    debug_print("Bluetooth: Powering on adapter %s\n", adapter->name);
    
    // Power on adapter hardware (implementation specific)
    if (adapter->power_on) {
        error_t result = adapter->power_on(adapter);
        if (result != ERROR_SUCCESS) {
            debug_print("Bluetooth: Failed to power on adapter %s: %d\n", adapter->name, result);
            return result;
        }
    }
    
    adapter->powered = true;
    adapter->state = BT_STATE_POWERED_ON;
    
    // Notify event
    bt_event_t event = {
        .type = BT_EVENT_ADAPTER_POWERED_ON,
        .adapter = adapter,
        .device = NULL,
        .data = NULL,
        .data_size = 0,
        .timestamp = timer_get_ticks()
    };
    bt_notify_event(&event);
    
    debug_print("Bluetooth: Adapter %s powered on successfully\n", adapter->name);
    return ERROR_SUCCESS;
}

// Power off Bluetooth adapter
error_t bt_power_off(bt_adapter_t* adapter) {
    if (!adapter) {
        return ERROR_INVALID_PARAMETER;
    }
    
    if (!adapter->powered) {
        return ERROR_SUCCESS;
    }
    
    debug_print("Bluetooth: Powering off adapter %s\n", adapter->name);
    
    // Stop discovery if active
    if (adapter->state == BT_STATE_SCANNING) {
        bt_stop_discovery(adapter);
    }
    
    // Power off adapter hardware (implementation specific)
    if (adapter->power_off) {
        error_t result = adapter->power_off(adapter);
        if (result != ERROR_SUCCESS) {
            debug_print("Bluetooth: Failed to power off adapter %s: %d\n", adapter->name, result);
            return result;
        }
    }
    
    adapter->powered = false;
    adapter->state = BT_STATE_POWERED_OFF;
    
    // Notify event
    bt_event_t event = {
        .type = BT_EVENT_ADAPTER_POWERED_OFF,
        .adapter = adapter,
        .device = NULL,
        .data = NULL,
        .data_size = 0,
        .timestamp = timer_get_ticks()
    };
    bt_notify_event(&event);
    
    debug_print("Bluetooth: Adapter %s powered off successfully\n", adapter->name);
    return ERROR_SUCCESS;
}

// Check if adapter is powered
bool bt_is_powered(bt_adapter_t* adapter) {
    if (!adapter) {
        return false;
    }
    return adapter->powered;
}

// Start device discovery
error_t bt_start_discovery(bt_adapter_t* adapter) {
    if (!adapter || !adapter->powered) {
        return ERROR_INVALID_PARAMETER;
    }
    
    debug_print("Bluetooth: Starting discovery on adapter %s\n", adapter->name);
    
    mutex_lock(&g_bt_subsystem.discovery_mutex);
    
    adapter->state = BT_STATE_SCANNING;
    
    // Start discovery (implementation specific)
    if (adapter->start_discovery) {
        error_t result = adapter->start_discovery(adapter);
        if (result != ERROR_SUCCESS) {
            adapter->state = BT_STATE_POWERED_ON;
            mutex_unlock(&g_bt_subsystem.discovery_mutex);
            debug_print("Bluetooth: Discovery failed on adapter %s: %d\n", adapter->name, result);
            return result;
        }
    } else {
        // Simulate discovery results for demonstration
        adapter->num_discovered = 2;
        adapter->discovered_devices = (bt_device_info_t*)kmalloc(sizeof(bt_device_info_t) * 2);
        
        // Example devices
        bt_address_t addr1 = {{0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}};
        bt_address_copy(&adapter->discovered_devices[0].address, &addr1);
        strcpy(adapter->discovered_devices[0].name, "Bluetooth Mouse");
        adapter->discovered_devices[0].device_class = BT_CLASS_PERIPHERAL;
        adapter->discovered_devices[0].rssi = -45;
        adapter->discovered_devices[0].paired = false;
        adapter->discovered_devices[0].connected = false;
        
        bt_address_t addr2 = {{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}};
        bt_address_copy(&adapter->discovered_devices[1].address, &addr2);
        strcpy(adapter->discovered_devices[1].name, "Bluetooth Headphones");
        adapter->discovered_devices[1].device_class = BT_CLASS_AUDIO_VIDEO;
        adapter->discovered_devices[1].rssi = -60;
        adapter->discovered_devices[1].paired = false;
        adapter->discovered_devices[1].connected = false;
    }
    
    mutex_unlock(&g_bt_subsystem.discovery_mutex);
    
    // Notify discovery started
    bt_event_t event = {
        .type = BT_EVENT_DISCOVERY_STARTED,
        .adapter = adapter,
        .device = NULL,
        .data = NULL,
        .data_size = 0,
        .timestamp = timer_get_ticks()
    };
    bt_notify_event(&event);
    
    debug_print("Bluetooth: Discovery started on adapter %s\n", adapter->name);
    return ERROR_SUCCESS;
}

// Stop device discovery
error_t bt_stop_discovery(bt_adapter_t* adapter) {
    if (!adapter) {
        return ERROR_INVALID_PARAMETER;
    }
    
    if (adapter->state != BT_STATE_SCANNING) {
        return ERROR_SUCCESS;
    }
    
    debug_print("Bluetooth: Stopping discovery on adapter %s\n", adapter->name);
    
    // Stop discovery (implementation specific)
    if (adapter->stop_discovery) {
        error_t result = adapter->stop_discovery(adapter);
        if (result != ERROR_SUCCESS) {
            debug_print("Bluetooth: Failed to stop discovery on adapter %s: %d\n", 
                      adapter->name, result);
            return result;
        }
    }
    
    adapter->state = BT_STATE_POWERED_ON;
    
    // Notify discovery stopped
    bt_event_t event = {
        .type = BT_EVENT_DISCOVERY_STOPPED,
        .adapter = adapter,
        .device = NULL,
        .data = adapter->discovered_devices,
        .data_size = adapter->num_discovered * sizeof(bt_device_info_t),
        .timestamp = timer_get_ticks()
    };
    bt_notify_event(&event);
    
    debug_print("Bluetooth: Discovery stopped on adapter %s, found %u devices\n", 
              adapter->name, adapter->num_discovered);
    return ERROR_SUCCESS;
}

// Check if adapter is discovering
bool bt_is_discovering(bt_adapter_t* adapter) {
    if (!adapter) {
        return false;
    }
    return adapter->state == BT_STATE_SCANNING;
}

// Get discovered devices
bt_device_info_t* bt_get_discovered_devices(bt_adapter_t* adapter, uint32_t* count) {
    if (!adapter || !count) {
        return NULL;
    }
    
    *count = adapter->num_discovered;
    return adapter->discovered_devices;
}

// Register event callback
error_t bt_register_event_callback(bt_event_callback_t callback) {
    if (!callback || num_event_callbacks >= 16) {
        return ERROR_INVALID_PARAMETER;
    }
    
    event_callbacks[num_event_callbacks++] = callback;
    return ERROR_SUCCESS;
}

// Notify Bluetooth event
void bt_notify_event(const bt_event_t* event) {
    if (!event) {
        return;
    }
    
    for (uint32_t i = 0; i < num_event_callbacks; i++) {
        event_callbacks[i](event);
    }
}

// Utility functions
const char* bt_version_to_string(bt_version_t version) {
    switch (version) {
        case BT_VERSION_1_0: return "1.0";
        case BT_VERSION_1_1: return "1.1";
        case BT_VERSION_1_2: return "1.2";
        case BT_VERSION_2_0_EDR: return "2.0 + EDR";
        case BT_VERSION_2_1_EDR: return "2.1 + EDR";
        case BT_VERSION_3_0_HS: return "3.0 + HS";
        case BT_VERSION_4_0_LE: return "4.0 LE";
        case BT_VERSION_4_1: return "4.1";
        case BT_VERSION_4_2: return "4.2";
        case BT_VERSION_5_0: return "5.0";
        case BT_VERSION_5_1: return "5.1";
        case BT_VERSION_5_2: return "5.2";
        case BT_VERSION_5_3: return "5.3";
        case BT_VERSION_5_4: return "5.4";
        default: return "Unknown";
    }
}

const char* bt_state_to_string(bt_state_t state) {
    switch (state) {
        case BT_STATE_POWERED_OFF: return "Powered Off";
        case BT_STATE_POWERED_ON: return "Powered On";
        case BT_STATE_DISCOVERABLE: return "Discoverable";
        case BT_STATE_SCANNING: return "Scanning";
        case BT_STATE_CONNECTING: return "Connecting";
        case BT_STATE_CONNECTED: return "Connected";
        case BT_STATE_DISCONNECTING: return "Disconnecting";
        case BT_STATE_PAIRING: return "Pairing";
        case BT_STATE_PAIRED: return "Paired";
        case BT_STATE_ERROR: return "Error";
        default: return "Unknown";
    }
}

const char* bt_device_class_to_string(bt_device_class_t class) {
    switch (class) {
        case BT_CLASS_MISCELLANEOUS: return "Miscellaneous";
        case BT_CLASS_COMPUTER: return "Computer";
        case BT_CLASS_PHONE: return "Phone";
        case BT_CLASS_LAN_ACCESS: return "LAN/Network Access";
        case BT_CLASS_AUDIO_VIDEO: return "Audio/Video";
        case BT_CLASS_PERIPHERAL: return "Peripheral";
        case BT_CLASS_IMAGING: return "Imaging";
        case BT_CLASS_WEARABLE: return "Wearable";
        case BT_CLASS_TOY: return "Toy";
        case BT_CLASS_HEALTH: return "Health";
        case BT_CLASS_UNCATEGORIZED: return "Uncategorized";
        default: return "Unknown";
    }
}

// Convert Bluetooth address to string
void bt_address_to_string(const bt_address_t* address, char* str) {
    if (!address || !str) {
        return;
    }
    
    snprintf(str, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
            address->addr[0], address->addr[1], address->addr[2],
            address->addr[3], address->addr[4], address->addr[5]);
}

// Compare Bluetooth addresses
bool bt_address_equal(const bt_address_t* addr1, const bt_address_t* addr2) {
    if (!addr1 || !addr2) {
        return false;
    }
    
    return memcmp(addr1->addr, addr2->addr, 6) == 0;
}

// Copy Bluetooth address
void bt_address_copy(bt_address_t* dest, const bt_address_t* src) {
    if (!dest || !src) {
        return;
    }
    
    memcpy(dest->addr, src->addr, 6);
}