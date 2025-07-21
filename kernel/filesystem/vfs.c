#include "vfs.h"
#include "ramfs.h"
#include "memory.h"
#include <string.h>
#include <stddef.h>

// Global VFS system
static vfs_system_t vfs_system = {0};

// Initialize VFS
int vfs_init(void) {
    memset(&vfs_system, 0, sizeof(vfs_system_t));
    vfs_system.next_inode = 1;
    
    // Create root filesystem using RAMFS
    vfs_system.root = vfs_create_node("/", VFS_TYPE_DIRECTORY);
    if (!vfs_system.root) {
        return -1;
    }
    
    // Mount RAMFS as root
    ramfs_mount_t* mount_data;
    if (ramfs_mount("/", vfs_system.root, &mount_data) != 0) {
        return -1;
    }
    
    // Add mount point
    vfs_mount_t* mount = kmalloc(sizeof(vfs_mount_t));
    if (!mount) {
        return -1;
    }
    
    strcpy(mount->path, "/");
    mount->mount_point = vfs_system.root;
    mount->root = mount_data->root_node;
    mount->fs_type = FS_TYPE_RAMFS;
    mount->fs_data = mount_data;
    mount->next = vfs_system.mounts;
    vfs_system.mounts = mount;
    
    KINFO("VFS initialized with RAMFS root");
    return 0;
}

void vfs_shutdown(void) {
    // Clean up all mount points
    vfs_mount_t* mount = vfs_system.mounts;
    while (mount) {
        vfs_mount_t* next = mount->next;
        
        if (mount->fs_type == FS_TYPE_RAMFS) {
            ramfs_unmount((ramfs_mount_t*)mount->fs_data);
        }
        
        kfree(mount);
        mount = next;
    }
    
    // Clean up root node
    if (vfs_system.root) {
        vfs_destroy_node(vfs_system.root);
    }
    
    KINFO("VFS shutdown complete");
}

// Create a new VFS node
vfs_node_t* vfs_create_node(const char* name, vfs_type_t type) {
    vfs_node_t* node = kmalloc(sizeof(vfs_node_t));
    if (!node) {
        return NULL;
    }
    
    memset(node, 0, sizeof(vfs_node_t));
    strncpy(node->name, name, VFS_MAX_NAME - 1);
    node->type = type;
    node->inode = vfs_system.next_inode++;
    node->ref_count = 1;
    node->permissions = 0755; // Default permissions
    node->uid = 0; // Root user
    node->gid = 0; // Root group
    
    return node;
}

// Destroy a VFS node
void vfs_destroy_node(vfs_node_t* node) {
    if (!node) {
        return;
    }
    
    // Free children recursively
    vfs_node_t* child = node->children;
    while (child) {
        vfs_node_t* next = child->next_sibling;
        vfs_destroy_node(child);
        child = next;
    }
    
    kfree(node);
}

// Look up a file by path
vfs_node_t* vfs_lookup(const char* path) {
    if (!path || !vfs_system.root) {
        return NULL;
    }
    
    // Handle absolute paths
    if (path[0] == '/') {
        return vfs_lookup_absolute(path);
    } else {
        return vfs_lookup_relative(vfs_system.root, path);
    }
}

// Look up absolute path
vfs_node_t* vfs_lookup_absolute(const char* path) {
    if (!path || path[0] != '/') {
        return NULL;
    }
    
    vfs_node_t* current = vfs_system.root;
    char path_copy[VFS_MAX_PATH];
    strncpy(path_copy, path, VFS_MAX_PATH - 1);
    
    char* token = strtok(path_copy, "/");
    while (token) {
        current = vfs_find_child(current, token);
        if (!current) {
            return NULL;
        }
        token = strtok(NULL, "/");
    }
    
    return current;
}

// Look up relative path
vfs_node_t* vfs_lookup_relative(vfs_node_t* base, const char* path) {
    if (!base || !path) {
        return NULL;
    }
    
    char path_copy[VFS_MAX_PATH];
    strncpy(path_copy, path, VFS_MAX_PATH - 1);
    
    vfs_node_t* current = base;
    char* token = strtok(path_copy, "/");
    while (token) {
        current = vfs_find_child(current, token);
        if (!current) {
            return NULL;
        }
        token = strtok(NULL, "/");
    }
    
    return current;
}

// Find child node by name
vfs_node_t* vfs_find_child(vfs_node_t* parent, const char* name) {
    if (!parent || !name || parent->type != VFS_TYPE_DIRECTORY) {
        return NULL;
    }
    
    vfs_node_t* child = parent->children;
    while (child) {
        if (strcmp(child->name, name) == 0) {
            return child;
        }
        child = child->next_sibling;
    }
    
    return NULL;
}

