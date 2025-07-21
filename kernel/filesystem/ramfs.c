#include "ramfs.h"
#include "vfs.h"
#include "memory.h"
#include <string.h>
#include <stddef.h>

// Initialize RAMFS
int ramfs_init(void) {
    KINFO("RAMFS initialized");
    return 0;
}

void ramfs_shutdown(void) {
    KINFO("RAMFS shutdown complete");
}

// Mount RAMFS
int ramfs_mount(const char* source, vfs_node_t* mount_point, ramfs_mount_t** mount_data) {
    if (!mount_point) {
        return -1;
    }
    
    // Create mount data
    ramfs_mount_t* mount = kmalloc(sizeof(ramfs_mount_t));
    if (!mount) {
        return -1;
    }
    
    // Create superblock
    ramfs_sb_t* sb = kmalloc(sizeof(ramfs_sb_t));
    if (!sb) {
        kfree(mount);
        return -1;
    }
    
    memset(sb, 0, sizeof(ramfs_sb_t));
    sb->magic = RAMFS_MAGIC;
    sb->version = RAMFS_VERSION;
    sb->inode_count = 0;
    sb->free_inodes = RAMFS_MAX_FILES;
    sb->total_size = RAMFS_MAX_SIZE;
    sb->free_size = RAMFS_MAX_SIZE;
    
    mount->sb = sb;
    mount->root_node = mount_point;
    
    // Create root directory inode
    ramfs_inode_t* root_inode = ramfs_alloc_inode(sb);
    if (!root_inode) {
        kfree(sb);
        kfree(mount);
        return -1;
    }
    
    root_inode->type = VFS_TYPE_DIRECTORY;
    root_inode->permissions = 0755;
    root_inode->size = 0;
    root_inode->data.dir.entries = NULL;
    root_inode->data.dir.entry_count = 0;
    
    // Set up VFS node
    mount_point->fs_data = root_inode;
    mount_point->ops = &ramfs_ops;
    
    *mount_data = mount;
    
    KINFO("RAMFS mounted at %s", mount_point->name);
    return 0;
}

// Unmount RAMFS
int ramfs_unmount(ramfs_mount_t* mount_data) {
    if (!mount_data) {
        return -1;
    }
    
    // Free all inodes
    ramfs_inode_t* inode = mount_data->sb->inode_list;
    while (inode) {
        ramfs_inode_t* next = inode->next;
        ramfs_destroy_inode(inode);
        inode = next;
    }
    
    // Free superblock
    kfree(mount_data->sb);
    kfree(mount_data);
    
    KINFO("RAMFS unmounted");
    return 0;
}

// Allocate inode
ramfs_inode_t* ramfs_alloc_inode(ramfs_sb_t* sb) {
    if (!sb || sb->free_inodes == 0) {
        return NULL;
    }
    
    ramfs_inode_t* inode = kmalloc(sizeof(ramfs_inode_t));
    if (!inode) {
        return NULL;
    }
    
    memset(inode, 0, sizeof(ramfs_inode_t));
    inode->inode_number = sb->next_inode++;
    inode->ref_count = 1;
    inode->next = sb->inode_list;
    sb->inode_list = inode;
    sb->inode_count++;
    sb->free_inodes--;
    
    return inode;
}

