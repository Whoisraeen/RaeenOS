#ifndef HAL_H
#define HAL_H

#include "types.h"
#include <stdarg.h>

// HAL initialization and management
error_t hal_init(void);
void hal_shutdown(void);

// CPU control
void hal_enable_interrupts(void);
void hal_disable_interrupts(void);
bool hal_are_interrupts_enabled(void);
void hal_halt_cpu(void);
void hal_pause_cpu(void);

// Console/Output
error_t hal_early_console_init(void);
bool hal_is_console_ready(void);
void hal_console_print(const char* format, ...);
void hal_console_vprint(const char* format, va_list args);
void hal_console_write(const char* data, size_t length);
void hal_console_set_color(u32 color);
void hal_console_clear(void);

// Memory management
phys_addr_t hal_get_total_memory(void);
phys_addr_t hal_get_available_memory(void);
void* hal_map_physical(phys_addr_t phys_addr, size_t size, u32 flags);
void hal_unmap_physical(void* virt_addr, size_t size);

// Time and timing
uint64_t hal_get_timestamp(void);
uint64_t hal_get_cpu_frequency(void);
void hal_sleep_ms(u32 milliseconds);
void hal_sleep_us(u32 microseconds);

// Interrupt handling
typedef void (*interrupt_handler_t)(u32 interrupt_num, void* context);
error_t hal_register_interrupt_handler(u32 interrupt_num, interrupt_handler_t handler);
error_t hal_unregister_interrupt_handler(u32 interrupt_num);
void hal_send_eoi(u32 interrupt_num);

// DMA and I/O
u8  hal_inb(u16 port);
u16 hal_inw(u16 port);
u32 hal_inl(u16 port);
void hal_outb(u16 port, u8 value);
void hal_outw(u16 port, u16 value);
void hal_outl(u16 port, u32 value);

// PCI access
error_t hal_pci_read_config(u8 bus, u8 device, u8 function, u8 offset, u32* value);
error_t hal_pci_write_config(u8 bus, u8 device, u8 function, u8 offset, u32 value);

// CPU information
typedef struct {
    char vendor[13];
    char brand[49];
    u32 family;
    u32 model;
    u32 stepping;
    u32 features;
    u32 cache_size_l1;
    u32 cache_size_l2;
    u32 cache_size_l3;
    u32 core_count;
    u32 thread_count;
} cpu_info_t;

error_t hal_get_cpu_info(cpu_info_t* info);

// Memory barriers
void hal_memory_barrier(void);
void hal_read_barrier(void);
void hal_write_barrier(void);

// Cache management
void hal_flush_cache(void);
void hal_invalidate_cache(void);
void hal_flush_tlb(void);
void hal_invalidate_tlb_entry(virt_addr_t addr);

// Debugging support
void hal_dump_registers(void);
void hal_dump_stack_trace(void);
void hal_breakpoint(void);

// Power management
error_t hal_set_power_state(u32 state);
u32 hal_get_power_state(void);

// Architecture-specific functions
#ifdef __x86_64__
#include "hal/x64/hal_x64.h"
#elif defined(__i386__)
#include "hal/x86/hal_x86.h"
#endif

// Common architecture functions
void hal_arch_init(void);
void hal_arch_shutdown(void);

// Page table flags
#define HAL_PAGE_PRESENT    (1 << 0)
#define HAL_PAGE_WRITABLE   (1 << 1)
#define HAL_PAGE_USER       (1 << 2)
#define HAL_PAGE_WRITETHROUGH (1 << 3)
#define HAL_PAGE_CACHE_DISABLE (1 << 4)
#define HAL_PAGE_ACCESSED   (1 << 5)
#define HAL_PAGE_DIRTY      (1 << 6)
#define HAL_PAGE_GLOBAL     (1 << 8)
#define HAL_PAGE_NO_EXECUTE (1 << 63)

// CPU features
#define CPU_FEATURE_FPU     (1 << 0)
#define CPU_FEATURE_SSE     (1 << 1)
#define CPU_FEATURE_SSE2    (1 << 2)
#define CPU_FEATURE_SSE3    (1 << 3)
#define CPU_FEATURE_SSSE3   (1 << 4)
#define CPU_FEATURE_SSE4_1  (1 << 5)
#define CPU_FEATURE_SSE4_2  (1 << 6)
#define CPU_FEATURE_AVX     (1 << 7)
#define CPU_FEATURE_AVX2    (1 << 8)
#define CPU_FEATURE_AVX512F (1 << 9)

// Power states
#define POWER_STATE_S0      0  // Working
#define POWER_STATE_S1      1  // Standby
#define POWER_STATE_S2      2  // Suspend to RAM
#define POWER_STATE_S3      3  // Suspend to RAM (deeper)
#define POWER_STATE_S4      4  // Hibernate
#define POWER_STATE_S5      5  // Soft Off

#endif // HAL_H