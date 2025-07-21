#include "filesystem/include/filesystem.h"
#include "kernel.h"
#include "memory/include/memory.h"

// TMPFS (Temporary Filesystem) implementation
// This is a RAM-based filesystem for temporary files

typedef struct tmpfs_inode {
    vfs_node_t* vfs_node;
    void* data;
    size_t allocated_size;
    struct tmpfs_inode* next;
} tmpfs_inode_t;

typedef struct tmpfs_superblock {
    u64 total_blocks;
    u64 free_blocks;
    u32 block_size;
    tmpfs_inode_t* inode_list;
    inode_t next_inode;
} tmpfs_superblock_t;

typedef struct tmpfs_dir_entry {
    char name[MAX_FILENAME_LENGTH + 1];
    inode_t inode;
    struct tmpfs_dir_entry* next;
} tmpfs_dir_entry_t;

// Forward declarations
static error_t tmpfs_mount(filesystem_t* fs, vfs_node_t* mount_point, u32 flags, const void* data);
static error_t tmpfs_unmount(filesystem_t* fs);
static error_t tmpfs_read_super(filesystem_t* fs, void* device);
static vfs_node_t* tmpfs_alloc_inode(filesystem_t* fs);
static error_t tmpfs_free_inode(filesystem_t* fs, vfs_node_t* node);

static error_t tmpfs_open(vfs_node_t* node, u32 flags);
static error_t tmpfs_close(vfs_node_t* node);
static ssize_t tmpfs_read(vfs_node_t* node, void* buffer, size_t size, off_t offset);
static ssize_t tmpfs_write(vfs_node_t* node, const void* buffer, size_t size, off_t offset);
static error_t tmpfs_mkdir(vfs_node_t* parent, const char* name, mode_t mode);
static vfs_node_t* tmpfs_lookup(vfs_node_t* parent, const char* name);
static error_t tmpfs_create(vfs_node_t* parent, const char* name, mode_t mode);

// Filesystem operations
filesystem_operations_t tmpfs_ops = {
    .mount = tmpfs_mount,
    .unmount = tmpfs_unmount,
    .read_super = tmpfs_read_super,
    .alloc_inode = tmpfs_alloc_inode,
    .free_inode = tmpfs_free_inode
};

// File operations
static vfs_operations_t tmpfs_file_ops = {
    .open = tmpfs_open,
    .close = tmpfs_close,
    .read = tmpfs_read,
    .write = tmpfs_write
};

// Directory operations
static vfs_operations_t tmpfs_dir_ops = {
    .open = tmpfs_open,
    .close = tmpfs_close,
    .lookup = tmpfs_lookup,
    .mkdir = tmpfs_mkdir,
    .create = tmpfs_create
};

static error_t tmpfs_mount(filesystem_t* fs, vfs_node_t* mount_point, u32 flags, const void* data) {
    (void)mount_point;
    (void)flags;
    (void)data;
    
    if (!fs) {
        return E_INVAL;
    }
    
    KDEBUG("Mounting tmpfs");
    
    // Allocate superblock
    tmpfs_superblock_t* sb = (tmpfs_superblock_t*)memory_alloc(sizeof(tmpfs_superblock_t));
    if (!sb) {
        return E_NOMEM;
    }
    
    memset(sb, 0, sizeof(tmpfs_superblock_t));
    
    // Initialize superblock
    sb->total_blocks = 1024; // 4MB with 4KB blocks
    sb->free_blocks = 1024;
    sb->block_size = 4096;
    sb->next_inode = 2; // Start from 2 (1 is root)
    
    fs->private_data = sb;
    fs->total_blocks = sb->total_blocks;
    fs->free_blocks = sb->free_blocks;
    fs->block_size = sb->block_size;
    fs->default_file_ops = &tmpfs_file_ops;
    fs->default_dir_ops = &tmpfs_dir_ops;
    
    return SUCCESS;
}

static error_t tmpfs_unmount(filesystem_t* fs) {
    if (!fs || !fs->private_data) {
        return E_INVAL;
    }
    
    KDEBUG("Unmounting tmpfs");
    
    tmpfs_superblock_t* sb = (tmpfs_superblock_t*)fs->private_data;
    
    // Free all inodes
    tmpfs_inode_t* inode = sb->inode_list;
    while (inode) {
        tmpfs_inode_t* next = inode->next;
        if (inode->data) {
            memory_free(inode->data);
        }
        memory_free(inode);
        inode = next;
    }
    
    // Free superblock
    memory_free(sb);
    fs->private_data = NULL;
    
    return SUCCESS;
}

static error_t tmpfs_read_super(filesystem_t* fs, void* device) {
    (void)device;
    
    if (!fs) {
        return E_INVAL;
    }
    
    // For tmpfs, we don't read from a device
    return tmpfs_mount(fs, NULL, 0, NULL);
}

