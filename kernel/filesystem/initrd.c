#include "kernel.h"
#include "filesystem/include/vfs.h"
#include "memory/memory.h"
#include "hal/hal.h"
#include <string.h>

// Initial RAM disk configuration
#define INITRD_MAX_FILES 64
#define INITRD_MAX_FILE_SIZE (1024 * 1024) // 1MB per file
#define INITRD_TOTAL_SIZE (16 * 1024 * 1024) // 16MB total
#define INITRD_MAGIC 0x44524E49 // "INRD"

// InitRD file header
typedef struct {
    char name[64];
    u32 size;
    u32 offset;
    u32 flags;
    u32 checksum;
} __attribute__((packed)) initrd_file_header_t;

// InitRD header
typedef struct {
    u32 magic;
    u32 version;
    u32 file_count;
    u32 total_size;
    u32 header_size;
} __attribute__((packed)) initrd_header_t;

// InitRD file entry
typedef struct {
    char name[64];
    u32 size;
    u32 offset;
    u32 flags;
    bool active;
} initrd_file_t;

// InitRD state
typedef struct {
    initrd_header_t* header;
    initrd_file_t files[INITRD_MAX_FILES];
    void* data;
    u32 file_count;
    bool initialized;
} initrd_state_t;

// Global InitRD state
static initrd_state_t initrd = {0};
static bool initrd_initialized = false;

// Forward declarations
static error_t initrd_parse_header(void* data, size_t size);
static error_t initrd_load_files(void);
static initrd_file_t* initrd_find_file(const char* name);
static error_t initrd_validate_checksum(initrd_file_t* file);
static u32 initrd_calculate_checksum(const void* data, u32 size);

// Initialize InitRD
error_t initrd_init(void* data, size_t size) {
    KINFO("Initializing InitRD (size: %zu bytes)", size);
    
    if (!data || size == 0) {
        KERROR("Invalid InitRD data");
        return E_INVAL;
    }
    
    // Parse InitRD header
    error_t result = initrd_parse_header(data, size);
    if (result != SUCCESS) {
        KERROR("Failed to parse InitRD header");
        return result;
    }
    
    // Load files
    result = initrd_load_files();
    if (result != SUCCESS) {
        KERROR("Failed to load InitRD files");
        return result;
    }
    
    initrd_initialized = true;
    
    KINFO("InitRD initialized: %u files", initrd.file_count);
    
    // List files
    for (u32 i = 0; i < initrd.file_count; i++) {
        if (initrd.files[i].active) {
            KINFO("  %s: %u bytes", initrd.files[i].name, initrd.files[i].size);
        }
    }
    
    return SUCCESS;
}

// Parse InitRD header
static error_t initrd_parse_header(void* data, size_t size) {
    if (!data || size < sizeof(initrd_header_t)) {
        return E_INVAL;
    }
    
    initrd.header = (initrd_header_t*)data;
    
    // Validate magic number
    if (initrd.header->magic != INITRD_MAGIC) {
        KERROR("Invalid InitRD magic: 0x%x", initrd.header->magic);
        return E_INVAL;
    }
    
    // Validate version
    if (initrd.header->version != 1) {
        KERROR("Unsupported InitRD version: %u", initrd.header->version);
        return E_INVAL;
    }
    
    // Validate file count
    if (initrd.header->file_count > INITRD_MAX_FILES) {
        KERROR("Too many files in InitRD: %u", initrd.header->file_count);
        return E_INVAL;
    }
    
    // Validate total size
    if (initrd.header->total_size > size) {
        KERROR("InitRD size mismatch: header=%u, actual=%zu", 
               initrd.header->total_size, size);
        return E_INVAL;
    }
    
    initrd.data = data;
    initrd.file_count = initrd.header->file_count;
    
    KDEBUG("InitRD header: version=%u, files=%u, size=%u", 
           initrd.header->version, initrd.header->file_count, initrd.header->total_size);
    
    return SUCCESS;
}

// Load InitRD files
static error_t initrd_load_files(void) {
    if (!initrd.header || !initrd.data) {
        return E_INVAL;
    }
    
    // Clear file entries
    memset(initrd.files, 0, sizeof(initrd.files));
    
    // Parse file headers
    initrd_file_header_t* file_headers = (initrd_file_header_t*)((u8*)initrd.data + initrd.header->header_size);
    
    for (u32 i = 0; i < initrd.file_count; i++) {
        initrd_file_header_t* header = &file_headers[i];
        initrd_file_t* file = &initrd.files[i];
        
        // Copy file information
        strncpy(file->name, header->name, sizeof(file->name) - 1);
        file->name[sizeof(file->name) - 1] = '\0';
        file->size = header->size;
        file->offset = header->offset;
        file->flags = header->flags;
        file->active = true;
        
        // Validate file size
        if (file->size > INITRD_MAX_FILE_SIZE) {
            KWARN("File %s too large: %u bytes", file->name, file->size);
            file->active = false;
            continue;
        }
        
        // Validate file offset
        if (file->offset + file->size > initrd.header->total_size) {
            KWARN("File %s extends beyond InitRD", file->name);
            file->active = false;
            continue;
        }
        
        KDEBUG("Loaded file: %s (%u bytes at offset %u)", 
               file->name, file->size, file->offset);
    }
    
    return SUCCESS;
}

// Find file in InitRD
static initrd_file_t* initrd_find_file(const char* name) {
    if (!name || !initrd_initialized) {
        return NULL;
    }
    
    for (u32 i = 0; i < initrd.file_count; i++) {
        if (initrd.files[i].active && strcmp(initrd.files[i].name, name) == 0) {
            return &initrd.files[i];
        }
    }
    
    return NULL;
}

