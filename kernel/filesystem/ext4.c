#include "filesystem/include/vfs.h"
#include "kernel.h"
#include "memory/memory.h"
#include <string.h>

// ext4 configuration
#define EXT4_SUPER_MAGIC 0xEF53
#define EXT4_MAX_BLOCK_SIZE 65536
#define EXT4_MIN_BLOCK_SIZE 1024
#define EXT4_MAX_FILENAME_LEN 255
#define EXT4_MAX_EXTENTS 4
#define EXT4_INODE_SIZE 256
#define EXT4_DIR_ENTRY_SIZE 8

// ext4 feature flags
#define EXT4_FEATURE_COMPAT_DIR_INDEX 0x0020
#define EXT4_FEATURE_COMPAT_EXTENTS 0x0040
#define EXT4_FEATURE_COMPAT_64BIT 0x0080
#define EXT4_FEATURE_INCOMPAT_EXTENTS 0x0040
#define EXT4_FEATURE_INCOMPAT_64BIT 0x0080
#define EXT4_FEATURE_INCOMPAT_FLEX_BG 0x0200
#define EXT4_FEATURE_INCOMPAT_EA_INODE 0x0400
#define EXT4_FEATURE_INCOMPAT_DIRDATA 0x1000
#define EXT4_FEATURE_INCOMPAT_CSUM_SEED 0x2000
#define EXT4_FEATURE_INCOMPAT_LARGEDIR 0x4000
#define EXT4_FEATURE_INCOMPAT_INLINE_DATA 0x8000
#define EXT4_FEATURE_INCOMPAT_ENCRYPT 0x10000

// ext4 inode flags
#define EXT4_INODE_EXTENTS 0x80000
#define EXT4_INODE_EA_INODE 0x20000000
#define EXT4_INODE_EOFBLOCKS 0x40000000
#define EXT4_INODE_SNAPSHOT 0x40000000

// ext4 directory entry flags
#define EXT4_DE_UNKNOWN 0
#define EXT4_DE_REG_FILE 1
#define EXT4_DE_DIR 2
#define EXT4_DE_CHRDEV 3
#define EXT4_DE_BLKDEV 4
#define EXT4_DE_FIFO 5
#define EXT4_DE_SOCK 6
#define EXT4_DE_SYMLINK 7

// ext4 inode types
#define EXT4_S_IFSOCK 0xC000
#define EXT4_S_IFLNK 0xA000
#define EXT4_S_IFREG 0x8000
#define EXT4_S_IFBLK 0x6000
#define EXT4_S_IFDIR 0x4000
#define EXT4_S_IFCHR 0x2000
#define EXT4_S_IFIFO 0x1000

// ext4 superblock structure
typedef struct {
    u32 s_inodes_count;
    u32 s_blocks_count_lo;
    u32 s_r_blocks_count_lo;
    u32 s_free_blocks_count_lo;
    u32 s_free_inodes_count;
    u32 s_first_data_block;
    u32 s_log_block_size;
    u32 s_log_cluster_size;
    u32 s_blocks_per_group;
    u32 s_clusters_per_group;
    u32 s_inodes_per_group;
    u32 s_mtime;
    u32 s_wtime;
    u16 s_mnt_count;
    u16 s_max_mnt_count;
    u16 s_magic;
    u16 s_state;
    u16 s_errors;
    u16 s_minor_rev_level;
    u32 s_lastcheck;
    u32 s_checkinterval;
    u32 s_creator_os;
    u32 s_rev_level;
    u16 s_def_resuid;
    u16 s_def_resgid;
    u32 s_first_ino;
    u16 s_inode_size;
    u16 s_block_group_nr;
    u32 s_feature_compat;
    u32 s_feature_incompat;
    u32 s_feature_ro_compat;
    u8 s_uuid[16];
    char s_volume_name[16];
    char s_last_mounted[64];
    u32 s_algorithm_usage_bitmap;
    u8 s_prealloc_blocks;
    u8 s_prealloc_dir_blocks;
    u16 s_reserved_gdt_blocks;
    u8 s_journal_uuid[16];
    u32 s_journal_inum;
    u32 s_journal_dev;
    u32 s_last_orphan;
    u32 s_hash_seed[4];
    u8 s_def_hash_version;
    u8 s_jnl_backup_type;
    u16 s_desc_size;
    u32 s_default_mount_opts;
    u32 s_first_meta_bg;
    u32 s_mkfs_time;
    u32 s_jnl_blocks[17];
    u32 s_blocks_count_hi;
    u32 s_r_blocks_count_hi;
    u32 s_free_blocks_count_hi;
    u16 s_min_extra_isize;
    u16 s_want_extra_isize;
    u32 s_flags;
    u16 s_raid_stride;
    u16 s_mmp_interval;
    u64 s_mmp_block;
    u32 s_raid_stripe_width;
    u8 s_log_groups_per_flex;
    u8 s_checksum_type;
    u8 s_encryption_level;
    u8 s_reserved_pad;
    u64 s_kbytes_written;
    u32 s_snapshot_inum;
    u32 s_snapshot_id;
    u64 s_snapshot_r_blocks_count;
    u32 s_snapshot_list;
    u32 s_error_count;
    u32 s_first_error_time;
    u32 s_first_error_ino;
    u64 s_first_error_block;
    u8 s_first_error_func[32];
    u32 s_first_error_line;
    u32 s_last_error_time;
    u32 s_last_error_ino;
    u32 s_last_error_line;
    u64 s_last_error_block;
    u8 s_last_error_func[32];
    u8 s_mount_opts[64];
    u32 s_usr_quota_inum;
    u32 s_grp_quota_inum;
    u32 s_overhead_blocks;
    u32 s_backup_bgs[2];
    u8 s_encrypt_algos[4];
    u8 s_encrypt_pw_salt[16];
    u32 s_lpf_ino;
    u32 s_prj_quota_inum;
    u32 s_checksum_seed;
    u32 s_reserved[98];
    u32 s_checksum;
} __attribute__((packed)) ext4_superblock_t;

// ext4 inode structure
typedef struct {
    u16 i_mode;
    u16 i_uid;
    u32 i_size_lo;
    u32 i_atime;
    u32 i_ctime;
    u32 i_mtime;
    u32 i_dtime;
    u16 i_gid;
    u16 i_links_count;
    u32 i_blocks_lo;
    u32 i_flags;
    u32 i_block[15];
    u32 i_generation;
    u32 i_file_acl_lo;
    u32 i_size_high;
    u32 i_faddr;
    u16 i_frag;
    u16 i_fsize;
    u16 i_pad1;
    u16 i_reserved2[2];
    u32 i_reserved2[2];
} __attribute__((packed)) ext4_inode_t;

