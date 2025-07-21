#include "filesystem/include/filesystem.h"
#include "kernel.h"
#include "memory/include/memory.h"
#include "security/security.h"
#include "process/include/process.h"
#include "filesystem/vfs.h"
#include <string.h>

// Global VFS state
static bool filesystem_initialized = false;
static vfs_node_t* vfs_root = NULL;
static vfs_node_t* current_working_directory = NULL;

// Filesystem registry
static filesystem_operations_t* registered_filesystems[16] = {NULL};
static const char* filesystem_names[16] = {NULL};

// Mount table
static mount_entry_t* mount_table_head = NULL;
static u32 mount_count = 0;

// File descriptor table
static file_descriptor_t* global_fd_table[MAX_OPEN_FILES] = {NULL};
static int next_fd = 3; // Start after stdin, stdout, stderr

// Statistics
static fs_stats_t filesystem_statistics = {0};

// Node cache for frequently accessed files
static vfs_node_t* node_cache_head = NULL;
static u32 node_cache_size = 0;
static const u32 MAX_CACHED_NODES = 256;

// VFS constants
#define MAX_MOUNT_POINTS 16
#define MAX_OPEN_FILES 256
#define MAX_FILE_SYSTEMS 8
#define VFS_PATH_MAX 256

// File system types
#define FS_TYPE_RAMFS 1
#define FS_TYPE_FAT32 2
#define FS_TYPE_EXT4 3

// File flags
#define FILE_FLAG_READ 0x01
#define FILE_FLAG_WRITE 0x02
#define FILE_FLAG_APPEND 0x04
#define FILE_FLAG_CREATE 0x08
#define FILE_FLAG_TRUNCATE 0x10

// Global VFS state
static vfs_state_t vfs_state = {0};
static mount_point_t mount_points[MAX_MOUNT_POINTS];
static int mount_point_count = 0;
static file_system_t file_systems[MAX_FILE_SYSTEMS];
static int file_system_count = 0;
static file_descriptor_t open_files[MAX_OPEN_FILES];
static int open_file_count = 0;

// Root file system (RAMFS)
static ramfs_t root_fs = {0};

// Forward declarations
static error_t create_root_filesystem(void);
static vfs_node_t* vfs_lookup_child(vfs_node_t* parent, const char* name);
static error_t vfs_cache_node(vfs_node_t* node);
static vfs_node_t* vfs_find_cached_node(filesystem_t* fs, inode_t inode);

error_t filesystem_init(void) {
    if (filesystem_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing Virtual File System");
    
    // Initialize statistics
    memset(&filesystem_statistics, 0, sizeof(filesystem_statistics));
    
    // Initialize file descriptor table
    memset(global_fd_table, 0, sizeof(global_fd_table));
    
    // Register built-in filesystems
    register_filesystem(FS_TYPE_TMPFS, "tmpfs", &tmpfs_ops);
    register_filesystem(FS_TYPE_DEVFS, "devfs", &devfs_ops);
    register_filesystem(FS_TYPE_PROCFS, "procfs", &procfs_ops);
    register_filesystem(FS_TYPE_RAEENFS, "raeenfs", &raeenfs_ops);
    
    // Create root filesystem (tmpfs for now)
    error_t result = create_root_filesystem();
    if (result != SUCCESS) {
        KERROR("Failed to create root filesystem: %d", result);
        return result;
    }
    
    // Set current working directory to root
    current_working_directory = vfs_get_node(vfs_root);
    
    // Create standard directories
    vfs_mkdir("/dev", PERM_DEFAULT_DIR);
    vfs_mkdir("/proc", PERM_DEFAULT_DIR);
    vfs_mkdir("/sys", PERM_DEFAULT_DIR);
    vfs_mkdir("/tmp", PERM_DEFAULT_DIR);
    vfs_mkdir("/home", PERM_DEFAULT_DIR);
    vfs_mkdir("/bin", PERM_DEFAULT_DIR);
    vfs_mkdir("/usr", PERM_DEFAULT_DIR);
    vfs_mkdir("/var", PERM_DEFAULT_DIR);
    
    // Mount devfs on /dev
    vfs_mount(NULL, "/dev", FS_TYPE_DEVFS, 0, NULL);
    
    // Mount procfs on /proc
    vfs_mount(NULL, "/proc", FS_TYPE_PROCFS, 0, NULL);
    
    filesystem_initialized = true;
    
    KINFO("Virtual File System initialized");
    return SUCCESS;
}

void filesystem_shutdown(void) {
    if (!filesystem_initialized) {
        return;
    }
    
    KINFO("Shutting down Virtual File System");
    
    // Unmount all filesystems
    mount_entry_t* mount = mount_table_head;
    while (mount) {
        mount_entry_t* next = mount->next;
        vfs_unmount(mount->mount_path);
        mount = next;
    }
    
    // Close all open file descriptors
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (global_fd_table[i]) {
            vfs_close(i);
        }
    }
    
    // Clear node cache
    vfs_node_t* node = node_cache_head;
    while (node) {
        vfs_node_t* next = node->next;
        vfs_destroy_node(node);
        node = next;
    }
    
    // Release root filesystem
    if (vfs_root) {
        vfs_put_node(vfs_root);
        vfs_root = NULL;
    }
    
    if (current_working_directory) {
        vfs_put_node(current_working_directory);
        current_working_directory = NULL;
    }
    
    filesystem_initialized = false;
}

