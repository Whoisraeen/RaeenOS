#include "include/hal.h"
#include "../core/include/kernel.h"

// HAL stub implementations for RaeenOS compilation
// These are minimal implementations to make the kernel compile and link

// Global HAL state
static bool hal_initialized = false;

// HAL initialization
error_t hal_init(void) {
    hal_initialized = true;
    return SUCCESS;
}

void hal_shutdown(void) {
    hal_initialized = false;
}

// CPU control
void hal_enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void hal_disable_interrupts(void) {
    __asm__ volatile ("cli");
}

bool hal_are_interrupts_enabled(void) {
    uint64_t flags;
    __asm__ volatile ("pushfq; popq %0" : "=r"(flags));
    return (flags & 0x200) != 0;
}

void hal_halt_cpu(void) {
    __asm__ volatile ("hlt");
}

void hal_pause_cpu(void) {
    __asm__ volatile ("pause");
}

// Console/Output (basic VGA implementation)
error_t hal_early_console_init(void) {
    return SUCCESS;
}

bool hal_is_console_ready(void) {
    return true;
}

void hal_console_print(const char* format, ...) {
    // Simple serial output for debugging
    (void)format;
    // TODO: Implement actual console output
}

void hal_console_vprint(const char* format, va_list args) {
    (void)format;
    (void)args;
    // TODO: Implement actual console output
}

void hal_debug_print(const char* format, ...) {
    // Simple debug output
    (void)format;
    // TODO: Implement actual debug output
}

void hal_console_write(const char* data, size_t length) {
    (void)data;
    (void)length;
    // TODO: Implement actual console write
}

void hal_console_set_color(u32 color) {
    (void)color;
    // TODO: Implement color setting
}

void hal_console_clear(void) {
    // TODO: Implement screen clear
}

// Memory management
phys_addr_t hal_get_total_memory(void) {
    return 128 * 1024 * 1024; // 128MB for testing
}

phys_addr_t hal_get_available_memory(void) {
    return 64 * 1024 * 1024; // 64MB for testing
}

void* hal_map_physical(phys_addr_t phys_addr, size_t size, u32 flags) {
    (void)flags;
    (void)size;
    // Direct mapping for now
    return (void*)phys_addr;
}

void hal_unmap_physical(void* virt_addr, size_t size) {
    (void)virt_addr;
    (void)size;
    // No unmapping needed for direct mapping
}

// Time and timing (basic implementation)
static uint64_t boot_time = 0;
static uint64_t tick_count = 0;

uint64_t hal_get_timestamp(void) {
    return tick_count * 1000; // Return microseconds
}

uint64_t hal_get_cpu_frequency(void) {
    return 2000000000ULL; // 2 GHz
}

void hal_sleep_ms(u32 milliseconds) {
    // Simple busy wait (not efficient but works for testing)
    uint64_t start = hal_get_timestamp();
    while ((hal_get_timestamp() - start) < (milliseconds * 1000)) {
        hal_pause_cpu();
    }
}

void hal_sleep_us(u32 microseconds) {
    uint64_t start = hal_get_timestamp();
    while ((hal_get_timestamp() - start) < microseconds) {
        hal_pause_cpu();
    }
}

// Interrupt handling
error_t hal_register_interrupt_handler(u32 interrupt_num, interrupt_handler_t handler) {
    (void)interrupt_num;
    (void)handler;
    return SUCCESS;
}

error_t hal_unregister_interrupt_handler(u32 interrupt_num) {
    (void)interrupt_num;
    return SUCCESS;
}

void hal_send_eoi(u32 interrupt_num) {
    (void)interrupt_num;
    // TODO: Send EOI to APIC/PIC
}

// I/O operations
u8 hal_inb(u16 port) {
    u8 result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

u16 hal_inw(u16 port) {
    u16 result;
    __asm__ volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

u32 hal_inl(u16 port) {
    u32 result;
    __asm__ volatile ("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void hal_outb(u16 port, u8 value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void hal_outw(u16 port, u16 value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

void hal_outl(u16 port, u32 value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Timer tick (called by timer interrupt)
void hal_timer_tick(void) {
    tick_count++;
}

// Simple memory allocation for early boot
static char early_heap[64 * 1024]; // 64KB early heap
static size_t early_heap_ptr = 0;

void* hal_early_malloc(size_t size) {
    if (early_heap_ptr + size > sizeof(early_heap)) {
        return NULL;
    }
    
    void* ptr = &early_heap[early_heap_ptr];
    early_heap_ptr += (size + 7) & ~7; // 8-byte align
    return ptr;
}

void hal_early_free(void* ptr) {
    (void)ptr;
    // Early allocations are not freed
}

// Get platform information
const char* hal_get_platform_name(void) {
    return "x86_64";
}

bool hal_is_platform_supported(void) {
    return true;
}