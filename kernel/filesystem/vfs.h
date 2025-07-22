#ifndef RAEENOS_VFS_H
#define RAEENOS_VFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"

// Forward declarations for kernel types
typedef uint32_t spinlock_t;
typedef int64_t ssize_t;
typedef int64_t off_t;
typedef uint32_t mode_t;

// File system types
typedef enum {
    FS_TYPE_RAMFS,
    FS_TYPE_FAT32,
    FS_TYPE_EXT4,
    FS_TYPE_TMPFS,
    FS_TYPE_PROC
} fs_type_t;

// File types
typedef enum {
    VFS_TYPE_UNKNOWN = 0,
    VFS_TYPE_REGULAR,
    VFS_TYPE_DIRECTORY,
    VFS_TYPE_SYMLINK,
    VFS_TYPE_CHAR_DEVICE,
    VFS_TYPE_BLOCK_DEVICE,
    VFS_TYPE_FIFO,
    VFS_TYPE_SOCKET
} vfs_type_t;

// File access modes
#define VFS_ACCESS_READ    0x0001
#define VFS_ACCESS_WRITE   0x0002
#define VFS_ACCESS_EXECUTE 0x0004

// File open flags
#define VFS_O_RDONLY       0x0000
#define VFS_O_WRONLY       0x0001
#define VFS_O_RDWR         0x0002
#define VFS_O_CREAT        0x0100
#define VFS_O_EXCL         0x0200
#define VFS_O_TRUNC        0x0400
#define VFS_O_APPEND       0x0800

// File seek origins
#define VFS_SEEK_SET       0
#define VFS_SEEK_CUR       1
#define VFS_SEEK_END       2

// Maximum path length
#define VFS_MAX_PATH       4096
#define VFS_MAX_NAME       256

// File system operations structure
typedef struct vfs_ops {
    // File operations
    int (*open)(struct vfs_node* node, int flags, mode_t mode);
    int (*close)(struct vfs_node* node);
    ssize_t (*read)(struct vfs_node* node, void* buffer, size_t size, off_t offset);
    ssize_t (*write)(struct vfs_node* node, const void* buffer, size_t size, off_t offset);
    int (*ioctl)(struct vfs_node* node, unsigned long request, void* arg);
    
    // Directory operations
    int (*readdir)(struct vfs_node* node, void* buffer, size_t size, off_t offset);
    int (*mkdir)(struct vfs_node* parent, const char* name, mode_t mode);
    int (*rmdir)(struct vfs_node* parent, const char* name);
    
    // File management
    int (*create)(struct vfs_node* parent, const char* name, mode_t mode);
    int (*unlink)(struct vfs_node* parent, const char* name);
    int (*link)(struct vfs_node* old_node, struct vfs_node* new_parent, const char* new_name);
    int (*symlink)(struct vfs_node* parent, const char* target, const char* name);
    
    // File system operations
    int (*mount)(struct vfs_node* mount_point, struct vfs_node* root);
    int (*unmount)(struct vfs_node* mount_point);
    int (*sync)(struct vfs_node* node);
    
    // Attribute operations
    int (*getattr)(struct vfs_node* node, struct vfs_stat* stat);
    int (*setattr)(struct vfs_node* node, struct vfs_stat* stat);
} vfs_ops_t;

// File system statistics
typedef struct vfs_stat {
    uint32_t st_mode;      // File type and permissions
    uint32_t st_uid;       // User ID of owner
    uint32_t st_gid;       // Group ID of owner
    uint64_t st_size;      // File size in bytes
    uint64_t st_atime;     // Last access time
    uint64_t st_mtime;     // Last modification time
    uint64_t st_ctime;     // Last status change time
    uint32_t st_nlink;     // Number of hard links
    uint32_t st_ino;       // Inode number
    uint32_t st_dev;       // Device ID
    uint32_t st_rdev;      // Device type (for device files)
    uint32_t st_blksize;   // Block size for filesystem I/O
    uint64_t st_blocks;    // Number of 512B blocks allocated
} vfs_stat_t;