static error_t create_root_filesystem(void) {
    // Create root tmpfs filesystem
    filesystem_t* rootfs = create_filesystem(FS_TYPE_TMPFS);
    if (!rootfs) {
        return E_NOMEM;
    }
    
    // Initialize root filesystem
    error_t result = rootfs->ops->read_super(rootfs, NULL);
    if (result != SUCCESS) {
        destroy_filesystem(rootfs);
        return result;
    }
    
    // Create root node
    vfs_root = vfs_create_node(rootfs, 1, FILE_TYPE_DIRECTORY);
    if (!vfs_root) {
        destroy_filesystem(rootfs);
        return E_NOMEM;
    }
    
    vfs_root->mode = PERM_DEFAULT_DIR;
    vfs_root->uid = 0;
    vfs_root->gid = 0;
    vfs_root->creation_time = hal_get_timestamp();
    vfs_root->modification_time = vfs_root->creation_time;
    vfs_root->access_time = vfs_root->creation_time;
    
    rootfs->root = vfs_root;
    rootfs->mount_point = NULL; // Root has no mount point
    
    KDEBUG("Created root filesystem");
    return SUCCESS;
}

error_t register_filesystem(filesystem_type_t type, const char* name, 
                           filesystem_operations_t* ops) {
    if (type >= 16 || !name || !ops) {
        return E_INVAL;
    }
    
    if (registered_filesystems[type] != NULL) {
        return E_EXIST;
    }
    
    registered_filesystems[type] = ops;
    filesystem_names[type] = name;
    filesystem_statistics.total_filesystems++;
    
    KDEBUG("Registered filesystem '%s' (type %d)", name, type);
    return SUCCESS;
}

filesystem_t* create_filesystem(filesystem_type_t type) {
    if (type >= 16 || !registered_filesystems[type]) {
        return NULL;
    }
    
    filesystem_t* fs = (filesystem_t*)memory_alloc(sizeof(filesystem_t));
    if (!fs) {
        return NULL;
    }
    
    memset(fs, 0, sizeof(filesystem_t));
    
    fs->type = type;
    fs->ops = registered_filesystems[type];
    strcpy(fs->name, filesystem_names[type]);
    fs->block_size = 4096; // Default block size
    
    KDEBUG("Created filesystem '%s'", fs->name);
    return fs;
}

void destroy_filesystem(filesystem_t* fs) {
    if (!fs) {
        return;
    }
    
    KDEBUG("Destroying filesystem '%s'", fs->name);
    
    // Cleanup filesystem-specific data
    if (fs->ops && fs->ops->unmount) {
        fs->ops->unmount(fs);
    }
    
    memory_free(fs);
}

vfs_node_t* vfs_create_node(filesystem_t* fs, inode_t inode, file_type_t type) {
    vfs_node_t* node = (vfs_node_t*)memory_alloc(sizeof(vfs_node_t));
    if (!node) {
        return NULL;
    }
    
    memset(node, 0, sizeof(vfs_node_t));
    
    node->inode = inode;
    node->type = type;
    node->filesystem = fs;
    node->ref_count = 1;
    node->creation_time = hal_get_timestamp();
    node->modification_time = node->creation_time;
    node->access_time = node->creation_time;
    node->link_count = 1;
    
    // Set default operations based on filesystem
    if (fs) {
        if (type == FILE_TYPE_DIRECTORY) {
            node->ops = fs->default_dir_ops;
        } else {
            node->ops = fs->default_file_ops;
        }
    }
    
    return node;
}

