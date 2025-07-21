#include <kernel/filesystem/ntfs.h>
#include <kernel/filesystem/vfs.h>
#include <kernel/memory/memory.h>
#include <kernel/libc/string.h>
#include <kernel/libc/stdio.h>
#include <kernel/libc/stdlib.h>

// NTFS Boot Sector Structure
typedef struct {
    uint8_t  jump_boot[3];        // Jump instruction
    uint8_t  oem_name[8];         // OEM name
    uint16_t bytes_per_sector;    // Bytes per sector
    uint8_t  sectors_per_cluster; // Sectors per cluster
    uint16_t reserved_sectors;    // Reserved sectors
    uint8_t  num_fats;            // Number of FATs (always 0 for NTFS)
    uint16_t root_entries;        // Root directory entries (always 0 for NTFS)
    uint16_t total_sectors_16;    // Total sectors (always 0 for NTFS)
    uint8_t  media_type;          // Media type
    uint16_t fat_size_16;         // FAT size (always 0 for NTFS)
    uint16_t sectors_per_track;   // Sectors per track
    uint16_t num_heads;           // Number of heads
    uint32_t hidden_sectors;      // Hidden sectors
    uint32_t total_sectors_32;    // Total sectors (always 0 for NTFS)
    uint64_t total_sectors_64;    // Total sectors (64-bit)
    uint64_t mft_cluster;         // MFT first cluster
    uint64_t mft_mirror_cluster;  // MFT mirror first cluster
    uint32_t mft_record_size;     // MFT record size
    uint32_t index_block_size;    // Index block size
    uint64_t volume_serial;       // Volume serial number
    uint32_t checksum;            // Checksum
} __attribute__((packed)) ntfs_boot_sector_t;

// NTFS File Record Header
typedef struct {
    uint32_t magic;               // Magic number ("FILE")
    uint16_t update_sequence_offset; // Update sequence offset
    uint16_t update_sequence_size;   // Update sequence size
    uint64_t log_sequence_number; // Log sequence number
    uint16_t sequence_number;     // Sequence number
    uint16_t hard_link_count;     // Hard link count
    uint16_t first_attribute_offset; // First attribute offset
    uint16_t flags;               // Flags
    uint32_t real_size;           // Real size
    uint32_t allocated_size;      // Allocated size
    uint64_t base_file_record;    // Base file record
    uint16_t next_attribute_id;   // Next attribute ID
    uint16_t record_number;       // Record number
} __attribute__((packed)) ntfs_file_record_header_t;

// NTFS Attribute Header
typedef struct {
    uint32_t attribute_type;      // Attribute type
    uint32_t length;              // Attribute length
    uint8_t  non_resident;        // Non-resident flag
    uint8_t  name_length;         // Name length
    uint16_t name_offset;         // Name offset
    uint16_t flags;               // Flags
    uint16_t attribute_id;        // Attribute ID
} __attribute__((packed)) ntfs_attribute_header_t;

// NTFS Resident Attribute
typedef struct {
    ntfs_attribute_header_t header; // Attribute header
    uint32_t value_length;        // Value length
    uint16_t value_offset;        // Value offset
    uint8_t  indexed_flag;        // Indexed flag
    uint8_t  padding;             // Padding
} __attribute__((packed)) ntfs_resident_attribute_t;

// NTFS Non-Resident Attribute
typedef struct {
    ntfs_attribute_header_t header; // Attribute header
    uint64_t starting_vcn;        // Starting VCN
    uint64_t ending_vcn;          // Ending VCN
    uint16_t runlist_offset;      // Runlist offset
    uint16_t compression_unit;    // Compression unit
    uint32_t padding;             // Padding
    uint64_t allocated_size;      // Allocated size
    uint64_t data_size;           // Data size
    uint64_t initialized_size;    // Initialized size
} __attribute__((packed)) ntfs_non_resident_attribute_t;

// NTFS Data Run
typedef struct {
    uint8_t  header;              // Header byte
    uint64_t length;              // Run length
    uint64_t offset;              // Run offset
} __attribute__((packed)) ntfs_data_run_t;

