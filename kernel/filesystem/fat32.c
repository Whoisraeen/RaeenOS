#include <kernel/filesystem/fat32.h>
#include <kernel/filesystem/vfs.h>
#include <kernel/memory/memory.h>
#include <kernel/libc/string.h>
#include <kernel/libc/stdio.h>
#include <kernel/libc/stdlib.h>

// FAT32 Boot Sector Structure
typedef struct {
    uint8_t  jump_boot[3];        // Jump instruction
    uint8_t  oem_name[8];         // OEM name
    uint16_t bytes_per_sector;    // Bytes per sector
    uint8_t  sectors_per_cluster; // Sectors per cluster
    uint16_t reserved_sectors;    // Reserved sectors
    uint8_t  num_fats;            // Number of FATs
    uint16_t root_entries;        // Root directory entries (FAT12/16 only)
    uint16_t total_sectors_16;    // Total sectors (FAT12/16 only)
    uint8_t  media_type;          // Media type
    uint16_t fat_size_16;         // FAT size in sectors (FAT12/16 only)
    uint16_t sectors_per_track;   // Sectors per track
    uint16_t num_heads;           // Number of heads
    uint32_t hidden_sectors;      // Hidden sectors
    uint32_t total_sectors_32;    // Total sectors (FAT32 only)
    uint32_t fat_size_32;         // FAT size in sectors (FAT32 only)
    uint16_t ext_flags;           // Extended flags
    uint16_t fs_version;          // File system version
    uint32_t root_cluster;        // Root directory first cluster
    uint16_t fs_info;             // File system info sector
    uint16_t backup_boot_sector;  // Backup boot sector
    uint8_t  reserved[12];        // Reserved
    uint8_t  drive_number;        // Drive number
    uint8_t  reserved1;           // Reserved
    uint8_t  boot_signature;      // Boot signature
    uint32_t volume_id;           // Volume ID
    uint8_t  volume_label[11];    // Volume label
    uint8_t  fs_type[8];          // File system type
} __attribute__((packed)) fat32_boot_sector_t;

// FAT32 Directory Entry Structure
typedef struct {
    uint8_t  name[11];            // File name (8.3 format)
    uint8_t  attributes;          // File attributes
    uint8_t  reserved1;           // Reserved
    uint8_t  create_time_ms;      // Creation time (10ms units)
    uint16_t create_time;         // Creation time
    uint16_t create_date;         // Creation date
    uint16_t access_date;         // Last access date
    uint16_t cluster_high;        // High 16 bits of first cluster
    uint16_t write_time;          // Last write time
    uint16_t write_date;          // Last write date
    uint16_t cluster_low;         // Low 16 bits of first cluster
    uint32_t file_size;           // File size in bytes
} __attribute__((packed)) fat32_dir_entry_t;

// FAT32 Long File Name Entry Structure
typedef struct {
    uint8_t  sequence;            // Sequence number
    uint16_t name1[5];            // Characters 1-5
    uint8_t  attributes;          // Attributes (must be 0x0F)
    uint8_t  type;                // Type (must be 0x00)
    uint8_t  checksum;            // Checksum of short name
    uint16_t name2[6];            // Characters 6-11
    uint16_t cluster_low;         // Low 16 bits of first cluster (must be 0x0000)
    uint16_t name3[2];            // Characters 12-13
} __attribute__((packed)) fat32_lfn_entry_t;

// FAT32 File Attributes
#define FAT32_ATTR_READ_ONLY  0x01
#define FAT32_ATTR_HIDDEN     0x02
#define FAT32_ATTR_SYSTEM     0x04
#define FAT32_ATTR_VOLUME_ID  0x08
#define FAT32_ATTR_DIRECTORY  0x10
#define FAT32_ATTR_ARCHIVE    0x20
#define FAT32_ATTR_LONG_NAME  0x0F

// FAT32 Cluster Values
#define FAT32_CLUSTER_FREE    0x00000000
#define FAT32_CLUSTER_BAD     0x0FFFFFF7
#define FAT32_CLUSTER_END     0x0FFFFFFF
#define FAT32_CLUSTER_MASK    0x0FFFFFFF