void vfs_destroy_node(vfs_node_t* node) {
    if (!node) {
        return;
    }
    
    KDEBUG("Destroying VFS node (inode %llu)", node->inode);
    
    // Free filesystem-specific data
    if (node->private_data) {
        memory_free(node->private_data);
    }
    
    memory_free(node);
}

vfs_node_t* vfs_get_node(vfs_node_t* node) {
    if (!node) {
        return NULL;
    }
    
    node->ref_count++;
    node->last_access = hal_get_timestamp();
    return node;
}

void vfs_put_node(vfs_node_t* node) {
    if (!node) {
        return;
    }
    
    node->ref_count--;
    if (node->ref_count == 0) {
        vfs_destroy_node(node);
    }
}

vfs_node_t* vfs_lookup_path(const char* path, bool follow_symlinks) {
    if (!path || !filesystem_initialized) {
        return NULL;
    }
    
    vfs_node_t* current;
    
    // Start from root or current directory
    if (path[0] == '/') {
        current = vfs_get_node(vfs_root);
        path++; // Skip leading slash
    } else {
        current = vfs_get_node(current_working_directory);
    }
    
    // Handle empty path or root path
    if (!path[0]) {
        return current;
    }
    
    // Parse path components
    char component[MAX_FILENAME_LENGTH + 1];
    const char* next_slash;
    
    while (*path) {
        // Find next component
        next_slash = strchr(path, '/');
        size_t component_len;
        
        if (next_slash) {
            component_len = next_slash - path;
        } else {
            component_len = strlen(path);
        }
        
        if (component_len > MAX_FILENAME_LENGTH) {
            vfs_put_node(current);
            return NULL;
        }
        
        strncpy(component, path, component_len);
        component[component_len] = '\0';
        
        // Skip empty components (double slashes)
        if (component_len == 0) {
            path = next_slash + 1;
            continue;
        }
        
        // Handle special directories
        if (strcmp(component, ".") == 0) {
            // Current directory, do nothing
        } else if (strcmp(component, "..") == 0) {
            // Parent directory
            if (current->parent) {
                vfs_node_t* parent = vfs_get_node(current->parent);
                vfs_put_node(current);
                current = parent;
            }
        } else {
            // Regular lookup
            vfs_node_t* child = vfs_lookup_child(current, component);
            if (!child) {
                vfs_put_node(current);
                return NULL;
            }
            
            // Handle symlinks if requested
            if (follow_symlinks && child->type == FILE_TYPE_SYMLINK) {
                // TODO: Implement symlink following
                // For now, just return the symlink node
            }
            
            vfs_put_node(current);
            current = child;
        }
        
        // Move to next component
        if (next_slash) {
            path = next_slash + 1;
        } else {
            break;
        }
    }
    
    return current;
}

static vfs_node_t* vfs_lookup_child(vfs_node_t* parent, const char* name) {
    if (!parent || parent->type != FILE_TYPE_DIRECTORY || !name) {
        return NULL;
    }
    
    // Use filesystem-specific lookup if available
    if (parent->ops && parent->ops->lookup) {
        return parent->ops->lookup(parent, name);
    }
    
    // Default implementation: linear search through children
    vfs_node_t* child = parent->first_child;
    while (child) {
        // TODO: Compare with actual filename stored in filesystem
        // For now, this is a placeholder
        child = child->next_sibling;
    }
    
    return NULL;
}

int vfs_open(const char* path, u32 flags, mode_t mode) {
    if (!path || !filesystem_initialized) {
        return -1;
    }
    
    KDEBUG("Opening file '%s' with flags 0x%x", path, flags);
    
    vfs_node_t* node = vfs_lookup_path(path, true);
    
    // Handle file creation
    if (!node && (flags & O_CREAT)) {
        error_t result = vfs_create(path, mode);
        if (result != SUCCESS) {
            return -1;
        }
        node = vfs_lookup_path(path, true);
    }
    
    if (!node) {
        return -1;
    }
    
    // Check permissions
    // TODO: Implement proper permission checking
    
    // Allocate file descriptor
    file_descriptor_t* fd = (file_descriptor_t*)memory_alloc(sizeof(file_descriptor_t));
    if (!fd) {
        vfs_put_node(node);
        return -1;
    }
    
    memset(fd, 0, sizeof(file_descriptor_t));
    
    // Find available FD number
    int fd_num = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!global_fd_table[i]) {
            fd_num = i;
            break;
        }
    }
    
    if (fd_num == -1) {
        memory_free(fd);
        vfs_put_node(node);
        return -1;
    }
    
    // Initialize file descriptor
    fd->fd = fd_num;
    fd->node = node;
    fd->flags = flags;
    fd->position = 0;
    fd->ref_count = 1;
    fd->owner = current_process;
    fd->ops = node->ops;
    
    // Handle O_TRUNC
    if (flags & O_TRUNC && node->type == FILE_TYPE_REGULAR) {
        node->size = 0;
        // TODO: Actually truncate the file
    }
    
    // Handle O_APPEND
    if (flags & O_APPEND) {
        fd->position = node->size;
    }
    
    // Call filesystem-specific open
    if (node->ops && node->ops->open) {
        error_t result = node->ops->open(node, flags);
        if (result != SUCCESS) {
            memory_free(fd);
            vfs_put_node(node);
            return -1;
        }
    }
    
    global_fd_table[fd_num] = fd;
    filesystem_statistics.open_files++;
    
    KDEBUG("Opened file '%s' as FD %d", path, fd_num);
    return fd_num;
}