static vfs_node_t* tmpfs_alloc_inode(filesystem_t* fs) {
    if (!fs || !fs->private_data) {
        return NULL;
    }
    
    tmpfs_superblock_t* sb = (tmpfs_superblock_t*)fs->private_data;
    
    // Allocate tmpfs inode
    tmpfs_inode_t* tmpfs_inode = (tmpfs_inode_t*)memory_alloc(sizeof(tmpfs_inode_t));
    if (!tmpfs_inode) {
        return NULL;
    }
    
    memset(tmpfs_inode, 0, sizeof(tmpfs_inode_t));
    
    // Create VFS node
    vfs_node_t* vfs_node = vfs_create_node(fs, sb->next_inode++, FILE_TYPE_REGULAR);
    if (!vfs_node) {
        memory_free(tmpfs_inode);
        return NULL;
    }
    
    // Link them together
    tmpfs_inode->vfs_node = vfs_node;
    vfs_node->private_data = tmpfs_inode;
    
    // Add to filesystem inode list
    tmpfs_inode->next = sb->inode_list;
    sb->inode_list = tmpfs_inode;
    
    return vfs_node;
}

static error_t tmpfs_free_inode(filesystem_t* fs, vfs_node_t* node) {
    if (!fs || !node || !fs->private_data) {
        return E_INVAL;
    }
    
    tmpfs_superblock_t* sb = (tmpfs_superblock_t*)fs->private_data;
    tmpfs_inode_t* tmpfs_inode = (tmpfs_inode_t*)node->private_data;
    
    if (!tmpfs_inode) {
        return E_INVAL;
    }
    
    // Remove from inode list
    if (sb->inode_list == tmpfs_inode) {
        sb->inode_list = tmpfs_inode->next;
    } else {
        tmpfs_inode_t* current = sb->inode_list;
        while (current && current->next != tmpfs_inode) {
            current = current->next;
        }
        if (current) {
            current->next = tmpfs_inode->next;
        }
    }
    
    // Free data
    if (tmpfs_inode->data) {
        memory_free(tmpfs_inode->data);
    }
    
    // Free tmpfs inode
    memory_free(tmpfs_inode);
    
    return SUCCESS;
}

static error_t tmpfs_open(vfs_node_t* node, u32 flags) {
    (void)flags;
    
    if (!node) {
        return E_INVAL;
    }
    
    // Update access time
    node->access_time = hal_get_timestamp();
    
    return SUCCESS;
}

static error_t tmpfs_close(vfs_node_t* node) {
    if (!node) {
        return E_INVAL;
    }
    
    // Nothing special to do for tmpfs close
    return SUCCESS;
}

static ssize_t tmpfs_read(vfs_node_t* node, void* buffer, size_t size, off_t offset) {
    if (!node || !buffer || node->type != FILE_TYPE_REGULAR) {
        return -1;
    }
    
    tmpfs_inode_t* tmpfs_inode = (tmpfs_inode_t*)node->private_data;
    if (!tmpfs_inode) {
        return -1;
    }
    
    // Check bounds
    if (offset >= (off_t)node->size) {
        return 0; // EOF
    }
    
    size_t bytes_to_read = MIN(size, node->size - offset);
    
    if (!tmpfs_inode->data) {
        // File has no data, return zeros
        memset(buffer, 0, bytes_to_read);
    } else {
        memcpy(buffer, (char*)tmpfs_inode->data + offset, bytes_to_read);
    }
    
    node->access_time = hal_get_timestamp();
    return bytes_to_read;
}

static ssize_t tmpfs_write(vfs_node_t* node, const void* buffer, size_t size, off_t offset) {
    if (!node || !buffer || node->type != FILE_TYPE_REGULAR) {
        return -1;
    }
    
    tmpfs_inode_t* tmpfs_inode = (tmpfs_inode_t*)node->private_data;
    if (!tmpfs_inode) {
        return -1;
    }
    
    size_t required_size = offset + size;
    
    // Expand data buffer if necessary
    if (required_size > tmpfs_inode->allocated_size) {
        size_t new_size = MAX(required_size, tmpfs_inode->allocated_size * 2);
        if (new_size < 4096) new_size = 4096; // Minimum allocation
        
        void* new_data = memory_alloc(new_size);
        if (!new_data) {
            return -1;
        }
        
        // Copy existing data if any
        if (tmpfs_inode->data && node->size > 0) {
            memcpy(new_data, tmpfs_inode->data, node->size);
        }
        
        // Zero any gap between old size and write offset
        if (offset > (off_t)node->size) {
            memset((char*)new_data + node->size, 0, offset - node->size);
        }
        
        // Free old data
        if (tmpfs_inode->data) {
            memory_free(tmpfs_inode->data);
        }
        
        tmpfs_inode->data = new_data;
        tmpfs_inode->allocated_size = new_size;
    }
    
    // Write data
    memcpy((char*)tmpfs_inode->data + offset, buffer, size);
    
    // Update file size
    if (required_size > node->size) {
        node->size = required_size;
    }
    
    node->modification_time = hal_get_timestamp();
    node->dirty = true;
    
    return size;
}

