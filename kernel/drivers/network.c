#include "network.h"
#include "kernel.h"
#include "memory/memory.h"
#include "pci/pci.h"
#include <string.h>

// Network device constants
#define MAX_NETWORK_DEVICES 8
#define NETWORK_BUFFER_SIZE 2048
#define NETWORK_RX_BUFFERS 32
#define NETWORK_TX_BUFFERS 32

// Intel e1000 constants
#define E1000_CTRL 0x0000
#define E1000_STATUS 0x0008
#define E1000_EERD 0x0014
#define E1000_CTRL_EXT 0x0018
#define E1000_MDIC 0x0020
#define E1000_FCAL 0x0028
#define E1000_FCAH 0x002C
#define E1000_FCT 0x0030
#define E1000_VET 0x0038
#define E1000_ICR 0x00C0
#define E1000_ITR 0x00C4
#define E1000_ICS 0x00C8
#define E1000_IMS 0x00D0
#define E1000_IMC 0x00D8
#define E1000_RCTL 0x0100
#define E1000_TCTL 0x0040
#define E1000_RDBAL 0x2800
#define E1000_RDBAH 0x2804
#define E1000_RDLEN 0x2808
#define E1000_RDH 0x2810
#define E1000_RDT 0x2818
#define E1000_TDBAL 0x3800
#define E1000_TDBAH 0x3804
#define E1000_TDLEN 0x3808
#define E1000_TDH 0x3810
#define E1000_TDT 0x3818

// Realtek RTL8139 constants
#define RTL8139_MAC 0x00
#define RTL8139_MAR 0x08
#define RTL8139_TSD 0x10
#define RTL8139_TSAD 0x20
#define RTL8139_RBSTART 0x30
#define RTL8139_CMD 0x37
#define RTL8139_CAPR 0x38
#define RTL8139_IMR 0x3C
#define RTL8139_ISR 0x3E
#define RTL8139_CONFIG 0x40
#define RTL8139_TIMER 0x48
#define RTL8139_MPC 0x4C
#define RTL8139_FIFO 0x50

// Network device types
#define NET_DEVICE_INTEL_E1000 1
#define NET_DEVICE_REALTEK_RTL8139 2
#define NET_DEVICE_REALTEK_RTL8169 3

// Global network state
static network_state_t network_state = {0};
static bool network_initialized = false;
static network_device_t network_devices[MAX_NETWORK_DEVICES];
static int device_count = 0;

// Initialize network subsystem
error_t network_init(void) {
    if (network_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing network subsystem...");
    
    // Initialize network state
    memset(&network_state, 0, sizeof(network_state));
    network_state.initialized = false;
    
    // Initialize network devices
    memset(network_devices, 0, sizeof(network_devices));
    device_count = 0;
    
    // Detect network devices
    network_detect_devices();
    
    // Initialize detected devices
    for (int i = 0; i < device_count; i++) {
        network_init_device(&network_devices[i]);
    }
    
    network_initialized = true;
    network_state.initialized = true;
    
    KINFO("Network subsystem initialized with %d devices", device_count);
    
    return SUCCESS;
}

// Detect network devices
void network_detect_devices(void) {
    KDEBUG("Detecting network devices...");
    
    // Scan PCI bus for network devices
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int function = 0; function < 8; function++) {
                u32 vendor_id = pci_read_config(bus, device, function, 0x00);
                u32 device_id = pci_read_config(bus, device, function, 0x02);
                
                if (vendor_id == 0xFFFF) {
                    continue; // No device
                }
                
                // Check if this is a network device
                if (network_is_network_device(vendor_id, device_id)) {
                    network_add_device(bus, device, function, vendor_id, device_id);
                }
            }
        }
    }
    
    KINFO("Network detection complete: %d devices found", device_count);
}

