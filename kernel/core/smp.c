#include "kernel.h"
#include "hal/hal.h"
#include "process/scheduler.h"
#include "memory/memory.h"
#include <string.h>

// SMP configuration
#define MAX_CPUS 64
#define APIC_BASE 0xFEE00000
#define APIC_ID_REG 0x20
#define APIC_VERSION_REG 0x30
#define APIC_TPR_REG 0x80
#define APIC_EOI_REG 0xB0
#define APIC_SIVR_REG 0xF0
#define APIC_ICR_LOW 0x300
#define APIC_ICR_HIGH 0x310

// CPU information structure
typedef struct {
    u32 cpu_id;
    u32 apic_id;
    bool online;
    bool boot_cpu;
    u64 stack_base;
    u64 stack_size;
    void* tss;
    void* gdt;
    void* idt;
    void* page_tables;
    thread_t* current_thread;
    process_t* current_process;
    u64 idle_time;
    u64 total_runtime;
    u32 context_switches;
    u32 interrupts_handled;
} cpu_info_t;

// Global SMP state
static cpu_info_t cpus[MAX_CPUS];
static u32 num_cpus = 0;
static u32 boot_cpu_id = 0;
static bool smp_initialized = false;
static volatile u32 cpus_ready = 0;
static volatile u32 cpus_online = 0;

// APIC access functions
static u32 apic_read(u32 reg) {
    return *(volatile u32*)(APIC_BASE + reg);
}

static void apic_write(u32 reg, u32 value) {
    *(volatile u32*)(APIC_BASE + reg) = value;
}

// CPU detection and initialization
static u32 detect_cpus(void) {
    KINFO("Detecting CPUs...");
    
    // Start with boot CPU
    cpus[0].cpu_id = 0;
    cpus[0].apic_id = 0;
    cpus[0].online = true;
    cpus[0].boot_cpu = true;
    cpus[0].stack_base = 0x9000; // Boot CPU stack
    cpus[0].stack_size = 0x1000;
    num_cpus = 1;
    
    // Check if APIC is available
    u32 apic_version = apic_read(APIC_VERSION_REG);
    if ((apic_version & 0xFF) == 0) {
        KWARN("APIC not available, running in single-CPU mode");
        return 1;
    }
    
    KINFO("APIC version: %u", apic_version & 0xFF);
    
    // For now, assume up to 4 CPUs (can be expanded)
    // In a real implementation, this would parse ACPI tables
    for (u32 i = 1; i < 4; i++) {
        cpus[i].cpu_id = i;
        cpus[i].apic_id = i;
        cpus[i].online = false;
        cpus[i].boot_cpu = false;
        cpus[i].stack_base = 0x10000 + (i * 0x1000); // Separate stack per CPU
        cpus[i].stack_size = 0x1000;
        num_cpus++;
    }
    
    KINFO("Detected %u CPUs", num_cpus);
    return num_cpus;
}

// Initialize APIC for SMP
static error_t init_apic(void) {
    KINFO("Initializing APIC for SMP");
    
    // Enable APIC
    apic_write(APIC_SIVR_REG, 0x100 | 0xFF); // Enable APIC, set spurious vector
    
    // Set task priority to 0 (allow all interrupts)
    apic_write(APIC_TPR_REG, 0);
    
    // Configure APIC timer (if available)
    // This would set up the local APIC timer for scheduling
    
    KINFO("APIC initialized");
    return SUCCESS;
}

// Send IPI (Inter-Processor Interrupt)
static void send_ipi(u32 target_cpu, u8 vector, u8 delivery_mode, u8 level, u8 trigger_mode) {
    u32 icr_low = vector | (delivery_mode << 8) | (level << 14) | (trigger_mode << 15);
    u32 icr_high = target_cpu << 24;
    
    apic_write(APIC_ICR_HIGH, icr_high);
    apic_write(APIC_ICR_LOW, icr_low);
}

// Broadcast IPI to all CPUs except self
static void broadcast_ipi(u8 vector, u8 delivery_mode) {
    u32 icr_low = vector | (delivery_mode << 8) | (1 << 18); // Broadcast
    u32 icr_high = 0;
    
    apic_write(APIC_ICR_HIGH, icr_high);
    apic_write(APIC_ICR_LOW, icr_low);
}

// CPU startup trampoline
extern void smp_startup_trampoline(void);
extern void smp_startup_trampoline_end(void);

