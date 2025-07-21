#ifndef RAEEEN_SYSCALL_H
#define RAEEEN_SYSCALL_H

#include <stdint.h>
#include <stddef.h>
#include "types.h"

// System call numbers
#define SYS_EXIT            1
#define SYS_FORK            2
#define SYS_READ            3
#define SYS_WRITE           4
#define SYS_OPEN            5
#define SYS_CLOSE           6
#define SYS_EXECVE          7
#define SYS_WAITPID         8
#define SYS_KILL            9
#define SYS_GETPID          10
#define SYS_SLEEP           11
#define SYS_YIELD           12
#define SYS_MMAP            13
#define SYS_MUNMAP          14
#define SYS_BRK             15
#define SYS_SOCKET          16
#define SYS_CONNECT         17
#define SYS_ACCEPT          18
#define SYS_SEND            19
#define SYS_RECV            20
#define SYS_PIPE            21
#define SYS_DUP2            22
#define SYS_CHDIR           23
#define SYS_GETCWD          24
#define SYS_MKDIR           25
#define SYS_RMDIR           26
#define SYS_LINK            27
#define SYS_UNLINK          28
#define SYS_STAT            29
#define SYS_FSTAT           30
#define SYS_FCNTL           31
#define SYS_IOCTL           32
#define SYS_SIGACTION       33
#define SYS_SIGPROCMASK     34
#define SYS_SIGSUSPEND      35
#define SYS_ALARM           36
#define SYS_GETTIMEOFDAY    37
#define SYS_SETTIMEOFDAY    38
#define SYS_GETUID          39
#define SYS_SETUID          40
#define SYS_GETGID          41
#define SYS_SETGID          42
#define SYS_CHMOD           43
#define SYS_CHOWN           44
#define SYS_UMASK           45
#define SYS_GETPPID         46
#define SYS_SETSID          47
#define SYS_GETSID          48
#define SYS_SYNC            49
#define SYS_FSYNC           50
#define SYS_TRUNCATE        51
#define SYS_FTRUNCATE       52
#define SYS_READDIR         53
#define SYS_MKNOD           54
#define SYS_MOUNT           55
#define SYS_UMOUNT          56
#define SYS_SYSCONF         57
#define SYS_GETRLIMIT       58
#define SYS_SETRLIMIT       59
#define SYS_GETPRIORITY     60
#define SYS_SETPRIORITY     61
#define SYS_SCHED_YIELD     62
#define SYS_SCHED_GETPARAM  63
#define SYS_SCHED_SETPARAM  64
#define SYS_SCHED_GETSCHEDULER 65
#define SYS_SCHED_SETSCHEDULER 66
#define SYS_CLONE           67
#define SYS_VFORK           68
#define SYS_EXIT_GROUP      69
#define SYS_WAIT4           70
#define SYS_SETSOCKOPT      71
#define SYS_GETSOCKOPT      72
#define SYS_SHUTDOWN        73
#define SYS_BIND            74
#define SYS_LISTEN          75
#define SYS_GETSOCKNAME     76
#define SYS_GETPEERNAME     77
#define SYS_SENDTO          78
#define SYS_RECVFROM        79
#define SYS_SENDMSG         80
#define SYS_RECVMSG         81
#define SYS_EPOLL_CREATE    82
#define SYS_EPOLL_CTL       83
#define SYS_EPOLL_WAIT      84
#define SYS_SIGNALFD        85
#define SYS_TIMERFD_CREATE  86
#define SYS_TIMERFD_SETTIME 87
#define SYS_TIMERFD_GETTIME 88
#define SYS_EVENTFD         89
#define SYS_FALLOCATE       90
#define SYS_TIMER_CREATE    91
#define SYS_TIMER_SETTIME   92
#define SYS_TIMER_GETTIME   93
#define SYS_TIMER_DELETE    94
#define SYS_CLOCK_GETTIME   95
#define SYS_CLOCK_SETTIME   96
#define SYS_CLOCK_GETRES    97
#define SYS_NANOSLEEP       98
#define SYS_GETRANDOM       99
#define SYS_MEMFD_CREATE    100

