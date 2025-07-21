#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Standard C Library for RaeenOS
// Provides system call wrappers and standard library functions

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

typedef int pid_t;
typedef int uid_t;
typedef int gid_t;
typedef int mode_t;
typedef int64_t off_t;
typedef uint64_t size_t;
typedef int64_t ssize_t;
typedef int64_t time_t;
typedef int64_t clock_t;

// File descriptor type
typedef int fd_t;

// Signal types
typedef int sig_atomic_t;
typedef unsigned int sigset_t;

// Process and thread types
typedef int tid_t;
typedef int pthread_t;

// ============================================================================
// CONSTANTS AND MACROS
// ============================================================================

// File access modes
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0100
#define O_EXCL      0x0200
#define O_TRUNC     0x0400
#define O_APPEND    0x0800
#define O_NONBLOCK  0x1000
#define O_SYNC      0x2000

// File permissions
#define S_IRWXU     0700    // User read, write, execute
#define S_IRUSR     0400    // User read
#define S_IWUSR     0200    // User write
#define S_IXUSR     0100    // User execute
#define S_IRWXG     0070    // Group read, write, execute
#define S_IRGRP     0040    // Group read
#define S_IWGRP     0020    // Group write
#define S_IXGRP     0010    // Group execute
#define S_IRWXO     0007    // Others read, write, execute
#define S_IROTH     0004    // Others read
#define S_IWOTH     0002    // Others write
#define S_IXOTH     0001    // Others execute

// Memory protection flags
#define PROT_NONE   0x0
#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define PROT_EXEC   0x4

// Memory mapping flags
#define MAP_PRIVATE     0x01
#define MAP_SHARED      0x02
#define MAP_FIXED       0x04
#define MAP_ANONYMOUS   0x08
#define MAP_STACK       0x10

// Signal numbers
#define SIGHUP     1
#define SIGINT     2
#define SIGQUIT    3
#define SIGILL     4
#define SIGTRAP    5
#define SIGABRT    6
#define SIGBUS     7
#define SIGFPE     8
#define SIGKILL    9
#define SIGUSR1    10
#define SIGSEGV    11
#define SIGUSR2    12
#define SIGPIPE    13
#define SIGALRM    14
#define SIGTERM    15
#define SIGSTKFLT  16
#define SIGCHLD    17
#define SIGCONT    18
#define SIGSTOP    19
#define SIGTSTP    20
#define SIGTTIN    21
#define SIGTTOU    22
#define SIGURG     23
#define SIGXCPU    24
#define SIGXFSZ    25
#define SIGVTALRM  26
#define SIGPROF    27
#define SIGWINCH   28
#define SIGIO      29
#define SIGPWR     30
#define SIGSYS     31

// Signal actions
#define SIG_DFL    0
#define SIG_IGN    1
#define SIG_ERR    -1

// Wait options
#define WNOHANG    0x00000001
#define WUNTRACED  0x00000002

// Standard file descriptors
#define STDIN_FILENO   0
#define STDOUT_FILENO  1
#define STDERR_FILENO  2

