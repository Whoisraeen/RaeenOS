#ifndef RAEENOS_RAMFS_H
#define RAEENOS_RAMFS_H

#include "vfs.h"
#include <stdint.h>
#include <stddef.h>

// RAMFS file data structure
typedef struct ramfs_file {
    char* data;                        // File data buffer
    size_t size;                       // Current file size
    size_t capacity;                   // Allocated capacity
    bool dirty;                        // Dirty flag for write-back
} ramfs_file_t;

// RAMFS directory entry structure
typedef struct ramfs_dirent {
    char name[VFS_MAX_NAME];           // Entry name
    vfs_type_t type;                   // Entry type
    uint32_t inode;                    // Inode number
    struct ramfs_dirent* next;         // Next entry in directory
} ramfs_dirent_t;

// RAMFS directory structure
typedef struct ramfs_dir {
    ramfs_dirent_t* entries;           // Directory entries
    uint32_t entry_count;              // Number of entries
    size_t size;                       // Directory size
} ramfs_dir_t;

// RAMFS inode structure
typedef struct ramfs_inode {
    uint32_t inode_number;             // Inode number
    vfs_type_t type;                   // File type
    uint32_t permissions;              // File permissions
    uint32_t uid;                      // Owner user ID
    uint32_t gid;                      // Owner group ID
    uint64_t atime;                    // Access time
    uint64_t mtime;                    // Modification time
    uint64_t ctime;                    // Creation time
    uint32_t size;                     // File size
    uint32_t ref_count;                // Reference count
    
    union {
        ramfs_file_t file;             // File data
        ramfs_dir_t dir;               // Directory data
    } data;
    
    struct ramfs_inode* next;          // Next inode in list
} ramfs_inode_t;

// RAMFS superblock structure
typedef struct ramfs_sb {
    uint32_t magic;                    // Magic number
    uint32_t version;                  // File system version
    uint32_t inode_count;              // Total number of inodes
    uint32_t free_inodes;              // Number of free inodes
    uint64_t total_size;               // Total file system size
    uint64_t free_size;                // Free space
    ramfs_inode_t* inode_list;         // List of all inodes
    spinlock_t lock;                   // Superblock lock
} ramfs_sb_t;

// RAMFS mount data structure
typedef struct ramfs_mount {
    ramfs_sb_t* sb;                    // Superblock
    vfs_node_t* root_node;             // Root VFS node
} ramfs_mount_t;

// Function prototypes
int ramfs_init(void);
void ramfs_shutdown(void);

// File system operations
int ramfs_mount(const char* source, vfs_node_t* mount_point, ramfs_mount_t** mount_data);
int ramfs_unmount(ramfs_mount_t* mount_data);
int ramfs_sync(ramfs_mount_t* mount_data);

// Inode operations
ramfs_inode_t* ramfs_alloc_inode(ramfs_sb_t* sb);
void ramfs_free_inode(ramfs_sb_t* sb, ramfs_inode_t* inode);
ramfs_inode_t* ramfs_get_inode(ramfs_sb_t* sb, uint32_t inode_number);
int ramfs_create_inode(ramfs_sb_t* sb, const char* name, vfs_type_t type, mode_t mode, ramfs_inode_t** inode);

// File operations
int ramfs_file_open(ramfs_inode_t* inode, int flags, mode_t mode);
int ramfs_file_close(ramfs_inode_t* inode);
ssize_t ramfs_file_read(ramfs_inode_t* inode, void* buffer, size_t size, off_t offset);
ssize_t ramfs_file_write(ramfs_inode_t* inode, const void* buffer, size_t size, off_t offset);
int ramfs_file_truncate(ramfs_inode_t* inode, off_t length);

// Directory operations
int ramfs_dir_read(ramfs_inode_t* inode, void* buffer, size_t size, off_t offset);
int ramfs_dir_create(ramfs_inode_t* parent, const char* name, mode_t mode);
int ramfs_dir_remove(ramfs_inode_t* parent, const char* name);
int ramfs_dir_lookup(ramfs_inode_t* parent, const char* name, ramfs_inode_t** inode);

// Attribute operations
int ramfs_getattr(ramfs_inode_t* inode, vfs_stat_t* stat);
int ramfs_setattr(ramfs_inode_t* inode, vfs_stat_t* stat);

// Utility functions
int ramfs_create_root(ramfs_mount_t* mount_data);
void ramfs_destroy_inode(ramfs_inode_t* inode);
char* ramfs_get_inode_path(ramfs_inode_t* inode);

// RAMFS constants
#define RAMFS_MAGIC       0x534D4152  // "RAMS"
#define RAMFS_VERSION     0x00010000  // Version 1.0
#define RAMFS_MAX_FILES   1024        // Maximum number of files
#define RAMFS_MAX_SIZE    (1024*1024) // Maximum file system size (1MB)

#endif // RAEENOS_RAMFS_H 