// FAT32 File Handle Structure
typedef struct {
    uint32_t first_cluster;       // First cluster
    uint32_t current_cluster;     // Current cluster
    uint32_t cluster_offset;      // Offset within cluster
    uint32_t file_offset;         // File offset
    uint32_t file_size;           // File size
    uint8_t  attributes;          // File attributes
    uint8_t  is_directory;        // Is directory
    char     name[256];           // File name
} fat32_file_handle_t;

// FAT32 Filesystem Structure
typedef struct {
    uint32_t sector_size;         // Sector size
    uint32_t cluster_size;        // Cluster size in bytes
    uint32_t fat_start_sector;    // FAT start sector
    uint32_t fat_size;            // FAT size in sectors
    uint32_t root_cluster;        // Root directory cluster
    uint32_t data_start_sector;   // Data area start sector
    uint32_t total_clusters;      // Total number of clusters
    uint8_t* fat_cache;           // FAT cache
    uint32_t fat_cache_dirty;     // FAT cache dirty flag
    block_device_t* device;       // Block device
} fat32_fs_t;

// Global FAT32 filesystem
static fat32_fs_t fat32_fs;

// Function prototypes
static int fat32_read_sectors(uint32_t sector, uint32_t count, void* buffer);
static int fat32_write_sectors(uint32_t sector, uint32_t count, const void* buffer);
static uint32_t fat32_get_cluster_sector(uint32_t cluster);
static uint32_t fat32_get_next_cluster(uint32_t cluster);
static int fat32_set_next_cluster(uint32_t cluster, uint32_t next);
static uint32_t fat32_allocate_cluster();
static int fat32_free_cluster_chain(uint32_t first_cluster);
static int fat32_read_cluster(uint32_t cluster, void* buffer);
static int fat32_write_cluster(uint32_t cluster, const void* buffer);
static int fat32_find_free_cluster();
static int fat32_parse_directory_entry(fat32_dir_entry_t* entry, char* name, uint32_t* cluster, uint32_t* size, uint8_t* attributes);
static int fat32_find_file_in_directory(uint32_t dir_cluster, const char* name, fat32_dir_entry_t* entry);
static int fat32_create_directory_entry(uint32_t dir_cluster, const char* name, uint32_t cluster, uint32_t size, uint8_t attributes);
static int fat32_delete_directory_entry(uint32_t dir_cluster, const char* name);
static int fat32_expand_file(fat32_file_handle_t* handle, uint32_t new_size);

// Initialize FAT32 filesystem
int fat32_init(block_device_t* device) {
    printf("FAT32: Initializing FAT32 filesystem...\n");
    
    fat32_fs.device = device;
    
    // Read boot sector
    fat32_boot_sector_t boot_sector;
    if (fat32_read_sectors(0, 1, &boot_sector) != 1) {
        printf("FAT32: Failed to read boot sector\n");
        return -1;
    }
    
    // Verify FAT32 signature
    if (boot_sector.fs_type[0] != 'F' || boot_sector.fs_type[1] != 'A' ||
        boot_sector.fs_type[2] != 'T' || boot_sector.fs_type[3] != '3' ||
        boot_sector.fs_type[4] != '2' || boot_sector.fs_type[5] != ' ' ||
        boot_sector.fs_type[6] != ' ' || boot_sector.fs_type[7] != ' ') {
        printf("FAT32: Not a FAT32 filesystem\n");
        return -1;
    }
    
    // Parse filesystem parameters
    fat32_fs.sector_size = boot_sector.bytes_per_sector;
    fat32_fs.cluster_size = boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;
    fat32_fs.fat_start_sector = boot_sector.reserved_sectors;
    fat32_fs.fat_size = boot_sector.fat_size_32;
    fat32_fs.root_cluster = boot_sector.root_cluster;
    fat32_fs.data_start_sector = boot_sector.reserved_sectors + (boot_sector.num_fats * boot_sector.fat_size_32);
    fat32_fs.total_clusters = boot_sector.total_sectors_32 - fat32_fs.data_start_sector;
    
    printf("FAT32: Sector size: %d, Cluster size: %d, Total clusters: %d\n",
           fat32_fs.sector_size, fat32_fs.cluster_size, fat32_fs.total_clusters);
    
    // Allocate FAT cache
    fat32_fs.fat_cache = kmalloc(fat32_fs.fat_size * fat32_fs.sector_size);
    if (!fat32_fs.fat_cache) {
        printf("FAT32: Failed to allocate FAT cache\n");
        return -1;
    }
    
    // Read FAT into cache
    if (fat32_read_sectors(fat32_fs.fat_start_sector, fat32_fs.fat_size, fat32_fs.fat_cache) != fat32_fs.fat_size) {
        printf("FAT32: Failed to read FAT\n");
        kfree(fat32_fs.fat_cache);
        return -1;
    }
    
    fat32_fs.fat_cache_dirty = 0;
    
    printf("FAT32: Filesystem initialized successfully\n");
    return 0;
}