// Maximum system call number
#define MAX_SYSCALL         100

// System call error codes
#define SYS_SUCCESS         0
#define SYS_ERROR           -1
#define SYS_EINVAL          -2
#define SYS_ENOMEM          -3
#define SYS_EPERM           -4
#define SYS_ENOENT          -5
#define SYS_EEXIST          -6
#define SYS_EBUSY           -7
#define SYS_EAGAIN          -8
#define SYS_EINTR           -9
#define SYS_EFAULT          -10
#define SYS_EMFILE          -11
#define SYS_ENFILE          -12
#define SYS_ESPIPE          -13
#define SYS_EROFS           -14
#define SYS_ENOSPC          -15
#define SYS_EDQUOT          -16
#define SYS_EFBIG           -17
#define SYS_ETIMEDOUT       -18
#define SYS_ECONNREFUSED    -19
#define SYS_ECONNRESET      -20
#define SYS_EHOSTUNREACH    -21
#define SYS_ENETUNREACH     -22
#define SYS_EMSGSIZE        -23
#define SYS_ENOBUFS         -24
#define SYS_EADDRINUSE      -25
#define SYS_EADDRNOTAVAIL   -26
#define SYS_EISCONN         -27
#define SYS_ENOTCONN        -28
#define SYS_ESHUTDOWN       -29
#define SYS_EPIPE           -30
#define SYS_EBADF           -31
#define SYS_ECHILD          -32
#define SYS_ESRCH           -33
#define SYS_EDEADLK         -34
#define SYS_ENAMETOOLONG    -35
#define SYS_ENOTEMPTY       -36
#define SYS_ELOOP           -37
#define SYS_EOVERFLOW       -38
#define SYS_ERANGE          -39
#define SYS_EDOM            -40
#define SYS_EMLINK          -41
#define SYS_EMULTIHOP       -42
#define SYS_ENOLINK         -43
#define SYS_ENOMSG          -44
#define SYS_EPROTO          -45
#define SYS_EPROTONOSUPPORT -46
#define SYS_ESOCKTNOSUPPORT -47
#define SYS_EOPNOTSUPP      -48
#define SYS_ENOTSUP         -49
#define SYS_EAFNOSUPPORT    -50
#define SYS_EPFNOSUPPORT    -51
#define SYS_EACCES          -52
#define SYS_ETOOMANYREFS    -53
#define SYS_EDESTADDRREQ    -54
#define SYS_EPROTOTYPE      -55
#define SYS_ENOTSOCK        -56
#define SYS_EALREADY        -57
#define SYS_EINPROGRESS     -58
#define SYS_ESTALE          -59
#define SYS_EUCLEAN         -60
#define SYS_ENOTNAM         -61
#define SYS_ENAVAIL         -62
#define SYS_EISNAM          -63
#define SYS_EREMOTEIO       -64
#define SYS_EDQUOT          -65
#define SYS_ENOMEDIUM       -66
#define SYS_EMEDIUMTYPE     -67
#define SYS_ECANCELED       -68
#define SYS_ENOKEY          -69
#define SYS_EKEYEXPIRED     -70
#define SYS_EKEYREVOKED     -71
#define SYS_EKEYREJECTED    -72
#define SYS_EOWNERDEAD      -73
#define SYS_ENOTRECOVERABLE -74
#define SYS_ERFKILL         -75
#define SYS_EHWPOISON       -76

// System call parameter structures
typedef struct {
    int fd;
    void *buf;
    size_t count;
} syscall_read_t;

typedef struct {
    int fd;
    const void *buf;
    size_t count;
} syscall_write_t;

typedef struct {
    const char *pathname;
    int flags;
    mode_t mode;
} syscall_open_t;

typedef struct {
    const char *filename;
    char *const argv[];
    char *const envp[];
} syscall_execve_t;