error_t vfs_close(int fd_num) {
    if (fd_num < 0 || fd_num >= MAX_OPEN_FILES || !global_fd_table[fd_num]) {
        return E_INVAL;
    }
    
    file_descriptor_t* fd = global_fd_table[fd_num];
    
    KDEBUG("Closing FD %d", fd_num);
    
    // Call filesystem-specific close
    if (fd->node && fd->node->ops && fd->node->ops->close) {
        fd->node->ops->close(fd->node);
    }
    
    // Release node reference
    if (fd->node) {
        vfs_put_node(fd->node);
    }
    
    // Free file descriptor
    memory_free(fd);
    global_fd_table[fd_num] = NULL;
    filesystem_statistics.open_files--;
    
    return SUCCESS;
}

ssize_t vfs_read(int fd_num, void* buffer, size_t size) {
    if (fd_num < 0 || fd_num >= MAX_OPEN_FILES || !global_fd_table[fd_num] || !buffer) {
        return -1;
    }
    
    file_descriptor_t* fd = global_fd_table[fd_num];
    
    if (!(fd->flags & (O_RDONLY | O_RDWR))) {
        return -1; // File not open for reading
    }
    
    if (!fd->node || !fd->node->ops || !fd->node->ops->read) {
        return -1;
    }
    
    ssize_t bytes_read = fd->node->ops->read(fd->node, buffer, size, fd->position);
    if (bytes_read > 0) {
        fd->position += bytes_read;
        fd->node->access_time = hal_get_timestamp();
        filesystem_statistics.read_operations++;
        filesystem_statistics.bytes_read += bytes_read;
    }
    
    return bytes_read;
}

ssize_t vfs_write(int fd_num, const void* buffer, size_t size) {
    if (fd_num < 0 || fd_num >= MAX_OPEN_FILES || !global_fd_table[fd_num] || !buffer) {
        return -1;
    }
    
    file_descriptor_t* fd = global_fd_table[fd_num];
    
    if (!(fd->flags & (O_WRONLY | O_RDWR))) {
        return -1; // File not open for writing
    }
    
    if (!fd->node || !fd->node->ops || !fd->node->ops->write) {
        return -1;
    }
    
    ssize_t bytes_written = fd->node->ops->write(fd->node, buffer, size, fd->position);
    if (bytes_written > 0) {
        fd->position += bytes_written;
        fd->node->modification_time = hal_get_timestamp();
        fd->node->size = MAX(fd->node->size, fd->position);
        filesystem_statistics.write_operations++;
        filesystem_statistics.bytes_written += bytes_written;
    }
    
    return bytes_written;
}

off_t vfs_seek(int fd_num, off_t offset, int whence) {
    if (fd_num < 0 || fd_num >= MAX_OPEN_FILES || !global_fd_table[fd_num]) {
        return -1;
    }
    
    file_descriptor_t* fd = global_fd_table[fd_num];
    
    off_t new_position;
    
    switch (whence) {
        case SEEK_SET:
            new_position = offset;
            break;
        case SEEK_CUR:
            new_position = fd->position + offset;
            break;
        case SEEK_END:
            new_position = fd->node->size + offset;
            break;
        default:
            return -1;
    }
    
    if (new_position < 0) {
        return -1;
    }
    
    fd->position = new_position;
    return new_position;
}