// ext4 extent header
typedef struct {
    u16 eh_magic;
    u16 eh_entries;
    u16 eh_max;
    u16 eh_depth;
    u32 eh_generation;
} __attribute__((packed)) ext4_extent_header_t;

// ext4 extent
typedef struct {
    u32 ee_block;
    u16 ee_len;
    u16 ee_start_hi;
    u32 ee_start_lo;
} __attribute__((packed)) ext4_extent_t;

// ext4 directory entry
typedef struct {
    u32 inode;
    u16 rec_len;
    u8 name_len;
    u8 file_type;
    char name[EXT4_MAX_FILENAME_LEN];
} __attribute__((packed)) ext4_dir_entry_t;

// ext4 block group descriptor
typedef struct {
    u32 bg_block_bitmap_lo;
    u32 bg_inode_bitmap_lo;
    u32 bg_inode_table_lo;
    u16 bg_free_blocks_count_lo;
    u16 bg_free_inodes_count_lo;
    u16 bg_used_dirs_count_lo;
    u16 bg_pad;
    u32 bg_reserved[3];
} __attribute__((packed)) ext4_group_desc_t;

// ext4 file system state
typedef struct {
    ext4_superblock_t* superblock;
    ext4_group_desc_t* group_descriptors;
    u32 block_size;
    u32 blocks_per_group;
    u32 inodes_per_group;
    u32 num_groups;
    u32 first_data_block;
    u32 inode_size;
    u32 feature_compat;
    u32 feature_incompat;
    u32 feature_ro_compat;
    bool extents_enabled;
    bool large_file_enabled;
    bool dir_index_enabled;
    void* journal;
    u32 journal_size;
    bool active;
} ext4_fs_t;

// ext4 inode cache entry
typedef struct {
    u32 inode_number;
    ext4_inode_t inode;
    void* data;
    u32 data_size;
    bool dirty;
    bool active;
} ext4_inode_cache_t;

// ext4 block cache entry
typedef struct {
    u32 block_number;
    void* data;
    bool dirty;
    bool active;
} ext4_block_cache_t;

// Global ext4 state
static ext4_fs_t ext4_fs = {0};
static ext4_inode_cache_t inode_cache[1024];
static ext4_block_cache_t block_cache[4096];
static u32 inode_cache_size = 0;
static u32 block_cache_size = 0;
static bool ext4_initialized = false;
static ext4_superblock_t* superblock = NULL;
static void* block_cache = NULL;
static u32 block_size = 0;
static u32 blocks_per_group = 0;
static u32 inodes_per_group = 0;

// Forward declarations
static error_t ext4_read_superblock(void);
static error_t ext4_read_group_descriptors(void);
static error_t ext4_read_inode(u32 inode_number, ext4_inode_t* inode);
static error_t ext4_write_inode(u32 inode_number, ext4_inode_t* inode);
static error_t ext4_read_block(u32 block_number, void* buffer);
static error_t ext4_write_block(u32 block_number, const void* buffer);
static error_t ext4_alloc_block(u32* block_number);
static error_t ext4_free_block(u32 block_number);
static error_t ext4_alloc_inode(u32* inode_number);
static error_t ext4_free_inode(u32 inode_number);
static error_t ext4_read_extent_tree(ext4_inode_t* inode, u32 block_number, u32* physical_block);
static error_t ext4_write_extent_tree(ext4_inode_t* inode, u32 block_number, u32 physical_block);
static error_t ext4_read_directory(ext4_inode_t* inode, ext4_dir_entry_t* entries, u32 max_entries);
static error_t ext4_write_directory(ext4_inode_t* inode, ext4_dir_entry_t* entries, u32 num_entries);
static ext4_inode_cache_t* ext4_get_inode_cache(u32 inode_number);
static ext4_block_cache_t* ext4_get_block_cache(u32 block_number);