// Read file from InitRD
error_t initrd_read_file(const char* name, void* buffer, size_t size, size_t* bytes_read) {
    if (!name || !buffer || !bytes_read) {
        return E_INVAL;
    }
    
    if (!initrd_initialized) {
        return E_INVAL;
    }
    
    // Find file
    initrd_file_t* file = initrd_find_file(name);
    if (!file) {
        return E_NOENT;
    }
    
    // Check buffer size
    if (size < file->size) {
        return E_INVAL;
    }
    
    // Calculate file data pointer
    void* file_data = (u8*)initrd.data + file->offset;
    
    // Copy file data
    memcpy(buffer, file_data, file->size);
    *bytes_read = file->size;
    
    KDEBUG("Read file %s: %u bytes", name, file->size);
    
    return SUCCESS;
}

// Get file size
error_t initrd_get_file_size(const char* name, size_t* size) {
    if (!name || !size) {
        return E_INVAL;
    }
    
    if (!initrd_initialized) {
        return E_INVAL;
    }
    
    // Find file
    initrd_file_t* file = initrd_find_file(name);
    if (!file) {
        return E_NOENT;
    }
    
    *size = file->size;
    return SUCCESS;
}

// List files in InitRD
error_t initrd_list_files(char* buffer, size_t buffer_size, size_t* bytes_written) {
    if (!buffer || !bytes_written) {
        return E_INVAL;
    }
    
    if (!initrd_initialized) {
        return E_INVAL;
    }
    
    size_t offset = 0;
    
    for (u32 i = 0; i < initrd.file_count; i++) {
        if (initrd.files[i].active) {
            // Format: "filename size\n"
            int written = snprintf(buffer + offset, buffer_size - offset, 
                                 "%s %u\n", initrd.files[i].name, initrd.files[i].size);
            
            if (written < 0 || (size_t)written >= buffer_size - offset) {
                break;
            }
            
            offset += written;
        }
    }
    
    *bytes_written = offset;
    return SUCCESS;
}

// Check if file exists
bool initrd_file_exists(const char* name) {
    if (!name || !initrd_initialized) {
        return false;
    }
    
    return initrd_find_file(name) != NULL;
}

// Get InitRD statistics
error_t initrd_get_stats(u32* file_count, u32* total_size) {
    if (!initrd_initialized) {
        return E_INVAL;
    }
    
    if (file_count) {
        *file_count = initrd.file_count;
    }
    
    if (total_size) {
        *total_size = initrd.header ? initrd.header->total_size : 0;
    }
    
    return SUCCESS;
}

// Validate file checksum
static error_t initrd_validate_checksum(initrd_file_t* file) {
    if (!file || !initrd.data) {
        return E_INVAL;
    }
    
    // Calculate checksum
    void* file_data = (u8*)initrd.data + file->offset;
    u32 calculated_checksum = initrd_calculate_checksum(file_data, file->size);
    
    // For now, we'll skip checksum validation
    // In a real implementation, you would compare with stored checksum
    
    return SUCCESS;
}

// Calculate checksum
static u32 initrd_calculate_checksum(const void* data, u32 size) {
    if (!data) {
        return 0;
    }
    
    u32 checksum = 0;
    const u8* bytes = (const u8*)data;
    
    for (u32 i = 0; i < size; i++) {
        checksum = ((checksum << 5) + checksum) + bytes[i];
    }
    
    return checksum;
}

// Create InitRD file (for testing)
error_t initrd_create_file(const char* name, const void* data, u32 size) {
    if (!name || !data || size == 0) {
        return E_INVAL;
    }
    
    if (!initrd_initialized) {
        return E_INVAL;
    }
    
    // Check if file already exists
    if (initrd_find_file(name)) {
        return E_EXIST;
    }
    
    // Find free slot
    u32 slot = (u32)-1;
    for (u32 i = 0; i < INITRD_MAX_FILES; i++) {
        if (!initrd.files[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == (u32)-1) {
        return E_NOMEM;
    }
    
    // Check if we have space
    u32 total_used = 0;
    for (u32 i = 0; i < initrd.file_count; i++) {
        if (initrd.files[i].active) {
            total_used += initrd.files[i].size;
        }
    }
    
    if (total_used + size > INITRD_TOTAL_SIZE) {
        return E_NOSPC;
    }
    
    // Add file entry
    initrd_file_t* file = &initrd.files[slot];
    strncpy(file->name, name, sizeof(file->name) - 1);
    file->name[sizeof(file->name) - 1] = '\0';
    file->size = size;
    file->offset = total_used;
    file->flags = 0;
    file->active = true;
    
    // Copy file data
    void* file_data = (u8*)initrd.data + file->offset;
    memcpy(file_data, data, size);
    
    initrd.file_count++;
    
    KDEBUG("Created file %s: %u bytes", name, size);
    
    return SUCCESS;
}

// Check if InitRD is initialized
bool initrd_is_initialized(void) {
    return initrd_initialized;
}

// Dump InitRD information
void initrd_dump_info(void) {
    KINFO("=== InitRD Information ===");
    KINFO("Initialized: %s", initrd_initialized ? "Yes" : "No");
    
    if (initrd_initialized && initrd.header) {
        KINFO("Header: version=%u, files=%u, size=%u", 
              initrd.header->version, initrd.header->file_count, initrd.header->total_size);
        
        KINFO("Files:");
        for (u32 i = 0; i < initrd.file_count; i++) {
            if (initrd.files[i].active) {
                KINFO("  %s: %u bytes at offset %u", 
                      initrd.files[i].name, initrd.files[i].size, initrd.files[i].offset);
            }
        }
    }
} 