// Read sectors from block device
static int fat32_read_sectors(uint32_t sector, uint32_t count, void* buffer) {
    return fat32_fs.device->read_sectors(fat32_fs.device, sector, count, buffer);
}

// Write sectors to block device
static int fat32_write_sectors(uint32_t sector, uint32_t count, const void* buffer) {
    return fat32_fs.device->write_sectors(fat32_fs.device, sector, count, buffer);
}

// Get sector number for cluster
static uint32_t fat32_get_cluster_sector(uint32_t cluster) {
    return fat32_fs.data_start_sector + ((cluster - 2) * (fat32_fs.cluster_size / fat32_fs.sector_size));
}

// Get next cluster in chain
static uint32_t fat32_get_next_cluster(uint32_t cluster) {
    if (cluster < 2 || cluster >= fat32_fs.total_clusters + 2) {
        return FAT32_CLUSTER_END;
    }
    
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fat_offset / fat32_fs.sector_size;
    uint32_t fat_index = fat_offset % fat32_fs.sector_size;
    
    uint32_t* fat_entry = (uint32_t*)(fat32_fs.fat_cache + fat_sector * fat32_fs.sector_size + fat_index);
    return *fat_entry & FAT32_CLUSTER_MASK;
}

// Set next cluster in chain
static int fat32_set_next_cluster(uint32_t cluster, uint32_t next) {
    if (cluster < 2 || cluster >= fat32_fs.total_clusters + 2) {
        return -1;
    }
    
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fat_offset / fat32_fs.sector_size;
    uint32_t fat_index = fat_offset % fat32_fs.sector_size;
    
    uint32_t* fat_entry = (uint32_t*)(fat32_fs.fat_cache + fat_sector * fat32_fs.sector_size + fat_index);
    *fat_entry = (*fat_entry & ~FAT32_CLUSTER_MASK) | (next & FAT32_CLUSTER_MASK);
    
    fat32_fs.fat_cache_dirty = 1;
    return 0;
}

// Allocate a new cluster
static uint32_t fat32_allocate_cluster() {
    uint32_t cluster = fat32_find_free_cluster();
    if (cluster == 0) {
        return 0; // No free clusters
    }
    
    fat32_set_next_cluster(cluster, FAT32_CLUSTER_END);
    return cluster;
}

// Free cluster chain
static int fat32_free_cluster_chain(uint32_t first_cluster) {
    uint32_t cluster = first_cluster;
    
    while (cluster != FAT32_CLUSTER_END && cluster != FAT32_CLUSTER_FREE) {
        uint32_t next = fat32_get_next_cluster(cluster);
        fat32_set_next_cluster(cluster, FAT32_CLUSTER_FREE);
        cluster = next;
    }
    
    return 0;
}

// Read cluster data
static int fat32_read_cluster(uint32_t cluster, void* buffer) {
    uint32_t sector = fat32_get_cluster_sector(cluster);
    uint32_t sectors_per_cluster = fat32_fs.cluster_size / fat32_fs.sector_size;
    
    return fat32_read_sectors(sector, sectors_per_cluster, buffer);
}

