#include "filesystem/include/filesystem.h"
#include "kernel.h"
#include "memory/include/memory.h"

// DevFS (Device Filesystem) implementation
// This filesystem exposes devices as files in /dev

// Device types
typedef enum {
    DEVICE_TYPE_CHARACTER,
    DEVICE_TYPE_BLOCK,
    DEVICE_TYPE_NETWORK
} device_type_t;

// Device structure
typedef struct device {
    char name[64];
    device_type_t type;
    dev_t device_id;
    vfs_operations_t* ops;
    void* private_data;
    struct device* next;
} device_t;

typedef struct devfs_superblock {
    device_t* device_list;
    u32 device_count;
} devfs_superblock_t;

// Forward declarations
static error_t devfs_mount(filesystem_t* fs, vfs_node_t* mount_point, u32 flags, const void* data);
static error_t devfs_unmount(filesystem_t* fs);
static error_t devfs_read_super(filesystem_t* fs, void* device);
static vfs_node_t* devfs_lookup(vfs_node_t* parent, const char* name);
static error_t devfs_readdir(vfs_node_t* node, directory_entry_t** entries, size_t* count);

// Console device operations
static ssize_t console_read(vfs_node_t* node, void* buffer, size_t size, off_t offset);
static ssize_t console_write(vfs_node_t* node, const void* buffer, size_t size, off_t offset);

// Null device operations
static ssize_t null_read(vfs_node_t* node, void* buffer, size_t size, off_t offset);
static ssize_t null_write(vfs_node_t* node, const void* buffer, size_t size, off_t offset);

// Zero device operations
static ssize_t zero_read(vfs_node_t* node, void* buffer, size_t size, off_t offset);
static ssize_t zero_write(vfs_node_t* node, const void* buffer, size_t size, off_t offset);

// Random device operations
static ssize_t random_read(vfs_node_t* node, void* buffer, size_t size, off_t offset);

// Filesystem operations
filesystem_operations_t devfs_ops = {
    .mount = devfs_mount,
    .unmount = devfs_unmount,
    .read_super = devfs_read_super
};

// Directory operations
static vfs_operations_t devfs_dir_ops = {
    .lookup = devfs_lookup,
    .readdir = devfs_readdir
};

// Console device operations
static vfs_operations_t console_ops = {
    .read = console_read,
    .write = console_write
};

// Null device operations
static vfs_operations_t null_ops = {
    .read = null_read,
    .write = null_write
};

// Zero device operations
static vfs_operations_t zero_ops = {
    .read = zero_read,
    .write = zero_write
};

// Random device operations
static vfs_operations_t random_ops = {
    .read = random_read,
    .write = null_write  // Writing to random is like null
};

static error_t devfs_mount(filesystem_t* fs, vfs_node_t* mount_point, u32 flags, const void* data) {
    (void)mount_point;
    (void)flags;
    (void)data;
    
    if (!fs) {
        return E_INVAL;
    }
    
    KDEBUG("Mounting devfs");
    
    // Allocate superblock
    devfs_superblock_t* sb = (devfs_superblock_t*)memory_alloc(sizeof(devfs_superblock_t));
    if (!sb) {
        return E_NOMEM;
    }
    
    memset(sb, 0, sizeof(devfs_superblock_t));
    
    fs->private_data = sb;
    fs->default_dir_ops = &devfs_dir_ops;
    
    // Register standard devices
    device_t* console = (device_t*)memory_alloc(sizeof(device_t));
    if (console) {
        strcpy(console->name, "console");
        console->type = DEVICE_TYPE_CHARACTER;
        console->device_id = 1;
        console->ops = &console_ops;
        console->next = sb->device_list;
        sb->device_list = console;
        sb->device_count++;
    }
    
    device_t* null_dev = (device_t*)memory_alloc(sizeof(device_t));
    if (null_dev) {
        strcpy(null_dev->name, "null");
        null_dev->type = DEVICE_TYPE_CHARACTER;
        null_dev->device_id = 2;
        null_dev->ops = &null_ops;
        null_dev->next = sb->device_list;
        sb->device_list = null_dev;
        sb->device_count++;
    }
    
    device_t* zero_dev = (device_t*)memory_alloc(sizeof(device_t));
    if (zero_dev) {
        strcpy(zero_dev->name, "zero");
        zero_dev->type = DEVICE_TYPE_CHARACTER;
        zero_dev->device_id = 3;
        zero_dev->ops = &zero_ops;
        zero_dev->next = sb->device_list;
        sb->device_list = zero_dev;
        sb->device_count++;
    }
    
    device_t* random_dev = (device_t*)memory_alloc(sizeof(device_t));
    if (random_dev) {
        strcpy(random_dev->name, "random");
        random_dev->type = DEVICE_TYPE_CHARACTER;
        random_dev->device_id = 4;
        random_dev->ops = &random_ops;
        random_dev->next = sb->device_list;
        sb->device_list = random_dev;
        sb->device_count++;
    }
    
    KDEBUG("DevFS mounted with %u devices", sb->device_count);
    return SUCCESS;
}