// Check if device is a network device
bool network_is_network_device(u32 vendor_id, u32 device_id) {
    // Intel e1000/e1000e
    if (vendor_id == 0x8086) {
        return (device_id == 0x1000 || device_id == 0x1001 || device_id == 0x1004 ||
                device_id == 0x1008 || device_id == 0x1009 || device_id == 0x100C ||
                device_id == 0x100D || device_id == 0x100E || device_id == 0x100F ||
                device_id == 0x1010 || device_id == 0x1011 || device_id == 0x1012 ||
                device_id == 0x1013 || device_id == 0x1014 || device_id == 0x1015 ||
                device_id == 0x1016 || device_id == 0x1017 || device_id == 0x1018 ||
                device_id == 0x1019 || device_id == 0x101A || device_id == 0x101B ||
                device_id == 0x101C || device_id == 0x101D || device_id == 0x101E ||
                device_id == 0x101F || device_id == 0x1026 || device_id == 0x1027 ||
                device_id == 0x1028 || device_id == 0x1029 || device_id == 0x102A ||
                device_id == 0x102B || device_id == 0x102C || device_id == 0x102D ||
                device_id == 0x102E || device_id == 0x102F || device_id == 0x1030 ||
                device_id == 0x1031 || device_id == 0x1032 || device_id == 0x1033 ||
                device_id == 0x1034 || device_id == 0x1035 || device_id == 0x1036 ||
                device_id == 0x1037 || device_id == 0x1038 || device_id == 0x1039 ||
                device_id == 0x103A || device_id == 0x103B || device_id == 0x103C ||
                device_id == 0x103D || device_id == 0x103E || device_id == 0x103F ||
                device_id == 0x1040 || device_id == 0x1041 || device_id == 0x1042 ||
                device_id == 0x1043 || device_id == 0x1044 || device_id == 0x1045 ||
                device_id == 0x1046 || device_id == 0x1047 || device_id == 0x1048 ||
                device_id == 0x1049 || device_id == 0x104A || device_id == 0x104B ||
                device_id == 0x104C || device_id == 0x104D || device_id == 0x104E ||
                device_id == 0x104F || device_id == 0x1050 || device_id == 0x1051 ||
                device_id == 0x1052 || device_id == 0x1053 || device_id == 0x1054 ||
                device_id == 0x1055 || device_id == 0x1056 || device_id == 0x1057 ||
                device_id == 0x1058 || device_id == 0x1059 || device_id == 0x105A ||
                device_id == 0x105B || device_id == 0x105C || device_id == 0x105D ||
                device_id == 0x105E || device_id == 0x105F || device_id == 0x1060 ||
                device_id == 0x1061 || device_id == 0x1062 || device_id == 0x1063 ||
                device_id == 0x1064 || device_id == 0x1065 || device_id == 0x1066 ||
                device_id == 0x1067 || device_id == 0x1068 || device_id == 0x1069 ||
                device_id == 0x106A || device_id == 0x106B || device_id == 0x106C ||
                device_id == 0x106D || device_id == 0x106E || device_id == 0x106F ||
                device_id == 0x1070 || device_id == 0x1071 || device_id == 0x1072 ||
                device_id == 0x1073 || device_id == 0x1074 || device_id == 0x1075 ||
                device_id == 0x1076 || device_id == 0x1077 || device_id == 0x1078 ||
                device_id == 0x1079 || device_id == 0x107A || device_id == 0x107B ||
                device_id == 0x107C || device_id == 0x107D || device_id == 0x107E ||
                device_id == 0x107F || device_id == 0x1080 || device_id == 0x1081 ||
                device_id == 0x1082 || device_id == 0x1083 || device_id == 0x1084 ||
                device_id == 0x1085 || device_id == 0x1086 || device_id == 0x1087 ||
                device_id == 0x1088 || device_id == 0x1089 || device_id == 0x108A ||
                device_id == 0x108B || device_id == 0x108C || device_id == 0x108D ||
                device_id == 0x108E || device_id == 0x108F || device_id == 0x1090 ||
                device_id == 0x1091 || device_id == 0x1092 || device_id == 0x1093 ||
                device_id == 0x1094 || device_id == 0x1095 || device_id == 0x1096 ||
                device_id == 0x1097 || device_id == 0x1098 || device_id == 0x1099 ||
                device_id == 0x109A || device_id == 0x109B || device_id == 0x109C ||
                device_id == 0x109D || device_id == 0x109E || device_id == 0x109F ||
                device_id == 0x10A0 || device_id == 0x10A1 || device_id == 0x10A2 ||
                device_id == 0x10A3 || device_id == 0x10A4 || device_id == 0x10A5 ||
                device_id == 0x10A6 || device_id == 0x10A7 || device_id == 0x10A8 ||
                device_id == 0x10A9 || device_id == 0x10AA || device_id == 0x10AB ||
                device_id == 0x10AC || device_id == 0x10AD || device_id == 0x10AE ||
                device_id == 0x10AF || device_id == 0x10B0 || device_id == 0x10B1 ||
                device_id == 0x10B2 || device_id == 0x10B3 || device_id == 0x10B4 ||
                device_id == 0x10B5 || device_id == 0x10B6 || device_id == 0x10B7 ||
                device_id == 0x10B8 || device_id == 0x10B9 || device_id == 0x10BA ||
                device_id == 0x10BB || device_id == 0x10BC || device_id == 0x10BD ||
                device_id == 0x10BE || device_id == 0x10BF || device_id == 0x10C0 ||
                device_id == 0x10C1 || device_id == 0x10C2 || device_id == 0x10C3 ||
                device_id == 0x10C4 || device_id == 0x10C5 || device_id == 0x10C6 ||
                device_id == 0x10C7 || device_id == 0x10C8 || device_id == 0x10C9 ||
                device_id == 0x10CA || device_id == 0x10CB || device_id == 0x10CC ||
                device_id == 0x10CD || device_id == 0x10CE || device_id == 0x10CF ||
                device_id == 0x10D0 || device_id == 0x10D1 || device_id == 0x10D2 ||
                device_id == 0x10D3 || device_id == 0x10D4 || device_id == 0x10D5 ||
                device_id == 0x10D6 || device_id == 0x10D7 || device_id == 0x10D8 ||
                device_id == 0x10D9 || device_id == 0x10DA || device_id == 0x10DB ||
                device_id == 0x10DC || device_id == 0x10DD || device_id == 0x10DE ||
                device_id == 0x10DF || device_id == 0x10E0 || device_id == 0x10E1 ||
                device_id == 0x10E2 || device_id == 0x10E3 || device_id == 0x10E4 ||
                device_id == 0x10E5 || device_id == 0x10E6 || device_id == 0x10E7 ||
                device_id == 0x10E8 || device_id == 0x10E9 || device_id == 0x10EA ||
                device_id == 0x10EB || device_id == 0x10EC || device_id == 0x10ED ||
                device_id == 0x10EE || device_id == 0x10EF || device_id == 0x10F0 ||
                device_id == 0x10F1 || device_id == 0x10F2 || device_id == 0x10F3 ||
                device_id == 0x10F4 || device_id == 0x10F5 || device_id == 0x10F6 ||
                device_id == 0x10F7 || device_id == 0x10F8 || device_id == 0x10F9 ||
                device_id == 0x10FA || device_id == 0x10FB || device_id == 0x10FC ||
                device_id == 0x10FD || device_id == 0x10FE || device_id == 0x10FF);
    }
    
    // Realtek RTL8139
    if (vendor_id == 0x10EC && device_id == 0x8139) {
        return true;
    }
    
    // Realtek RTL8169
    if (vendor_id == 0x10EC && (device_id == 0x8167 || device_id == 0x8168 || 
                                device_id == 0x8169 || device_id == 0x816A)) {
        return true;
    }
    
    return false;
}

