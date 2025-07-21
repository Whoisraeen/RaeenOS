#ifndef ERROR_H
#define ERROR_H

#include "types.h"

// Success code
#define SUCCESS             0

// General error codes
#define E_FAIL             -1      // Generic failure
#define E_INVAL            -2      // Invalid argument
#define E_NOMEM            -3      // Out of memory
#define E_NOENT            -4      // No such entry
#define E_EXIST            -5      // Already exists
#define E_BUSY             -6      // Resource busy
#define E_TIMEOUT          -7      // Operation timed out
#define E_PERM             -8      // Permission denied
#define E_IO               -9      // I/O error
#define E_NOSPACE          -10     // No space left
#define E_AGAIN            -11     // Try again
#define E_PIPE             -12     // Broken pipe
#define E_RANGE            -13     // Out of range
#define E_DEADLK           -14     // Deadlock detected
#define E_NAMETOOLONG      -15     // Name too long
#define E_NOLCK            -16     // No locks available
#define E_NOSYS            -17     // Function not implemented
#define E_NOTEMPTY         -18     // Directory not empty
#define E_LOOP             -19     // Too many symbolic links
#define E_OVERFLOW         -20     // Value too large
#define E_BADFD            -21     // Bad file descriptor
#define E_ALREADY          -22     // Already initialized/exists
#define E_NOTREADY         -23     // Not ready
#define E_CORRUPT          -24     // Data corruption detected

// System-specific error codes
#define E_HARDWARE         -100    // Hardware error
#define E_DRIVER           -101    // Driver error
#define E_PROTOCOL         -102    // Protocol error
#define E_NETWORK          -103    // Network error
#define E_FILESYSTEM       -104    // Filesystem error
#define E_SECURITY         -105    // Security violation
#define E_MULTIBOOT        -106    // Multiboot error
#define E_BOOTLOADER       -107    // Bootloader error
#define E_MEMORY_MAP       -108    // Memory map error
#define E_INVALID_MAGIC    -109    // Invalid magic number
#define E_UNSUPPORTED      -110    // Unsupported operation/feature

// POSIX error codes for syscalls (positive values for compatibility)
#define ENOSYS             38    // Function not implemented
#define ESRCH              3     // No such process
#define EPERM              1     // Operation not permitted
#define ENOMEM             12    // Out of memory
#define EFAULT             14    // Bad address
#define EINVAL             22    // Invalid argument
#define EINTR              4     // Interrupted system call

// Time structures for system calls
struct timeval {
    long tv_sec;
    long tv_usec;
};

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

struct timespec {
    long tv_sec;
    long tv_nsec;
};

// Error code validation
static inline bool is_error(error_t err) {
    return err < 0;
}

static inline bool is_success(error_t err) {
    return err >= 0;
}

// Error to string conversion (for debugging)
const char* error_to_string(error_t err);

#endif // ERROR_H