// NTFS File Name Attribute
typedef struct {
    uint64_t parent_directory;    // Parent directory
    uint64_t creation_time;       // Creation time
    uint64_t last_modified_time;  // Last modified time
    uint64_t mft_modified_time;   // MFT modified time
    uint64_t last_access_time;    // Last access time
    uint64_t allocated_size;      // Allocated size
    uint64_t real_size;           // Real size
    uint32_t flags;               // File flags
    uint32_t reparse_value;       // Reparse value
    uint8_t  name_length;         // Name length
    uint8_t  name_type;           // Name type
    uint16_t name[1];             // File name (Unicode)
} __attribute__((packed)) ntfs_file_name_attribute_t;

// NTFS Standard Information Attribute
typedef struct {
    uint64_t creation_time;       // Creation time
    uint64_t last_modified_time;  // Last modified time
    uint64_t mft_modified_time;   // MFT modified time
    uint64_t last_access_time;    // Last access time
    uint32_t file_attributes;     // File attributes
    uint32_t max_versions;        // Maximum versions
    uint32_t version_number;      // Version number
    uint32_t class_id;            // Class ID
    uint32_t owner_id;            // Owner ID
    uint32_t security_id;         // Security ID
    uint64_t quota_charged;       // Quota charged
    uint64_t usn;                 // Update sequence number
} __attribute__((packed)) ntfs_standard_information_t;

// NTFS Attribute Types
#define NTFS_ATTR_STANDARD_INFORMATION 0x10
#define NTFS_ATTR_ATTRIBUTE_LIST      0x20
#define NTFS_ATTR_FILE_NAME           0x30
#define NTFS_ATTR_OBJECT_ID           0x40
#define NTFS_ATTR_SECURITY_DESCRIPTOR 0x50
#define NTFS_ATTR_VOLUME_NAME         0x60
#define NTFS_ATTR_VOLUME_INFORMATION  0x70
#define NTFS_ATTR_DATA                0x80
#define NTFS_ATTR_INDEX_ROOT          0x90
#define NTFS_ATTR_INDEX_ALLOCATION    0xA0
#define NTFS_ATTR_BITMAP              0xB0
#define NTFS_ATTR_REPARSE_POINT       0xC0
#define NTFS_ATTR_EA_INFORMATION      0xD0
#define NTFS_ATTR_EA                  0xE0
#define NTFS_ATTR_PROPERTY_SET        0xF0
#define NTFS_ATTR_LOGGED_UTILITY_STREAM 0x100

// NTFS File Flags
#define NTFS_FILE_READ_ONLY           0x00000001
#define NTFS_FILE_HIDDEN              0x00000002
#define NTFS_FILE_SYSTEM              0x00000004
#define NTFS_FILE_ARCHIVE             0x00000020
#define NTFS_FILE_DEVICE              0x00000040
#define NTFS_FILE_NORMAL              0x00000080
#define NTFS_FILE_TEMPORARY           0x00000100
#define NTFS_FILE_SPARSE              0x00000200
#define NTFS_FILE_REPARSE_POINT       0x00000400
#define NTFS_FILE_COMPRESSED          0x00000800
#define NTFS_FILE_OFFLINE             0x00001000
#define NTFS_FILE_NOT_CONTENT_INDEXED 0x00002000
#define NTFS_FILE_ENCRYPTED           0x00004000
#define NTFS_FILE_DIRECTORY           0x10000000

// NTFS File Handle Structure
typedef struct {
    uint64_t mft_record;          // MFT record number
    uint64_t file_size;           // File size
    uint64_t file_offset;         // Current file offset
    uint32_t attributes;          // File attributes
    uint8_t  is_directory;        // Is directory
    char     name[256];           // File name
    uint64_t parent_directory;    // Parent directory MFT record
} ntfs_file_handle_t;

// NTFS Filesystem Structure
typedef struct {
    uint32_t sector_size;         // Sector size
    uint32_t cluster_size;        // Cluster size in bytes
    uint64_t total_sectors;       // Total sectors
    uint64_t mft_cluster;         // MFT first cluster
    uint64_t mft_mirror_cluster;  // MFT mirror first cluster
    uint32_t mft_record_size;     // MFT record size
    uint32_t index_block_size;    // Index block size
    uint64_t volume_serial;       // Volume serial number
    uint8_t* mft_cache;           // MFT cache
    uint32_t mft_cache_size;      // MFT cache size
    uint32_t mft_cache_dirty;     // MFT cache dirty flag
    block_device_t* device;       // Block device
} ntfs_fs_t;