typedef struct {
    void *addr;
    size_t length;
    int prot;
    int flags;
    int fd;
    off_t offset;
} syscall_mmap_t;

typedef struct {
    void *addr;
    size_t length;
} syscall_munmap_t;

typedef struct {
    int domain;
    int type;
    int protocol;
} syscall_socket_t;

typedef struct {
    int sockfd;
    const struct sockaddr *addr;
    socklen_t addrlen;
} syscall_connect_t;

typedef struct {
    int sockfd;
    struct sockaddr *addr;
    socklen_t *addrlen;
} syscall_accept_t;

typedef struct {
    int sockfd;
    const void *buf;
    size_t len;
    int flags;
} syscall_send_t;

typedef struct {
    int sockfd;
    void *buf;
    size_t len;
    int flags;
} syscall_recv_t;

typedef struct {
    int pipefd[2];
} syscall_pipe_t;

typedef struct {
    const char *path;
    mode_t mode;
} syscall_mkdir_t;

typedef struct {
    const char *path;
} syscall_rmdir_t;

typedef struct {
    const char *oldpath;
    const char *newpath;
} syscall_link_t;

typedef struct {
    const char *pathname;
} syscall_unlink_t;

typedef struct {
    const char *path;
    struct stat *statbuf;
} syscall_stat_t;

typedef struct {
    int fd;
    struct stat *statbuf;
} syscall_fstat_t;

typedef struct {
    int fd;
    int cmd;
    void *arg;
} syscall_fcntl_t;

typedef struct {
    int fd;
    unsigned long request;
    void *arg;
} syscall_ioctl_t;

typedef struct {
    int sig;
    const struct sigaction *act;
    struct sigaction *oact;
} syscall_sigaction_t;

typedef struct {
    int how;
    const sigset_t *set;
    sigset_t *oldset;
} syscall_sigprocmask_t;

typedef struct {
    const sigset_t *mask;
} syscall_sigsuspend_t;

typedef struct {
    unsigned int seconds;
} syscall_alarm_t;

typedef struct {
    struct timeval *tv;
    struct timezone *tz;
} syscall_gettimeofday_t;

typedef struct {
    const struct timeval *tv;
    const struct timezone *tz;
} syscall_settimeofday_t;

typedef struct {
    const char *path;
    mode_t mode;
    dev_t dev;
} syscall_mknod_t;

typedef struct {
    const char *source;
    const char *target;
    const char *filesystemtype;
    unsigned long mountflags;
    const void *data;
} syscall_mount_t;

typedef struct {
    const char *target;
    int flags;
} syscall_umount_t;

typedef struct {
    int name;
} syscall_sysconf_t;

typedef struct {
    int resource;
    struct rlimit *rlim;
} syscall_getrlimit_t;

typedef struct {
    int resource;
    const struct rlimit *rlim;
} syscall_setrlimit_t;

typedef struct {
    int which;
    id_t who;
} syscall_getpriority_t;

typedef struct {
    int which;
    id_t who;
    int prio;
} syscall_setpriority_t;

typedef struct {
    int which;
    const struct sched_param *param;
} syscall_sched_getparam_t;

typedef struct {
    int which;
    const struct sched_param *param;
} syscall_sched_setparam_t;

typedef struct {
    int which;
} syscall_sched_getscheduler_t;

typedef struct {
    int which;
    int policy;
    const struct sched_param *param;
} syscall_sched_setscheduler_t;

typedef struct {
    unsigned long flags;
    void *child_stack;
    void *ptid;
    void *tls;
    void *ctid;
} syscall_clone_t;

typedef struct {
    pid_t pid;
    int *wstatus;
    int options;
    struct rusage *rusage;
} syscall_wait4_t;

typedef struct {
    int sockfd;
    int level;
    int optname;
    const void *optval;
    socklen_t optlen;
} syscall_setsockopt_t;

typedef struct {
    int sockfd;
    int level;
    int optname;
    void *optval;
    socklen_t *optlen;
} syscall_getsockopt_t;

typedef struct {
    int sockfd;
    int how;
} syscall_shutdown_t;