// Assembly trampoline for CPU startup
__attribute__((section(".text")))
void smp_startup_trampoline(void) {
    // This will be called by secondary CPUs
    // Set up basic environment and jump to smp_secondary_startup
    __asm__ volatile(
        "cli\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%ss\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "jmp smp_secondary_startup\n"
        : : : "ax", "memory"
    );
}

void smp_secondary_startup(void) {
    // Get CPU ID from stack (set by primary CPU)
    u32 cpu_id = 0; // This would be passed via stack or register
    
    KINFO("Secondary CPU %u starting up", cpu_id);
    
    // Initialize CPU-specific structures
    cpu_info_t* cpu = &cpus[cpu_id];
    cpu->online = true;
    
    // Set up GDT, IDT, and page tables for this CPU
    // This would be similar to the boot CPU setup
    
    // Initialize scheduler for this CPU
    scheduler_init_cpu(cpu_id);
    
    // Signal that this CPU is ready
    __sync_fetch_and_add(&cpus_ready, 1);
    
    // Wait for all CPUs to be ready
    while (cpus_ready < num_cpus) {
        hal_pause_cpu();
    }
    
    // Enable interrupts
    hal_enable_interrupts();
    
    // Enter scheduler loop
    while (1) {
        scheduler_cpu_loop(cpu_id);
    }
}