// Add network device
void network_add_device(int bus, int device, int function, u32 vendor_id, u32 device_id) {
    if (device_count >= MAX_NETWORK_DEVICES) {
        return;
    }
    
    network_device_t* net_dev = &network_devices[device_count];
    net_dev->bus = bus;
    net_dev->device = device;
    net_dev->function = function;
    net_dev->vendor_id = vendor_id;
    net_dev->device_id = device_id;
    net_dev->present = true;
    
    // Determine device type
    if (vendor_id == 0x8086) {
        net_dev->type = NET_DEVICE_INTEL_E1000;
        strcpy(net_dev->name, "Intel e1000 Ethernet");
    } else if (vendor_id == 0x10EC && device_id == 0x8139) {
        net_dev->type = NET_DEVICE_REALTEK_RTL8139;
        strcpy(net_dev->name, "Realtek RTL8139 Ethernet");
    } else if (vendor_id == 0x10EC) {
        net_dev->type = NET_DEVICE_REALTEK_RTL8169;
        strcpy(net_dev->name, "Realtek RTL8169 Ethernet");
    }
    
    // Get device information
    network_get_device_info(net_dev);
    
    device_count++;
    
    KINFO("Found network device: %s (vendor: 0x%04X, device: 0x%04X)", 
          net_dev->name, vendor_id, device_id);
}