error_t vfs_mkdir(const char* path, mode_t mode) {
    if (!path) {
        return E_INVAL;
    }
    
    KDEBUG("Creating directory '%s'", path);
    
    // Find parent directory
    char parent_path[MAX_PATH_LENGTH];
    char dir_name[MAX_FILENAME_LENGTH + 1];
    
    const char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        strcpy(parent_path, ".");
        strcpy(dir_name, path);
    } else {
        size_t parent_len = last_slash - path;
        if (parent_len == 0) {
            strcpy(parent_path, "/");
        } else {
            strncpy(parent_path, path, parent_len);
            parent_path[parent_len] = '\0';
        }
        strcpy(dir_name, last_slash + 1);
    }
    
    vfs_node_t* parent = vfs_lookup_path(parent_path, true);
    if (!parent || parent->type != FILE_TYPE_DIRECTORY) {
        if (parent) vfs_put_node(parent);
        return E_FS_NOT_DIR;
    }
    
    // Check if directory already exists
    vfs_node_t* existing = vfs_lookup_child(parent, dir_name);
    if (existing) {
        vfs_put_node(existing);
        vfs_put_node(parent);
        return E_FS_EXISTS;
    }
    
    // Create directory
    error_t result = E_OPNOTSUPP;
    if (parent->ops && parent->ops->mkdir) {
        result = parent->ops->mkdir(parent, dir_name, mode);
    }
    
    vfs_put_node(parent);
    return result;
}

// Statistics and debugging
fs_stats_t* filesystem_get_stats(void) {
    return &filesystem_statistics;
}

void filesystem_dump_stats(void) {
    hal_console_print("Filesystem Statistics:\n");
    hal_console_print("  Total filesystems: %u\n", filesystem_statistics.total_filesystems);
    hal_console_print("  Mounted filesystems: %u\n", filesystem_statistics.mounted_filesystems);
    hal_console_print("  Open files: %u\n", filesystem_statistics.open_files);
    hal_console_print("  Read operations: %llu\n", filesystem_statistics.read_operations);
    hal_console_print("  Write operations: %llu\n", filesystem_statistics.write_operations);
    hal_console_print("  Bytes read: %llu\n", filesystem_statistics.bytes_read);
    hal_console_print("  Bytes written: %llu\n", filesystem_statistics.bytes_written);
}