// Add child to parent
int vfs_add_child(vfs_node_t* parent, vfs_node_t* child) {
    if (!parent || !child || parent->type != VFS_TYPE_DIRECTORY) {
        return -1;
    }
    
    child->parent = parent;
    child->next_sibling = parent->children;
    if (parent->children) {
        parent->children->prev_sibling = child;
    }
    parent->children = child;
    
    return 0;
}

// Remove child from parent
int vfs_remove_child(vfs_node_t* parent, vfs_node_t* child) {
    if (!parent || !child) {
        return -1;
    }
    
    if (child->prev_sibling) {
        child->prev_sibling->next_sibling = child->next_sibling;
    } else {
        parent->children = child->next_sibling;
    }
    
    if (child->next_sibling) {
        child->next_sibling->prev_sibling = child->prev_sibling;
    }
    
    child->parent = NULL;
    child->next_sibling = NULL;
    child->prev_sibling = NULL;
    
    return 0;
}

// Open a file
int vfs_open(const char* path, int flags, mode_t mode, vfs_fd_t** fd) {
    if (!path || !fd) {
        return -1;
    }
    
    // Look up the file
    vfs_node_t* node = vfs_lookup(path);
    if (!node) {
        // File doesn't exist, check if we should create it
        if (!(flags & VFS_O_CREAT)) {
            return -1; // ENOENT
        }
        
        // Create the file
        char* dir_path = vfs_get_dirname(path);
        char* filename = vfs_get_basename(path);
        
        vfs_node_t* parent = vfs_lookup(dir_path);
        if (!parent || parent->type != VFS_TYPE_DIRECTORY) {
            kfree(dir_path);
            kfree(filename);
            return -1; // ENOTDIR
        }
        
        node = vfs_create_node(filename, VFS_TYPE_REGULAR);
        if (!node) {
            kfree(dir_path);
            kfree(filename);
            return -1; // ENOMEM
        }
        
        vfs_add_child(parent, node);
        kfree(dir_path);
        kfree(filename);
    }
    
    // Check permissions
    if (!vfs_check_permissions(node, flags)) {
        return -1; // EACCES
    }
    
    // Create file descriptor
    *fd = vfs_alloc_fd();
    if (!*fd) {
        return -1; // ENOMEM
    }
    
    (*fd)->node = node;
    (*fd)->flags = flags;
    (*fd)->offset = 0;
    (*fd)->ref_count = 1;
    
    // Call filesystem-specific open
    if (node->ops && node->ops->open) {
        int result = node->ops->open(node, flags, mode);
        if (result != 0) {
            vfs_free_fd(*fd);
            *fd = NULL;
            return result;
        }
    }
    
    return 0;
}

// Close a file
int vfs_close(vfs_fd_t* fd) {
    if (!fd) {
        return -1;
    }
    
    // Call filesystem-specific close
    if (fd->node && fd->node->ops && fd->node->ops->close) {
        fd->node->ops->close(fd->node);
    }
    
    vfs_free_fd(fd);
    return 0;
}

// Read from file
ssize_t vfs_read(vfs_fd_t* fd, void* buffer, size_t size) {
    if (!fd || !buffer || fd->node->type != VFS_TYPE_REGULAR) {
        return -1;
    }
    
    if (!(fd->flags & VFS_O_RDONLY) && !(fd->flags & VFS_O_RDWR)) {
        return -1; // EBADF
    }
    
    // Call filesystem-specific read
    if (fd->node->ops && fd->node->ops->read) {
        return fd->node->ops->read(fd->node, buffer, size, fd->offset);
    }
    
    return -1; // ENOSYS
}

// Write to file
ssize_t vfs_write(vfs_fd_t* fd, const void* buffer, size_t size) {
    if (!fd || !buffer || fd->node->type != VFS_TYPE_REGULAR) {
        return -1;
    }
    
    if (!(fd->flags & VFS_O_WRONLY) && !(fd->flags & VFS_O_RDWR)) {
        return -1; // EBADF
    }
    
    // Call filesystem-specific write
    if (fd->node->ops && fd->node->ops->write) {
        return fd->node->ops->write(fd->node, buffer, size, fd->offset);
    }
    
    return -1; // ENOSYS
}

// Seek in file
off_t vfs_seek(vfs_fd_t* fd, off_t offset, int whence) {
    if (!fd) {
        return -1;
    }
    
    off_t new_offset = fd->offset;
    
    switch (whence) {
        case VFS_SEEK_SET:
            new_offset = offset;
            break;
        case VFS_SEEK_CUR:
            new_offset += offset;
            break;
        case VFS_SEEK_END:
            new_offset = fd->node->size + offset;
            break;
        default:
            return -1; // EINVAL
    }
    
    if (new_offset < 0) {
        return -1; // EINVAL
    }
    
    fd->offset = new_offset;
    return new_offset;
}