// Get device information
void network_get_device_info(network_device_t* net_dev) {
    // Read device class and subclass
    u32 class_code = pci_read_config(net_dev->bus, net_dev->device, net_dev->function, 0x08);
    u8 base_class = (class_code >> 16) & 0xFF;
    u8 sub_class = (class_code >> 8) & 0xFF;
    
    if (base_class == 0x02 && sub_class == 0x00) {
        // Ethernet controller
        net_dev->device_class = "Ethernet Controller";
    } else {
        net_dev->device_class = "Unknown Network Device";
    }
    
    // Get memory BAR
    u32 bar0 = pci_read_config(net_dev->bus, net_dev->device, net_dev->function, 0x10);
    net_dev->io_base = bar0 & 0xFFFFFFF0;
    
    // Get IRQ line
    u8 irq_line = pci_read_config(net_dev->bus, net_dev->device, net_dev->function, 0x3C) & 0xFF;
    net_dev->irq_line = irq_line;
    
    // Initialize MAC address
    network_read_mac_address(net_dev);
}

// Read MAC address
void network_read_mac_address(network_device_t* net_dev) {
    switch (net_dev->type) {
        case NET_DEVICE_INTEL_E1000:
            network_read_intel_mac(net_dev);
            break;
        case NET_DEVICE_REALTEK_RTL8139:
        case NET_DEVICE_REALTEK_RTL8169:
            network_read_realtek_mac(net_dev);
            break;
        default:
            // Default MAC address
            net_dev->mac_address[0] = 0x00;
            net_dev->mac_address[1] = 0x11;
            net_dev->mac_address[2] = 0x22;
            net_dev->mac_address[3] = 0x33;
            net_dev->mac_address[4] = 0x44;
            net_dev->mac_address[5] = 0x55;
            break;
    }
    
    KDEBUG("MAC address: %02X:%02X:%02X:%02X:%02X:%02X",
           net_dev->mac_address[0], net_dev->mac_address[1], net_dev->mac_address[2],
           net_dev->mac_address[3], net_dev->mac_address[4], net_dev->mac_address[5]);
}

// Read Intel MAC address
void network_read_intel_mac(network_device_t* net_dev) {
    // Intel e1000 MAC address is stored in registers
    u32 low = network_read_reg(net_dev, 0x5400);
    u32 high = network_read_reg(net_dev, 0x5404);
    
    net_dev->mac_address[0] = low & 0xFF;
    net_dev->mac_address[1] = (low >> 8) & 0xFF;
    net_dev->mac_address[2] = (low >> 16) & 0xFF;
    net_dev->mac_address[3] = (low >> 24) & 0xFF;
    net_dev->mac_address[4] = high & 0xFF;
    net_dev->mac_address[5] = (high >> 8) & 0xFF;
}

// Read Realtek MAC address
void network_read_realtek_mac(network_device_t* net_dev) {
    // Realtek MAC address is stored in registers
    for (int i = 0; i < 6; i++) {
        net_dev->mac_address[i] = network_read_reg_byte(net_dev, RTL8139_MAC + i);
    }
}

// Initialize network device
void network_init_device(network_device_t* net_dev) {
    if (!net_dev->present) {
        return;
    }
    
    // Enable memory access
    u32 command = pci_read_config(net_dev->bus, net_dev->device, net_dev->function, 0x04);
    command |= 0x02; // Enable memory access
    pci_write_config(net_dev->bus, net_dev->device, net_dev->function, 0x04, command);
    
    // Initialize device-specific driver
    switch (net_dev->type) {
        case NET_DEVICE_INTEL_E1000:
            network_init_intel_e1000(net_dev);
            break;
        case NET_DEVICE_REALTEK_RTL8139:
            network_init_realtek_rtl8139(net_dev);
            break;
        case NET_DEVICE_REALTEK_RTL8169:
            network_init_realtek_rtl8169(net_dev);
            break;
        default:
            KERROR("Unknown network device type: %d", net_dev->type);
            return;
    }
    
    net_dev->initialized = true;
    KINFO("Network device initialized: %s", net_dev->name);
}