// Initialize VFS
error_t vfs_init(void) {
    if (vfs_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing Virtual File System...");
    
    // Initialize VFS state
    memset(&vfs_state, 0, sizeof(vfs_state));
    vfs_state.root_mounted = false;
    vfs_state.current_directory[0] = '/';
    vfs_state.current_directory[1] = '\0';
    
    // Initialize mount points
    memset(mount_points, 0, sizeof(mount_points));
    mount_point_count = 0;
    
    // Initialize file systems
    memset(file_systems, 0, sizeof(file_systems));
    file_system_count = 0;
    
    // Initialize open files
    memset(open_files, 0, sizeof(open_files));
    open_file_count = 0;
    
    // Register file system types
    vfs_register_filesystem("ramfs", FS_TYPE_RAMFS, ramfs_ops);
    vfs_register_filesystem("fat32", FS_TYPE_FAT32, fat32_ops);
    vfs_register_filesystem("ext4", FS_TYPE_EXT4, ext4_ops);
    
    // Initialize root file system (RAMFS)
    ramfs_init(&root_fs);
    
    // Mount root file system
    vfs_mount_root();
    
    vfs_initialized = true;
    KINFO("Virtual File System initialized successfully");
    
    return SUCCESS;
}

// Register a file system type
error_t vfs_register_filesystem(const char* name, int type, file_system_ops_t* ops) {
    if (!name || !ops || file_system_count >= MAX_FILE_SYSTEMS) {
        return E_INVAL;
    }
    
    file_system_t* fs = &file_systems[file_system_count];
    strncpy(fs->name, name, sizeof(fs->name) - 1);
    fs->type = type;
    fs->ops = ops;
    
    file_system_count++;
    
    KDEBUG("Registered file system: %s (type %d)", name, type);
    
    return SUCCESS;
}

// Mount root file system
error_t vfs_mount_root(void) {
    if (vfs_state.root_mounted) {
        return SUCCESS;
    }
    
    // Mount RAMFS as root
    mount_point_t* mount = &mount_points[0];
    strcpy(mount->path, "/");
    mount->fs_type = FS_TYPE_RAMFS;
    mount->fs_data = &root_fs;
    mount->mounted = true;
    
    mount_point_count = 1;
    vfs_state.root_mounted = true;
    
    KINFO("Root file system mounted at /");
    
    return SUCCESS;
}

// Mount a file system
error_t vfs_mount(const char* device, const char* path, const char* fs_type, int flags) {
    if (!device || !path || !fs_type || mount_point_count >= MAX_MOUNT_POINTS) {
        return E_INVAL;
    }
    
    // Find file system type
    file_system_t* fs = NULL;
    for (int i = 0; i < file_system_count; i++) {
        if (strcmp(file_systems[i].name, fs_type) == 0) {
            fs = &file_systems[i];
            break;
        }
    }
    
    if (!fs) {
        KERROR("Unknown file system type: %s", fs_type);
        return E_INVAL;
    }
    
    // Create mount point
    mount_point_t* mount = &mount_points[mount_point_count];
    strncpy(mount->path, path, sizeof(mount->path) - 1);
    mount->fs_type = fs->type;
    mount->flags = flags;
    
    // Initialize file system
    if (fs->ops->mount) {
        error_t result = fs->ops->mount(device, mount);
        if (result != SUCCESS) {
            KERROR("Failed to mount %s at %s", device, path);
            return result;
        }
    }
    
    mount->mounted = true;
    mount_point_count++;
    
    KINFO("Mounted %s at %s", device, path);
    
    return SUCCESS;
}

// Unmount a file system
error_t vfs_umount(const char* path) {
    if (!path) {
        return E_INVAL;
    }
    
    for (int i = 0; i < mount_point_count; i++) {
        mount_point_t* mount = &mount_points[i];
        if (strcmp(mount->path, path) == 0 && mount->mounted) {
            // Unmount file system
            if (mount->fs_data) {
                file_system_t* fs = vfs_get_filesystem(mount->fs_type);
                if (fs && fs->ops->umount) {
                    fs->ops->umount(mount);
                }
            }
            
            mount->mounted = false;
            
            KINFO("Unmounted %s", path);
            return SUCCESS;
        }
    }
    
    KERROR("Mount point not found: %s", path);
    return E_NOENT;
}

// Open a file
int vfs_open(const char* pathname, int flags, mode_t mode) {
    if (!pathname) {
        return -EINVAL;
    }
    
    KDEBUG("vfs_open: %s, flags=%d, mode=%d", pathname, flags, mode);
    
    // Find mount point
    mount_point_t* mount = vfs_find_mount_point(pathname);
    if (!mount) {
        KERROR("No mount point found for path: %s", pathname);
        return -ENOENT;
    }
    
    // Get relative path
    char rel_path[VFS_PATH_MAX];
    if (strcmp(mount->path, "/") == 0) {
        strcpy(rel_path, pathname);
    } else {
        strcpy(rel_path, pathname + strlen(mount->path));
    }
    
    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!open_files[i].used) {
            fd = i;
            break;
        }
    }
    
    if (fd == -1) {
        KERROR("No free file descriptors");
        return -EMFILE;
    }
    
    // Get file system operations
    file_system_t* fs = vfs_get_filesystem(mount->fs_type);
    if (!fs || !fs->ops->open) {
        KERROR("File system operations not available");
        return -ENOSYS;
    }
    
    // Open file
    file_descriptor_t* file = &open_files[fd];
    memset(file, 0, sizeof(file_descriptor_t));
    
    error_t result = fs->ops->open(mount, rel_path, flags, mode, file);
    if (result != SUCCESS) {
        KERROR("Failed to open file: %s", pathname);
        return -result;
    }
    
    file->used = true;
    file->fd = fd;
    file->flags = flags;
    file->mount = mount;
    strncpy(file->path, pathname, sizeof(file->path) - 1);
    
    open_file_count++;
    
    KDEBUG("Opened file %s as fd %d", pathname, fd);
    
    return fd;
}

// Close a file
error_t vfs_close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        return E_INVAL;
    }
    
    file_descriptor_t* file = &open_files[fd];
    if (!file->used) {
        return E_INVAL;
    }
    
    KDEBUG("vfs_close: fd=%d, path=%s", fd, file->path);
    
    // Get file system operations
    file_system_t* fs = vfs_get_filesystem(file->mount->fs_type);
    if (fs && fs->ops->close) {
        fs->ops->close(file);
    }
    
    // Clear file descriptor
    memset(file, 0, sizeof(file_descriptor_t));
    open_file_count--;
    
    KDEBUG("Closed file fd %d", fd);
    
    return SUCCESS;
}

