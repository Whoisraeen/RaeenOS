#include <kernel/core/kernel.h>
#include <kernel/memory/memory.h>
#include <kernel/process/process.h>
#include <kernel/filesystem/vfs.h>
#include <kernel/network/network.h>
#include <kernel/security/security.h>
#include <kernel/drivers/display/display_system.h>
#include <kernel/drivers/input/input_system.h>
#include <kernel/drivers/audio/audio_system.h>
#include <kernel/network/wifi/wifi_system.h>
#include <kernel/security/authentication_system.h>
#include <kernel/gui/desktop/desktop_environment.h>
#include <kernel/gui/apps/file_manager.h>
#include <kernel/gui/apps/web_browser.h>
#include <kernel/gui/apps/email_client.h>
#include <kernel/gaming/directx_compatibility.h>
#include <kernel/power/power_management.h>
#include <kernel/apps/terminal/terminal_emulator.h>
#include <kernel/boot/boot_system.h>
#include <kernel/system_integration.h>

// RaeenOS Main Kernel - Complete Operating System Integration
// Orchestrates all subsystems into a unified, world-class OS

static system_integration_t* g_system = NULL;
static bool g_system_running = false;

// System initialization sequence
static bool raeenos_init_system(void) {
    printf("RaeenOS: Initializing complete operating system...\n");
    
    // Create system configuration
    system_config_t config = system_integration_preset_desktop_style();
    config.enable_gui = true;
    config.enable_networking = true;
    config.enable_audio = true;
    config.enable_gaming = true;
    config.enable_ai = true;
    config.enable_quantum = true;
    config.enable_blockchain = true;
    config.enable_xr = true;
    config.enable_enterprise = true;
    config.enable_security = true;
    config.enable_power_management = true;
    
    // Initialize system integration
    g_system = system_integration_init(&config);
    if (!g_system) {
        printf("RaeenOS: Failed to initialize system integration\n");
        return false;
    }
    
    printf("RaeenOS: System integration initialized successfully\n");
    return true;
}

// Main kernel entry point
void kernel_main(multiboot_info_t* mbinfo) {
    printf("RaeenOS: Starting world-class operating system...\n");
    printf("RaeenOS: Version 1.0 - The Future of Computing\n");
    
    // Initialize core kernel systems
    if (!kernel_init(mbinfo)) {
        printf("RaeenOS: Failed to initialize core kernel\n");
        return;
    }
    
    // Initialize memory system
    if (!memory_system_init()) {
        printf("RaeenOS: Failed to initialize memory system\n");
        return;
    }
    
    // Initialize process system
    if (!process_system_init()) {
        printf("RaeenOS: Failed to initialize process system\n");
        return;
    }
    
    // Initialize file system
    if (!vfs_init()) {
        printf("RaeenOS: Failed to initialize file system\n");
        return;
    }
    
    // Initialize network system
    if (!network_system_init()) {
        printf("RaeenOS: Failed to initialize network system\n");
        return;
    }
    
    // Initialize security system
    if (!security_system_init()) {
        printf("RaeenOS: Failed to initialize security system\n");
        return;
    }
    
    // Initialize complete RaeenOS system
    if (!raeenos_init_system()) {
        printf("RaeenOS: Failed to initialize complete system\n");
        return;
    }
    
    // Start the system
    if (!system_integration_start(g_system)) {
        printf("RaeenOS: Failed to start system\n");
        return;
    }
    
    g_system_running = true;
    printf("RaeenOS: System started successfully!\n");
    printf("RaeenOS: Welcome to the future of computing!\n");
    
    // Main system loop
    while (g_system_running) {
        // Update system integration
        system_integration_update_performance(g_system);
        
        // Process system events
        system_integration_process_events(g_system);
        
        // Update subsystems
        if (g_system->display_system) {
            display_system_update(g_system->display_system);
        }
        
        if (g_system->input_system) {
            input_system_process_events(g_system->input_system);
        }
        
        if (g_system->audio_system) {
            audio_system_update(g_system->audio_system);
        }
        
        if (g_system->desktop_environment) {
            desktop_environment_update(g_system->desktop_environment);
        }
        
        if (g_system->power_system) {
            power_management_update(g_system->power_system);
        }
        
        // Check for system shutdown
        if (system_integration_get_state(g_system) == SYSTEM_STATE_SHUTTING_DOWN) {
            g_system_running = false;
            break;
        }
        
        // Yield to other processes
        process_yield();
    }
    
    // Shutdown system
    printf("RaeenOS: Shutting down system...\n");
    system_integration_shutdown(g_system);
    printf("RaeenOS: System shutdown complete\n");
}

// System shutdown function
void raeenos_shutdown(void) {
    if (g_system) {
        system_integration_set_state(g_system, SYSTEM_STATE_SHUTTING_DOWN);
    }
    g_system_running = false;
}

// Get system integration context
system_integration_t* raeenos_get_system(void) {
    return g_system;
}

// Check if system is running
bool raeenos_is_running(void) {
    return g_system_running;
} 