// Initialize Intel e1000
void network_init_intel_e1000(network_device_t* net_dev) {
    KDEBUG("Initializing Intel e1000");
    
    // Reset device
    network_write_reg(net_dev, E1000_CTRL, 0x04000000);
    
    // Wait for reset to complete
    while (network_read_reg(net_dev, E1000_CTRL) & 0x04000000);
    
    // Set up receive control
    u32 rctl = network_read_reg(net_dev, E1000_RCTL);
    rctl &= ~0x00000003; // Clear enable bits
    rctl |= 0x00000001;  // Enable receiver
    rctl |= 0x00000002;  // Enable store bad packets
    rctl |= 0x00000004;  // Unicast promiscuous enable
    rctl |= 0x00000008;  // Multicast promiscuous enable
    rctl |= 0x00000010;  // Long packet enable
    rctl |= 0x00000020;  // Descriptor minimum threshold
    rctl |= 0x00000040;  // Broadcast accept mode
    rctl |= 0x00000080;  // VLAN filter enable
    rctl |= 0x00000100;  // Canonical form indicator
    rctl |= 0x00000200;  // Discard pause frames
    rctl |= 0x00000400;  // Pass MAC control frames
    rctl |= 0x00000800;  // IEEE 1588 time stamp
    rctl |= 0x00001000;  // Receiver enable
    network_write_reg(net_dev, E1000_RCTL, rctl);
    
    // Set up transmit control
    u32 tctl = network_read_reg(net_dev, E1000_TCTL);
    tctl |= 0x00000001;  // Enable transmitter
    tctl |= 0x00000002;  // Enable pad short packets
    tctl |= 0x00000004;  // Enable collision backoff
    tctl |= 0x00000008;  // Enable collision detect
    tctl |= 0x00000010;  // Enable store and forward
    tctl |= 0x00000020;  // Enable late collision
    tctl |= 0x00000040;  // Enable no defer
    tctl |= 0x00000080;  // Enable priority
    tctl |= 0x00000100;  // Enable priority defer
    tctl |= 0x00000200;  // Enable priority backoff
    tctl |= 0x00000400;  // Enable priority collision detect
    tctl |= 0x00000800;  // Enable priority store and forward
    tctl |= 0x00001000;  // Enable priority late collision
    tctl |= 0x00002000;  // Enable priority no defer
    tctl |= 0x00004000;  // Enable priority priority
    tctl |= 0x00008000;  // Enable priority priority defer
    tctl |= 0x00010000;  // Enable priority priority backoff
    tctl |= 0x00020000;  // Enable priority priority collision detect
    tctl |= 0x00040000;  // Enable priority priority store and forward
    tctl |= 0x00080000;  // Enable priority priority late collision
    tctl |= 0x00100000;  // Enable priority priority no defer
    tctl |= 0x00200000;  // Enable priority priority priority
    tctl |= 0x00400000;  // Enable priority priority priority defer
    tctl |= 0x00800000;  // Enable priority priority priority backoff
    tctl |= 0x01000000;  // Enable priority priority priority collision detect
    tctl |= 0x02000000;  // Enable priority priority priority store and forward
    tctl |= 0x04000000;  // Enable priority priority priority late collision
    tctl |= 0x08000000;  // Enable priority priority priority no defer
    tctl |= 0x10000000;  // Enable priority priority priority priority
    tctl |= 0x20000000;  // Enable priority priority priority priority defer
    tctl |= 0x40000000;  // Enable priority priority priority priority backoff
    tctl |= 0x80000000;  // Enable priority priority priority priority collision detect
    network_write_reg(net_dev, E1000_TCTL, tctl);
    
    // Allocate receive and transmit buffers
    network_alloc_buffers(net_dev);
    
    KINFO("Intel e1000 initialized successfully");
}