// Global NTFS filesystem
static ntfs_fs_t ntfs_fs;

// Function prototypes
static int ntfs_read_sectors(uint64_t sector, uint32_t count, void* buffer);
static int ntfs_write_sectors(uint64_t sector, uint32_t count, const void* buffer);
static uint64_t ntfs_cluster_to_sector(uint64_t cluster);
static uint64_t ntfs_sector_to_cluster(uint64_t sector);
static int ntfs_read_mft_record(uint64_t record_number, void* buffer);
static int ntfs_write_mft_record(uint64_t record_number, const void* buffer);
static int ntfs_find_attribute(void* record, uint32_t attribute_type, void** attribute);
static int ntfs_read_data_runs(ntfs_non_resident_attribute_t* attr, uint64_t vcn, uint32_t count, void* buffer);
static int ntfs_write_data_runs(ntfs_non_resident_attribute_t* attr, uint64_t vcn, uint32_t count, const void* buffer);
static int ntfs_allocate_clusters(uint64_t start_vcn, uint32_t count, uint64_t* clusters);
static int ntfs_free_clusters(uint64_t start_vcn, uint32_t count);
static int ntfs_find_file_in_directory(uint64_t dir_record, const char* name, uint64_t* file_record);
static int ntfs_create_file_record(uint64_t parent_record, const char* name, uint32_t attributes, uint64_t* file_record);
static int ntfs_delete_file_record(uint64_t file_record);
static int ntfs_expand_file(uint64_t file_record, uint64_t new_size);

// Initialize NTFS filesystem
int ntfs_init(block_device_t* device) {
    printf("NTFS: Initializing NTFS filesystem...\n");
    
    ntfs_fs.device = device;
    
    // Read boot sector
    ntfs_boot_sector_t boot_sector;
    if (ntfs_read_sectors(0, 1, &boot_sector) != 1) {
        printf("NTFS: Failed to read boot sector\n");
        return -1;
    }
    
    // Verify NTFS signature
    if (boot_sector.oem_name[0] != 'N' || boot_sector.oem_name[1] != 'T' ||
        boot_sector.oem_name[2] != 'F' || boot_sector.oem_name[3] != 'S' ||
        boot_sector.oem_name[4] != ' ' || boot_sector.oem_name[5] != ' ' ||
        boot_sector.oem_name[6] != ' ' || boot_sector.oem_name[7] != ' ') {
        printf("NTFS: Not an NTFS filesystem\n");
        return -1;
    }
    
    // Parse filesystem parameters
    ntfs_fs.sector_size = boot_sector.bytes_per_sector;
    ntfs_fs.cluster_size = boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;
    ntfs_fs.total_sectors = boot_sector.total_sectors_64;
    ntfs_fs.mft_cluster = boot_sector.mft_cluster;
    ntfs_fs.mft_mirror_cluster = boot_sector.mft_mirror_cluster;
    ntfs_fs.mft_record_size = boot_sector.mft_record_size;
    ntfs_fs.index_block_size = boot_sector.index_block_size;
    ntfs_fs.volume_serial = boot_sector.volume_serial;
    
    printf("NTFS: Sector size: %d, Cluster size: %d, Total sectors: %llu\n",
           ntfs_fs.sector_size, ntfs_fs.cluster_size, ntfs_fs.total_sectors);
    
    // Allocate MFT cache
    ntfs_fs.mft_cache_size = ntfs_fs.mft_record_size * 1024; // Cache 1024 records
    ntfs_fs.mft_cache = kmalloc(ntfs_fs.mft_cache_size);
    if (!ntfs_fs.mft_cache) {
        printf("NTFS: Failed to allocate MFT cache\n");
        return -1;
    }
    
    ntfs_fs.mft_cache_dirty = 0;
    
    printf("NTFS: Filesystem initialized successfully\n");
    return 0;
}