// Free inode
void ramfs_free_inode(ramfs_sb_t* sb, ramfs_inode_t* inode) {
    if (!sb || !inode) {
        return;
    }
    
    // Remove from list
    ramfs_inode_t* current = sb->inode_list;
    ramfs_inode_t* prev = NULL;
    
    while (current) {
        if (current == inode) {
            if (prev) {
                prev->next = current->next;
            } else {
                sb->inode_list = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }
    
    ramfs_destroy_inode(inode);
    sb->inode_count--;
    sb->free_inodes++;
}

// Get inode by number
ramfs_inode_t* ramfs_get_inode(ramfs_sb_t* sb, uint32_t inode_number) {
    if (!sb) {
        return NULL;
    }
    
    ramfs_inode_t* inode = sb->inode_list;
    while (inode) {
        if (inode->inode_number == inode_number) {
            return inode;
        }
        inode = inode->next;
    }
    
    return NULL;
}

// Create inode
int ramfs_create_inode(ramfs_sb_t* sb, const char* name, vfs_type_t type, mode_t mode, ramfs_inode_t** inode) {
    if (!sb || !name || !inode) {
        return -1;
    }
    
    *inode = ramfs_alloc_inode(sb);
    if (!*inode) {
        return -1;
    }
    
    (*inode)->type = type;
    (*inode)->permissions = mode;
    (*inode)->size = 0;
    
    if (type == VFS_TYPE_REGULAR) {
        (*inode)->data.file.data = NULL;
        (*inode)->data.file.size = 0;
        (*inode)->data.file.capacity = 0;
        (*inode)->data.file.dirty = false;
    } else if (type == VFS_TYPE_DIRECTORY) {
        (*inode)->data.dir.entries = NULL;
        (*inode)->data.dir.entry_count = 0;
        (*inode)->data.dir.size = 0;
    }
    
    return 0;
}

// File operations
int ramfs_file_open(ramfs_inode_t* inode, int flags, mode_t mode) {
    if (!inode || inode->type != VFS_TYPE_REGULAR) {
        return -1;
    }
    
    inode->ref_count++;
    return 0;
}

int ramfs_file_close(ramfs_inode_t* inode) {
    if (!inode || inode->type != VFS_TYPE_REGULAR) {
        return -1;
    }
    
    inode->ref_count--;
    if (inode->ref_count == 0) {
        // Free file data if no more references
        if (inode->data.file.data) {
            kfree(inode->data.file.data);
            inode->data.file.data = NULL;
        }
    }
    
    return 0;
}

ssize_t ramfs_file_read(ramfs_inode_t* inode, void* buffer, size_t size, off_t offset) {
    if (!inode || inode->type != VFS_TYPE_REGULAR || !buffer) {
        return -1;
    }
    
    if (offset >= inode->data.file.size) {
        return 0; // EOF
    }
    
    size_t bytes_to_read = size;
    if (offset + bytes_to_read > inode->data.file.size) {
        bytes_to_read = inode->data.file.size - offset;
    }
    
    if (bytes_to_read > 0) {
        memcpy(buffer, inode->data.file.data + offset, bytes_to_read);
    }
    
    return bytes_to_read;
}

ssize_t ramfs_file_write(ramfs_inode_t* inode, const void* buffer, size_t size, off_t offset) {
    if (!inode || inode->type != VFS_TYPE_REGULAR || !buffer) {
        return -1;
    }
    
    // Ensure we have enough space
    size_t required_size = offset + size;
    if (required_size > inode->data.file.capacity) {
        size_t new_capacity = required_size * 2; // Double the capacity
        if (new_capacity < 1024) {
            new_capacity = 1024; // Minimum capacity
        }
        
        char* new_data = kmalloc(new_capacity);
        if (!new_data) {
            return -1; // ENOMEM
        }
        
        // Copy existing data
        if (inode->data.file.data) {
            memcpy(new_data, inode->data.file.data, inode->data.file.size);
            kfree(inode->data.file.data);
        }
        
        inode->data.file.data = new_data;
        inode->data.file.capacity = new_capacity;
    }
    
    // Write data
    memcpy(inode->data.file.data + offset, buffer, size);
    
    // Update size if we wrote beyond current size
    if (offset + size > inode->data.file.size) {
        inode->data.file.size = offset + size;
        inode->size = inode->data.file.size;
    }
    
    inode->data.file.dirty = true;
    return size;
}

int ramfs_file_truncate(ramfs_inode_t* inode, off_t length) {
    if (!inode || inode->type != VFS_TYPE_REGULAR) {
        return -1;
    }
    
    if (length < 0) {
        return -1;
    }
    
    if (length > inode->data.file.capacity) {
        // Need to expand
        size_t new_capacity = length * 2;
        if (new_capacity < 1024) {
            new_capacity = 1024;
        }
        
        char* new_data = kmalloc(new_capacity);
        if (!new_data) {
            return -1; // ENOMEM
        }
        
        // Copy existing data
        if (inode->data.file.data) {
            memcpy(new_data, inode->data.file.data, inode->data.file.size);
            kfree(inode->data.file.data);
        }
        
        inode->data.file.data = new_data;
        inode->data.file.capacity = new_capacity;
    }
    
    inode->data.file.size = length;
    inode->size = length;
    inode->data.file.dirty = true;
    
    return 0;
}

// Directory operations
int ramfs_dir_read(ramfs_inode_t* inode, void* buffer, size_t size, off_t offset) {
    if (!inode || inode->type != VFS_TYPE_DIRECTORY || !buffer) {
        return -1;
    }
    
    // For now, return a simple directory listing
    // In a real implementation, this would return directory entries
    const char* entries[] = {".", ".."};
    size_t total_size = 0;
    
    for (int i = 0; i < 2; i++) {
        size_t entry_size = strlen(entries[i]) + 1;
        if (total_size + entry_size <= size) {
            strcpy((char*)buffer + total_size, entries[i]);
            total_size += entry_size;
        }
    }
    
    return total_size;
}

int ramfs_dir_create(ramfs_inode_t* parent, const char* name, mode_t mode) {
    if (!parent || parent->type != VFS_TYPE_DIRECTORY || !name) {
        return -1;
    }
    
    // Check if entry already exists
    ramfs_dirent_t* entry = parent->data.dir.entries;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return -1; // EEXIST
        }
        entry = entry->next;
    }
    
    // Create new directory entry
    entry = kmalloc(sizeof(ramfs_dirent_t));
    if (!entry) {
        return -1; // ENOMEM
    }
    
    strncpy(entry->name, name, VFS_MAX_NAME - 1);
    entry->type = VFS_TYPE_DIRECTORY;
    entry->inode = 0; // Will be set when inode is created
    entry->next = parent->data.dir.entries;
    parent->data.dir.entries = entry;
    parent->data.dir.entry_count++;
    
    return 0;
}