static error_t tmpfs_mkdir(vfs_node_t* parent, const char* name, mode_t mode) {
    if (!parent || !name || parent->type != FILE_TYPE_DIRECTORY) {
        return E_INVAL;
    }
    
    // Create new directory inode
    vfs_node_t* dir_node = tmpfs_alloc_inode(parent->filesystem);
    if (!dir_node) {
        return E_NOMEM;
    }
    
    dir_node->type = FILE_TYPE_DIRECTORY;
    dir_node->mode = mode;
    dir_node->uid = parent->uid; // Inherit from parent
    dir_node->gid = parent->gid;
    dir_node->parent = parent;
    dir_node->ops = &tmpfs_dir_ops;
    
    // Add directory entry to parent
    tmpfs_inode_t* parent_tmpfs = (tmpfs_inode_t*)parent->private_data;
    if (!parent_tmpfs) {
        vfs_destroy_node(dir_node);
        return E_INVAL;
    }
    
    // For simplicity, store directory entries as a linked list in the data
    tmpfs_dir_entry_t* dir_entry = (tmpfs_dir_entry_t*)memory_alloc(sizeof(tmpfs_dir_entry_t));
    if (!dir_entry) {
        vfs_destroy_node(dir_node);
        return E_NOMEM;
    }
    
    strcpy(dir_entry->name, name);
    dir_entry->inode = dir_node->inode;
    dir_entry->next = (tmpfs_dir_entry_t*)parent_tmpfs->data;
    parent_tmpfs->data = dir_entry;
    
    parent->modification_time = hal_get_timestamp();
    
    KDEBUG("Created tmpfs directory '%s' (inode %llu)", name, dir_node->inode);
    return SUCCESS;
}

static vfs_node_t* tmpfs_lookup(vfs_node_t* parent, const char* name) {
    if (!parent || !name || parent->type != FILE_TYPE_DIRECTORY) {
        return NULL;
    }
    
    tmpfs_inode_t* parent_tmpfs = (tmpfs_inode_t*)parent->private_data;
    if (!parent_tmpfs || !parent_tmpfs->data) {
        return NULL;
    }
    
    // Search directory entries
    tmpfs_dir_entry_t* entry = (tmpfs_dir_entry_t*)parent_tmpfs->data;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            // Find the inode in the filesystem
            tmpfs_superblock_t* sb = (tmpfs_superblock_t*)parent->filesystem->private_data;
            tmpfs_inode_t* inode = sb->inode_list;
            
            while (inode) {
                if (inode->vfs_node->inode == entry->inode) {
                    return vfs_get_node(inode->vfs_node);
                }
                inode = inode->next;
            }
            break;
        }
        entry = entry->next;
    }
    
    return NULL;
}

static error_t tmpfs_create(vfs_node_t* parent, const char* name, mode_t mode) {
    if (!parent || !name || parent->type != FILE_TYPE_DIRECTORY) {
        return E_INVAL;
    }
    
    // Create new file inode
    vfs_node_t* file_node = tmpfs_alloc_inode(parent->filesystem);
    if (!file_node) {
        return E_NOMEM;
    }
    
    file_node->type = FILE_TYPE_REGULAR;
    file_node->mode = mode;
    file_node->uid = parent->uid; // Inherit from parent
    file_node->gid = parent->gid;
    file_node->parent = parent;
    file_node->ops = &tmpfs_file_ops;
    
    // Add directory entry to parent
    tmpfs_inode_t* parent_tmpfs = (tmpfs_inode_t*)parent->private_data;
    if (!parent_tmpfs) {
        vfs_destroy_node(file_node);
        return E_INVAL;
    }
    
    tmpfs_dir_entry_t* dir_entry = (tmpfs_dir_entry_t*)memory_alloc(sizeof(tmpfs_dir_entry_t));
    if (!dir_entry) {
        vfs_destroy_node(file_node);
        return E_NOMEM;
    }
    
    strcpy(dir_entry->name, name);
    dir_entry->inode = file_node->inode;
    dir_entry->next = (tmpfs_dir_entry_t*)parent_tmpfs->data;
    parent_tmpfs->data = dir_entry;
    
    parent->modification_time = hal_get_timestamp();
    
    KDEBUG("Created tmpfs file '%s' (inode %llu)", name, file_node->inode);
    return SUCCESS;
}