#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"
#include "memory/include/memory.h"
#include <stddef.h>
#include <stdbool.h>

// File system constants
#define MAX_FILENAME_LENGTH     255
#define MAX_PATH_LENGTH         4096
#define MAX_FILESYSTEMS         32
#define MAX_OPEN_FILES          1024
#define MAX_MOUNTS              64

// File types
typedef enum {
    FILE_TYPE_REGULAR = 1,
    FILE_TYPE_DIRECTORY = 2,
    FILE_TYPE_SYMLINK = 3,
    FILE_TYPE_DEVICE_BLOCK = 4,
    FILE_TYPE_DEVICE_CHAR = 5,
    FILE_TYPE_FIFO = 6,
    FILE_TYPE_SOCKET = 7
} file_type_t;

// File permissions
#define PERM_OWNER_READ     0400
#define PERM_OWNER_WRITE    0200
#define PERM_OWNER_EXEC     0100
#define PERM_GROUP_READ     0040
#define PERM_GROUP_WRITE    0020
#define PERM_GROUP_EXEC     0010
#define PERM_OTHER_READ     0004
#define PERM_OTHER_WRITE    0002
#define PERM_OTHER_EXEC     0001

#define PERM_DEFAULT_FILE   0644
#define PERM_DEFAULT_DIR    0755

// File open flags
#define O_RDONLY            0x0000
#define O_WRONLY            0x0001
#define O_RDWR              0x0002
#define O_ACCMODE           0x0003
#define O_CREAT             0x0040
#define O_EXCL              0x0080
#define O_NOCTTY            0x0100
#define O_TRUNC             0x0200
#define O_APPEND            0x0400
#define O_NONBLOCK          0x0800
#define O_SYNC              0x1000
#define O_ASYNC             0x2000
#define O_DIRECT            0x4000
#define O_LARGEFILE         0x8000
#define O_DIRECTORY         0x10000
#define O_NOFOLLOW          0x20000
#define O_NOATIME           0x40000
#define O_CLOEXEC           0x80000

// Seek whence
#define SEEK_SET            0
#define SEEK_CUR            1
#define SEEK_END            2

// File system types
typedef enum {
    FS_TYPE_NONE = 0,
    FS_TYPE_RAEENFS,    // RaeenOS native filesystem
    FS_TYPE_FAT32,
    FS_TYPE_NTFS,
    FS_TYPE_EXT4,
    FS_TYPE_TMPFS,      // Temporary filesystem in RAM
    FS_TYPE_DEVFS,      // Device filesystem
    FS_TYPE_PROCFS,     // Process filesystem
    FS_TYPE_SYSFS       // System filesystem
} filesystem_type_t;

// File attributes
typedef struct file_attributes {
    file_type_t type;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    off_t size;
    u64 creation_time;
    u64 modification_time;
    u64 access_time;
    u32 link_count;
    dev_t device;
    inode_t inode;
    u32 block_size;
    u64 block_count;
} file_attributes_t;

// Directory entry
typedef struct directory_entry {
    char name[MAX_FILENAME_LENGTH + 1];
    inode_t inode;
    file_type_t type;
    off_t offset;
    struct directory_entry* next;
} directory_entry_t;

// Virtual File System (VFS) structures
typedef struct vfs_node vfs_node_t;
typedef struct filesystem filesystem_t;
typedef struct file_descriptor file_descriptor_t;