// Error codes
#define EPERM       1
#define ENOENT      2
#define ESRCH       3
#define EINTR       4
#define EIO         5
#define ENXIO       6
#define E2BIG       7
#define ENOEXEC     8
#define EBADF       9
#define ECHILD      10
#define EAGAIN      11
#define ENOMEM      12
#define EACCES      13
#define EFAULT      14
#define ENOTBLK     15
#define EBUSY       16
#define EEXIST      17
#define EXDEV       18
#define ENODEV      19
#define ENOTDIR     20
#define EISDIR      21
#define EINVAL      22
#define ENFILE      23
#define EMFILE      24
#define ENOTTY      25
#define ETXTBSY     26
#define EFBIG       27
#define ENOSPC      28
#define ESPIPE      29
#define EROFS       30
#define EMLINK      31
#define EPIPE       32
#define EDOM        33
#define ERANGE      34
#define EDEADLK     35
#define ENAMETOOLONG 36
#define ENOLCK      37
#define ENOSYS      38
#define ENOTEMPTY   39
#define ELOOP       40
#define ENOMSG      42
#define EIDRM       43
#define ECHRNG      44
#define EL2NSYNC    45
#define EL3HLT      46
#define EL3RST      47
#define ELNRNG      48
#define EUNATCH     49
#define ENOCSI      50
#define EL2HLT      51
#define EBADE       52
#define EBADR       53
#define EXFULL      54
#define ENOANO      55
#define EBADRQC     56
#define EBADSLT     57
#define EDEADLOCK   58
#define EBFONT      59
#define ENOSTR      60
#define ENODATA     61
#define ETIME       62
#define ENOSR       63
#define ENONET      64
#define ENOPKG      65
#define EREMOTE     66
#define ENOLINK     67
#define EADV        68
#define ESRMNT      69
#define ECOMM       70
#define EPROTO      71
#define EMULTIHOP   72
#define EDOTDOT     73
#define EBADMSG     74
#define EOVERFLOW   75
#define ENOTUNIQ    76
#define EBADFD      77
#define EREMCHG     78
#define ELIBACC     79
#define ELIBBAD     80
#define ELIBSCN     81
#define ELIBMAX     82
#define ELIBEXEC    83
#define EILSEQ      84
#define ERESTART    85
#define ESTRPIPE    86
#define EUSERS      87
#define ENOTSOCK    88
#define EDESTADDRREQ 89
#define EMSGSIZE    90
#define EPROTOTYPE  91
#define ENOPROTOOPT 92
#define EPROTONOSUPPORT 93
#define ESOCKTNOSUPPORT 94
#define EOPNOTSUPP  95
#define EPFNOSUPPORT 96
#define EAFNOSUPPORT 97
#define EADDRINUSE  98
#define EADDRNOTAVAIL 99
#define ENETDOWN    100
#define ENETUNREACH 101
#define ENETRESET   102
#define ECONNABORTED 103
#define ECONNRESET  104
#define ENOBUFS     105
#define EISCONN     106
#define ENOTCONN    107
#define ESHUTDOWN   108
#define ETOOMANYREFS 109
#define ETIMEDOUT   110
#define ECONNREFUSED 111
#define EHOSTDOWN   112
#define EHOSTUNREACH 113
#define EALREADY    114
#define EINPROGRESS 115
#define ESTALE      116
#define EUCLEAN     117
#define ENOTNAM     118
#define ENAVAIL     119
#define EISNAM      120
#define EREMOTEIO   121
#define EDQUOT      122
#define ENOMEDIUM   123
#define EMEDIUMTYPE 124
#define ECANCELED   125
#define ENOKEY      126
#define EKEYEXPIRED 127
#define EKEYREVOKED 128
#define EKEYREJECTED 129
#define EOWNERDEAD  130
#define ENOTRECOVERABLE 131
#define ERFKILL     132
#define EHWPOISON   133

// ============================================================================
// STRUCTURE DEFINITIONS
// ============================================================================

// File status structure
struct stat {
    uint64_t st_dev;
    uint64_t st_ino;
    uint32_t st_mode;
    uint32_t st_nlink;
    uint32_t st_uid;
    uint32_t st_gid;
    uint64_t st_rdev;
    uint64_t st_size;
    uint64_t st_blksize;
    uint64_t st_blocks;
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
};

// Directory entry structure
struct dirent {
    uint64_t d_ino;
    int64_t d_off;
    uint16_t d_reclen;
    uint8_t d_type;
    char d_name[256];
};

// Signal action structure
struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};

// Time structure
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct timeval {
    time_t tv_sec;
    long tv_usec;
};

// Process times structure
struct tms {
    clock_t tms_utime;
    clock_t tms_stime;
    clock_t tms_cutime;
    clock_t tms_cstime;
};

// Resource limits structure
struct rlimit {
    uint64_t rlim_cur;
    uint64_t rlim_max;
};

// Resource usage structure
struct rusage {
    struct timeval ru_utime;
    struct timeval ru_stime;
    uint64_t ru_maxrss;
    uint64_t ru_ixrss;
    uint64_t ru_idrss;
    uint64_t ru_isrss;
    uint64_t ru_minflt;
    uint64_t ru_majflt;
    uint64_t ru_nswap;
    uint64_t ru_inblock;
    uint64_t ru_oublock;
    uint64_t ru_msgsnd;
    uint64_t ru_msgrcv;
    uint64_t ru_nsignals;
    uint64_t ru_nvcsw;
    uint64_t ru_nivcsw;
};