// Write cluster data
static int fat32_write_cluster(uint32_t cluster, const void* buffer) {
    uint32_t sector = fat32_get_cluster_sector(cluster);
    uint32_t sectors_per_cluster = fat32_fs.cluster_size / fat32_fs.sector_size;
    
    return fat32_write_sectors(sector, sectors_per_cluster, buffer);
}

// Find free cluster
static int fat32_find_free_cluster() {
    for (uint32_t cluster = 2; cluster < fat32_fs.total_clusters + 2; cluster++) {
        if (fat32_get_next_cluster(cluster) == FAT32_CLUSTER_FREE) {
            return cluster;
        }
    }
    return 0; // No free clusters
}

// Parse directory entry
static int fat32_parse_directory_entry(fat32_dir_entry_t* entry, char* name, uint32_t* cluster, uint32_t* size, uint8_t* attributes) {
    // Check if entry is valid
    if (entry->name[0] == 0x00) {
        return -1; // End of directory
    }
    
    if (entry->name[0] == 0xE5) {
        return -1; // Deleted file
    }
    
    if (entry->attributes == FAT32_ATTR_LONG_NAME) {
        return -1; // Long name entry (handled separately)
    }
    
    // Extract file name
    int name_len = 0;
    for (int i = 0; i < 8; i++) {
        if (entry->name[i] != ' ') {
            name[name_len++] = entry->name[i];
        }
    }
    
    if (entry->name[8] != ' ') {
        name[name_len++] = '.';
        for (int i = 8; i < 11; i++) {
            if (entry->name[i] != ' ') {
                name[name_len++] = entry->name[i];
            }
        }
    }
    
    name[name_len] = '\0';
    
    // Extract other information
    *cluster = (entry->cluster_high << 16) | entry->cluster_low;
    *size = entry->file_size;
    *attributes = entry->attributes;
    
    return 0;
}

// Find file in directory
static int fat32_find_file_in_directory(uint32_t dir_cluster, const char* name, fat32_dir_entry_t* entry) {
    uint8_t* cluster_buffer = kmalloc(fat32_fs.cluster_size);
    if (!cluster_buffer) {
        return -1;
    }
    
    uint32_t cluster = dir_cluster;
    
    while (cluster != FAT32_CLUSTER_END && cluster != FAT32_CLUSTER_FREE) {
        if (fat32_read_cluster(cluster, cluster_buffer) != 0) {
            kfree(cluster_buffer);
            return -1;
        }
        
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)cluster_buffer;
        int entries_per_cluster = fat32_fs.cluster_size / sizeof(fat32_dir_entry_t);
        
        for (int i = 0; i < entries_per_cluster; i++) {
            char entry_name[256];
            uint32_t entry_cluster, entry_size;
            uint8_t entry_attributes;
            
            if (fat32_parse_directory_entry(&entries[i], entry_name, &entry_cluster, &entry_size, &entry_attributes) == 0) {
                if (strcmp(entry_name, name) == 0) {
                    *entry = entries[i];
                    kfree(cluster_buffer);
                    return 0;
                }
            }
        }
        
        cluster = fat32_get_next_cluster(cluster);
    }
    
    kfree(cluster_buffer);
    return -1;
}

