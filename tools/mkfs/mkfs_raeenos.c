/*
 * RaeenOS Filesystem Creation Tool (mkfs.raeenos)
 * Creates RaeenFS filesystems for RaeenOS
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define RAEENFS_MAGIC 0x5241454E  // "RAEN"
#define RAEENFS_VERSION 1
#define BLOCK_SIZE 4096
#define INODE_SIZE 256
#define MAX_FILENAME 255

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t free_blocks;
    uint32_t inode_count;
    uint32_t free_inodes;
    uint32_t root_inode;
    uint32_t bitmap_blocks;
    uint32_t inode_table_blocks;
    uint64_t created_time;
    uint64_t modified_time;
    char label[32];
    uint8_t reserved[64];
} __attribute__((packed)) raeenfs_superblock_t;

typedef struct {
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint64_t size;
    uint64_t created_time;
    uint64_t modified_time;
    uint64_t accessed_time;
    uint32_t links_count;
    uint32_t blocks_count;
    uint32_t flags;
    uint32_t direct_blocks[12];
    uint32_t indirect_block;
    uint32_t double_indirect_block;
    uint32_t triple_indirect_block;
    uint8_t reserved[64];
} __attribute__((packed)) raeenfs_inode_t;

void print_usage(const char* program_name) {
    printf("RaeenOS Filesystem Creation Tool v1.0\n");
    printf("Usage: %s [options] <device/file> <size_mb>\n", program_name);
    printf("\nOptions:\n");
    printf("  -l <label>    Set filesystem label (max 31 chars)\n");
    printf("  -b <size>     Set block size (default: 4096)\n");
    printf("  -i <ratio>    Inode ratio (bytes per inode, default: 16384)\n");
    printf("  -f            Force creation (overwrite existing)\n");
    printf("  -v            Verbose output\n");
    printf("  -h            Show this help\n");
    printf("\nExamples:\n");
    printf("  %s disk.img 100          # Create 100MB filesystem\n", program_name);
    printf("  %s -l \"RaeenOS\" -v disk.img 256  # 256MB with label\n", program_name);
}

int create_filesystem(const char* device, uint64_t size_bytes, 
                     const char* label, uint32_t block_size, 
                     uint32_t inode_ratio, int verbose) {
    FILE* fp;
    raeenfs_superblock_t superblock = {0};
    raeenfs_inode_t root_inode = {0};
    uint32_t total_blocks, bitmap_blocks, inode_table_blocks;
    uint32_t inode_count;
    uint8_t* buffer;
    time_t current_time;
    
    if (verbose) {
        printf("Creating RaeenFS filesystem...\n");
        printf("Device: %s\n", device);
        printf("Size: %llu bytes (%.2f MB)\n", 
               (unsigned long long)size_bytes, 
               (double)size_bytes / (1024 * 1024));
        printf("Block size: %u bytes\n", block_size);
        printf("Inode ratio: %u bytes per inode\n", inode_ratio);
    }
    
    // Calculate filesystem parameters
    total_blocks = size_bytes / block_size;
    inode_count = size_bytes / inode_ratio;
    bitmap_blocks = (total_blocks + (block_size * 8) - 1) / (block_size * 8);
    inode_table_blocks = (inode_count * INODE_SIZE + block_size - 1) / block_size;
    
    if (verbose) {
        printf("Total blocks: %u\n", total_blocks);
        printf("Inodes: %u\n", inode_count);
        printf("Bitmap blocks: %u\n", bitmap_blocks);
        printf("Inode table blocks: %u\n", inode_table_blocks);
    }
    
    // Open device/file
    fp = fopen(device, "wb");
    if (!fp) {
        perror("Failed to open device/file");
        return -1;
    }
    
    // Initialize superblock
    current_time = time(NULL);
    superblock.magic = RAEENFS_MAGIC;
    superblock.version = RAEENFS_VERSION;
    superblock.block_size = block_size;
    superblock.total_blocks = total_blocks;
    superblock.free_blocks = total_blocks - 1 - bitmap_blocks - inode_table_blocks;
    superblock.inode_count = inode_count;
    superblock.free_inodes = inode_count - 1; // Root inode is used
    superblock.root_inode = 1;
    superblock.bitmap_blocks = bitmap_blocks;
    superblock.inode_table_blocks = inode_table_blocks;
    superblock.created_time = current_time;
    superblock.modified_time = current_time;
    
    if (label) {
        strncpy(superblock.label, label, sizeof(superblock.label) - 1);
    } else {
        strcpy(superblock.label, "RaeenFS");
    }
    
    // Write superblock
    if (fwrite(&superblock, sizeof(superblock), 1, fp) != 1) {
        perror("Failed to write superblock");
        fclose(fp);
        return -1;
    }
    
    // Pad superblock to block boundary
    buffer = calloc(1, block_size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(fp);
        return -1;
    }
    
    if (fwrite(buffer, block_size - sizeof(superblock), 1, fp) != 1) {
        perror("Failed to pad superblock");
        free(buffer);
        fclose(fp);
        return -1;
    }
    
    // Write bitmap (mark used blocks)
    memset(buffer, 0, block_size);
    // Mark superblock, bitmap blocks, and inode table blocks as used
    for (uint32_t i = 0; i < 1 + bitmap_blocks + inode_table_blocks; i++) {
        buffer[i / 8] |= (1 << (i % 8));
    }
    
    for (uint32_t i = 0; i < bitmap_blocks; i++) {
        if (fwrite(buffer, block_size, 1, fp) != 1) {
            perror("Failed to write bitmap");
            free(buffer);
            fclose(fp);
            return -1;
        }
        if (i == 0) {
            memset(buffer, 0, block_size); // Clear for subsequent bitmap blocks
        }
    }
    
    // Initialize root inode
    root_inode.mode = 0755 | 0x4000; // Directory with rwxr-xr-x permissions
    root_inode.uid = 0;
    root_inode.gid = 0;
    root_inode.size = 0;
    root_inode.created_time = current_time;
    root_inode.modified_time = current_time;
    root_inode.accessed_time = current_time;
    root_inode.links_count = 2; // . and ..
    root_inode.blocks_count = 0;
    root_inode.flags = 0;
    
    // Write inode table
    memset(buffer, 0, block_size);
    memcpy(buffer + INODE_SIZE, &root_inode, sizeof(root_inode)); // Inode 1 is root
    
    for (uint32_t i = 0; i < inode_table_blocks; i++) {
        if (fwrite(buffer, block_size, 1, fp) != 1) {
            perror("Failed to write inode table");
            free(buffer);
            fclose(fp);
            return -1;
        }
        if (i == 0) {
            memset(buffer, 0, block_size); // Clear for subsequent inode blocks
        }
    }
    
    // Zero out remaining blocks
    for (uint32_t i = 1 + bitmap_blocks + inode_table_blocks; i < total_blocks; i++) {
        if (fwrite(buffer, block_size, 1, fp) != 1) {
            perror("Failed to write data blocks");
            free(buffer);
            fclose(fp);
            return -1;
        }
    }
    
    free(buffer);
    fclose(fp);
    
    if (verbose) {
        printf("✓ RaeenFS filesystem created successfully\n");
        printf("Label: %s\n", superblock.label);
        printf("Free blocks: %u\n", superblock.free_blocks);
        printf("Free inodes: %u\n", superblock.free_inodes);
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    const char* device = NULL;
    const char* label = NULL;
    uint64_t size_mb = 0;
    uint32_t block_size = BLOCK_SIZE;
    uint32_t inode_ratio = 16384;
    int verbose = 0;
    int force = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-f") == 0) {
            force = 1;
        } else if (strcmp(argv[i], "-l") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: -l requires a label\n");
                return 1;
            }
            label = argv[i];
        } else if (strcmp(argv[i], "-b") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: -b requires a block size\n");
                return 1;
            }
            block_size = atoi(argv[i]);
            if (block_size < 512 || block_size > 65536 || (block_size & (block_size - 1))) {
                fprintf(stderr, "Error: Block size must be a power of 2 between 512 and 65536\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-i") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: -i requires an inode ratio\n");
                return 1;
            }
            inode_ratio = atoi(argv[i]);
            if (inode_ratio < 1024) {
                fprintf(stderr, "Error: Inode ratio must be at least 1024\n");
                return 1;
            }
        } else if (!device) {
            device = argv[i];
        } else if (size_mb == 0) {
            size_mb = atoll(argv[i]);
        } else {
            fprintf(stderr, "Error: Unexpected argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (!device || size_mb == 0) {
        fprintf(stderr, "Error: Device and size are required\n");
        print_usage(argv[0]);
        return 1;
    }
    
    if (size_mb < 1) {
        fprintf(stderr, "Error: Size must be at least 1 MB\n");
        return 1;
    }
    
    uint64_t size_bytes = size_mb * 1024 * 1024;
    
    return create_filesystem(device, size_bytes, label, block_size, inode_ratio, verbose);
}