// VFS operations
typedef struct vfs_operations {
    // File operations
    error_t (*open)(vfs_node_t* node, u32 flags);
    error_t (*close)(vfs_node_t* node);
    ssize_t (*read)(vfs_node_t* node, void* buffer, size_t size, off_t offset);
    ssize_t (*write)(vfs_node_t* node, const void* buffer, size_t size, off_t offset);
    off_t (*seek)(vfs_node_t* node, off_t offset, int whence);
    error_t (*ioctl)(vfs_node_t* node, u32 cmd, void* arg);
    error_t (*mmap)(vfs_node_t* node, void** addr, size_t length, int prot, int flags, off_t offset);
    
    // Directory operations
    error_t (*readdir)(vfs_node_t* node, directory_entry_t** entries, size_t* count);
    error_t (*mkdir)(vfs_node_t* parent, const char* name, mode_t mode);
    error_t (*rmdir)(vfs_node_t* node);
    vfs_node_t* (*lookup)(vfs_node_t* parent, const char* name);
    
    // File management
    error_t (*create)(vfs_node_t* parent, const char* name, mode_t mode);
    error_t (*unlink)(vfs_node_t* parent, const char* name);
    error_t (*rename)(vfs_node_t* old_parent, const char* old_name, 
                     vfs_node_t* new_parent, const char* new_name);
    error_t (*link)(vfs_node_t* target, vfs_node_t* parent, const char* name);
    error_t (*symlink)(const char* target, vfs_node_t* parent, const char* name);
    
    // Attribute operations
    error_t (*getattr)(vfs_node_t* node, file_attributes_t* attr);
    error_t (*setattr)(vfs_node_t* node, file_attributes_t* attr);
    error_t (*chmod)(vfs_node_t* node, mode_t mode);
    error_t (*chown)(vfs_node_t* node, uid_t uid, gid_t gid);
    
    // Synchronization
    error_t (*sync)(vfs_node_t* node);
    error_t (*fsync)(vfs_node_t* node);
    
    // File locking
    error_t (*lock)(vfs_node_t* node, int cmd, struct flock* lock);
} vfs_operations_t;

// VFS node (inode)
struct vfs_node {
    inode_t inode;
    file_type_t type;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    off_t size;
    u64 creation_time;
    u64 modification_time;
    u64 access_time;
    u32 link_count;
    u32 ref_count;
    
    // Filesystem-specific data
    void* private_data;
    filesystem_t* filesystem;
    
    // Operations
    vfs_operations_t* ops;
    
    // Parent directory (for .. lookup)
    struct vfs_node* parent;
    
    // Children (for directories)
    struct vfs_node* first_child;
    struct vfs_node* next_sibling;
    
    // Links
    struct vfs_node* next;
    struct vfs_node* prev;
    
    // Caching
    bool dirty;
    u64 last_access;
    
    // Locking
    bool locked;
    struct thread* lock_owner;
};

// File descriptor
struct file_descriptor {
    int fd;
    vfs_node_t* node;
    u32 flags;
    off_t position;
    u32 ref_count;
    struct process* owner;
    
    // File-specific operations (can override VFS ops)
    vfs_operations_t* ops;
    void* private_data;
};

// Filesystem operations
typedef struct filesystem_operations {
    // Mounting
    error_t (*mount)(filesystem_t* fs, vfs_node_t* mount_point, u32 flags, const void* data);
    error_t (*unmount)(filesystem_t* fs);
    
    // Superblock operations
    error_t (*read_super)(filesystem_t* fs, void* device);
    error_t (*write_super)(filesystem_t* fs);
    error_t (*sync_fs)(filesystem_t* fs);
    
    // Inode operations
    vfs_node_t* (*alloc_inode)(filesystem_t* fs);
    error_t (*free_inode)(filesystem_t* fs, vfs_node_t* node);
    error_t (*read_inode)(filesystem_t* fs, vfs_node_t* node);
    error_t (*write_inode)(filesystem_t* fs, vfs_node_t* node);
    
    // Statistics
    error_t (*statfs)(filesystem_t* fs, struct statfs* buf);
} filesystem_operations_t;

// Filesystem structure
struct filesystem {
    filesystem_type_t type;
    char name[32];
    u32 flags;
    
    // Mount information
    vfs_node_t* root;
    vfs_node_t* mount_point;
    void* device;
    
    // Filesystem-specific data
    void* private_data;
    
    // Operations
    filesystem_operations_t* ops;
    vfs_operations_t* default_file_ops;
    vfs_operations_t* default_dir_ops;
    
    // Statistics
    u64 total_blocks;
    u64 free_blocks;
    u64 total_inodes;
    u64 free_inodes;
    u32 block_size;
    
    // Links
    struct filesystem* next;
    struct filesystem* prev;
};

// Mount entry
typedef struct mount_entry {
    filesystem_t* filesystem;
    vfs_node_t* mount_point;
    char device_path[256];
    char mount_path[256];
    u32 flags;
    struct mount_entry* next;
} mount_entry_t;

// File system statistics
typedef struct fs_stats {
    u32 total_filesystems;
    u32 mounted_filesystems;
    u32 open_files;
    u32 total_inodes;
    u32 free_inodes;
    u64 total_space;
    u64 free_space;
    u64 read_operations;
    u64 write_operations;
    u64 bytes_read;
    u64 bytes_written;
} fs_stats_t;