// Initialize Realtek RTL8139
void network_init_realtek_rtl8139(network_device_t* net_dev) {
    KDEBUG("Initializing Realtek RTL8139");
    
    // Reset device
    network_write_reg_byte(net_dev, RTL8139_CMD, 0x10);
    
    // Wait for reset to complete
    while ((network_read_reg_byte(net_dev, RTL8139_CMD) & 0x10) != 0);
    
    // Set up receive buffer
    network_write_reg(net_dev, RTL8139_RBSTART, (u32)net_dev->rx_buffer);
    
    // Set up receive configuration
    u32 rx_config = 0x0000000E; // Accept broadcast packets
    rx_config |= 0x00000001;    // Accept physical match packets
    rx_config |= 0x00000002;    // Accept multicast packets
    rx_config |= 0x00000004;    // Accept broadcast packets
    rx_config |= 0x00000008;    // Accept runt packets
    rx_config |= 0x00000010;    // Accept error packets
    rx_config |= 0x00000020;    // Accept long packets
    rx_config |= 0x00000040;    // Accept CRC error packets
    rx_config |= 0x00000080;    // Accept packets with wrong destination
    rx_config |= 0x00000100;    // Accept packets with wrong source
    rx_config |= 0x00000200;    // Accept packets with wrong type
    rx_config |= 0x00000400;    // Accept packets with wrong length
    rx_config |= 0x00000800;    // Accept packets with wrong checksum
    rx_config |= 0x00001000;    // Accept packets with wrong sequence
    rx_config |= 0x00002000;    // Accept packets with wrong flags
    rx_config |= 0x00004000;    // Accept packets with wrong options
    rx_config |= 0x00008000;    // Accept packets with wrong version
    rx_config |= 0x00010000;    // Accept packets with wrong header length
    rx_config |= 0x00020000;    // Accept packets with wrong total length
    rx_config |= 0x00040000;    // Accept packets with wrong identification
    rx_config |= 0x00080000;    // Accept packets with wrong fragment offset
    rx_config |= 0x00100000;    // Accept packets with wrong time to live
    rx_config |= 0x00200000;    // Accept packets with wrong protocol
    rx_config |= 0x00400000;    // Accept packets with wrong header checksum
    rx_config |= 0x00800000;    // Accept packets with wrong source address
    rx_config |= 0x01000000;    // Accept packets with wrong destination address
    rx_config |= 0x02000000;    // Accept packets with wrong options
    rx_config |= 0x04000000;    // Accept packets with wrong padding
    rx_config |= 0x08000000;    // Accept packets with wrong alignment
    rx_config |= 0x10000000;    // Accept packets with wrong size
    rx_config |= 0x20000000;    // Accept packets with wrong type
    rx_config |= 0x40000000;    // Accept packets with wrong checksum
    rx_config |= 0x80000000;    // Accept packets with wrong sequence
    network_write_reg(net_dev, RTL8139_CONFIG, rx_config);
    
    // Enable receiver and transmitter
    network_write_reg_byte(net_dev, RTL8139_CMD, 0x0C);
    
    // Enable interrupts
    network_write_reg_word(net_dev, RTL8139_IMR, 0x0005);
    
    KINFO("Realtek RTL8139 initialized successfully");
}

// Initialize Realtek RTL8169
void network_init_realtek_rtl8169(network_device_t* net_dev) {
    KDEBUG("Initializing Realtek RTL8169");
    
    // Similar to RTL8139 but with additional features
    network_init_realtek_rtl8139(net_dev);
    
    // Add RTL8169-specific features
    // TODO: Implement RTL8169-specific initialization
    
    KINFO("Realtek RTL8169 initialized successfully");
}

// Allocate network buffers
void network_alloc_buffers(network_device_t* net_dev) {
    // Allocate receive buffer
    net_dev->rx_buffer = memory_allocate_pages(NETWORK_RX_BUFFERS);
    if (net_dev->rx_buffer) {
        memset(net_dev->rx_buffer, 0, NETWORK_RX_BUFFERS * 4096);
    }
    
    // Allocate transmit buffer
    net_dev->tx_buffer = memory_allocate_pages(NETWORK_TX_BUFFERS);
    if (net_dev->tx_buffer) {
        memset(net_dev->tx_buffer, 0, NETWORK_TX_BUFFERS * 4096);
    }
    
    KDEBUG("Network buffers allocated: RX=%p, TX=%p", 
           net_dev->rx_buffer, net_dev->tx_buffer);
}

// Read network register
u32 network_read_reg(network_device_t* net_dev, u32 offset) {
    return *(volatile u32*)(net_dev->io_base + offset);
}

// Write network register
void network_write_reg(network_device_t* net_dev, u32 offset, u32 value) {
    *(volatile u32*)(net_dev->io_base + offset) = value;
}

// Read network register byte
u8 network_read_reg_byte(network_device_t* net_dev, u32 offset) {
    return *(volatile u8*)(net_dev->io_base + offset);
}