typedef struct {
    int sockfd;
    const struct sockaddr *addr;
    socklen_t addrlen;
} syscall_bind_t;

typedef struct {
    int sockfd;
    int backlog;
} syscall_listen_t;

typedef struct {
    int sockfd;
    struct sockaddr *addr;
    socklen_t *addrlen;
} syscall_getsockname_t;

typedef struct {
    int sockfd;
    struct sockaddr *addr;
    socklen_t *addrlen;
} syscall_getpeername_t;

typedef struct {
    int sockfd;
    const void *buf;
    size_t len;
    int flags;
    const struct sockaddr *dest_addr;
    socklen_t addrlen;
} syscall_sendto_t;

typedef struct {
    int sockfd;
    void *buf;
    size_t len;
    int flags;
    struct sockaddr *src_addr;
    socklen_t *addrlen;
} syscall_recvfrom_t;

typedef struct {
    int sockfd;
    const struct msghdr *msg;
    int flags;
} syscall_sendmsg_t;

typedef struct {
    int sockfd;
    struct msghdr *msg;
    int flags;
} syscall_recvmsg_t;

typedef struct {
    int size;
} syscall_epoll_create_t;

typedef struct {
    int epfd;
    int op;
    int fd;
    struct epoll_event *event;
} syscall_epoll_ctl_t;

typedef struct {
    int epfd;
    struct epoll_event *events;
    int maxevents;
    int timeout;
} syscall_epoll_wait_t;

typedef struct {
    sigset_t *mask;
    int flags;
} syscall_signalfd_t;

typedef struct {
    int clockid;
    int flags;
} syscall_timerfd_create_t;

typedef struct {
    int fd;
    int flags;
    const struct itimerspec *new_value;
    struct itimerspec *old_value;
} syscall_timerfd_settime_t;

typedef struct {
    int fd;
    struct itimerspec *curr_value;
} syscall_timerfd_gettime_t;

typedef struct {
    unsigned int initval;
    int flags;
} syscall_eventfd_t;

typedef struct {
    int fd;
    int mode;
    off_t offset;
    off_t len;
} syscall_fallocate_t;

typedef struct {
    clockid_t clockid;
    struct sigevent *sevp;
    timer_t *timerid;
} syscall_timer_create_t;

typedef struct {
    timer_t timerid;
    int flags;
    const struct itimerspec *new_value;
    struct itimerspec *old_value;
} syscall_timer_settime_t;

typedef struct {
    timer_t timerid;
    struct itimerspec *curr_value;
} syscall_timer_gettime_t;

typedef struct {
    timer_t timerid;
} syscall_timer_delete_t;

typedef struct {
    clockid_t clk_id;
    struct timespec *tp;
} syscall_clock_gettime_t;

typedef struct {
    clockid_t clk_id;
    const struct timespec *tp;
} syscall_clock_settime_t;

typedef struct {
    clockid_t clk_id;
    struct timespec *res;
} syscall_clock_getres_t;

typedef struct {
    const struct timespec *req;
    struct timespec *rem;
} syscall_nanosleep_t;

typedef struct {
    void *buf;
    size_t buflen;
    unsigned int flags;
} syscall_getrandom_t;

typedef struct {
    const char *name;
    unsigned int flags;
} syscall_memfd_create_t;

// System call handler function type
typedef int64_t (*syscall_handler_t)(void *args);

// System call table entry
typedef struct {
    syscall_handler_t handler;
    const char *name;
    int num_args;
} syscall_entry_t;