// VFS node structure
typedef struct vfs_node {
    char name[VFS_MAX_NAME];           // Node name
    vfs_type_t type;                   // Node type
    uint32_t flags;                    // Node flags
    uint32_t inode;                    // Inode number
    uint32_t ref_count;                // Reference count
    uint32_t size;                     // File size
    uint32_t permissions;              // File permissions
    uint32_t uid;                      // Owner user ID
    uint32_t gid;                      // Owner group ID
    uint64_t atime;                    // Access time
    uint64_t mtime;                    // Modification time
    uint64_t ctime;                    // Creation time
    
    // File system specific data
    void* fs_data;                     // File system specific data
    struct vfs_ops* ops;               // File system operations
    
    // Tree structure
    struct vfs_node* parent;           // Parent directory
    struct vfs_node* children;         // First child
    struct vfs_node* next_sibling;     // Next sibling
    struct vfs_node* prev_sibling;     // Previous sibling
    
    // Mount point information
    struct vfs_node* mount_point;      // Mount point if this is a mount
    struct vfs_node* mounted_fs;       // Root of mounted file system
} vfs_node_t;

// File descriptor structure
typedef struct vfs_fd {
    vfs_node_t* node;                  // Associated VFS node
    uint32_t flags;                    // Open flags
    off_t offset;                      // Current file offset
    uint32_t ref_count;                // Reference count
} vfs_fd_t;

// Mount point structure
typedef struct vfs_mount {
    char path[VFS_MAX_PATH];           // Mount path
    vfs_node_t* mount_point;           // Mount point node
    vfs_node_t* root;                  // Root of mounted file system
    fs_type_t fs_type;                 // File system type
    void* fs_data;                     // File system specific data
    struct vfs_mount* next;            // Next mount point
} vfs_mount_t;

// VFS global structure
typedef struct vfs_system {
    vfs_node_t* root;                  // Root file system
    vfs_mount_t* mounts;               // Mount points list
    uint32_t next_inode;               // Next available inode number
    spinlock_t lock;                   // VFS lock
} vfs_system_t;

// Function prototypes
int vfs_init(void);
void vfs_shutdown(void);

// File operations
int vfs_open(const char* path, int flags, mode_t mode, vfs_fd_t** fd);
int vfs_close(vfs_fd_t* fd);
ssize_t vfs_read(vfs_fd_t* fd, void* buffer, size_t size);
ssize_t vfs_write(vfs_fd_t* fd, const void* buffer, size_t size);
off_t vfs_seek(vfs_fd_t* fd, off_t offset, int whence);
int vfs_ioctl(vfs_fd_t* fd, unsigned long request, void* arg);

// Directory operations
int vfs_readdir(vfs_fd_t* fd, void* buffer, size_t size);
int vfs_mkdir(const char* path, mode_t mode);
int vfs_rmdir(const char* path);

// File management
int vfs_create(const char* path, mode_t mode);
int vfs_unlink(const char* path);
int vfs_link(const char* old_path, const char* new_path);
int vfs_symlink(const char* target, const char* path);

// File system operations
int vfs_mount(const char* source, const char* target, fs_type_t fs_type);
int vfs_unmount(const char* target);
int vfs_sync(void);

// Attribute operations
int vfs_stat(const char* path, vfs_stat_t* stat);
int vfs_chmod(const char* path, mode_t mode);
int vfs_chown(const char* path, uid_t uid, gid_t gid);

// Utility functions
vfs_node_t* vfs_lookup(const char* path);
vfs_node_t* vfs_create_node(const char* name, vfs_type_t type);
void vfs_destroy_node(vfs_node_t* node);
char* vfs_get_absolute_path(vfs_node_t* node);

// File descriptor management
vfs_fd_t* vfs_alloc_fd(void);
void vfs_free_fd(vfs_fd_t* fd);
int vfs_dup_fd(vfs_fd_t* old_fd, vfs_fd_t** new_fd);

#endif // RAEENOS_VFS_H 