// Create directory entry
static int fat32_create_directory_entry(uint32_t dir_cluster, const char* name, uint32_t cluster, uint32_t size, uint8_t attributes) {
    uint8_t* cluster_buffer = kmalloc(fat32_fs.cluster_size);
    if (!cluster_buffer) {
        return -1;
    }
    
    // Find free entry in directory
    uint32_t current_cluster = dir_cluster;
    fat32_dir_entry_t* free_entry = NULL;
    
    while (current_cluster != FAT32_CLUSTER_END && current_cluster != FAT32_CLUSTER_FREE) {
        if (fat32_read_cluster(current_cluster, cluster_buffer) != 0) {
            kfree(cluster_buffer);
            return -1;
        }
        
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)cluster_buffer;
        int entries_per_cluster = fat32_fs.cluster_size / sizeof(fat32_dir_entry_t);
        
        for (int i = 0; i < entries_per_cluster; i++) {
            if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
                free_entry = &entries[i];
                break;
            }
        }
        
        if (free_entry) {
            break;
        }
        
        current_cluster = fat32_get_next_cluster(current_cluster);
    }
    
    if (!free_entry) {
        // Need to allocate new cluster for directory
        uint32_t new_cluster = fat32_allocate_cluster();
        if (new_cluster == 0) {
            kfree(cluster_buffer);
            return -1;
        }
        
        fat32_set_next_cluster(current_cluster, new_cluster);
        fat32_set_next_cluster(new_cluster, FAT32_CLUSTER_END);
        
        memset(cluster_buffer, 0, fat32_fs.cluster_size);
        free_entry = (fat32_dir_entry_t*)cluster_buffer;
    }
    
    // Fill directory entry
    memset(free_entry, 0, sizeof(fat32_dir_entry_t));
    
    // Set file name (8.3 format)
    char base_name[9], ext[4];
    char* dot = strchr(name, '.');
    
    if (dot) {
        int base_len = dot - name;
        if (base_len > 8) base_len = 8;
        strncpy(base_name, name, base_len);
        base_name[base_len] = '\0';
        
        strncpy(ext, dot + 1, 3);
        ext[3] = '\0';
    } else {
        strncpy(base_name, name, 8);
        base_name[8] = '\0';
        ext[0] = '\0';
    }
    
    // Pad with spaces
    for (int i = strlen(base_name); i < 8; i++) {
        base_name[i] = ' ';
    }
    for (int i = strlen(ext); i < 3; i++) {
        ext[i] = ' ';
    }
    
    memcpy(free_entry->name, base_name, 8);
    memcpy(free_entry->name + 8, ext, 3);
    
    free_entry->attributes = attributes;
    free_entry->cluster_high = (cluster >> 16) & 0xFFFF;
    free_entry->cluster_low = cluster & 0xFFFF;
    free_entry->file_size = size;
    
    // Write cluster back
    fat32_write_cluster(current_cluster, cluster_buffer);
    
    kfree(cluster_buffer);
    return 0;
}

// Delete directory entry
static int fat32_delete_directory_entry(uint32_t dir_cluster, const char* name) {
    uint8_t* cluster_buffer = kmalloc(fat32_fs.cluster_size);
    if (!cluster_buffer) {
        return -1;
    }
    
    uint32_t cluster = dir_cluster;
    
    while (cluster != FAT32_CLUSTER_END && cluster != FAT32_CLUSTER_FREE) {
        if (fat32_read_cluster(cluster, cluster_buffer) != 0) {
            kfree(cluster_buffer);
            return -1;
        }
        
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)cluster_buffer;
        int entries_per_cluster = fat32_fs.cluster_size / sizeof(fat32_dir_entry_t);
        
        for (int i = 0; i < entries_per_cluster; i++) {
            char entry_name[256];
            uint32_t entry_cluster, entry_size;
            uint8_t entry_attributes;
            
            if (fat32_parse_directory_entry(&entries[i], entry_name, &entry_cluster, &entry_size, &entry_attributes) == 0) {
                if (strcmp(entry_name, name) == 0) {
                    // Mark as deleted
                    entries[i].name[0] = 0xE5;
                    
                    // Free cluster chain if it's a file
                    if (!(entry_attributes & FAT32_ATTR_DIRECTORY)) {
                        fat32_free_cluster_chain(entry_cluster);
                    }
                    
                    fat32_write_cluster(cluster, cluster_buffer);
                    kfree(cluster_buffer);
                    return 0;
                }
            }
        }
        
        cluster = fat32_get_next_cluster(cluster);
    }
    
    kfree(cluster_buffer);
    return -1;
}

