#include "hal/hal.h"
#include "hal/x64/hal_x64.h"
#include "syscall/syscall.h"
#include "kernel.h"
#include <stdarg.h>
#include <string.h>

// x64-specific HAL implementation
static bool x64_initialized = false;

// VGA text mode console
#define VGA_MEMORY      0xB8000
#define VGA_WIDTH       80
#define VGA_HEIGHT      25
#define VGA_COLOR_DEFAULT 0x07

static u16* vga_buffer = (u16*)VGA_MEMORY;
static u32 console_x = 0;
static u32 console_y = 0;
static u8 console_color = VGA_COLOR_DEFAULT;

// APIC timer frequency (adjust as needed, based on your CPU)
#define APIC_TIMER_FREQUENCY 1000 // 1 KHz (1ms tick)

void hal_arch_init(void) {
    if (x64_initialized) {
        return;
    }
    
    KDEBUG("Initializing x64 HAL");
    
    // Initialize x64-specific features
    x64_setup_gdt();    // Assuming you have this function
    x64_syscall_init(); // Initialize the syscall MSRs
    
    x64_initialized = true;
    KDEBUG("x64 HAL initialized");
}

void hal_arch_shutdown(void) {
    if (!x64_initialized) {
        return;
    }
    
    KDEBUG("Shutting down x64 HAL");
    x64_initialized = false;
}

// CPU control functions
void hal_enable_interrupts(void) {
    x64_enable_interrupts();
}

void hal_disable_interrupts(void) {
    x64_disable_interrupts();
}

bool hal_are_interrupts_enabled(void) {
    return x64_are_interrupts_enabled();
}

void hal_halt_cpu(void) {
    x64_halt_cpu();
}

void hal_pause_cpu(void) {
    x64_pause_cpu();
}

// x64 assembly functions
void x64_enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void x64_disable_interrupts(void) {
    __asm__ volatile ("cli");
}

bool x64_are_interrupts_enabled(void) {
    u64 flags = x64_read_rflags();
    return (flags & RFLAGS_IF) != 0;
}

void x64_halt_cpu(void) {
    __asm__ volatile ("hlt");
}

void x64_pause_cpu(void) {
    __asm__ volatile ("pause");
}

// Control register functions
u64 x64_read_cr0(void) {
    u64 value;
    __asm__ volatile ("mov %%cr0, %0" : "=r" (value));
    return value;
}

u64 x64_read_cr2(void) {
    u64 value;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (value));
    return value;
}

u64 x64_read_cr3(void) {
    u64 value;
    __asm__ volatile ("mov %%cr3, %0" : "=r" (value));
    return value;
}

u64 x64_read_cr4(void) {
    u64 value;
    __asm__ volatile ("mov %%cr4, %0" : "=r" (value));
    return value;
}

void x64_write_cr0(u64 value) {
    __asm__ volatile ("mov %0, %%cr0" :: "r" (value) : "memory");
}

void x64_write_cr3(u64 value) {
    __asm__ volatile ("mov %0, %%cr3" :: "r" (value) : "memory");
}

void x64_write_cr4(u64 value) {
    __asm__ volatile ("mov %0, %%cr4" :: "r" (value) : "memory");
}