// Function prototypes
void syscall_init(void);
int64_t syscall_handler(uint64_t number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
void syscall_register(int number, syscall_handler_t handler, const char *name, int num_args);

// Individual system call handlers
int64_t sys_exit(void *args);
int64_t sys_fork(void *args);
int64_t sys_read(void *args);
int64_t sys_write(void *args);
int64_t sys_open(void *args);
int64_t sys_close(void *args);
int64_t sys_execve(void *args);
int64_t sys_waitpid(void *args);
int64_t sys_kill(void *args);
int64_t sys_getpid(void *args);
int64_t sys_sleep(void *args);
int64_t sys_yield(void *args);
int64_t sys_mmap(void *args);
int64_t sys_munmap(void *args);
int64_t sys_brk(void *args);
int64_t sys_socket(void *args);
int64_t sys_connect(void *args);
int64_t sys_accept(void *args);
int64_t sys_send(void *args);
int64_t sys_recv(void *args);
int64_t sys_pipe(void *args);
int64_t sys_dup2(void *args);
int64_t sys_chdir(void *args);
int64_t sys_getcwd(void *args);
int64_t sys_mkdir(void *args);
int64_t sys_rmdir(void *args);
int64_t sys_link(void *args);
int64_t sys_unlink(void *args);
int64_t sys_stat(void *args);
int64_t sys_fstat(void *args);
int64_t sys_fcntl(void *args);
int64_t sys_ioctl(void *args);
int64_t sys_sigaction(void *args);
int64_t sys_sigprocmask(void *args);
int64_t sys_sigsuspend(void *args);
int64_t sys_alarm(void *args);
int64_t sys_gettimeofday(void *args);
int64_t sys_settimeofday(void *args);
int64_t sys_getuid(void *args);
int64_t sys_setuid(void *args);
int64_t sys_getgid(void *args);
int64_t sys_setgid(void *args);
int64_t sys_chmod(void *args);
int64_t sys_chown(void *args);
int64_t sys_umask(void *args);
int64_t sys_getppid(void *args);
int64_t sys_setsid(void *args);
int64_t sys_getsid(void *args);
int64_t sys_sync(void *args);
int64_t sys_fsync(void *args);
int64_t sys_truncate(void *args);
int64_t sys_ftruncate(void *args);
int64_t sys_readdir(void *args);
int64_t sys_mknod(void *args);
int64_t sys_mount(void *args);
int64_t sys_umount(void *args);
int64_t sys_sysconf(void *args);
int64_t sys_getrlimit(void *args);
int64_t sys_setrlimit(void *args);
int64_t sys_getpriority(void *args);
int64_t sys_setpriority(void *args);
int64_t sys_sched_yield(void *args);
int64_t sys_sched_getparam(void *args);
int64_t sys_sched_setparam(void *args);
int64_t sys_sched_getscheduler(void *args);
int64_t sys_sched_setscheduler(void *args);
int64_t sys_clone(void *args);
int64_t sys_vfork(void *args);
int64_t sys_exit_group(void *args);
int64_t sys_wait4(void *args);
int64_t sys_setsockopt(void *args);
int64_t sys_getsockopt(void *args);
int64_t sys_shutdown(void *args);
int64_t sys_bind(void *args);
int64_t sys_listen(void *args);
int64_t sys_getsockname(void *args);
int64_t sys_getpeername(void *args);
int64_t sys_sendto(void *args);
int64_t sys_recvfrom(void *args);
int64_t sys_sendmsg(void *args);
int64_t sys_recvmsg(void *args);
int64_t sys_epoll_create(void *args);
int64_t sys_epoll_ctl(void *args);
int64_t sys_epoll_wait(void *args);
int64_t sys_signalfd(void *args);
int64_t sys_timerfd_create(void *args);
int64_t sys_timerfd_settime(void *args);
int64_t sys_timerfd_gettime(void *args);
int64_t sys_eventfd(void *args);
int64_t sys_fallocate(void *args);
int64_t sys_timer_create(void *args);
int64_t sys_timer_settime(void *args);
int64_t sys_timer_gettime(void *args);
int64_t sys_timer_delete(void *args);
int64_t sys_clock_gettime(void *args);
int64_t sys_clock_settime(void *args);
int64_t sys_clock_getres(void *args);
int64_t sys_nanosleep(void *args);
int64_t sys_getrandom(void *args);
int64_t sys_memfd_create(void *args);

#endif // RAEEEN_SYSCALL_H 