// Expand file size
static int fat32_expand_file(fat32_file_handle_t* handle, uint32_t new_size) {
    uint32_t current_clusters = (handle->file_size + fat32_fs.cluster_size - 1) / fat32_fs.cluster_size;
    uint32_t needed_clusters = (new_size + fat32_fs.cluster_size - 1) / fat32_fs.cluster_size;
    
    if (needed_clusters <= current_clusters) {
        handle->file_size = new_size;
        return 0;
    }
    
    // Find last cluster
    uint32_t last_cluster = handle->first_cluster;
    if (last_cluster == 0) {
        last_cluster = fat32_allocate_cluster();
        if (last_cluster == 0) {
            return -1;
        }
        handle->first_cluster = last_cluster;
    } else {
        while (fat32_get_next_cluster(last_cluster) != FAT32_CLUSTER_END) {
            last_cluster = fat32_get_next_cluster(last_cluster);
        }
    }
    
    // Allocate additional clusters
    for (uint32_t i = current_clusters; i < needed_clusters; i++) {
        uint32_t new_cluster = fat32_allocate_cluster();
        if (new_cluster == 0) {
            return -1;
        }
        
        fat32_set_next_cluster(last_cluster, new_cluster);
        last_cluster = new_cluster;
    }
    
    fat32_set_next_cluster(last_cluster, FAT32_CLUSTER_END);
    handle->file_size = new_size;
    
    return 0;
}

// FAT32 file operations
int fat32_open(const char* path, int flags, fat32_file_handle_t* handle) {
    // Parse path
    char* path_copy = strdup(path);
    char* last_slash = strrchr(path_copy, '/');
    
    if (last_slash) {
        *last_slash = '\0';
        char* dir_path = path_copy;
        char* filename = last_slash + 1;
        
        // Find directory
        uint32_t dir_cluster = fat32_fs.root_cluster;
        if (strlen(dir_path) > 0) {
            // TODO: Navigate to directory
        }
        
        // Find file in directory
        fat32_dir_entry_t entry;
        if (fat32_find_file_in_directory(dir_cluster, filename, &entry) == 0) {
            // File exists
            if (flags & O_CREAT) {
                kfree(path_copy);
                return -1; // File already exists
            }
            
            uint32_t cluster = (entry.cluster_high << 16) | entry.cluster_low;
            handle->first_cluster = cluster;
            handle->current_cluster = cluster;
            handle->cluster_offset = 0;
            handle->file_offset = 0;
            handle->file_size = entry.file_size;
            handle->attributes = entry.attributes;
            handle->is_directory = (entry.attributes & FAT32_ATTR_DIRECTORY) ? 1 : 0;
            strncpy(handle->name, filename, sizeof(handle->name) - 1);
            
            kfree(path_copy);
            return 0;
        } else {
            // File doesn't exist
            if (flags & O_CREAT) {
                // Create new file
                uint32_t cluster = fat32_allocate_cluster();
                if (cluster == 0) {
                    kfree(path_copy);
                    return -1;
                }
                
                if (fat32_create_directory_entry(dir_cluster, filename, cluster, 0, FAT32_ATTR_ARCHIVE) != 0) {
                    fat32_free_cluster_chain(cluster);
                    kfree(path_copy);
                    return -1;
                }
                
                handle->first_cluster = cluster;
                handle->current_cluster = cluster;
                handle->cluster_offset = 0;
                handle->file_offset = 0;
                handle->file_size = 0;
                handle->attributes = FAT32_ATTR_ARCHIVE;
                handle->is_directory = 0;
                strncpy(handle->name, filename, sizeof(handle->name) - 1);
                
                kfree(path_copy);
                return 0;
            }
        }
    }
    
    kfree(path_copy);
    return -1;
}