// Read from a file
ssize_t vfs_read(int fd, void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !buf) {
        return -EINVAL;
    }
    
    file_descriptor_t* file = &open_files[fd];
    if (!file->used) {
        return -EINVAL;
    }
    
    KDEBUG("vfs_read: fd=%d, count=%zu", fd, count);
    
    // Get file system operations
    file_system_t* fs = vfs_get_filesystem(file->mount->fs_type);
    if (!fs || !fs->ops->read) {
        return -ENOSYS;
    }
    
    // Read from file
    ssize_t result = fs->ops->read(file, buf, count);
    
    KDEBUG("vfs_read: fd=%d, read %zd bytes", fd, result);
    
    return result;
}

// Write to a file
ssize_t vfs_write(int fd, const void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !buf) {
        return -EINVAL;
    }
    
    file_descriptor_t* file = &open_files[fd];
    if (!file->used) {
        return -EINVAL;
    }
    
    KDEBUG("vfs_write: fd=%d, count=%zu", fd, count);
    
    // Get file system operations
    file_system_t* fs = vfs_get_filesystem(file->mount->fs_type);
    if (!fs || !fs->ops->write) {
        return -ENOSYS;
    }
    
    // Write to file
    ssize_t result = fs->ops->write(file, buf, count);
    
    KDEBUG("vfs_write: fd=%d, wrote %zd bytes", fd, result);
    
    return result;
}

// Seek in a file
off_t vfs_lseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        return -EINVAL;
    }
    
    file_descriptor_t* file = &open_files[fd];
    if (!file->used) {
        return -EINVAL;
    }
    
    KDEBUG("vfs_lseek: fd=%d, offset=%ld, whence=%d", fd, offset, whence);
    
    // Get file system operations
    file_system_t* fs = vfs_get_filesystem(file->mount->fs_type);
    if (!fs || !fs->ops->lseek) {
        return -ENOSYS;
    }
    
    // Seek in file
    off_t result = fs->ops->lseek(file, offset, whence);
    
    KDEBUG("vfs_lseek: fd=%d, new position=%ld", fd, result);
    
    return result;
}

// Get file status
error_t vfs_stat(const char* pathname, struct stat* statbuf) {
    if (!pathname || !statbuf) {
        return E_INVAL;
    }
    
    KDEBUG("vfs_stat: %s", pathname);
    
    // Find mount point
    mount_point_t* mount = vfs_find_mount_point(pathname);
    if (!mount) {
        return E_NOENT;
    }
    
    // Get relative path
    char rel_path[VFS_PATH_MAX];
    if (strcmp(mount->path, "/") == 0) {
        strcpy(rel_path, pathname);
    } else {
        strcpy(rel_path, pathname + strlen(mount->path));
    }
    
    // Get file system operations
    file_system_t* fs = vfs_get_filesystem(mount->fs_type);
    if (!fs || !fs->ops->stat) {
        return E_NOSYS;
    }
    
    // Get file status
    error_t result = fs->ops->stat(mount, rel_path, statbuf);
    
    KDEBUG("vfs_stat: %s, result=%d", pathname, result);
    
    return result;
}

// Create a directory
error_t vfs_mkdir(const char* pathname, mode_t mode) {
    if (!pathname) {
        return E_INVAL;
    }
    
    KDEBUG("vfs_mkdir: %s, mode=%d", pathname, mode);
    
    // Find mount point
    mount_point_t* mount = vfs_find_mount_point(pathname);
    if (!mount) {
        return E_NOENT;
    }
    
    // Get relative path
    char rel_path[VFS_PATH_MAX];
    if (strcmp(mount->path, "/") == 0) {
        strcpy(rel_path, pathname);
    } else {
        strcpy(rel_path, pathname + strlen(mount->path));
    }
    
    // Get file system operations
    file_system_t* fs = vfs_get_filesystem(mount->fs_type);
    if (!fs || !fs->ops->mkdir) {
        return E_NOSYS;
    }
    
    // Create directory
    error_t result = fs->ops->mkdir(mount, rel_path, mode);
    
    KDEBUG("vfs_mkdir: %s, result=%d", pathname, result);
    
    return result;
}

// Remove a directory
error_t vfs_rmdir(const char* pathname) {
    if (!pathname) {
        return E_INVAL;
    }
    
    KDEBUG("vfs_rmdir: %s", pathname);
    
    // Find mount point
    mount_point_t* mount = vfs_find_mount_point(pathname);
    if (!mount) {
        return E_NOENT;
    }
    
    // Get relative path
    char rel_path[VFS_PATH_MAX];
    if (strcmp(mount->path, "/") == 0) {
        strcpy(rel_path, pathname);
    } else {
        strcpy(rel_path, pathname + strlen(mount->path));
    }
    
    // Get file system operations
    file_system_t* fs = vfs_get_filesystem(mount->fs_type);
    if (!fs || !fs->ops->rmdir) {
        return E_NOSYS;
    }
    
    // Remove directory
    error_t result = fs->ops->rmdir(mount, rel_path);
    
    KDEBUG("vfs_rmdir: %s, result=%d", pathname, result);
    
    return result;
}