// Initialize SMP system
error_t smp_init(void) {
    if (smp_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing SMP system");
    
    // Detect available CPUs
    num_cpus = detect_cpus();
    if (num_cpus == 0) {
        KERROR("No CPUs detected");
        return E_INVAL;
    }
    
    // Initialize APIC if multiple CPUs
    if (num_cpus > 1) {
        error_t result = init_apic();
        if (result != SUCCESS) {
            KERROR("Failed to initialize APIC");
            return result;
        }
    }
    
    // Set up boot CPU
    boot_cpu_id = 0;
    cpus[boot_cpu_id].online = true;
    cpus_online = 1;
    
    // Initialize scheduler for boot CPU
    scheduler_init_cpu(boot_cpu_id);
    
    // Start secondary CPUs if available
    if (num_cpus > 1) {
        KINFO("Starting %u secondary CPUs", num_cpus - 1);
        
        for (u32 i = 1; i < num_cpus; i++) {
            // Set up stack for secondary CPU
            u64 stack_top = cpus[i].stack_base + cpus[i].stack_size;
            
            // Push CPU ID on stack for trampoline
            *(u32*)(stack_top - 4) = i;
            
            // Send startup IPI
            send_ipi(i, 0x20, 0x5, 1, 0); // INIT IPI
            hal_sleep_us(10000); // 10ms delay
            send_ipi(i, 0x20, 0x6, 1, 0); // SIPI with trampoline address
            
            KDEBUG("Sent startup IPI to CPU %u", i);
        }
        
        // Wait for all CPUs to come online
        u32 timeout = 1000; // 1 second timeout
        while (cpus_online < num_cpus && timeout > 0) {
            hal_sleep_us(1000);
            timeout--;
        }
        
        if (cpus_online < num_cpus) {
            KWARN("Only %u of %u CPUs came online", cpus_online, num_cpus);
        }
    }
    
    smp_initialized = true;
    
    KINFO("SMP initialized: %u CPUs online", cpus_online);
    return SUCCESS;
}

// Get current CPU ID
u32 smp_get_cpu_id(void) {
    // In a real implementation, this would read from a CPU-specific register
    // For now, return 0 (boot CPU)
    return 0;
}

// Get CPU information
cpu_info_t* smp_get_cpu_info(u32 cpu_id) {
    if (cpu_id >= num_cpus) {
        return NULL;
    }
    return &cpus[cpu_id];
}

// Get number of online CPUs
u32 smp_get_num_cpus(void) {
    return cpus_online;
}

// Check if SMP is enabled
bool smp_is_enabled(void) {
    return smp_initialized && cpus_online > 1;
}

// Send IPI to specific CPU
void smp_send_ipi(u32 cpu_id, u8 vector) {
    if (!smp_is_enabled() || cpu_id >= num_cpus) {
        return;
    }
    
    send_ipi(cpu_id, vector, 0x0, 1, 0); // Fixed delivery mode
}

// Broadcast IPI to all CPUs
void smp_broadcast_ipi(u8 vector) {
    if (!smp_is_enabled()) {
        return;
    }
    
    broadcast_ipi(vector, 0x0); // Fixed delivery mode
}

// TLB shootdown (invalidate TLB on all CPUs)
void smp_tlb_shootdown(virt_addr_t addr, size_t size) {
    if (!smp_is_enabled()) {
        // Single CPU, just invalidate local TLB
        hal_invalidate_tlb_entry(addr);
        return;
    }
    
    // Send IPI to all other CPUs to invalidate their TLBs
    for (u32 i = 0; i < num_cpus; i++) {
        if (i != smp_get_cpu_id() && cpus[i].online) {
            smp_send_ipi(i, 0x30); // TLB shootdown vector
        }
    }
    
    // Invalidate local TLB
    hal_invalidate_tlb_entry(addr);
}

// CPU-specific scheduler functions
void scheduler_init_cpu(u32 cpu_id) {
    if (cpu_id >= num_cpus) {
        return;
    }
    
    cpu_info_t* cpu = &cpus[cpu_id];
    
    // Initialize CPU-specific scheduler state
    // This would set up per-CPU ready queues, etc.
    
    KDEBUG("Initialized scheduler for CPU %u", cpu_id);
}

void scheduler_cpu_loop(u32 cpu_id) {
    if (cpu_id >= num_cpus) {
        return;
    }
    
    cpu_info_t* cpu = &cpus[cpu_id];
    
    // Main scheduler loop for this CPU
    while (1) {
        // Get next thread to run
        thread_t* thread = scheduler_next_thread_cpu(cpu_id);
        
        if (thread) {
            // Switch to thread
            if (cpu->current_thread != thread) {
                context_switch(cpu->current_thread, thread);
                cpu->current_thread = thread;
                cpu->context_switches++;
            }
            
            // Run thread for its time slice
            // This would involve setting up timer and running the thread
        } else {
            // No threads to run, idle
            cpu->idle_time++;
            hal_pause_cpu();
        }
    }
}

thread_t* scheduler_next_thread_cpu(u32 cpu_id) {
    // Get next thread from this CPU's ready queue
    // This would be similar to the single-CPU version but per-CPU
    return scheduler_next_thread();
}

// SMP statistics
void smp_dump_stats(void) {
    KINFO("=== SMP Statistics ===");
    KINFO("Total CPUs: %u", num_cpus);
    KINFO("Online CPUs: %u", cpus_online);
    KINFO("SMP enabled: %s", smp_is_enabled() ? "Yes" : "No");
    
    for (u32 i = 0; i < num_cpus; i++) {
        cpu_info_t* cpu = &cpus[i];
        KINFO("CPU %u: online=%s, boot=%s, context_switches=%u, idle_time=%llu",
              i, cpu->online ? "Yes" : "No", cpu->boot_cpu ? "Yes" : "No",
              cpu->context_switches, cpu->idle_time);
    }
}

// CPU affinity functions
error_t thread_set_cpu_affinity(thread_t* thread, u64 cpu_mask) {
    if (!thread) {
        return E_INVAL;
    }
    
    thread->cpu_affinity = cpu_mask;
    return SUCCESS;
}

u64 thread_get_cpu_affinity(thread_t* thread) {
    if (!thread) {
        return 0;
    }
    
    return thread->cpu_affinity;
}

// Load balancing
void smp_load_balance(void) {
    if (!smp_is_enabled()) {
        return;
    }
    
    // Simple load balancing: move threads from busy CPUs to idle CPUs
    // This is a basic implementation - real load balancing would be more sophisticated
    
    u32 current_cpu = smp_get_cpu_id();
    u32 busiest_cpu = current_cpu;
    u32 idlest_cpu = current_cpu;
    u64 max_load = 0;
    u64 min_load = UINT64_MAX;
    
    // Find busiest and idlest CPUs
    for (u32 i = 0; i < num_cpus; i++) {
        if (!cpus[i].online) {
            continue;
        }
        
        u64 load = cpus[i].total_runtime - cpus[i].idle_time;
        if (load > max_load) {
            max_load = load;
            busiest_cpu = i;
        }
        if (load < min_load) {
            min_load = load;
            idlest_cpu = i;
        }
    }
    
    // If load difference is significant, move some threads
    if (max_load > min_load * 2 && busiest_cpu != idlest_cpu) {
        KDEBUG("Load balancing: moving threads from CPU %u to CPU %u", 
               busiest_cpu, idlest_cpu);
        
        // This would involve moving threads between CPU ready queues
        // Implementation depends on the scheduler design
    }
} 