// Initialize ext4 file system
error_t ext4_init(void) {
    if (ext4_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing ext4 file system...");
    
    // Initialize ext4 state
    memset(&ext4_fs, 0, sizeof(ext4_fs));
    ext4_fs.active = false;
    
    // Initialize inode cache
    memset(inode_cache, 0, sizeof(inode_cache));
    for (u32 i = 0; i < 1024; i++) {
        inode_cache[i].active = false;
    }
    
    // Initialize block cache
    memset(block_cache, 0, sizeof(block_cache));
    for (u32 i = 0; i < 4096; i++) {
        block_cache[i].active = false;
    }
    
    // Read superblock
    error_t result = ext4_read_superblock();
    if (result != SUCCESS) {
        KERROR("Failed to read ext4 superblock");
        return result;
    }
    
    // Read group descriptors
    result = ext4_read_group_descriptors();
    if (result != SUCCESS) {
        KERROR("Failed to read ext4 group descriptors");
        return result;
    }
    
    ext4_fs.active = true;
    ext4_initialized = true;
    
    KINFO("ext4 initialized: %u blocks, %u inodes, block size: %u", 
          ext4_fs.superblock->s_blocks_count_lo, 
          ext4_fs.superblock->s_inodes_count,
          ext4_fs.block_size);
    
    return SUCCESS;
}

// Read ext4 superblock
static error_t ext4_read_superblock(void) {
    // Allocate superblock
    ext4_fs.superblock = (ext4_superblock_t*)memory_alloc(sizeof(ext4_superblock_t));
    if (!ext4_fs.superblock) {
        return E_NOMEM;
    }
    
    // Read superblock from block 1 (1024 bytes offset)
    error_t result = vfs_read_block(1, ext4_fs.superblock);
    if (result != SUCCESS) {
        memory_free(ext4_fs.superblock);
        return result;
    }
    
    // Verify magic number
    if (ext4_fs.superblock->s_magic != EXT4_SUPER_MAGIC) {
        KERROR("Invalid ext4 superblock magic: 0x%x", ext4_fs.superblock->s_magic);
        memory_free(ext4_fs.superblock);
        return E_INVAL;
    }
    
    // Calculate block size
    ext4_fs.block_size = 1024 << ext4_fs.superblock->s_log_block_size;
    if (ext4_fs.block_size < EXT4_MIN_BLOCK_SIZE || ext4_fs.block_size > EXT4_MAX_BLOCK_SIZE) {
        KERROR("Invalid ext4 block size: %u", ext4_fs.block_size);
        memory_free(ext4_fs.superblock);
        return E_INVAL;
    }
    
    // Set up file system parameters
    ext4_fs.blocks_per_group = ext4_fs.superblock->s_blocks_per_group;
    ext4_fs.inodes_per_group = ext4_fs.superblock->s_inodes_per_group;
    ext4_fs.num_groups = (ext4_fs.superblock->s_blocks_count_lo + ext4_fs.blocks_per_group - 1) / ext4_fs.blocks_per_group;
    ext4_fs.first_data_block = ext4_fs.superblock->s_first_data_block;
    ext4_fs.inode_size = ext4_fs.superblock->s_inode_size;
    
    // Check features
    ext4_fs.feature_compat = ext4_fs.superblock->s_feature_compat;
    ext4_fs.feature_incompat = ext4_fs.superblock->s_feature_incompat;
    ext4_fs.feature_ro_compat = ext4_fs.superblock->s_feature_ro_compat;
    
    ext4_fs.extents_enabled = (ext4_fs.feature_incompat & EXT4_FEATURE_INCOMPAT_EXTENTS) != 0;
    ext4_fs.large_file_enabled = (ext4_fs.feature_incompat & EXT4_FEATURE_INCOMPAT_64BIT) != 0;
    ext4_fs.dir_index_enabled = (ext4_fs.feature_compat & EXT4_FEATURE_COMPAT_DIR_INDEX) != 0;
    
    KDEBUG("ext4 features: extents=%s, large_file=%s, dir_index=%s",
           ext4_fs.extents_enabled ? "yes" : "no",
           ext4_fs.large_file_enabled ? "yes" : "no",
           ext4_fs.dir_index_enabled ? "yes" : "no");
    
    return SUCCESS;
}

// Read ext4 group descriptors
static error_t ext4_read_group_descriptors(void) {
    if (!ext4_fs.superblock) {
        return E_INVAL;
    }
    
    // Calculate group descriptor block
    u32 group_desc_block = ext4_fs.first_data_block + 1;
    
    // Allocate group descriptors
    u32 desc_size = sizeof(ext4_group_desc_t);
    if (ext4_fs.superblock->s_desc_size > 0) {
        desc_size = ext4_fs.superblock->s_desc_size;
    }
    
    u32 total_desc_size = ext4_fs.num_groups * desc_size;
    ext4_fs.group_descriptors = (ext4_group_desc_t*)memory_alloc(total_desc_size);
    if (!ext4_fs.group_descriptors) {
        return E_NOMEM;
    }
    
    // Read group descriptors
    error_t result = vfs_read_block(group_desc_block, ext4_fs.group_descriptors);
    if (result != SUCCESS) {
        memory_free(ext4_fs.group_descriptors);
        return result;
    }
    
    return SUCCESS;
}

// Read ext4 inode
static error_t ext4_read_inode(u32 inode_number, ext4_inode_t* inode) {
    if (!inode || inode_number == 0) {
        return E_INVAL;
    }
    
    // Check cache first
    ext4_inode_cache_t* cache = ext4_get_inode_cache(inode_number);
    if (cache && cache->active) {
        memcpy(inode, &cache->inode, sizeof(ext4_inode_t));
        return SUCCESS;
    }
    
    // Calculate inode location
    u32 group = (inode_number - 1) / ext4_fs.inodes_per_group;
    u32 index = (inode_number - 1) % ext4_fs.inodes_per_group;
    
    if (group >= ext4_fs.num_groups) {
        return E_NOENT;
    }
    
    // Get inode table block
    u32 inode_table_block = ext4_fs.group_descriptors[group].bg_inode_table_lo;
    u32 inode_block = inode_table_block + (index * ext4_fs.inode_size) / ext4_fs.block_size;
    u32 inode_offset = (index * ext4_fs.inode_size) % ext4_fs.block_size;
    
    // Read inode block
    void* block_buffer = memory_alloc(ext4_fs.block_size);
    if (!block_buffer) {
        return E_NOMEM;
    }
    
    error_t result = ext4_read_block(inode_block, block_buffer);
    if (result == SUCCESS) {
        memcpy(inode, (u8*)block_buffer + inode_offset, sizeof(ext4_inode_t));
        
        // Cache the inode
        if (cache) {
            memcpy(&cache->inode, inode, sizeof(ext4_inode_t));
            cache->inode_number = inode_number;
            cache->active = true;
            cache->dirty = false;
        }
    }
    
    memory_free(block_buffer);
    return result;
}

// Write ext4 inode
static error_t ext4_write_inode(u32 inode_number, ext4_inode_t* inode) {
    if (!inode || inode_number == 0) {
        return E_INVAL;
    }
    
    // Calculate inode location
    u32 group = (inode_number - 1) / ext4_fs.inodes_per_group;
    u32 index = (inode_number - 1) % ext4_fs.inodes_per_group;
    
    if (group >= ext4_fs.num_groups) {
        return E_NOENT;
    }
    
    // Get inode table block
    u32 inode_table_block = ext4_fs.group_descriptors[group].bg_inode_table_lo;
    u32 inode_block = inode_table_block + (index * ext4_fs.inode_size) / ext4_fs.block_size;
    u32 inode_offset = (index * ext4_fs.inode_size) % ext4_fs.block_size;
    
    // Read inode block
    void* block_buffer = memory_alloc(ext4_fs.block_size);
    if (!block_buffer) {
        return E_NOMEM;
    }
    
    error_t result = ext4_read_block(inode_block, block_buffer);
    if (result == SUCCESS) {
        // Update inode
        memcpy((u8*)block_buffer + inode_offset, inode, sizeof(ext4_inode_t));
        
        // Write back block
        result = ext4_write_block(inode_block, block_buffer);
        
        // Update cache
        ext4_inode_cache_t* cache = ext4_get_inode_cache(inode_number);
        if (cache && cache->active) {
            memcpy(&cache->inode, inode, sizeof(ext4_inode_t));
            cache->dirty = false;
        }
    }
    
    memory_free(block_buffer);
    return result;
}

// Read ext4 block
static error_t ext4_read_block(u32 block_number, void* buffer) {
    if (!buffer) {
        return E_INVAL;
    }
    
    // Check cache first
    ext4_block_cache_t* cache = ext4_get_block_cache(block_number);
    if (cache && cache->active) {
        memcpy(buffer, cache->data, ext4_fs.block_size);
        return SUCCESS;
    }
    
    // Read from disk
    error_t result = vfs_read_block(block_number, buffer);
    if (result == SUCCESS) {
        // Cache the block
        if (cache) {
            if (!cache->data) {
                cache->data = memory_alloc(ext4_fs.block_size);
            }
            if (cache->data) {
                memcpy(cache->data, buffer, ext4_fs.block_size);
                cache->block_number = block_number;
                cache->active = true;
                cache->dirty = false;
            }
        }
    }
    
    return result;
}

// Write ext4 block
static error_t ext4_write_block(u32 block_number, const void* buffer) {
    if (!buffer) {
        return E_INVAL;
    }
    
    // Write to disk
    error_t result = vfs_write_block(block_number, buffer);
    if (result == SUCCESS) {
        // Update cache
        ext4_block_cache_t* cache = ext4_get_block_cache(block_number);
        if (cache && cache->active && cache->data) {
            memcpy(cache->data, buffer, ext4_fs.block_size);
            cache->dirty = false;
        }
    }
    
    return result;
}

// Allocate ext4 block
static error_t ext4_alloc_block(u32* block_number) {
    if (!block_number) {
        return E_INVAL;
    }
    
    // Find free block in any group
    for (u32 group = 0; group < ext4_fs.num_groups; group++) {
        if (ext4_fs.group_descriptors[group].bg_free_blocks_count_lo > 0) {
            // Read block bitmap
            u32 bitmap_block = ext4_fs.group_descriptors[group].bg_block_bitmap_lo;
            void* bitmap = memory_alloc(ext4_fs.block_size);
            if (!bitmap) {
                return E_NOMEM;
            }
            
            error_t result = ext4_read_block(bitmap_block, bitmap);
            if (result == SUCCESS) {
                // Find free bit
                u8* bits = (u8*)bitmap;
                for (u32 i = 0; i < ext4_fs.block_size * 8; i++) {
                    u32 byte = i / 8;
                    u32 bit = i % 8;
                    if (!(bits[byte] & (1 << bit))) {
                        // Mark as allocated
                        bits[byte] |= (1 << bit);
                        ext4_write_block(bitmap_block, bitmap);
                        
                        // Calculate block number
                        *block_number = ext4_fs.first_data_block + group * ext4_fs.blocks_per_group + i;
                        
                        // Update group descriptor
                        ext4_fs.group_descriptors[group].bg_free_blocks_count_lo--;
                        ext4_fs.superblock->s_free_blocks_count_lo--;
                        
                        memory_free(bitmap);
                        return SUCCESS;
                    }
                }
            }
            
            memory_free(bitmap);
        }
    }
    
    return E_NOSPC;
}

// Free ext4 block
static error_t ext4_free_block(u32 block_number) {
    // Calculate group and bit
    u32 group = (block_number - ext4_fs.first_data_block) / ext4_fs.blocks_per_group;
    u32 bit = (block_number - ext4_fs.first_data_block) % ext4_fs.blocks_per_group;
    
    if (group >= ext4_fs.num_groups) {
        return E_INVAL;
    }
    
    // Read block bitmap
    u32 bitmap_block = ext4_fs.group_descriptors[group].bg_block_bitmap_lo;
    void* bitmap = memory_alloc(ext4_fs.block_size);
    if (!bitmap) {
        return E_NOMEM;
    }
    
    error_t result = ext4_read_block(bitmap_block, bitmap);
    if (result == SUCCESS) {
        // Clear bit
        u8* bits = (u8*)bitmap;
        u32 byte = bit / 8;
        u32 bit_pos = bit % 8;
        bits[byte] &= ~(1 << bit_pos);
        ext4_write_block(bitmap_block, bitmap);
        
        // Update group descriptor
        ext4_fs.group_descriptors[group].bg_free_blocks_count_lo++;
        ext4_fs.superblock->s_free_blocks_count_lo++;
    }
    
    memory_free(bitmap);
    return result;
}

// Allocate ext4 inode
static error_t ext4_alloc_inode(u32* inode_number) {
    if (!inode_number) {
        return E_INVAL;
    }
    
    // Find free inode in any group
    for (u32 group = 0; group < ext4_fs.num_groups; group++) {
        if (ext4_fs.group_descriptors[group].bg_free_inodes_count_lo > 0) {
            // Read inode bitmap
            u32 bitmap_block = ext4_fs.group_descriptors[group].bg_inode_bitmap_lo;
            void* bitmap = memory_alloc(ext4_fs.block_size);
            if (!bitmap) {
                return E_NOMEM;
            }
            
            error_t result = ext4_read_block(bitmap_block, bitmap);
            if (result == SUCCESS) {
                // Find free bit (skip inode 0)
                u8* bits = (u8*)bitmap;
                for (u32 i = 1; i < ext4_fs.inodes_per_group; i++) {
                    u32 byte = i / 8;
                    u32 bit = i % 8;
                    if (!(bits[byte] & (1 << bit))) {
                        // Mark as allocated
                        bits[byte] |= (1 << bit);
                        ext4_write_block(bitmap_block, bitmap);
                        
                        // Calculate inode number
                        *inode_number = group * ext4_fs.inodes_per_group + i + 1;
                        
                        // Update group descriptor
                        ext4_fs.group_descriptors[group].bg_free_inodes_count_lo--;
                        ext4_fs.superblock->s_free_inodes_count--;
                        
                        memory_free(bitmap);
                        return SUCCESS;
                    }
                }
            }
            
            memory_free(bitmap);
        }
    }
    
    return E_NOSPC;
}

// Free ext4 inode
static error_t ext4_free_inode(u32 inode_number) {
    // Calculate group and bit
    u32 group = (inode_number - 1) / ext4_fs.inodes_per_group;
    u32 bit = (inode_number - 1) % ext4_fs.inodes_per_group;
    
    if (group >= ext4_fs.num_groups) {
        return E_INVAL;
    }
    
    // Read inode bitmap
    u32 bitmap_block = ext4_fs.group_descriptors[group].bg_inode_bitmap_lo;
    void* bitmap = memory_alloc(ext4_fs.block_size);
    if (!bitmap) {
        return E_NOMEM;
    }
    
    error_t result = ext4_read_block(bitmap_block, bitmap);
    if (result == SUCCESS) {
        // Clear bit
        u8* bits = (u8*)bitmap;
        u32 byte = bit / 8;
        u32 bit_pos = bit % 8;
        bits[byte] &= ~(1 << bit_pos);
        ext4_write_block(bitmap_block, bitmap);
        
        // Update group descriptor
        ext4_fs.group_descriptors[group].bg_free_inodes_count_lo++;
        ext4_fs.superblock->s_free_inodes_count++;
    }
    
    memory_free(bitmap);
    return result;
}

// Read ext4 extent tree
static error_t ext4_read_extent_tree(ext4_inode_t* inode, u32 block_number, u32* physical_block) {
    if (!inode || !physical_block) {
        return E_INVAL;
    }
    
    if (!ext4_fs.extents_enabled || !(inode->i_flags & EXT4_INODE_EXTENTS)) {
        // Use traditional block mapping
        if (block_number < 12) {
            *physical_block = inode->i_block[block_number];
            return SUCCESS;
        } else {
            // Handle indirect blocks (simplified)
            return E_NOSYS;
        }
    }
    
    // Read extent tree
    ext4_extent_header_t* header = (ext4_extent_header_t*)inode->i_block;
    if (header->eh_magic != 0xF30A) {
        return E_INVAL;
    }
    
    if (header->eh_depth == 0) {
        // Leaf node
        ext4_extent_t* extents = (ext4_extent_t*)(header + 1);
        for (u32 i = 0; i < header->eh_entries; i++) {
            if (block_number >= extents[i].ee_block && 
                block_number < extents[i].ee_block + extents[i].ee_len) {
                *physical_block = extents[i].ee_start_lo + (block_number - extents[i].ee_block);
                return SUCCESS;
            }
        }
    } else {
        // Internal node (simplified)
        return E_NOSYS;
    }
    
    return E_NOENT;
}

// Write ext4 extent tree
static error_t ext4_write_extent_tree(ext4_inode_t* inode, u32 block_number, u32 physical_block) {
    if (!inode) {
        return E_INVAL;
    }
    
    if (!ext4_fs.extents_enabled || !(inode->i_flags & EXT4_INODE_EXTENTS)) {
        // Use traditional block mapping
        if (block_number < 12) {
            inode->i_block[block_number] = physical_block;
            return SUCCESS;
        } else {
            // Handle indirect blocks (simplified)
            return E_NOSYS;
        }
    }
    
    // Write extent tree (simplified)
    return E_NOSYS;
}

// Read ext4 directory
static error_t ext4_read_directory(ext4_inode_t* inode, ext4_dir_entry_t* entries, u32 max_entries) {
    if (!inode || !entries) {
        return E_INVAL;
    }
    
    u32 file_size = inode->i_size_lo;
    u32 block_count = (file_size + ext4_fs.block_size - 1) / ext4_fs.block_size;
    u32 entry_count = 0;
    
    void* block_buffer = memory_alloc(ext4_fs.block_size);
    if (!block_buffer) {
        return E_NOMEM;
    }
    
    for (u32 block = 0; block < block_count && entry_count < max_entries; block++) {
        u32 physical_block;
        error_t result = ext4_read_extent_tree(inode, block, &physical_block);
        if (result != SUCCESS) {
            break;
        }
        
        result = ext4_read_block(physical_block, block_buffer);
        if (result != SUCCESS) {
            break;
        }
        
        // Parse directory entries
        u32 offset = 0;
        while (offset < ext4_fs.block_size && entry_count < max_entries) {
            ext4_dir_entry_t* entry = (ext4_dir_entry_t*)((u8*)block_buffer + offset);
            
            if (entry->inode == 0) {
                // Empty entry
                offset += entry->rec_len;
                continue;
            }
            
            if (entry->rec_len == 0) {
                break;
            }
            
            // Copy entry
            memcpy(&entries[entry_count], entry, sizeof(ext4_dir_entry_t));
            entry_count++;
            
            offset += entry->rec_len;
        }
    }
    
    memory_free(block_buffer);
    return SUCCESS;
}

// Write ext4 directory
static error_t ext4_write_directory(ext4_inode_t* inode, ext4_dir_entry_t* entries, u32 num_entries) {
    if (!inode || !entries) {
        return E_INVAL;
    }
    
    // Simplified directory writing
    // In a real implementation, this would handle directory entry allocation and deallocation
    
    return SUCCESS;
}

// Get inode cache entry
static ext4_inode_cache_t* ext4_get_inode_cache(u32 inode_number) {
    for (u32 i = 0; i < 1024; i++) {
        if (!inode_cache[i].active) {
            return &inode_cache[i];
        }
        if (inode_cache[i].inode_number == inode_number) {
            return &inode_cache[i];
        }
    }
    
    // Replace least recently used entry
    return &inode_cache[inode_cache_size % 1024];
}

// Get block cache entry
static ext4_block_cache_t* ext4_get_block_cache(u32 block_number) {
    for (u32 i = 0; i < 4096; i++) {
        if (!block_cache[i].active) {
            return &block_cache[i];
        }
        if (block_cache[i].block_number == block_number) {
            return &block_cache[i];
        }
    }
    
    // Replace least recently used entry
    return &block_cache[block_cache_size % 4096];
}

// Create ext4 file
error_t ext4_create_file(const char* path, mode_t mode, u32* inode_number) {
    if (!path || !inode_number) {
        return E_INVAL;
    }
    
    // Allocate inode
    error_t result = ext4_alloc_inode(inode_number);
    if (result != SUCCESS) {
        return result;
    }
    
    // Initialize inode
    ext4_inode_t inode = {0};
    inode.i_mode = mode;
    inode.i_uid = current_process ? current_process->uid : 0;
    inode.i_gid = current_process ? current_process->gid : 0;
    inode.i_size_lo = 0;
    inode.i_blocks_lo = 0;
    inode.i_links_count = 1;
    inode.i_atime = hal_get_timestamp();
    inode.i_ctime = hal_get_timestamp();
    inode.i_mtime = hal_get_timestamp();
    
    if (ext4_fs.extents_enabled) {
        inode.i_flags |= EXT4_INODE_EXTENTS;
    }
    
    // Write inode
    result = ext4_write_inode(*inode_number, &inode);
    if (result != SUCCESS) {
        ext4_free_inode(*inode_number);
        return result;
    }
    
    KDEBUG("Created ext4 file: %s (inode: %u)", path, *inode_number);
    return SUCCESS;
}

// Delete ext4 file
error_t ext4_delete_file(u32 inode_number) {
    if (inode_number == 0) {
        return E_INVAL;
    }
    
    // Read inode
    ext4_inode_t inode;
    error_t result = ext4_read_inode(inode_number, &inode);
    if (result != SUCCESS) {
        return result;
    }
    
    // Free blocks (simplified)
    u32 block_count = (inode.i_size_lo + ext4_fs.block_size - 1) / ext4_fs.block_size;
    for (u32 i = 0; i < block_count && i < 12; i++) {
        if (inode.i_block[i] != 0) {
            ext4_free_block(inode.i_block[i]);
        }
    }
    
    // Free inode
    result = ext4_free_inode(inode_number);
    if (result != SUCCESS) {
        return result;
    }
    
    KDEBUG("Deleted ext4 file: inode %u", inode_number);
    return SUCCESS;
}

// Read ext4 file
ssize_t ext4_read_file(u32 inode_number, void* buffer, size_t size, off_t offset) {
    if (!buffer || inode_number == 0) {
        return -E_INVAL;
    }
    
    // Read inode
    ext4_inode_t inode;
    error_t result = ext4_read_inode(inode_number, &inode);
    if (result != SUCCESS) {
        return -result;
    }
    
    if (offset >= inode.i_size_lo) {
        return 0;
    }
    
    // Calculate read size
    size_t read_size = size;
    if (offset + read_size > inode.i_size_lo) {
        read_size = inode.i_size_lo - offset;
    }
    
    // Read blocks
    u32 start_block = offset / ext4_fs.block_size;
    u32 end_block = (offset + read_size - 1) / ext4_fs.block_size;
    u32 block_offset = offset % ext4_fs.block_size;
    
    void* block_buffer = memory_alloc(ext4_fs.block_size);
    if (!block_buffer) {
        return -E_NOMEM;
    }
    
    size_t bytes_read = 0;
    u8* buf = (u8*)buffer;
    
    for (u32 block = start_block; block <= end_block && bytes_read < read_size; block++) {
        u32 physical_block;
        result = ext4_read_extent_tree(&inode, block, &physical_block);
        if (result != SUCCESS) {
            break;
        }
        
        result = ext4_read_block(physical_block, block_buffer);
        if (result != SUCCESS) {
            break;
        }
        
        // Copy data
        size_t copy_size = ext4_fs.block_size - block_offset;
        if (bytes_read + copy_size > read_size) {
            copy_size = read_size - bytes_read;
        }
        
        memcpy(buf + bytes_read, (u8*)block_buffer + block_offset, copy_size);
        bytes_read += copy_size;
        block_offset = 0;
    }
    
    memory_free(block_buffer);
    return bytes_read;
}

// Write ext4 file
ssize_t ext4_write_file(u32 inode_number, const void* buffer, size_t size, off_t offset) {
    if (!buffer || inode_number == 0) {
        return -E_INVAL;
    }
    
    // Read inode
    ext4_inode_t inode;
    error_t result = ext4_read_inode(inode_number, &inode);
    if (result != SUCCESS) {
        return -result;
    }
    
    // Calculate write size and new file size
    size_t write_size = size;
    u32 new_size = offset + write_size;
    if (new_size > inode.i_size_lo) {
        inode.i_size_lo = new_size;
    }
    
    // Write blocks
    u32 start_block = offset / ext4_fs.block_size;
    u32 end_block = (offset + write_size - 1) / ext4_fs.block_size;
    u32 block_offset = offset % ext4_fs.block_size;
    
    void* block_buffer = memory_alloc(ext4_fs.block_size);
    if (!block_buffer) {
        return -E_NOMEM;
    }
    
    size_t bytes_written = 0;
    const u8* buf = (const u8*)buffer;
    
    for (u32 block = start_block; block <= end_block && bytes_written < write_size; block++) {
        u32 physical_block;
        result = ext4_read_extent_tree(&inode, block, &physical_block);
        if (result != SUCCESS) {
            // Allocate new block
            result = ext4_alloc_block(&physical_block);
            if (result != SUCCESS) {
                break;
            }
            
            // Update extent tree
            result = ext4_write_extent_tree(&inode, block, physical_block);
            if (result != SUCCESS) {
                ext4_free_block(physical_block);
                break;
            }
        }
        
        // Read existing block data
        if (block_offset > 0 || bytes_written + ext4_fs.block_size > write_size) {
            result = ext4_read_block(physical_block, block_buffer);
            if (result != SUCCESS) {
                memset(block_buffer, 0, ext4_fs.block_size);
            }
        } else {
            memset(block_buffer, 0, ext4_fs.block_size);
        }
        
        // Copy data
        size_t copy_size = ext4_fs.block_size - block_offset;
        if (bytes_written + copy_size > write_size) {
            copy_size = write_size - bytes_written;
        }
        
        memcpy((u8*)block_buffer + block_offset, buf + bytes_written, copy_size);
        
        // Write block
        result = ext4_write_block(physical_block, block_buffer);
        if (result != SUCCESS) {
            break;
        }
        
        bytes_written += copy_size;
        block_offset = 0;
    }
    
    memory_free(block_buffer);
    
    // Update inode
    if (bytes_written > 0) {
        inode.i_mtime = hal_get_timestamp();
        ext4_write_inode(inode_number, &inode);
    }
    
    return bytes_written;
}

// Dump ext4 information
void ext4_dump_info(void) {
    KINFO("=== ext4 File System Information ===");
    KINFO("Initialized: %s", ext4_initialized ? "Yes" : "No");
    
    if (ext4_fs.active && ext4_fs.superblock) {
        KINFO("Volume: %s", ext4_fs.superblock->s_volume_name);
        KINFO("Blocks: %u total, %u free", 
              ext4_fs.superblock->s_blocks_count_lo,
              ext4_fs.superblock->s_free_blocks_count_lo);
        KINFO("Inodes: %u total, %u free",
              ext4_fs.superblock->s_inodes_count,
              ext4_fs.superblock->s_free_inodes_count);
        KINFO("Block size: %u bytes", ext4_fs.block_size);
        KINFO("Inode size: %u bytes", ext4_fs.inode_size);
        KINFO("Groups: %u", ext4_fs.num_groups);
        KINFO("Features: extents=%s, large_file=%s, dir_index=%s",
              ext4_fs.extents_enabled ? "yes" : "no",
              ext4_fs.large_file_enabled ? "yes" : "no",
              ext4_fs.dir_index_enabled ? "yes" : "no");
    }
    
    KINFO("Cache: %u inodes, %u blocks", inode_cache_size, block_cache_size);
}

// Mount ext4 file system
error_t ext4_mount(const char* device, mount_point_t* mount) {
    if (!ext4_initialized || !device || !mount) {
        return E_INVAL;
    }
    
    KDEBUG("Mounting ext4 file system on %s", device);
    
    // Read superblock
    superblock = memory_allocate_pages(1);
    if (!superblock) {
        KERROR("Failed to allocate memory for superblock");
        return E_NOMEM;
    }
    
    // Read superblock from device
    // TODO: Implement actual device reading
    memset(superblock, 0, 4096);
    
    // Validate superblock
    if (superblock->s_magic != EXT4_SUPER_MAGIC) {
        KERROR("Invalid ext4 superblock magic: 0x%04X", superblock->s_magic);
        memory_free_pages(superblock, 1);
        superblock = NULL;
        return E_INVAL;
    }
    
    // Initialize file system parameters
    block_size = 1024 << superblock->s_log_block_size;
    blocks_per_group = superblock->s_blocks_per_group;
    inodes_per_group = superblock->s_inodes_per_group;
    
    // Allocate block cache
    block_cache = memory_allocate_pages(block_size / 4096);
    if (!block_cache) {
        KERROR("Failed to allocate block cache");
        memory_free_pages(superblock, 1);
        superblock = NULL;
        return E_NOMEM;
    }
    
    // Set up mount point
    mount->fs_data = &ext4_fs;
    mount->fs_type = FS_TYPE_EXT4;
    
    KINFO("ext4 file system mounted: block_size=%u, blocks_per_group=%u, inodes_per_group=%u",
          block_size, blocks_per_group, inodes_per_group);
    
    return SUCCESS;
}

// Unmount ext4 file system
error_t ext4_umount(mount_point_t* mount) {
    if (!ext4_initialized || !mount) {
        return E_INVAL;
    }
    
    KDEBUG("Unmounting ext4 file system");
    
    // Free resources
    if (superblock) {
        memory_free_pages(superblock, 1);
        superblock = NULL;
    }
    
    if (block_cache) {
        memory_free_pages(block_cache, block_size / 4096);
        block_cache = NULL;
    }
    
    KINFO("ext4 file system unmounted");
    
    return SUCCESS;
}

// Open file in ext4
error_t ext4_open(mount_point_t* mount, const char* path, int flags, mode_t mode, file_descriptor_t* file) {
    if (!mount || !path || !file) {
        return E_INVAL;
    }
    
    KDEBUG("ext4_open: %s, flags=%d", path, flags);
    
    // Find inode for path
    u32 inode_num = ext4_find_inode(path);
    if (inode_num == 0) {
        if (flags & FILE_FLAG_CREATE) {
            // Create new file
            inode_num = ext4_create_inode(path, mode);
            if (inode_num == 0) {
                return E_FAIL;
            }
        } else {
            return E_NOENT;
        }
    }
    
    // Read inode
    ext4_inode_t inode;
    if (ext4_read_inode(inode_num, &inode) != SUCCESS) {
        return E_IO;
    }
    
    // Check file type
    if ((inode.i_mode & 0xF000) != EXT4_S_IFREG) {
        return E_ISDIR;
    }
    
    // Set up file descriptor
    file->inode = inode_num;
    file->size = inode.i_size_lo | ((u64)inode.i_size_high << 32);
    file->position = 0;
    file->flags = flags;
    
    KDEBUG("ext4_open: inode=%u, size=%llu", inode_num, file->size);
    
    return SUCCESS;
}

// Close file in ext4
error_t ext4_close(file_descriptor_t* file) {
    if (!file) {
        return E_INVAL;
    }
    
    KDEBUG("ext4_close: inode=%u", file->inode);
    
    // TODO: Update inode if file was modified
    
    return SUCCESS;
}

// Read from ext4 file
ssize_t ext4_read(file_descriptor_t* file, void* buffer, size_t count) {
    if (!file || !buffer) {
        return -EINVAL;
    }
    
    KDEBUG("ext4_read: inode=%u, count=%zu, position=%llu", file->inode, count, file->position);
    
    // Check bounds
    if (file->position >= file->size) {
        return 0; // EOF
    }
    
    size_t bytes_to_read = count;
    if (file->position + bytes_to_read > file->size) {
        bytes_to_read = file->size - file->position;
    }
    
    // Read data blocks
    size_t bytes_read = 0;
    u64 position = file->position;
    
    while (bytes_read < bytes_to_read) {
        u32 block_num = position / block_size;
        u32 block_offset = position % block_size;
        u32 bytes_in_block = block_size - block_offset;
        
        if (bytes_in_block > bytes_to_read - bytes_read) {
            bytes_in_block = bytes_to_read - bytes_read;
        }
        
        // Read block
        void* block_data = ext4_read_block(block_num);
        if (!block_data) {
            break;
        }
        
        // Copy data
        memcpy((u8*)buffer + bytes_read, (u8*)block_data + block_offset, bytes_in_block);
        
        bytes_read += bytes_in_block;
        position += bytes_in_block;
    }
    
    file->position += bytes_read;
    
    KDEBUG("ext4_read: read %zu bytes", bytes_read);
    
    return bytes_read;
}

// Write to ext4 file
ssize_t ext4_write(file_descriptor_t* file, const void* buffer, size_t count) {
    if (!file || !buffer) {
        return -EINVAL;
    }
    
    KDEBUG("ext4_write: inode=%u, count=%zu, position=%llu", file->inode, count, file->position);
    
    // TODO: Implement file writing
    // This requires implementing block allocation and inode updates
    
    KDEBUG("ext4_write: write not yet implemented");
    
    return count; // Placeholder
}

// Seek in ext4 file
off_t ext4_lseek(file_descriptor_t* file, off_t offset, int whence) {
    if (!file) {
        return -EINVAL;
    }
    
    off_t new_position;
    
    switch (whence) {
        case SEEK_SET:
            new_position = offset;
            break;
        case SEEK_CUR:
            new_position = file->position + offset;
            break;
        case SEEK_END:
            new_position = file->size + offset;
            break;
        default:
            return -EINVAL;
    }
    
    if (new_position < 0) {
        return -EINVAL;
    }
    
    file->position = new_position;
    
    KDEBUG("ext4_lseek: new position=%llu", file->position);
    
    return new_position;
}

// Get file status in ext4
error_t ext4_stat(mount_point_t* mount, const char* path, struct stat* statbuf) {
    if (!mount || !path || !statbuf) {
        return E_INVAL;
    }
    
    KDEBUG("ext4_stat: %s", path);
    
    // Find inode
    u32 inode_num = ext4_find_inode(path);
    if (inode_num == 0) {
        return E_NOENT;
    }
    
    // Read inode
    ext4_inode_t inode;
    if (ext4_read_inode(inode_num, &inode) != SUCCESS) {
        return E_IO;
    }
    
    // Fill stat structure
    memset(statbuf, 0, sizeof(struct stat));
    statbuf->st_ino = inode_num;
    statbuf->st_mode = inode.i_mode;
    statbuf->st_uid = inode.i_uid;
    statbuf->st_gid = inode.i_gid;
    statbuf->st_size = inode.i_size_lo | ((u64)inode.i_size_high << 32);
    statbuf->st_atime = inode.i_atime;
    statbuf->st_mtime = inode.i_mtime;
    statbuf->st_ctime = inode.i_ctime;
    statbuf->st_nlink = inode.i_links_count;
    statbuf->st_blocks = inode.i_blocks_lo;
    
    KDEBUG("ext4_stat: inode=%u, size=%llu, mode=0x%04X", 
           inode_num, statbuf->st_size, statbuf->st_mode);
    
    return SUCCESS;
}

// Create directory in ext4
error_t ext4_mkdir(mount_point_t* mount, const char* path, mode_t mode) {
    if (!mount || !path) {
        return E_INVAL;
    }
    
    KDEBUG("ext4_mkdir: %s, mode=0x%04X", path, mode);
    
    // TODO: Implement directory creation
    // This requires implementing inode allocation and directory entry creation
    
    KDEBUG("ext4_mkdir: not yet implemented");
    
    return SUCCESS; // Placeholder
}

// Remove directory in ext4
error_t ext4_rmdir(mount_point_t* mount, const char* path) {
    if (!mount || !path) {
        return E_INVAL;
    }
    
    KDEBUG("ext4_rmdir: %s", path);
    
    // TODO: Implement directory removal
    // This requires checking if directory is empty and removing inode
    
    KDEBUG("ext4_rmdir: not yet implemented");
    
    return SUCCESS; // Placeholder
}

// Remove file in ext4
error_t ext4_unlink(mount_point_t* mount, const char* path) {
    if (!mount || !path) {
        return E_INVAL;
    }
    
    KDEBUG("ext4_unlink: %s", path);
    
    // TODO: Implement file removal
    // This requires removing directory entry and freeing inode
    
    KDEBUG("ext4_unlink: not yet implemented");
    
    return SUCCESS; // Placeholder
}

// Rename file in ext4
error_t ext4_rename(mount_point_t* mount, const char* oldpath, const char* newpath) {
    if (!mount || !oldpath || !newpath) {
        return E_INVAL;
    }
    
    KDEBUG("ext4_rename: %s -> %s", oldpath, newpath);
    
    // TODO: Implement file renaming
    // This requires updating directory entries
    
    KDEBUG("ext4_rename: not yet implemented");
    
    return SUCCESS; // Placeholder
}

// Find inode for path
u32 ext4_find_inode(const char* path) {
    if (!path || !superblock) {
        return 0;
    }
    
    KDEBUG("ext4_find_inode: %s", path);
    
    // Start from root inode (inode 2)
    u32 current_inode = 2;
    
    // Parse path components
    char* path_copy = strdup(path);
    char* component = strtok(path_copy, "/");
    
    while (component) {
        // Read current inode
        ext4_inode_t inode;
        if (ext4_read_inode(current_inode, &inode) != SUCCESS) {
            free(path_copy);
            return 0;
        }
        
        // Check if it's a directory
        if ((inode.i_mode & 0xF000) != EXT4_S_IFDIR) {
            free(path_copy);
            return 0;
        }
        
        // Search for component in directory
        u32 found_inode = ext4_find_in_directory(&inode, component);
        if (found_inode == 0) {
            free(path_copy);
            return 0;
        }
        
        current_inode = found_inode;
        component = strtok(NULL, "/");
    }
    
    free(path_copy);
    return current_inode;
}

// Find inode in directory
u32 ext4_find_in_directory(ext4_inode_t* dir_inode, const char* name) {
    if (!dir_inode || !name) {
        return 0;
    }
    
    KDEBUG("ext4_find_in_directory: %s", name);
    
    // Read directory blocks
    u64 dir_size = dir_inode->i_size_lo | ((u64)dir_inode->i_size_high << 32);
    u64 position = 0;
    
    while (position < dir_size) {
        u32 block_num = position / block_size;
        void* block_data = ext4_read_block(block_num);
        if (!block_data) {
            break;
        }
        
        // Parse directory entries
        u32 block_offset = 0;
        while (block_offset < block_size && position < dir_size) {
            ext4_dir_entry_t* entry = (ext4_dir_entry_t*)((u8*)block_data + block_offset);
            
            if (entry->inode != 0 && entry->name_len > 0) {
                // Compare name
                if (entry->name_len == strlen(name) && 
                    memcmp(entry->name, name, entry->name_len) == 0) {
                    KDEBUG("ext4_find_in_directory: found inode %u", entry->inode);
                    return entry->inode;
                }
            }
            
            if (entry->rec_len == 0) {
                break;
            }
            
            block_offset += entry->rec_len;
            position += entry->rec_len;
        }
    }
    
    return 0;
}

// Create inode
u32 ext4_create_inode(const char* path, mode_t mode) {
    if (!path || !superblock) {
        return 0;
    }
    
    KDEBUG("ext4_create_inode: %s, mode=0x%04X", path, mode);
    
    // TODO: Implement inode creation
    // This requires finding a free inode and initializing it
    
    KDEBUG("ext4_create_inode: not yet implemented");
    
    return 0;
}

// Get ext4 state
ext4_fs_t* ext4_get_state(void) {
    return &ext4_fs;
}

// Check if ext4 is initialized
bool ext4_is_initialized(void) {
    return ext4_initialized;
}

// Shutdown ext4
void ext4_shutdown(void) {
    if (!ext4_initialized) {
        return;
    }
    
    KINFO("Shutting down ext4 file system...");
    
    // Free resources
    if (superblock) {
        memory_free_pages(superblock, 1);
        superblock = NULL;
    }
    
    if (block_cache) {
        memory_free_pages(block_cache, block_size / 4096);
        block_cache = NULL;
    }
    
    ext4_initialized = false;
    ext4_fs.active = false;
    
    KINFO("ext4 file system shutdown complete");
} 