// Remove a file
error_t vfs_unlink(const char* pathname) {
    if (!pathname) {
        return E_INVAL;
    }
    
    KDEBUG("vfs_unlink: %s", pathname);
    
    // Find mount point
    mount_point_t* mount = vfs_find_mount_point(pathname);
    if (!mount) {
        return E_NOENT;
    }
    
    // Get relative path
    char rel_path[VFS_PATH_MAX];
    if (strcmp(mount->path, "/") == 0) {
        strcpy(rel_path, pathname);
    } else {
        strcpy(rel_path, pathname + strlen(mount->path));
    }
    
    // Get file system operations
    file_system_t* fs = vfs_get_filesystem(mount->fs_type);
    if (!fs || !fs->ops->unlink) {
        return E_NOSYS;
    }
    
    // Remove file
    error_t result = fs->ops->unlink(mount, rel_path);
    
    KDEBUG("vfs_unlink: %s, result=%d", pathname, result);
    
    return result;
}

// Rename a file
error_t vfs_rename(const char* oldpath, const char* newpath) {
    if (!oldpath || !newpath) {
        return E_INVAL;
    }
    
    KDEBUG("vfs_rename: %s -> %s", oldpath, newpath);
    
    // Find mount points
    mount_point_t* old_mount = vfs_find_mount_point(oldpath);
    mount_point_t* new_mount = vfs_find_mount_point(newpath);
    
    if (!old_mount || !new_mount) {
        return E_NOENT;
    }
    
    // For now, only support renaming within the same file system
    if (old_mount != new_mount) {
        KERROR("Cross-file-system rename not supported");
        return E_NOSYS;
    }
    
    // Get relative paths
    char old_rel_path[VFS_PATH_MAX];
    char new_rel_path[VFS_PATH_MAX];
    
    if (strcmp(old_mount->path, "/") == 0) {
        strcpy(old_rel_path, oldpath);
        strcpy(new_rel_path, newpath);
    } else {
        strcpy(old_rel_path, oldpath + strlen(old_mount->path));
        strcpy(new_rel_path, newpath + strlen(new_mount->path));
    }
    
    // Get file system operations
    file_system_t* fs = vfs_get_filesystem(old_mount->fs_type);
    if (!fs || !fs->ops->rename) {
        return E_NOSYS;
    }
    
    // Rename file
    error_t result = fs->ops->rename(old_mount, old_rel_path, new_rel_path);
    
    KDEBUG("vfs_rename: %s -> %s, result=%d", oldpath, newpath, result);
    
    return result;
}

// Find mount point for a path
mount_point_t* vfs_find_mount_point(const char* path) {
    if (!path) {
        return NULL;
    }
    
    // Find the longest matching mount point
    mount_point_t* best_match = NULL;
    size_t best_length = 0;
    
    for (int i = 0; i < mount_point_count; i++) {
        mount_point_t* mount = &mount_points[i];
        if (!mount->mounted) {
            continue;
        }
        
        size_t mount_len = strlen(mount->path);
        if (strncmp(path, mount->path, mount_len) == 0 && mount_len > best_length) {
            best_match = mount;
            best_length = mount_len;
        }
    }
    
    return best_match;
}

// Get file system by type
file_system_t* vfs_get_filesystem(int type) {
    for (int i = 0; i < file_system_count; i++) {
        if (file_systems[i].type == type) {
            return &file_systems[i];
        }
    }
    return NULL;
}

// Get VFS state
vfs_state_t* vfs_get_state(void) {
    return &vfs_state;
}

// Check if VFS is initialized
bool vfs_is_initialized(void) {
    return vfs_initialized;
}

// Get current working directory
const char* vfs_get_current_directory(void) {
    return vfs_state.current_directory;
}

// Change current working directory
error_t vfs_change_directory(const char* path) {
    if (!path) {
        return E_INVAL;
    }
    
    // TODO: Validate path exists and is a directory
    
    strncpy(vfs_state.current_directory, path, sizeof(vfs_state.current_directory) - 1);
    
    KDEBUG("Changed current directory to: %s", path);
    
    return SUCCESS;
}