// Read sectors from block device
static int ntfs_read_sectors(uint64_t sector, uint32_t count, void* buffer) {
    return ntfs_fs.device->read_sectors(ntfs_fs.device, sector, count, buffer);
}

// Write sectors to block device
static int ntfs_write_sectors(uint64_t sector, uint32_t count, const void* buffer) {
    return ntfs_fs.device->write_sectors(ntfs_fs.device, sector, count, buffer);
}

// Convert cluster to sector
static uint64_t ntfs_cluster_to_sector(uint64_t cluster) {
    return cluster * (ntfs_fs.cluster_size / ntfs_fs.sector_size);
}

// Convert sector to cluster
static uint64_t ntfs_sector_to_cluster(uint64_t sector) {
    return sector / (ntfs_fs.cluster_size / ntfs_fs.sector_size);
}

// Read MFT record
static int ntfs_read_mft_record(uint64_t record_number, void* buffer) {
    uint64_t mft_sector = ntfs_cluster_to_sector(ntfs_fs.mft_cluster) + (record_number * ntfs_fs.mft_record_size / ntfs_fs.sector_size);
    uint32_t sectors_per_record = ntfs_fs.mft_record_size / ntfs_fs.sector_size;
    
    return ntfs_read_sectors(mft_sector, sectors_per_record, buffer);
}

// Write MFT record
static int ntfs_write_mft_record(uint64_t record_number, const void* buffer) {
    uint64_t mft_sector = ntfs_cluster_to_sector(ntfs_fs.mft_cluster) + (record_number * ntfs_fs.mft_record_size / ntfs_fs.sector_size);
    uint32_t sectors_per_record = ntfs_fs.mft_record_size / ntfs_fs.sector_size;
    
    return ntfs_write_sectors(mft_sector, sectors_per_record, buffer);
}

// Find attribute in MFT record
static int ntfs_find_attribute(void* record, uint32_t attribute_type, void** attribute) {
    ntfs_file_record_header_t* header = (ntfs_file_record_header_t*)record;
    
    if (header->magic != 0x454C4946) { // "FILE"
        return -1;
    }
    
    uint8_t* current = (uint8_t*)record + header->first_attribute_offset;
    
    while (current < (uint8_t*)record + header->real_size) {
        ntfs_attribute_header_t* attr_header = (ntfs_attribute_header_t*)current;
        
        if (attr_header->attribute_type == 0xFFFFFFFF) {
            break; // End of attributes
        }
        
        if (attr_header->attribute_type == attribute_type) {
            *attribute = current;
            return 0;
        }
        
        current += attr_header->length;
    }
    
    return -1;
}

// Read data runs
static int ntfs_read_data_runs(ntfs_non_resident_attribute_t* attr, uint64_t vcn, uint32_t count, void* buffer) {
    uint8_t* runlist = (uint8_t*)attr + attr->runlist_offset;
    uint64_t current_vcn = 0;
    uint64_t current_lcn = 0;
    
    // Find the run containing the requested VCN
    while (current_vcn <= vcn) {
        uint8_t header = *runlist++;
        
        if (header == 0) {
            return -1; // End of runlist
        }
        
        uint8_t length_size = header & 0x0F;
        uint8_t offset_size = (header >> 4) & 0x0F;
        
        uint64_t length = 0;
        uint64_t offset = 0;
        
        // Read length
        for (int i = 0; i < length_size; i++) {
            length |= ((uint64_t)*runlist++) << (i * 8);
        }
        
        // Read offset
        for (int i = 0; i < offset_size; i++) {
            offset |= ((uint64_t)*runlist++) << (i * 8);
        }
        
        if (current_vcn + length > vcn) {
            // This run contains the requested data
            uint64_t run_offset = vcn - current_vcn;
            uint64_t lcn = current_lcn + offset + run_offset;
            uint32_t clusters_to_read = count;
            
            if (run_offset + count > length) {
                clusters_to_read = length - run_offset;
            }
            
            uint64_t sector = ntfs_cluster_to_sector(lcn);
            uint32_t sectors = clusters_to_read * (ntfs_fs.cluster_size / ntfs_fs.sector_size);
            
            return ntfs_read_sectors(sector, sectors, buffer);
        }
        
        current_vcn += length;
        current_lcn += offset;
    }
    
    return -1;
}