static error_t devfs_unmount(filesystem_t* fs) {
    if (!fs || !fs->private_data) {
        return E_INVAL;
    }
    
    KDEBUG("Unmounting devfs");
    
    devfs_superblock_t* sb = (devfs_superblock_t*)fs->private_data;
    
    // Free all devices
    device_t* device = sb->device_list;
    while (device) {
        device_t* next = device->next;
        memory_free(device);
        device = next;
    }
    
    // Free superblock
    memory_free(sb);
    fs->private_data = NULL;
    
    return SUCCESS;
}

static error_t devfs_read_super(filesystem_t* fs, void* device) {
    (void)device;
    return devfs_mount(fs, NULL, 0, NULL);
}

static vfs_node_t* devfs_lookup(vfs_node_t* parent, const char* name) {
    if (!parent || !name || parent->type != FILE_TYPE_DIRECTORY) {
        return NULL;
    }
    
    devfs_superblock_t* sb = (devfs_superblock_t*)parent->filesystem->private_data;
    if (!sb) {
        return NULL;
    }
    
    // Search for device
    device_t* device = sb->device_list;
    while (device) {
        if (strcmp(device->name, name) == 0) {
            // Create VFS node for device
            vfs_node_t* node = vfs_create_node(parent->filesystem, device->device_id, 
                                             device->type == DEVICE_TYPE_CHARACTER ? 
                                             FILE_TYPE_DEVICE_CHAR : FILE_TYPE_DEVICE_BLOCK);
            if (node) {
                node->ops = device->ops;
                node->private_data = device;
                node->mode = 0666; // Default device permissions
                node->parent = parent;
            }
            return node;
        }
        device = device->next;
    }
    
    return NULL;
}

static error_t devfs_readdir(vfs_node_t* node, directory_entry_t** entries, size_t* count) {
    if (!node || !entries || !count || node->type != FILE_TYPE_DIRECTORY) {
        return E_INVAL;
    }
    
    devfs_superblock_t* sb = (devfs_superblock_t*)node->filesystem->private_data;
    if (!sb) {
        return E_INVAL;
    }
    
    // Allocate directory entries
    directory_entry_t* dir_entries = (directory_entry_t*)memory_alloc(
        sizeof(directory_entry_t) * sb->device_count);
    if (!dir_entries) {
        return E_NOMEM;
    }
    
    // Fill directory entries
    device_t* device = sb->device_list;
    size_t i = 0;
    
    while (device && i < sb->device_count) {
        strcpy(dir_entries[i].name, device->name);
        dir_entries[i].inode = device->device_id;
        dir_entries[i].type = device->type == DEVICE_TYPE_CHARACTER ? 
                             FILE_TYPE_DEVICE_CHAR : FILE_TYPE_DEVICE_BLOCK;
        dir_entries[i].offset = i;
        dir_entries[i].next = (i < sb->device_count - 1) ? &dir_entries[i + 1] : NULL;
        
        device = device->next;
        i++;
    }
    
    *entries = dir_entries;
    *count = sb->device_count;
    
    return SUCCESS;
}

// Device implementations

static ssize_t console_read(vfs_node_t* node, void* buffer, size_t size, off_t offset) {
    (void)node;
    (void)offset;
    
    // For now, just return empty - real implementation would read from keyboard buffer
    (void)buffer;
    (void)size;
    return 0;
}

static ssize_t console_write(vfs_node_t* node, const void* buffer, size_t size, off_t offset) {
    (void)node;
    (void)offset;
    
    if (!buffer) {
        return -1;
    }
    
    // Write to console
    const char* str = (const char*)buffer;
    for (size_t i = 0; i < size; i++) {
        hal_console_print("%c", str[i]);
    }
    
    return size;
}

static ssize_t null_read(vfs_node_t* node, void* buffer, size_t size, off_t offset) {
    (void)node;
    (void)buffer;
    (void)size;
    (void)offset;
    
    // /dev/null always returns EOF
    return 0;
}

static ssize_t null_write(vfs_node_t* node, const void* buffer, size_t size, off_t offset) {
    (void)node;
    (void)buffer;
    (void)offset;
    
    // /dev/null accepts everything and discards it
    return size;
}

static ssize_t zero_read(vfs_node_t* node, void* buffer, size_t size, off_t offset) {
    (void)node;
    (void)offset;
    
    if (!buffer) {
        return -1;
    }
    
    // /dev/zero returns zeros
    memset(buffer, 0, size);
    return size;
}

static ssize_t zero_write(vfs_node_t* node, const void* buffer, size_t size, off_t offset) {
    (void)node;
    (void)buffer;
    (void)offset;
    
    // /dev/zero accepts everything and discards it
    return size;
}

static ssize_t random_read(vfs_node_t* node, void* buffer, size_t size, off_t offset) {
    (void)node;
    (void)offset;
    
    if (!buffer) {
        return -1;
    }
    
    // Simple pseudo-random number generator
    static u32 seed = 1;
    u8* bytes = (u8*)buffer;
    
    for (size_t i = 0; i < size; i++) {
        seed = seed * 1103515245 + 12345;
        bytes[i] = (u8)(seed >> 16);
    }
    
    return size;
}