// ============================================================================
// SYSTEM CALL WRAPPERS
// ============================================================================

// Process management
pid_t fork(void);
int execve(const char *pathname, char *const argv[], char *const envp[]);
pid_t waitpid(pid_t pid, int *wstatus, int options);
int kill(pid_t pid, int sig);
pid_t getpid(void);
pid_t getppid(void);
int setuid(uid_t uid);
int setgid(gid_t gid);
uid_t getuid(void);
gid_t getgid(void);

// File operations
int open(const char *pathname, int flags, mode_t mode);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int fstat(int fd, struct stat *statbuf);
int stat(const char *pathname, struct stat *statbuf);
int chmod(const char *pathname, mode_t mode);
int chown(const char *pathname, uid_t owner, gid_t group);
int unlink(const char *pathname);
int mkdir(const char *pathname, mode_t mode);
int rmdir(const char *pathname);
int rename(const char *oldpath, const char *newpath);
int link(const char *oldpath, const char *newpath);
int symlink(const char *target, const char *linkpath);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);

// Directory operations
int opendir(const char *name);
struct dirent *readdir(int fd);
int closedir(int fd);

// Memory management
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
void *brk(void *addr);
void *sbrk(intptr_t increment);

// Signal handling
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigsuspend(const sigset_t *mask);
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);
int sigismember(const sigset_t *set, int signum);

// Time functions
time_t time(time_t *tloc);
int gettimeofday(struct timeval *tv, void *tz);
int settimeofday(const struct timeval *tv, const void *tz);
unsigned int alarm(unsigned int seconds);
int sleep(unsigned int seconds);
int usleep(unsigned long usec);
int nanosleep(const struct timespec *req, struct timespec *rem);

// System information
long sysconf(int name);
int getrlimit(int resource, struct rlimit *rlim);
int setrlimit(int resource, const struct rlimit *rlim);
int getrusage(int who, struct rusage *usage);

// Process scheduling
int sched_yield(void);
int sched_getparam(pid_t pid, struct sched_param *param);
int sched_setparam(pid_t pid, const struct sched_param *param);
int sched_getscheduler(pid_t pid);
int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);

// ============================================================================
// STANDARD LIBRARY FUNCTIONS
// ============================================================================

// Memory allocation
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void *memalign(size_t alignment, size_t size);
size_t malloc_usable_size(void *ptr);

// String operations
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
char *strtok(char *str, const char *delim);
char *strtok_r(char *str, const char *delim, char **saveptr);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char *strpbrk(const char *s, const char *accept);

// Memory operations
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);

// Character classification
int isalpha(int c);
int isdigit(int c);
int isalnum(int c);
int isspace(int c);
int isupper(int c);
int islower(int c);
int isprint(int c);
int iscntrl(int c);
int ispunct(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);

// Conversion functions
int atoi(const char *nptr);
long atol(const char *nptr);
long long atoll(const char *nptr);
double atof(const char *nptr);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);
double strtod(const char *nptr, char **endptr);

// Random number generation
int rand(void);
void srand(unsigned int seed);
long random(void);
void srandom(unsigned int seed);

// Environment
char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);
int putenv(char *string);
int clearenv(void);

// Working directory
char *getcwd(char *buf, size_t size);
int chdir(const char *path);

// Process termination
void exit(int status);
void _exit(int status);
void abort(void);

// System calls
int system(const char *command);
int access(const char *pathname, int mode);
int pipe(int pipefd[2]);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int fcntl(int fd, int cmd, ...);
int ioctl(int fd, unsigned long request, ...);

// Error handling
extern int errno;
char *strerror(int errnum);
void perror(const char *s);

// ============================================================================
// THREADING SUPPORT
// ============================================================================

// Thread creation and management
int pthread_create(pthread_t *thread, const void *attr, void *(*start_routine)(void *), void *arg);
int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);
void pthread_exit(void *retval);
pthread_t pthread_self(void);
int pthread_equal(pthread_t t1, pthread_t t2);