// MSR functions
u64 x64_read_msr(u32 msr) {
    u32 low, high;
    __asm__ volatile ("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));
    return ((u64)high << 32) | low;
}

void x64_write_msr(u32 msr, u64 value) {
    u32 low = (u32)value;
    u32 high = (u32)(value >> 32);
    __asm__ volatile ("wrmsr" :: "a" (low), "d" (high), "c" (msr));
}

// RFLAGS functions
u64 x64_read_rflags(void) {
    u64 value;
    __asm__ volatile ("pushfq; popq %0" : "=r" (value));
    return value;
}

void x64_write_rflags(u64 value) {
    __asm__ volatile ("pushq %0; popfq" :: "r" (value) : "memory", "cc");
}

// TLB functions
void x64_invlpg(virt_addr_t addr) {
    __asm__ volatile ("invlpg (%0)" :: "r" (addr) : "memory");
}

void x64_flush_tlb(void) {
    u64 cr3 = x64_read_cr3();
    x64_write_cr3(cr3);
}

// CPUID function
void x64_cpuid(u32 leaf, u32* eax, u32* ebx, u32* ecx, u32* edx) {
    __asm__ volatile ("cpuid"
                     : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                     : "a" (leaf));
}

// Memory barriers
void x64_memory_barrier(void) {
    __asm__ volatile ("mfence" ::: "memory");
}

void x64_read_barrier(void) {
    __asm__ volatile ("lfence" ::: "memory");
}

void x64_write_barrier(void) {
    __asm__ volatile ("sfence" ::: "memory");
}

// Cache operations
void x64_wbinvd(void) {
    __asm__ volatile ("wbinvd" ::: "memory");
}

void x64_invd(void) {
    __asm__ volatile ("invd" ::: "memory");
}

void x64_clflush(void* addr) {
    __asm__ volatile ("clflush (%0)" :: "r" (addr) : "memory");
}

// APIC timer operations
void hal_apic_init(void) {
    // Check if APIC is available
    u32 eax, ebx, ecx, edx;
    x64_cpuid(1, &eax, &ebx, &ecx, &edx);
    if (!(edx & (1 << 9))) { // Check APIC feature bit
        KERROR("APIC not supported on this CPU!");
        return;
    }

    // Enable APIC (if not already enabled by BIOS)
    u64 apic_base = x64_read_msr(0x1B);
    if (!(apic_base & (1 << 11))) {
        x64_write_msr(0x1B, apic_base | (1 << 11));
    }

    // Get APIC ID (for multi-core support - not used in this example)
    u32 apic_id = (x64_read_msr(0x80B) >> 24) & 0xFF;
    KDEBUG("APIC Initialized. APIC ID: %u", apic_id);

    // Initialize timer
    // Divide configuration (divide by 16)
    x64_write_apic_reg(0x3E0, 0x3);

    // Set initial count (for now, use a fixed frequency)
    u32 initial_count = hal_get_cpu_frequency() / APIC_TIMER_FREQUENCY / 16;
    x64_write_apic_reg(0x380, initial_count);

    // Set timer mode (periodic) and interrupt vector (adjust as needed)
    x64_write_apic_reg(0x320, 0x20 | 0x10000); // Vector 32 (0x20), Periodic mode

    KINFO("APIC Timer Initialized. Frequency: %u Hz", APIC_TIMER_FREQUENCY);
}

void x64_write_apic_reg(u32 reg, u32 value) {
    // Get APIC base address
    u64 apic_base = x64_read_msr(0x1B) & 0xFFFFFFFFFFFFF000;

    // Write to APIC register
    *((volatile u32*)(apic_base + reg)) = value;
}

u32 x64_read_apic_reg(u32 reg) {
    // Get APIC base address
    u64 apic_base = x64_read_msr(0x1B) & 0xFFFFFFFFFFFFF000;

    // Read from APIC register
    return *((volatile u32*)(apic_base + reg));
}

// Timer interrupt handler
void hal_timer_interrupt_handler(void) {
    // Acknowledge interrupt (important!)
    x64_write_apic_reg(0xB0, 0);

    // Call the scheduler
    extern void schedule(void);
    schedule();
}

void x64_syscall_init(void) {
    // This function sets up the MSRs for the SYSCALL/SYSRET instructions.
    extern void syscall_entry(void); // Get address of our assembly handler

    // Kernel and User segment selectors from your GDT
    u32 k_cs = 0x08; // Kernel Code Segment
    u32 u_cs = 0x1B; // User Code Segment (0x18 | 3)
    u32 u_ss = 0x23; // User Data/Stack Segment (0x20 | 3)

    // MSR_STAR: Specifies kernel and user segment selectors on syscall/sysret
    // Bits 47-32: Kernel CS
    // Bits 63-48: User CS (is set to u_cs-8 on sysret)
    x64_write_msr(MSR_STAR, ((u64)(u_cs - 8) << 48) | ((u64)k_cs << 32));

    // MSR_LSTAR: The 64-bit address of the syscall entry point
    x64_write_msr(MSR_LSTAR, (u64)syscall_entry);

    // MSR_SYSCALL_MASK: RFLAGS mask. Flags set here will be cleared on syscall.
    x64_write_msr(MSR_SYSCALL_MASK, RFLAGS_IF); // Clear interrupt flag on entry
}

// Setup GDT (example - replace with your actual GDT setup)
void x64_setup_gdt(void) {
    KDEBUG("Setting up GDT (placeholder)");
    // In a real OS, you'd initialize a proper GDT here.
}

// Setup IDT (example - replace with your actual IDT setup)
void x64_setup_idt(void) {
    KDEBUG("Setting up IDT (placeholder)");
    // In a real OS, you'd initialize a proper IDT here and
    // register an interrupt handler for the timer vector (0x20 in this example).
    // Example (using placeholder interrupt handler functions):
    // set_idt_entry(0x20, timer_interrupt_handler_wrapper, 0x08, 0x8E);
    // ... where timer_interrupt_handler_wrapper calls hal_timer_interrupt_handler
    // and is defined in assembly to handle the interrupt frame.
    // You also need to unmask the timer interrupt in the PIC/IOAPIC.
}

// Time operations
u64 x64_rdtsc(void) {
    u32 low, high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ((u64)high << 32) | low;
}

u64 x64_rdtscp(u32* aux) {
    u32 low, high;
    __asm__ volatile ("rdtscp" : "=a" (low), "=d" (high), "=c" (*aux));
    return ((u64)high << 32) | low;
}

// I/O port operations
u8 hal_inb(u16 port) {
    u8 value;
    __asm__ volatile ("inb %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

u16 hal_inw(u16 port) {
    u16 value;
    __asm__ volatile ("inw %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

u32 hal_inl(u16 port) {
    u32 value;
    __asm__ volatile ("inl %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

void hal_outb(u16 port, u8 value) {
    __asm__ volatile ("outb %0, %1" :: "a" (value), "Nd" (port));
}

void hal_outw(u16 port, u16 value) {
    __asm__ volatile ("outw %0, %1" :: "a" (value), "Nd" (port));
}

void hal_outl(u16 port, u32 value) {
    __asm__ volatile ("outl %0, %1" :: "a" (value), "Nd" (port));
}

// Memory management HAL functions
void hal_flush_tlb(void) {
    x64_flush_tlb();
}

void hal_invalidate_tlb_entry(virt_addr_t addr) {
    x64_invlpg(addr);
}

void hal_flush_cache(void) {
    x64_wbinvd();
}

void hal_invalidate_cache(void) {
    x64_invd();
}

void hal_memory_barrier(void) {
    x64_memory_barrier();
}

void hal_read_barrier(void) {
    x64_read_barrier();
}

void hal_write_barrier(void) {
    x64_write_barrier();
}

// Page directory management
error_t hal_set_page_directory(phys_addr_t pml4_phys) {
    if (pml4_phys % PAGE_SIZE != 0) {
        return E_INVAL;
    }
    
    x64_write_cr3(pml4_phys);
    return SUCCESS;
}

phys_addr_t hal_get_page_directory(void) {
    return x64_read_cr3() & ~0xFFF; // Mask off flags
}

// CPU information
error_t hal_get_cpu_info(cpu_info_t* info) {
    if (!info) {
        return E_INVAL;
    }
    
    memset(info, 0, sizeof(cpu_info_t));
    
    // Get vendor string
    u32 eax, ebx, ecx, edx;
    x64_cpuid(0, &eax, &ebx, &ecx, &edx);
    
    // Vendor string is stored in EBX, EDX, ECX
    memcpy(info->vendor, &ebx, 4);
    memcpy(info->vendor + 4, &edx, 4);
    memcpy(info->vendor + 8, &ecx, 4);
    info->vendor[12] = '\0';
    
    // Get basic CPU info
    if (eax >= 1) {
        x64_cpuid(1, &eax, &ebx, &ecx, &edx);
        
        info->family = ((eax >> 8) & 0xF) + ((eax >> 20) & 0xFF);
        info->model = ((eax >> 4) & 0xF) | (((eax >> 16) & 0xF) << 4);
        info->stepping = eax & 0xF;
        info->features = edx; // Basic features in EDX
        
        // Additional features in ECX
        if (ecx & (1 << 0)) info->features |= CPU_FEATURE_SSE3;
        if (ecx & (1 << 9)) info->features |= CPU_FEATURE_SSSE3;
        if (ecx & (1 << 19)) info->features |= CPU_FEATURE_SSE4_1;
        if (ecx & (1 << 20)) info->features |= CPU_FEATURE_SSE4_2;
        if (ecx & (1 << 28)) info->features |= CPU_FEATURE_AVX;
    }
    
    // Get brand string
    u32 brand_eax, brand_ebx, brand_ecx, brand_edx;
    x64_cpuid(0x80000000, &brand_eax, &brand_ebx, &brand_ecx, &brand_edx);
    
    if (brand_eax >= 0x80000004) {
        char* brand_ptr = info->brand;
        
        for (u32 i = 0x80000002; i <= 0x80000004; i++) {
            x64_cpuid(i, &eax, &ebx, &ecx, &edx);
            memcpy(brand_ptr, &eax, 4);
            memcpy(brand_ptr + 4, &ebx, 4);
            memcpy(brand_ptr + 8, &ecx, 4);
            memcpy(brand_ptr + 12, &edx, 4);
            brand_ptr += 16;
        }
        info->brand[48] = '\0';
    }
    
    // TODO: Get cache sizes, core count, etc.
    info->cache_size_l1 = 32; // Default values
    info->cache_size_l2 = 256;
    info->cache_size_l3 = 1024;
    info->core_count = 1;
    info->thread_count = 1;
    
    return SUCCESS;
}

// Time and timing functions
uint64_t hal_get_timestamp(void) {
    // Use TSC for now - this should be calibrated properly
    return x64_rdtsc();
}

uint64_t hal_get_cpu_frequency(void) {
    // TODO: Implement proper CPU frequency detection
    // For now, return a default value
    return 2000000000ULL; // 2 GHz
}

// Debug functions
void hal_dump_registers(void) {
    u64 cr0 = x64_read_cr0();
    u64 cr2 = x64_read_cr2();
    u64 cr3 = x64_read_cr3();
    u64 cr4 = x64_read_cr4();
    u64 rflags = x64_read_rflags();
    
    hal_console_print("Register dump:\n");
    hal_console_print("  CR0: 0x%016llx\n", cr0);
    hal_console_print("  CR2: 0x%016llx\n", cr2);
    hal_console_print("  CR3: 0x%016llx\n", cr3);
    hal_console_print("  CR4: 0x%016llx\n", cr4);
    hal_console_print("  RFLAGS: 0x%016llx\n", rflags);
}

void hal_dump_stack_trace(void) {
    // Simple stack trace implementation
    u64* rbp;
    __asm__ volatile ("mov %%rbp, %0" : "=r" (rbp));
    
    hal_console_print("Stack trace:\n");
    
    for (int i = 0; i < 10 && rbp; i++) {
        u64 return_addr = rbp[1];
        hal_console_print("  [%d] 0x%016llx\n", i, return_addr);
        rbp = (u64*)rbp[0];
        
        // Basic sanity check
        if ((u64)rbp < 0x1000 || (u64)rbp > 0xFFFFFFFFFFFFFFFFULL - 16) {
            break;
        }
    }
}

void hal_breakpoint(void) {
    __asm__ volatile ("int3");
}

// Console implementation
static void console_scroll(void) {
    // Move all lines up by one
    for (u32 y = 0; y < VGA_HEIGHT - 1; y++) {
        for (u32 x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear the last line
    for (u32 x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (console_color << 8) | ' ';
    }
}

static void console_putchar(char c) {
    if (c == '\n') {
        console_x = 0;
        console_y++;
    } else if (c == '\r') {
        console_x = 0;
    } else if (c == '\t') {
        console_x = (console_x + 8) & ~7;
    } else if (c == '\b') {
        if (console_x > 0) {
            console_x--;
            vga_buffer[console_y * VGA_WIDTH + console_x] = (console_color << 8) | ' ';
        }
    } else {
        vga_buffer[console_y * VGA_WIDTH + console_x] = (console_color << 8) | c;
        console_x++;
    }
    
    if (console_x >= VGA_WIDTH) {
        console_x = 0;
        console_y++;
    }
    
    if (console_y >= VGA_HEIGHT) {
        console_scroll();
        console_y = VGA_HEIGHT - 1;
    }
}

error_t hal_early_console_init(void) {
    // Clear screen
    for (u32 i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (console_color << 8) | ' ';
    }
    
    console_x = 0;
    console_y = 0;
    
    return SUCCESS;
}

void hal_console_vprint(const char* format, va_list args) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    for (const char* p = buffer; *p; p++) {
        console_putchar(*p);
    }
}

void hal_console_set_color(u32 color) {
    console_color = (u8)(color & 0xFF);
}

void hal_console_clear(void) {
    for (u32 i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (console_color << 8) | ' ';
    }
    console_x = 0;
    console_y = 0;
}