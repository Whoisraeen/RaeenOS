#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Basic type definitions
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef uintptr_t uptr;
typedef intptr_t  sptr;

// Architecture-specific types
#ifdef __x86_64__
typedef u64 reg_t;
typedef u64 addr_t;
typedef u64 phys_addr_t;
typedef u64 virt_addr_t;
#else
typedef u32 reg_t;
typedef u32 addr_t;
typedef u32 phys_addr_t;
typedef u32 virt_addr_t;
#endif

// Process and thread types
typedef u32 pid_t;
typedef u32 tid_t;
typedef u32 uid_t;
typedef u32 gid_t;

// File system types
typedef u64 inode_t;
typedef s64 off_t;
typedef u32 mode_t;
typedef u32 dev_t;

// Time types
typedef u64 time_t;
typedef u64 timestamp_t;

// Error codes
typedef enum {
    SUCCESS = 0,
    E_GENERIC = -1,
    E_NOMEM = -2,
    E_INVAL = -3,
    E_PERM = -4,
    E_NOENT = -5,
    E_IO = -6,
    E_BUSY = -7,
    E_EXIST = -8,
    E_NOTDIR = -9,
    E_ISDIR = -10,
    E_FBIG = -11,
    E_NOSPC = -12,
    E_SPIPE = -13,
    E_ROFS = -14,
    E_MLINK = -15,
    E_PIPE = -16,
    E_DOM = -17,
    E_RANGE = -18,
    E_AGAIN = -19,
    E_INPROGRESS = -20,
    E_ALREADY = -21,
    E_NOTSOCK = -22,
    E_DESTADDRREQ = -23,
    E_MSGSIZE = -24,
    E_PROTOTYPE = -25,
    E_NOPROTOOPT = -26,
    E_PROTONOSUPPORT = -27,
    E_SOCKTNOSUPPORT = -28,
    E_OPNOTSUPP = -29,
    E_PFNOSUPPORT = -30,
    E_AFNOSUPPORT = -31,
    E_ADDRINUSE = -32,
    E_ADDRNOTAVAIL = -33,
    E_NETDOWN = -34,
    E_NETUNREACH = -35,
    E_NETRESET = -36,
    E_CONNABORTED = -37,
    E_CONNRESET = -38,
    E_NOBUFS = -39,
    E_ISCONN = -40,
    E_NOTCONN = -41,
    E_SHUTDOWN = -42,
    E_TOOMANYREFS = -43,
    E_TIMEDOUT = -44,
    E_CONNREFUSED = -45,
    E_LOOP = -46,
    E_NAMETOOLONG = -47,
    E_HOSTDOWN = -48,
    E_HOSTUNREACH = -49,
    E_NOTEMPTY = -50,
    E_MAX = -51
} error_t;

// Status flags
#define STATUS_OK        0x00000000
#define STATUS_ERROR     0x80000000
#define STATUS_PENDING   0x40000000
#define STATUS_COMPLETE  0x20000000

// Alignment macros
#define ALIGN_UP(x, align)   (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
#define IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

// Bit manipulation macros
#define BIT(n)           (1UL << (n))
#define BITMASK(n)       (BIT(n) - 1)
#define SET_BIT(x, n)    ((x) |= BIT(n))
#define CLEAR_BIT(x, n)  ((x) &= ~BIT(n))
#define TOGGLE_BIT(x, n) ((x) ^= BIT(n))
#define TEST_BIT(x, n)   (((x) & BIT(n)) != 0)

// Min/Max macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Array size macro
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Null pointer
#ifndef NULL
#define NULL ((void*)0)
#endif

// Boolean values for C code that doesn't use stdbool.h
#ifndef __cplusplus
#ifndef true
#define true  1
#define false 0
#endif
#endif

#endif // TYPES_H