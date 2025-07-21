#include "include/interrupts.h"
#include "include/kernel.h"
#include "../hal/include/hal.h"
#include "../process/include/process.h"

// Global interrupt statistics
extern interrupt_stats_t interrupt_statistics;

// Timer interrupt handler - drives preemptive multitasking
void irq_timer(interrupt_context_t* context) {
    (void)context;  // Unused
    
    // Update timer statistics
    interrupt_statistics.irq_counts[IRQ_TIMER]++;
    
    // Call scheduler tick for preemptive multitasking
    scheduler_tick();
    
    // The PIC EOI is sent by interrupt_dispatch automatically
}

// Keyboard interrupt handler
void irq_keyboard(interrupt_context_t* context) {
    (void)context;  // Unused
    
    // Update keyboard statistics
    interrupt_statistics.irq_counts[IRQ_KEYBOARD]++;
    
    // Read scan code from keyboard controller
    u8 scan_code = hal_inb(0x60);
    
    KDEBUG("Keyboard scan code: 0x%02x", scan_code);
    
    // TODO: Process keyboard input when keyboard driver is implemented
    // For now, just acknowledge the interrupt
}

// Real-time clock interrupt handler
void irq_rtc(interrupt_context_t* context) {
    (void)context;  // Unused
    
    // Update RTC statistics
    interrupt_statistics.irq_counts[IRQ_RTC]++;
    
    // Read RTC status register to clear interrupt
    hal_outb(0x70, 0x0C);
    hal_inb(0x71);
    
    KDEBUG("RTC interrupt received");
    
    // TODO: Update system time when RTC driver is implemented
}

// Mouse interrupt handler
void irq_mouse(interrupt_context_t* context) {
    (void)context;  // Unused
    
    // Update mouse statistics
    interrupt_statistics.irq_counts[IRQ_MOUSE]++;
    
    // Read mouse data from PS/2 controller
    u8 mouse_data = hal_inb(0x60);
    
    KDEBUG("Mouse data: 0x%02x", mouse_data);
    
    // TODO: Process mouse input when mouse driver is implemented
}

// Spurious interrupt handler
void irq_spurious(interrupt_context_t* context) {
    (void)context;  // Unused
    
    // Update spurious interrupt statistics
    interrupt_statistics.spurious_interrupts++;
    
    KWARN("Spurious interrupt detected");
    
    // Don't send EOI for spurious interrupts
}