// Core filesystem functions
error_t filesystem_init(void);
void filesystem_shutdown(void);

// Filesystem registration
error_t register_filesystem(filesystem_type_t type, const char* name, 
                           filesystem_operations_t* ops);
error_t unregister_filesystem(filesystem_type_t type);
filesystem_t* create_filesystem(filesystem_type_t type);
void destroy_filesystem(filesystem_t* fs);

// VFS operations
vfs_node_t* vfs_create_node(filesystem_t* fs, inode_t inode, file_type_t type);
void vfs_destroy_node(vfs_node_t* node);
vfs_node_t* vfs_get_node(vfs_node_t* node);
void vfs_put_node(vfs_node_t* node);

// Path resolution
vfs_node_t* vfs_lookup_path(const char* path, bool follow_symlinks);
vfs_node_t* vfs_lookup_parent(const char* path, char* name);
char* vfs_get_absolute_path(vfs_node_t* node);

// File operations
int vfs_open(const char* path, u32 flags, mode_t mode);
error_t vfs_close(int fd);
ssize_t vfs_read(int fd, void* buffer, size_t size);
ssize_t vfs_write(int fd, const void* buffer, size_t size);
off_t vfs_seek(int fd, off_t offset, int whence);
error_t vfs_ioctl(int fd, u32 cmd, void* arg);

// Directory operations
error_t vfs_mkdir(const char* path, mode_t mode);
error_t vfs_rmdir(const char* path);
int vfs_opendir(const char* path);
directory_entry_t* vfs_readdir(int fd);
error_t vfs_closedir(int fd);

// File management
error_t vfs_create(const char* path, mode_t mode);
error_t vfs_unlink(const char* path);
error_t vfs_rename(const char* oldpath, const char* newpath);
error_t vfs_link(const char* oldpath, const char* newpath);
error_t vfs_symlink(const char* target, const char* linkpath);
ssize_t vfs_readlink(const char* path, char* buffer, size_t size);

// File attributes
error_t vfs_stat(const char* path, file_attributes_t* attr);
error_t vfs_lstat(const char* path, file_attributes_t* attr);
error_t vfs_fstat(int fd, file_attributes_t* attr);
error_t vfs_chmod(const char* path, mode_t mode);
error_t vfs_chown(const char* path, uid_t uid, gid_t gid);
error_t vfs_utime(const char* path, u64 access_time, u64 modification_time);

// Mounting
error_t vfs_mount(const char* device, const char* mount_point, 
                  filesystem_type_t type, u32 flags, const void* data);
error_t vfs_unmount(const char* mount_point);
mount_entry_t* vfs_get_mounts(void);

// File descriptor management
file_descriptor_t* fd_alloc(struct process* process);
void fd_free(file_descriptor_t* fd);
file_descriptor_t* fd_get(struct process* process, int fd);
int fd_assign(struct process* process, file_descriptor_t* fd);

// Working directory
error_t vfs_chdir(const char* path);
char* vfs_getcwd(char* buffer, size_t size);

// File synchronization
error_t vfs_sync(void);
error_t vfs_fsync(int fd);

// Memory mapping
void* vfs_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
error_t vfs_munmap(void* addr, size_t length);

// File locking
error_t vfs_flock(int fd, int operation);

// Built-in filesystems
extern filesystem_operations_t raeenfs_ops;
extern filesystem_operations_t tmpfs_ops;
extern filesystem_operations_t devfs_ops;
extern filesystem_operations_t procfs_ops;

// Root filesystem
extern vfs_node_t* vfs_root;

// Current working directory per process
extern vfs_node_t* current_working_directory;

// Statistics
fs_stats_t* filesystem_get_stats(void);
void filesystem_dump_stats(void);
void vfs_dump_tree(vfs_node_t* node, int depth);

// Standard file descriptors
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

// Error codes specific to filesystem
#define E_FS_NOT_FOUND          -3000
#define E_FS_EXISTS             -3001
#define E_FS_NOT_DIR            -3002
#define E_FS_IS_DIR             -3003
#define E_FS_NOT_EMPTY          -3004
#define E_FS_NO_SPACE           -3005
#define E_FS_READ_ONLY          -3006
#define E_FS_NAME_TOO_LONG      -3007
#define E_FS_INVALID_PATH       -3008
#define E_FS_CROSS_DEVICE       -3009
#define E_FS_TOO_MANY_LINKS     -3010

#endif // FILESYSTEM_H