// Thread attributes
typedef struct {
    size_t stack_size;
    void *stack_addr;
    int detachstate;
    int schedpolicy;
    struct sched_param schedparam;
    int inheritsched;
    int scope;
} pthread_attr_t;

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);

// Mutex support
typedef struct {
    void *data;
} pthread_mutex_t;

typedef struct {
    void *data;
} pthread_mutexattr_t;

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

// Condition variables
typedef struct {
    void *data;
} pthread_cond_t;

typedef struct {
    void *data;
} pthread_condattr_t;

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

// Thread-local storage
typedef unsigned int pthread_key_t;

int pthread_key_create(pthread_key_t *key, void (*destr_function)(void *));
int pthread_key_delete(pthread_key_t key);
void *pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);

// ============================================================================
// NETWORKING SUPPORT
// ============================================================================

// Socket types
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3
#define SOCK_RDM        4
#define SOCK_SEQPACKET  5

// Address families
#define AF_UNSPEC       0
#define AF_UNIX         1
#define AF_INET         2
#define AF_INET6        10

// Protocol families
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6

// Socket options
#define SOL_SOCKET      1
#define SO_REUSEADDR    2
#define SO_TYPE         3
#define SO_ERROR        4
#define SO_DONTROUTE    5
#define SO_BROADCAST    6
#define SO_SNDBUF       7
#define SO_RCVBUF       8
#define SO_KEEPALIVE    9
#define SO_OOBINLINE    10
#define SO_NO_CHECK     11
#define SO_PRIORITY     12
#define SO_LINGER       13
#define SO_BSDCOMPAT    14
#define SO_REUSEPORT    15

// Socket address structure
struct sockaddr {
    uint16_t sa_family;
    char sa_data[14];
};

struct sockaddr_in {
    uint16_t sin_family;
    uint16_t sin_port;
    uint32_t sin_addr;
    char sin_zero[8];
};

struct sockaddr_in6 {
    uint16_t sin6_family;
    uint16_t sin6_port;
    uint32_t sin6_flowinfo;
    uint8_t sin6_addr[16];
    uint32_t sin6_scope_id;
};

// Network functions
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, size_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, size_t *addrlen);
int connect(int sockfd, const struct sockaddr *addr, size_t addrlen);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, size_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, size_t *addrlen);
int shutdown(int sockfd, int how);
int close(int fd);

// Network byte order functions
uint16_t htons(uint16_t hostshort);
uint16_t ntohs(uint16_t netshort);
uint32_t htonl(uint32_t hostlong);
uint32_t ntohl(uint32_t netlong);

// DNS functions
struct hostent {
    char *h_name;
    char **h_aliases;
    int h_addrtype;
    int h_length;
    char **h_addr_list;
};

struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyaddr(const void *addr, size_t len, int type);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// String formatting
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

// Input/output
int printf(const char *format, ...);
int fprintf(int stream, const char *format, ...);
int scanf(const char *format, ...);
int fscanf(int stream, const char *format, ...);
int sscanf(const char *str, const char *format, ...);

// File operations
int fflush(int stream);
int fclose(int stream);
int fseek(int stream, long offset, int whence);
long ftell(int stream);
int feof(int stream);
int ferror(int stream);
void clearerr(int stream);

// Buffer operations
void setbuf(int stream, char *buf);
int setvbuf(int stream, char *buf, int mode, size_t size);

// ============================================================================
// RAEENOS-SPECIFIC EXTENSIONS
// ============================================================================

// Game Mode support
int raeen_enable_game_mode(void);
int raeen_disable_game_mode(void);
int raeen_is_game_mode_enabled(void);

// AI acceleration hints
int raeen_ai_accelerate_hint(int workload_type);
int raeen_ai_get_capabilities(void);

// Performance monitoring
int raeen_get_performance_stats(struct rusage *stats);
int raeen_set_performance_profile(int profile);

// Security extensions
int raeen_secure_alloc(void **ptr, size_t size);
int raeen_secure_free(void *ptr);
int raeen_verify_integrity(const char *path);

#endif // STDLIB_H 