int ramfs_dir_remove(ramfs_inode_t* parent, const char* name) {
    if (!parent || parent->type != VFS_TYPE_DIRECTORY || !name) {
        return -1;
    }
    
    ramfs_dirent_t* entry = parent->data.dir.entries;
    ramfs_dirent_t* prev = NULL;
    
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            // Remove entry
            if (prev) {
                prev->next = entry->next;
            } else {
                parent->data.dir.entries = entry->next;
            }
            
            kfree(entry);
            parent->data.dir.entry_count--;
            return 0;
        }
        
        prev = entry;
        entry = entry->next;
    }
    
    return -1; // ENOENT
}

int ramfs_dir_lookup(ramfs_inode_t* parent, const char* name, ramfs_inode_t** inode) {
    if (!parent || parent->type != VFS_TYPE_DIRECTORY || !name || !inode) {
        return -1;
    }
    
    ramfs_dirent_t* entry = parent->data.dir.entries;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            // Find the inode
            *inode = ramfs_get_inode(parent->inode_number, entry->inode);
            return 0;
        }
        entry = entry->next;
    }
    
    return -1; // ENOENT
}

// Attribute operations
int ramfs_getattr(ramfs_inode_t* inode, vfs_stat_t* stat) {
    if (!inode || !stat) {
        return -1;
    }
    
    stat->st_mode = inode->permissions;
    stat->st_uid = inode->uid;
    stat->st_gid = inode->gid;
    stat->st_size = inode->size;
    stat->st_atime = inode->atime;
    stat->st_mtime = inode->mtime;
    stat->st_ctime = inode->ctime;
    stat->st_ino = inode->inode_number;
    stat->st_nlink = 1;
    stat->st_dev = 0;
    stat->st_rdev = 0;
    stat->st_blksize = 4096;
    stat->st_blocks = (inode->size + 511) / 512;
    
    return 0;
}

int ramfs_setattr(ramfs_inode_t* inode, vfs_stat_t* stat) {
    if (!inode || !stat) {
        return -1;
    }
    
    inode->permissions = stat->st_mode;
    inode->uid = stat->st_uid;
    inode->gid = stat->st_gid;
    inode->atime = stat->st_atime;
    inode->mtime = stat->st_mtime;
    inode->ctime = stat->st_ctime;
    
    return 0;
}

// Create root directory
int ramfs_create_root(ramfs_mount_t* mount_data) {
    if (!mount_data || !mount_data->sb) {
        return -1;
    }
    
    // Root directory is already created in mount
    return 0;
}

// Destroy inode
void ramfs_destroy_inode(ramfs_inode_t* inode) {
    if (!inode) {
        return;
    }
    
    if (inode->type == VFS_TYPE_REGULAR) {
        if (inode->data.file.data) {
            kfree(inode->data.file.data);
        }
    } else if (inode->type == VFS_TYPE_DIRECTORY) {
        // Free directory entries
        ramfs_dirent_t* entry = inode->data.dir.entries;
        while (entry) {
            ramfs_dirent_t* next = entry->next;
            kfree(entry);
            entry = next;
        }
    }
    
    kfree(inode);
}

// Get inode path
char* ramfs_get_inode_path(ramfs_inode_t* inode) {
    if (!inode) {
        return NULL;
    }
    
    // For now, return a simple path
    char* path = kmalloc(64);
    if (path) {
        snprintf(path, 64, "/inode_%d", inode->inode_number);
    }
    
    return path;
}

// RAMFS operations structure
static vfs_ops_t ramfs_ops = {
    .open = (int (*)(struct vfs_node*, int, mode_t))ramfs_file_open,
    .close = (int (*)(struct vfs_node*))ramfs_file_close,
    .read = (ssize_t (*)(struct vfs_node*, void*, size_t, off_t))ramfs_file_read,
    .write = (ssize_t (*)(struct vfs_node*, const void*, size_t, off_t))ramfs_file_write,
    .ioctl = NULL,
    .readdir = (int (*)(struct vfs_node*, void*, size_t, off_t))ramfs_dir_read,
    .mkdir = (int (*)(struct vfs_node*, const char*, mode_t))ramfs_dir_create,
    .rmdir = (int (*)(struct vfs_node*, const char*))ramfs_dir_remove,
    .create = NULL,
    .unlink = NULL,
    .link = NULL,
    .symlink = NULL,
    .mount = NULL,
    .unmount = NULL,
    .sync = NULL,
    .getattr = (int (*)(struct vfs_node*, struct vfs_stat*))ramfs_getattr,
    .setattr = (int (*)(struct vfs_node*, struct vfs_stat*))ramfs_setattr
}; 