int fat32_read(fat32_file_handle_t* handle, void* buffer, uint32_t size) {
    if (handle->file_offset >= handle->file_size) {
        return 0; // End of file
    }
    
    uint32_t bytes_read = 0;
    uint8_t* cluster_buffer = kmalloc(fat32_fs.cluster_size);
    
    if (!cluster_buffer) {
        return -1;
    }
    
    while (bytes_read < size && handle->file_offset < handle->file_size) {
        // Calculate cluster and offset
        uint32_t cluster_index = handle->file_offset / fat32_fs.cluster_size;
        uint32_t cluster_offset = handle->file_offset % fat32_fs.cluster_size;
        
        // Find current cluster
        uint32_t cluster = handle->first_cluster;
        for (uint32_t i = 0; i < cluster_index; i++) {
            cluster = fat32_get_next_cluster(cluster);
            if (cluster == FAT32_CLUSTER_END || cluster == FAT32_CLUSTER_FREE) {
                kfree(cluster_buffer);
                return bytes_read;
            }
        }
        
        // Read cluster
        if (fat32_read_cluster(cluster, cluster_buffer) != 0) {
            kfree(cluster_buffer);
            return -1;
        }
        
        // Copy data
        uint32_t bytes_to_copy = fat32_fs.cluster_size - cluster_offset;
        if (bytes_to_copy > size - bytes_read) {
            bytes_to_copy = size - bytes_read;
        }
        if (bytes_to_copy > handle->file_size - handle->file_offset) {
            bytes_to_copy = handle->file_size - handle->file_offset;
        }
        
        memcpy((uint8_t*)buffer + bytes_read, cluster_buffer + cluster_offset, bytes_to_copy);
        
        bytes_read += bytes_to_copy;
        handle->file_offset += bytes_to_copy;
    }
    
    kfree(cluster_buffer);
    return bytes_read;
}

int fat32_write(fat32_file_handle_t* handle, const void* buffer, uint32_t size) {
    uint32_t bytes_written = 0;
    uint8_t* cluster_buffer = kmalloc(fat32_fs.cluster_size);
    
    if (!cluster_buffer) {
        return -1;
    }
    
    while (bytes_written < size) {
        // Calculate cluster and offset
        uint32_t cluster_index = handle->file_offset / fat32_fs.cluster_size;
        uint32_t cluster_offset = handle->file_offset % fat32_fs.cluster_size;
        
        // Find or allocate cluster
        uint32_t cluster = handle->first_cluster;
        if (cluster == 0) {
            cluster = fat32_allocate_cluster();
            if (cluster == 0) {
                kfree(cluster_buffer);
                return -1;
            }
            handle->first_cluster = cluster;
        } else {
            for (uint32_t i = 0; i < cluster_index; i++) {
                uint32_t next = fat32_get_next_cluster(cluster);
                if (next == FAT32_CLUSTER_END) {
                    next = fat32_allocate_cluster();
                    if (next == 0) {
                        kfree(cluster_buffer);
                        return -1;
                    }
                    fat32_set_next_cluster(cluster, next);
                }
                cluster = next;
            }
        }
        
        // Read existing cluster data if needed
        if (cluster_offset > 0) {
            if (fat32_read_cluster(cluster, cluster_buffer) != 0) {
                kfree(cluster_buffer);
                return -1;
            }
        }
        
        // Copy data to cluster buffer
        uint32_t bytes_to_copy = fat32_fs.cluster_size - cluster_offset;
        if (bytes_to_copy > size - bytes_written) {
            bytes_to_copy = size - bytes_written;
        }
        
        memcpy(cluster_buffer + cluster_offset, (uint8_t*)buffer + bytes_written, bytes_to_copy);
        
        // Write cluster
        if (fat32_write_cluster(cluster, cluster_buffer) != 0) {
            kfree(cluster_buffer);
            return -1;
        }
        
        bytes_written += bytes_to_copy;
        handle->file_offset += bytes_to_copy;
        
        if (handle->file_offset > handle->file_size) {
            handle->file_size = handle->file_offset;
        }
    }
    
    kfree(cluster_buffer);
    return bytes_written;
}

int fat32_close(fat32_file_handle_t* handle) {
    // Update file size in directory entry if needed
    // This would require finding and updating the directory entry
    return 0;
}

// Flush FAT cache to disk
int fat32_flush() {
    if (fat32_fs.fat_cache_dirty) {
        if (fat32_write_sectors(fat32_fs.fat_start_sector, fat32_fs.fat_size, fat32_fs.fat_cache) != fat32_fs.fat_size) {
            return -1;
        }
        fat32_fs.fat_cache_dirty = 0;
    }
    return 0;
} 