// Create directory
int vfs_mkdir(const char* path, mode_t mode) {
    if (!path) {
        return -1;
    }
    
    char* dir_path = vfs_get_dirname(path);
    char* dirname = vfs_get_basename(path);
    
    vfs_node_t* parent = vfs_lookup(dir_path);
    if (!parent || parent->type != VFS_TYPE_DIRECTORY) {
        kfree(dir_path);
        kfree(dirname);
        return -1; // ENOTDIR
    }
    
    // Check if directory already exists
    if (vfs_find_child(parent, dirname)) {
        kfree(dir_path);
        kfree(dirname);
        return -1; // EEXIST
    }
    
    // Create directory
    vfs_node_t* new_dir = vfs_create_node(dirname, VFS_TYPE_DIRECTORY);
    if (!new_dir) {
        kfree(dir_path);
        kfree(dirname);
        return -1; // ENOMEM
    }
    
    new_dir->permissions = mode;
    vfs_add_child(parent, new_dir);
    
    kfree(dir_path);
    kfree(dirname);
    return 0;
}

// Remove directory
int vfs_rmdir(const char* path) {
    if (!path) {
        return -1;
    }
    
    vfs_node_t* node = vfs_lookup(path);
    if (!node) {
        return -1; // ENOENT
    }
    
    if (node->type != VFS_TYPE_DIRECTORY) {
        return -1; // ENOTDIR
    }
    
    if (node->children) {
        return -1; // ENOTEMPTY
    }
    
    if (node->parent) {
        vfs_remove_child(node->parent, node);
    }
    
    vfs_destroy_node(node);
    return 0;
}

// Create file
int vfs_create(const char* path, mode_t mode) {
    if (!path) {
        return -1;
    }
    
    char* dir_path = vfs_get_dirname(path);
    char* filename = vfs_get_basename(path);
    
    vfs_node_t* parent = vfs_lookup(dir_path);
    if (!parent || parent->type != VFS_TYPE_DIRECTORY) {
        kfree(dir_path);
        kfree(filename);
        return -1; // ENOTDIR
    }
    
    // Check if file already exists
    if (vfs_find_child(parent, filename)) {
        kfree(dir_path);
        kfree(filename);
        return -1; // EEXIST
    }
    
    // Create file
    vfs_node_t* new_file = vfs_create_node(filename, VFS_TYPE_REGULAR);
    if (!new_file) {
        kfree(dir_path);
        kfree(filename);
        return -1; // ENOMEM
    }
    
    new_file->permissions = mode;
    vfs_add_child(parent, new_file);
    
    kfree(dir_path);
    kfree(filename);
    return 0;
}

// Delete file
int vfs_unlink(const char* path) {
    if (!path) {
        return -1;
    }
    
    vfs_node_t* node = vfs_lookup(path);
    if (!node) {
        return -1; // ENOENT
    }
    
    if (node->type == VFS_TYPE_DIRECTORY) {
        return -1; // EISDIR
    }
    
    if (node->parent) {
        vfs_remove_child(node->parent, node);
    }
    
    vfs_destroy_node(node);
    return 0;
}

// Mount filesystem
int vfs_mount(const char* source, const char* target, fs_type_t fs_type) {
    if (!target) {
        return -1;
    }
    
    vfs_node_t* mount_point = vfs_lookup(target);
    if (!mount_point || mount_point->type != VFS_TYPE_DIRECTORY) {
        return -1; // ENOTDIR
    }
    
    // Create mount point
    vfs_mount_t* mount = kmalloc(sizeof(vfs_mount_t));
    if (!mount) {
        return -1; // ENOMEM
    }
    
    strncpy(mount->path, target, VFS_MAX_PATH - 1);
    mount->mount_point = mount_point;
    mount->fs_type = fs_type;
    mount->next = vfs_system.mounts;
    vfs_system.mounts = mount;
    
    // Mount filesystem-specific data
    switch (fs_type) {
        case FS_TYPE_RAMFS:
            return ramfs_mount(source, mount_point, (ramfs_mount_t**)&mount->fs_data);
        default:
            kfree(mount);
            return -1; // ENODEV
    }
}

