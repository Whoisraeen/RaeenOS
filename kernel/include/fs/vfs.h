#ifndef VFS_H
#define VFS_H

#include "types.h"

struct vfs_node;

// Filesystem driver operations
typedef struct {
    error_t (*open)(struct vfs_node* node, uint32_t flags);
    error_t (*close)(struct vfs_node* node);
    ssize_t (*read)(struct vfs_node* node, uint64_t offset, size_t size, void* buffer);
    ssize_t (*write)(struct vfs_node* node, uint64_t offset, size_t size, const void* buffer);
    struct vfs_node* (*finddir)(struct vfs_node* node, const char* name);
} fs_ops_t;

// Represents a file or directory in the VFS
typedef struct vfs_node {
    char name[256];
    uint32_t flags; // VFS_FILE, VFS_DIRECTORY, etc.
    uint64_t length;
    fs_ops_t* ops;
    // Filesystem-specific data
    void* fs_private;
    struct vfs_node* parent;
} vfs_node_t;

#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02

// VFS public API
void vfs_init(void);
vfs_node_t* vfs_lookup(const char* path);
ssize_t vfs_read(vfs_node_t* node, uint64_t offset, size_t size, void* buffer);

// Mounts a filesystem at a given path
error_t vfs_mount(const char* path, fs_ops_t* ops, void* fs_data);

#endif // VFS_H