// Write data runs
static int ntfs_write_data_runs(ntfs_non_resident_attribute_t* attr, uint64_t vcn, uint32_t count, const void* buffer) {
    uint8_t* runlist = (uint8_t*)attr + attr->runlist_offset;
    uint64_t current_vcn = 0;
    uint64_t current_lcn = 0;
    
    // Find the run containing the requested VCN
    while (current_vcn <= vcn) {
        uint8_t header = *runlist++;
        
        if (header == 0) {
            return -1; // End of runlist
        }
        
        uint8_t length_size = header & 0x0F;
        uint8_t offset_size = (header >> 4) & 0x0F;
        
        uint64_t length = 0;
        uint64_t offset = 0;
        
        // Read length
        for (int i = 0; i < length_size; i++) {
            length |= ((uint64_t)*runlist++) << (i * 8);
        }
        
        // Read offset
        for (int i = 0; i < offset_size; i++) {
            offset |= ((uint64_t)*runlist++) << (i * 8);
        }
        
        if (current_vcn + length > vcn) {
            // This run contains the requested data
            uint64_t run_offset = vcn - current_vcn;
            uint64_t lcn = current_lcn + offset + run_offset;
            uint32_t clusters_to_write = count;
            
            if (run_offset + count > length) {
                clusters_to_write = length - run_offset;
            }
            
            uint64_t sector = ntfs_cluster_to_sector(lcn);
            uint32_t sectors = clusters_to_write * (ntfs_fs.cluster_size / ntfs_fs.sector_size);
            
            return ntfs_write_sectors(sector, sectors, buffer);
        }
        
        current_vcn += length;
        current_lcn += offset;
    }
    
    return -1;
}

// Allocate clusters
static int ntfs_allocate_clusters(uint64_t start_vcn, uint32_t count, uint64_t* clusters) {
    // This is a simplified implementation
    // Real NTFS would need to manage the bitmap and handle fragmentation
    static uint64_t next_cluster = 0x1000; // Start after system files
    
    for (uint32_t i = 0; i < count; i++) {
        clusters[i] = next_cluster++;
    }
    
    return 0;
}

// Free clusters
static int ntfs_free_clusters(uint64_t start_vcn, uint32_t count) {
    // This is a simplified implementation
    // Real NTFS would need to update the bitmap
    return 0;
}

