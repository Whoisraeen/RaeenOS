#include "hal/hal.h"
#include "kernel.h"

static bool hal_initialized = false;
static bool console_ready = false;

error_t hal_init(void) {
    if (hal_initialized) {
        return SUCCESS;
    }
    
    KDEBUG("Initializing Hardware Abstraction Layer");
    
    // Initialize architecture-specific components
    hal_arch_init();
    
    // Initialize console early for debugging
    if (hal_early_console_init() != SUCCESS) {
        return E_IO;
    }
    console_ready = true;
    
    // Initialize interrupt controller
    // TODO: Initialize APIC/PIC
    
    // Initialize timer
    // TODO: Initialize system timer
    
    // Initialize memory management unit
    // TODO: Initialize MMU
    
    hal_initialized = true;
    KINFO("Hardware Abstraction Layer initialized successfully");
    
    return SUCCESS;
}

void hal_shutdown(void) {
    if (!hal_initialized) {
        return;
    }
    
    KINFO("Shutting down Hardware Abstraction Layer");
    
    hal_disable_interrupts();
    
    // Shutdown architecture-specific components
    hal_arch_shutdown();
    
    hal_initialized = false;
}

bool hal_are_interrupts_enabled(void) {
    // This will be implemented in architecture-specific code
    return false;
}

bool hal_is_console_ready(void) {
    return console_ready;
}

void hal_console_print(const char* format, ...) {
    if (!console_ready) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    hal_console_vprint(format, args);
    va_end(args);
}

// Default implementations that can be overridden by architecture-specific code
__attribute__((weak)) error_t hal_early_console_init(void) {
    // Default implementation - no early console
    return E_OPNOTSUPP;
}

__attribute__((weak)) void hal_console_vprint(const char* format, va_list args) {
    // Default implementation - no output
    (void)format;
    (void)args;
}

__attribute__((weak)) void hal_console_set_color(u32 color) {
    // Default implementation - no color support
    (void)color;
}

__attribute__((weak)) void hal_console_clear(void) {
    // Default implementation - no clear support
}

__attribute__((weak)) uint64_t hal_get_timestamp(void) {
    // Default implementation - return 0
    // This should be overridden by architecture-specific code
    return 0;
}

__attribute__((weak)) uint64_t hal_get_cpu_frequency(void) {
    // Default implementation - return 0
    // This should be overridden by architecture-specific code
    return 0;
}

__attribute__((weak)) void hal_sleep_ms(u32 milliseconds) {
    // Default implementation - busy wait
    // This should be overridden by architecture-specific code
    for (volatile u32 i = 0; i < milliseconds * 1000; i++) {
        hal_pause_cpu();
    }
}

__attribute__((weak)) void hal_sleep_us(u32 microseconds) {
    // Default implementation - busy wait
    // This should be overridden by architecture-specific code
    for (volatile u32 i = 0; i < microseconds; i++) {
        hal_pause_cpu();
    }
}

__attribute__((weak)) error_t hal_register_interrupt_handler(u32 interrupt_num, interrupt_handler_t handler) {
    // Default implementation - not supported
    (void)interrupt_num;
    (void)handler;
    return E_OPNOTSUPP;
}

__attribute__((weak)) error_t hal_unregister_interrupt_handler(u32 interrupt_num) {
    // Default implementation - not supported
    (void)interrupt_num;
    return E_OPNOTSUPP;
}

__attribute__((weak)) void hal_send_eoi(u32 interrupt_num) {
    // Default implementation - no operation
    (void)interrupt_num;
}

__attribute__((weak)) phys_addr_t hal_get_total_memory(void) {
    // Default implementation - return 0
    return 0;
}

__attribute__((weak)) phys_addr_t hal_get_available_memory(void) {
    // Default implementation - return 0
    return 0;
}

__attribute__((weak)) void* hal_map_physical(phys_addr_t phys_addr, size_t size, u32 flags) {
    // Default implementation - not supported
    (void)phys_addr;
    (void)size;
    (void)flags;
    return NULL;
}

__attribute__((weak)) void hal_unmap_physical(void* virt_addr, size_t size) {
    // Default implementation - no operation
    (void)virt_addr;
    (void)size;
}

__attribute__((weak)) error_t hal_get_cpu_info(cpu_info_t* info) {
    // Default implementation - not supported
    (void)info;
    return E_OPNOTSUPP;
}

__attribute__((weak)) void hal_memory_barrier(void) {
    // Default implementation - compiler barrier only
    __asm__ volatile("" ::: "memory");
}

__attribute__((weak)) void hal_read_barrier(void) {
    // Default implementation - compiler barrier only
    __asm__ volatile("" ::: "memory");
}

__attribute__((weak)) void hal_write_barrier(void) {
    // Default implementation - compiler barrier only
    __asm__ volatile("" ::: "memory");
}

__attribute__((weak)) void hal_flush_cache(void) {
    // Default implementation - no operation
}

__attribute__((weak)) void hal_invalidate_cache(void) {
    // Default implementation - no operation
}

__attribute__((weak)) void hal_flush_tlb(void) {
    // Default implementation - no operation
}

__attribute__((weak)) void hal_invalidate_tlb_entry(virt_addr_t addr) {
    // Default implementation - no operation
    (void)addr;
}

__attribute__((weak)) void hal_dump_registers(void) {
    hal_console_print("Register dump not implemented for this architecture\n");
}

__attribute__((weak)) void hal_dump_stack_trace(void) {
    hal_console_print("Stack trace not implemented for this architecture\n");
}

__attribute__((weak)) void hal_breakpoint(void) {
    // Default implementation - no operation
}

__attribute__((weak)) error_t hal_set_power_state(u32 state) {
    // Default implementation - not supported
    (void)state;
    return E_OPNOTSUPP;
}

__attribute__((weak)) u32 hal_get_power_state(void) {
    // Default implementation - always return working state
    return POWER_STATE_S0;
}