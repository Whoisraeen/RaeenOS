#ifndef MEMORY_INTEGRATION_H
#define MEMORY_INTEGRATION_H

#include "../core/include/types.h"
#include "../core/include/error.h"

// Forward declarations to avoid circular dependencies
struct process;
struct interrupt_frame;

// Function prototypes for integration
error_t page_fault_init(void);
void page_fault_handler(struct interrupt_frame* frame);
struct process* get_current_process(void);
error_t register_interrupt_handler(int vector, void (*handler)(struct interrupt_frame*));

// Missing macros and definitions
#define VIRT_TO_PHYS(addr) ((phys_addr_t)(addr))
#define PHYS_TO_VIRT(addr) ((virt_addr_t)(addr))

// Spinlock implementation (simplified)
typedef struct {
    volatile int locked;
} spinlock_t;

#define SPINLOCK_INIT {0}

static inline void spinlock_init(spinlock_t* lock) {
    lock->locked = 0;
}

static inline void spinlock_acquire(spinlock_t* lock) {
    while (__sync_lock_test_and_set(&lock->locked, 1)) {
        // Spin
    }
}

static inline void spinlock_release(spinlock_t* lock) {
    __sync_lock_release(&lock->locked);
}

// RWLock implementation (simplified)
typedef struct {
    volatile int readers;
    volatile int writer;
} rwlock_t;

static inline void rwlock_init(rwlock_t* lock) {
    lock->readers = 0;
    lock->writer = 0;
}

static inline void rwlock_acquire_read(rwlock_t* lock) {
    while (lock->writer) {
        // Wait for writer
    }
    __sync_fetch_and_add(&lock->readers, 1);
}

static inline void rwlock_release_read(rwlock_t* lock) {
    __sync_fetch_and_sub(&lock->readers, 1);
}

static inline void rwlock_acquire_write(rwlock_t* lock) {
    while (__sync_lock_test_and_set(&lock->writer, 1)) {
        // Wait for lock
    }
    while (lock->readers > 0) {
        // Wait for readers
    }
}

static inline void rwlock_release_write(rwlock_t* lock) {
    __sync_lock_release(&lock->writer);
}

#endif // MEMORY_INTEGRATION_H