// Unmount filesystem
int vfs_unmount(const char* target) {
    if (!target) {
        return -1;
    }
    
    vfs_mount_t* mount = vfs_system.mounts;
    vfs_mount_t* prev = NULL;
    
    while (mount) {
        if (strcmp(mount->path, target) == 0) {
            // Remove from list
            if (prev) {
                prev->next = mount->next;
            } else {
                vfs_system.mounts = mount->next;
            }
            
            // Unmount filesystem-specific data
            switch (mount->fs_type) {
                case FS_TYPE_RAMFS:
                    ramfs_unmount((ramfs_mount_t*)mount->fs_data);
                    break;
            }
            
            kfree(mount);
            return 0;
        }
        
        prev = mount;
        mount = mount->next;
    }
    
    return -1; // EINVAL
}

// Get file attributes
int vfs_stat(const char* path, vfs_stat_t* stat) {
    if (!path || !stat) {
        return -1;
    }
    
    vfs_node_t* node = vfs_lookup(path);
    if (!node) {
        return -1; // ENOENT
    }
    
    stat->st_mode = node->permissions;
    stat->st_uid = node->uid;
    stat->st_gid = node->gid;
    stat->st_size = node->size;
    stat->st_atime = node->atime;
    stat->st_mtime = node->mtime;
    stat->st_ctime = node->ctime;
    stat->st_ino = node->inode;
    stat->st_nlink = 1;
    
    return 0;
}

// Change file permissions
int vfs_chmod(const char* path, mode_t mode) {
    if (!path) {
        return -1;
    }
    
    vfs_node_t* node = vfs_lookup(path);
    if (!node) {
        return -1; // ENOENT
    }
    
    node->permissions = mode;
    return 0;
}

// Change file owner
int vfs_chown(const char* path, uid_t uid, gid_t gid) {
    if (!path) {
        return -1;
    }
    
    vfs_node_t* node = vfs_lookup(path);
    if (!node) {
        return -1; // ENOENT
    }
    
    node->uid = uid;
    node->gid = gid;
    return 0;
}

// Get absolute path
char* vfs_get_absolute_path(vfs_node_t* node) {
    if (!node) {
        return NULL;
    }
    
    char* path = kmalloc(VFS_MAX_PATH);
    if (!path) {
        return NULL;
    }
    
    path[0] = '\0';
    
    // Build path from root to node
    vfs_node_t* current = node;
    char temp_path[VFS_MAX_PATH];
    temp_path[0] = '\0';
    
    while (current && current != vfs_system.root) {
        char temp[VFS_MAX_PATH];
        snprintf(temp, sizeof(temp), "/%s%s", current->name, temp_path);
        strcpy(temp_path, temp);
        current = current->parent;
    }
    
    if (strlen(temp_path) == 0) {
        strcpy(path, "/");
    } else {
        strcpy(path, temp_path);
    }
    
    return path;
}

// Allocate file descriptor
vfs_fd_t* vfs_alloc_fd(void) {
    vfs_fd_t* fd = kmalloc(sizeof(vfs_fd_t));
    if (fd) {
        memset(fd, 0, sizeof(vfs_fd_t));
    }
    return fd;
}

// Free file descriptor
void vfs_free_fd(vfs_fd_t* fd) {
    if (fd) {
        kfree(fd);
    }
}

// Duplicate file descriptor
int vfs_dup_fd(vfs_fd_t* old_fd, vfs_fd_t** new_fd) {
    if (!old_fd || !new_fd) {
        return -1;
    }
    
    *new_fd = vfs_alloc_fd();
    if (!*new_fd) {
        return -1; // ENOMEM
    }
    
    (*new_fd)->node = old_fd->node;
    (*new_fd)->flags = old_fd->flags;
    (*new_fd)->offset = old_fd->offset;
    (*new_fd)->ref_count = 1;
    
    return 0;
}

// Check file permissions
bool vfs_check_permissions(vfs_node_t* node, int flags) {
    if (!node) {
        return false;
    }
    
    // For now, allow all operations
    // In a real implementation, this would check user permissions
    return true;
}

// Get directory name from path
char* vfs_get_dirname(const char* path) {
    if (!path) {
        return NULL;
    }
    
    char* path_copy = kmalloc(strlen(path) + 1);
    if (!path_copy) {
        return NULL;
    }
    
    strcpy(path_copy, path);
    
    char* last_slash = strrchr(path_copy, '/');
    if (last_slash) {
        *last_slash = '\0';
        if (strlen(path_copy) == 0) {
            strcpy(path_copy, "/");
        }
    } else {
        strcpy(path_copy, ".");
    }
    
    return path_copy;
}

// Get base name from path
char* vfs_get_basename(const char* path) {
    if (!path) {
        return NULL;
    }
    
    char* path_copy = kmalloc(strlen(path) + 1);
    if (!path_copy) {
        return NULL;
    }
    
    strcpy(path_copy, path);
    
    char* last_slash = strrchr(path_copy, '/');
    if (last_slash) {
        return last_slash + 1;
    } else {
        return path_copy;
    }
} 