// Find file in directory
static int ntfs_find_file_in_directory(uint64_t dir_record, const char* name, uint64_t* file_record) {
    uint8_t* record_buffer = kmalloc(ntfs_fs.mft_record_size);
    if (!record_buffer) {
        return -1;
    }
    
    if (ntfs_read_mft_record(dir_record, record_buffer) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    // Find index root attribute
    void* index_root_attr;
    if (ntfs_find_attribute(record_buffer, NTFS_ATTR_INDEX_ROOT, &index_root_attr) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    // Parse index root to find file
    // This is a simplified implementation
    // Real NTFS would need to parse the B+ tree structure
    
    kfree(record_buffer);
    return -1;
}

// Create file record
static int ntfs_create_file_record(uint64_t parent_record, const char* name, uint32_t attributes, uint64_t* file_record) {
    // Find free MFT record
    static uint64_t next_mft_record = 0x20; // Start after system files
    *file_record = next_mft_record++;
    
    // Allocate record buffer
    uint8_t* record_buffer = kmalloc(ntfs_fs.mft_record_size);
    if (!record_buffer) {
        return -1;
    }
    
    memset(record_buffer, 0, ntfs_fs.mft_record_size);
    
    // Initialize file record header
    ntfs_file_record_header_t* header = (ntfs_file_record_header_t*)record_buffer;
    header->magic = 0x454C4946; // "FILE"
    header->sequence_number = 1;
    header->hard_link_count = 1;
    header->first_attribute_offset = sizeof(ntfs_file_record_header_t);
    header->flags = 0x0001; // In use
    header->real_size = sizeof(ntfs_file_record_header_t);
    header->allocated_size = ntfs_fs.mft_record_size;
    header->next_attribute_id = 1;
    header->record_number = *file_record;
    
    // Add standard information attribute
    ntfs_resident_attribute_t* std_info = (ntfs_resident_attribute_t*)(record_buffer + header->real_size);
    std_info->header.attribute_type = NTFS_ATTR_STANDARD_INFORMATION;
    std_info->header.length = sizeof(ntfs_resident_attribute_t) + sizeof(ntfs_standard_information_t);
    std_info->header.non_resident = 0;
    std_info->header.name_length = 0;
    std_info->header.name_offset = 0;
    std_info->header.flags = 0;
    std_info->header.attribute_id = 1;
    std_info->value_length = sizeof(ntfs_standard_information_t);
    std_info->value_offset = sizeof(ntfs_resident_attribute_t);
    std_info->indexed_flag = 0;
    std_info->padding = 0;
    
    ntfs_standard_information_t* std_info_data = (ntfs_standard_information_t*)((uint8_t*)std_info + std_info->value_offset);
    std_info_data->file_attributes = attributes;
    std_info_data->creation_time = 0; // TODO: Get current time
    std_info_data->last_modified_time = 0;
    std_info_data->mft_modified_time = 0;
    std_info_data->last_access_time = 0;
    
    header->real_size += std_info->header.length;
    
    // Add file name attribute
    ntfs_resident_attribute_t* file_name = (ntfs_resident_attribute_t*)(record_buffer + header->real_size);
    file_name->header.attribute_type = NTFS_ATTR_FILE_NAME;
    file_name->header.length = sizeof(ntfs_resident_attribute_t) + sizeof(ntfs_file_name_attribute_t) + strlen(name) * 2;
    file_name->header.non_resident = 0;
    file_name->header.name_length = 0;
    file_name->header.name_offset = 0;
    file_name->header.flags = 0;
    file_name->header.attribute_id = 2;
    file_name->value_length = sizeof(ntfs_file_name_attribute_t) + strlen(name) * 2;
    file_name->value_offset = sizeof(ntfs_resident_attribute_t);
    file_name->indexed_flag = 0;
    file_name->padding = 0;
    
    ntfs_file_name_attribute_t* file_name_data = (ntfs_file_name_attribute_t*)((uint8_t*)file_name + file_name->value_offset);
    file_name_data->parent_directory = parent_record;
    file_name_data->flags = attributes;
    file_name_data->name_length = strlen(name);
    file_name_data->name_type = 3; // Win32 namespace
    
    // Convert name to Unicode
    for (int i = 0; i < strlen(name); i++) {
        file_name_data->name[i] = name[i];
    }
    
    header->real_size += file_name->header.length;
    
    // Add data attribute if not directory
    if (!(attributes & NTFS_FILE_DIRECTORY)) {
        ntfs_non_resident_attribute_t* data = (ntfs_non_resident_attribute_t*)(record_buffer + header->real_size);
        data->header.attribute_type = NTFS_ATTR_DATA;
        data->header.length = sizeof(ntfs_non_resident_attribute_t);
        data->header.non_resident = 1;
        data->header.name_length = 0;
        data->header.name_offset = 0;
        data->header.flags = 0;
        data->header.attribute_id = 3;
        data->starting_vcn = 0;
        data->ending_vcn = 0;
        data->runlist_offset = sizeof(ntfs_non_resident_attribute_t);
        data->compression_unit = 0;
        data->padding = 0;
        data->allocated_size = 0;
        data->data_size = 0;
        data->initialized_size = 0;
        
        header->real_size += data->header.length;
    }
    
    // Write record
    if (ntfs_write_mft_record(*file_record, record_buffer) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    kfree(record_buffer);
    return 0;
}

// Delete file record
static int ntfs_delete_file_record(uint64_t file_record) {
    uint8_t* record_buffer = kmalloc(ntfs_fs.mft_record_size);
    if (!record_buffer) {
        return -1;
    }
    
    if (ntfs_read_mft_record(file_record, record_buffer) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    ntfs_file_record_header_t* header = (ntfs_file_record_header_t*)record_buffer;
    header->flags = 0x0000; // Not in use
    
    if (ntfs_write_mft_record(file_record, record_buffer) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    kfree(record_buffer);
    return 0;
}

// Expand file size
static int ntfs_expand_file(uint64_t file_record, uint64_t new_size) {
    uint8_t* record_buffer = kmalloc(ntfs_fs.mft_record_size);
    if (!record_buffer) {
        return -1;
    }
    
    if (ntfs_read_mft_record(file_record, record_buffer) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    // Find data attribute
    void* data_attr;
    if (ntfs_find_attribute(record_buffer, NTFS_ATTR_DATA, &data_attr) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    ntfs_non_resident_attribute_t* data = (ntfs_non_resident_attribute_t*)data_attr;
    data->data_size = new_size;
    data->initialized_size = new_size;
    
    if (ntfs_write_mft_record(file_record, record_buffer) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    kfree(record_buffer);
    return 0;
}

// NTFS file operations
int ntfs_open(const char* path, int flags, ntfs_file_handle_t* handle) {
    // Parse path
    char* path_copy = strdup(path);
    char* last_slash = strrchr(path_copy, '/');
    
    if (last_slash) {
        *last_slash = '\0';
        char* dir_path = path_copy;
        char* filename = last_slash + 1;
        
        // Find directory
        uint64_t dir_record = 5; // Root directory MFT record
        if (strlen(dir_path) > 0) {
            // TODO: Navigate to directory
        }
        
        // Find file in directory
        uint64_t file_record;
        if (ntfs_find_file_in_directory(dir_record, filename, &file_record) == 0) {
            // File exists
            if (flags & O_CREAT) {
                kfree(path_copy);
                return -1; // File already exists
            }
            
            // Read file record
            uint8_t* record_buffer = kmalloc(ntfs_fs.mft_record_size);
            if (!record_buffer) {
                kfree(path_copy);
                return -1;
            }
            
            if (ntfs_read_mft_record(file_record, record_buffer) != 0) {
                kfree(record_buffer);
                kfree(path_copy);
                return -1;
            }
            
            // Parse file information
            void* std_info_attr;
            if (ntfs_find_attribute(record_buffer, NTFS_ATTR_STANDARD_INFORMATION, &std_info_attr) == 0) {
                ntfs_resident_attribute_t* std_info = (ntfs_resident_attribute_t*)std_info_attr;
                ntfs_standard_information_t* std_info_data = (ntfs_standard_information_t*)((uint8_t*)std_info + std_info->value_offset);
                handle->attributes = std_info_data->file_attributes;
            }
            
            void* file_name_attr;
            if (ntfs_find_attribute(record_buffer, NTFS_ATTR_FILE_NAME, &file_name_attr) == 0) {
                ntfs_resident_attribute_t* file_name = (ntfs_resident_attribute_t*)file_name_attr;
                ntfs_file_name_attribute_t* file_name_data = (ntfs_file_name_attribute_t*)((uint8_t*)file_name + file_name->value_offset);
                handle->parent_directory = file_name_data->parent_directory;
                handle->file_size = file_name_data->real_size;
            }
            
            handle->mft_record = file_record;
            handle->file_offset = 0;
            handle->is_directory = (handle->attributes & NTFS_FILE_DIRECTORY) ? 1 : 0;
            strncpy(handle->name, filename, sizeof(handle->name) - 1);
            
            kfree(record_buffer);
            kfree(path_copy);
            return 0;
        } else {
            // File doesn't exist
            if (flags & O_CREAT) {
                // Create new file
                uint32_t file_attributes = NTFS_FILE_ARCHIVE;
                if (flags & O_DIRECTORY) {
                    file_attributes = NTFS_FILE_DIRECTORY;
                }
                
                if (ntfs_create_file_record(dir_record, filename, file_attributes, &file_record) != 0) {
                    kfree(path_copy);
                    return -1;
                }
                
                handle->mft_record = file_record;
                handle->file_offset = 0;
                handle->file_size = 0;
                handle->attributes = file_attributes;
                handle->is_directory = (file_attributes & NTFS_FILE_DIRECTORY) ? 1 : 0;
                strncpy(handle->name, filename, sizeof(handle->name) - 1);
                
                kfree(path_copy);
                return 0;
            }
        }
    }
    
    kfree(path_copy);
    return -1;
}

int ntfs_read(ntfs_file_handle_t* handle, void* buffer, uint32_t size) {
    if (handle->file_offset >= handle->file_size) {
        return 0; // End of file
    }
    
    uint8_t* record_buffer = kmalloc(ntfs_fs.mft_record_size);
    if (!record_buffer) {
        return -1;
    }
    
    if (ntfs_read_mft_record(handle->mft_record, record_buffer) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    // Find data attribute
    void* data_attr;
    if (ntfs_find_attribute(record_buffer, NTFS_ATTR_DATA, &data_attr) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    ntfs_non_resident_attribute_t* data = (ntfs_non_resident_attribute_t*)data_attr;
    
    // Calculate VCN and read data
    uint64_t vcn = handle->file_offset / ntfs_fs.cluster_size;
    uint32_t clusters_to_read = (size + ntfs_fs.cluster_size - 1) / ntfs_fs.cluster_size;
    
    if (vcn + clusters_to_read > data->ending_vcn - data->starting_vcn + 1) {
        clusters_to_read = data->ending_vcn - data->starting_vcn + 1 - vcn;
    }
    
    int result = ntfs_read_data_runs(data, vcn, clusters_to_read, buffer);
    
    kfree(record_buffer);
    return result;
}

int ntfs_write(ntfs_file_handle_t* handle, const void* buffer, uint32_t size) {
    uint8_t* record_buffer = kmalloc(ntfs_fs.mft_record_size);
    if (!record_buffer) {
        return -1;
    }
    
    if (ntfs_read_mft_record(handle->mft_record, record_buffer) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    // Find data attribute
    void* data_attr;
    if (ntfs_find_attribute(record_buffer, NTFS_ATTR_DATA, &data_attr) != 0) {
        kfree(record_buffer);
        return -1;
    }
    
    ntfs_non_resident_attribute_t* data = (ntfs_non_resident_attribute_t*)data_attr;
    
    // Calculate VCN and write data
    uint64_t vcn = handle->file_offset / ntfs_fs.cluster_size;
    uint32_t clusters_to_write = (size + ntfs_fs.cluster_size - 1) / ntfs_fs.cluster_size;
    
    int result = ntfs_write_data_runs(data, vcn, clusters_to_write, buffer);
    
    if (result > 0) {
        // Update file size
        uint64_t new_size = handle->file_offset + result;
        if (new_size > handle->file_size) {
            handle->file_size = new_size;
            data->data_size = new_size;
            data->initialized_size = new_size;
            ntfs_write_mft_record(handle->mft_record, record_buffer);
        }
    }
    
    kfree(record_buffer);
    return result;
}

int ntfs_close(ntfs_file_handle_t* handle) {
    // Update file timestamps
    uint8_t* record_buffer = kmalloc(ntfs_fs.mft_record_size);
    if (!record_buffer) {
        return -1;
    }
    
    if (ntfs_read_mft_record(handle->mft_record, record_buffer) == 0) {
        void* std_info_attr;
        if (ntfs_find_attribute(record_buffer, NTFS_ATTR_STANDARD_INFORMATION, &std_info_attr) == 0) {
            ntfs_resident_attribute_t* std_info = (ntfs_resident_attribute_t*)std_info_attr;
            ntfs_standard_information_t* std_info_data = (ntfs_standard_information_t*)((uint8_t*)std_info + std_info->value_offset);
            
            // Update timestamps
            std_info_data->last_modified_time = 0; // TODO: Get current time
            std_info_data->mft_modified_time = 0;
            std_info_data->last_access_time = 0;
            
            ntfs_write_mft_record(handle->mft_record, record_buffer);
        }
    }
    
    kfree(record_buffer);
    return 0;
}

// Flush NTFS cache
int ntfs_flush() {
    if (ntfs_fs.mft_cache_dirty) {
        // Write MFT cache to disk
        ntfs_fs.mft_cache_dirty = 0;
    }
    return 0;
} 