// Write network register byte
void network_write_reg_byte(network_device_t* net_dev, u32 offset, u8 value) {
    *(volatile u8*)(net_dev->io_base + offset) = value;
}

// Read network register word
u16 network_read_reg_word(network_device_t* net_dev, u32 offset) {
    return *(volatile u16*)(net_dev->io_base + offset);
}

// Write network register word
void network_write_reg_word(network_device_t* net_dev, u32 offset, u16 value) {
    *(volatile u16*)(net_dev->io_base + offset) = value;
}

// Send packet
error_t network_send_packet(network_device_t* net_dev, void* data, u32 length) {
    if (!net_dev->initialized) {
        return E_INVAL;
    }
    
    KDEBUG("Sending packet: %u bytes", length);
    
    switch (net_dev->type) {
        case NET_DEVICE_INTEL_E1000:
            return network_send_intel_packet(net_dev, data, length);
        case NET_DEVICE_REALTEK_RTL8139:
        case NET_DEVICE_REALTEK_RTL8169:
            return network_send_realtek_packet(net_dev, data, length);
        default:
            return E_NOSYS;
    }
}

// Send Intel packet
error_t network_send_intel_packet(network_device_t* net_dev, void* data, u32 length) {
    // TODO: Implement Intel e1000 packet transmission
    KDEBUG("Intel e1000 packet transmission not yet implemented");
    return SUCCESS;
}

// Send Realtek packet
error_t network_send_realtek_packet(network_device_t* net_dev, void* data, u32 length) {
    // TODO: Implement Realtek packet transmission
    KDEBUG("Realtek packet transmission not yet implemented");
    return SUCCESS;
}

// Receive packet
error_t network_receive_packet(network_device_t* net_dev, void* buffer, u32* length) {
    if (!net_dev->initialized) {
        return E_INVAL;
    }
    
    switch (net_dev->type) {
        case NET_DEVICE_INTEL_E1000:
            return network_receive_intel_packet(net_dev, buffer, length);
        case NET_DEVICE_REALTEK_RTL8139:
        case NET_DEVICE_REALTEK_RTL8169:
            return network_receive_realtek_packet(net_dev, buffer, length);
        default:
            return E_NOSYS;
    }
}

// Receive Intel packet
error_t network_receive_intel_packet(network_device_t* net_dev, void* buffer, u32* length) {
    // TODO: Implement Intel e1000 packet reception
    KDEBUG("Intel e1000 packet reception not yet implemented");
    return E_NOSYS;
}

// Receive Realtek packet
error_t network_receive_realtek_packet(network_device_t* net_dev, void* buffer, u32* length) {
    // TODO: Implement Realtek packet reception
    KDEBUG("Realtek packet reception not yet implemented");
    return E_NOSYS;
}

// Get network device
network_device_t* network_get_device(int device_index) {
    if (device_index < 0 || device_index >= device_count) {
        return NULL;
    }
    return &network_devices[device_index];
}

// Get device count
int network_get_device_count(void) {
    return device_count;
}

// Get network state
network_state_t* network_get_state(void) {
    return &network_state;
}

// Check if networking is initialized
bool network_is_initialized(void) {
    return network_initialized;
}

// Update networking (called periodically)
void network_update(void) {
    if (!network_initialized) {
        return;
    }
    
    // Process received packets
    for (int i = 0; i < device_count; i++) {
        network_device_t* net_dev = &network_devices[i];
        if (net_dev->initialized) {
            // TODO: Process received packets
        }
    }
}

// Shutdown networking
void network_shutdown(void) {
    if (!network_initialized) {
        return;
    }
    
    KINFO("Shutting down network subsystem...");
    
    // Shutdown all devices
    for (int i = 0; i < device_count; i++) {
        network_device_t* net_dev = &network_devices[i];
        if (net_dev->initialized) {
            // Free buffers
            if (net_dev->rx_buffer) {
                memory_free_pages(net_dev->rx_buffer, NETWORK_RX_BUFFERS);
                net_dev->rx_buffer = NULL;
            }
            if (net_dev->tx_buffer) {
                memory_free_pages(net_dev->tx_buffer, NETWORK_TX_BUFFERS);
                net_dev->tx_buffer = NULL;
            }
        }
    }
    
    network_initialized = false;
    network_state.initialized = false;
    
    KINFO("Network subsystem shutdown complete");
} 