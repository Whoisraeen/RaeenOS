#include "fs/vfs.h"
#include "kernel.h"
#include "memory/memory.h"
#include <string.h>

// Tar header structure
typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
} tar_header_t;

static vfs_node_t* initrd_root = NULL;
static u8* initrd_data = NULL;
static size_t initrd_size = 0;

static unsigned int get_tar_size(const char* in) {
    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;
    for (j = 11; j > 0; j--, count *= 8) {
        size += ((in[j - 1] - '0') * count);
    }
    return size;
}

static ssize_t initrd_read(vfs_node_t* node, uint64_t offset, size_t size, void* buffer) {
    tar_header_t* header = (tar_header_t*)node->fs_private;
    uintptr_t file_start = (uintptr_t)header + 512;
    size_t file_size = get_tar_size(header->size);

    if (offset >= file_size) {
        return 0; // End of file
    }
    if (offset + size > file_size) {
        size = file_size - offset;
    }

    memcpy(buffer, (void*)(file_start + offset), size);
    return size;
}

static vfs_node_t* initrd_finddir(vfs_node_t* node, const char* name) {
    // The initrd is flat, so we just search from the beginning
    tar_header_t* header = (tar_header_t*)initrd_data;
    while ((u8*)header < initrd_data + initrd_size && header->name[0] != '\0') {
        if (strcmp(header->name, name) == 0) {
            // Found the file. Populate the VFS node.
            vfs_node_t* found_node = memory_alloc(sizeof(vfs_node_t));
            if (!found_node) return NULL;

            memset(found_node, 0, sizeof(vfs_node_t));
            strncpy(found_node->name, header->name, sizeof(found_node->name) - 1);
            found_node->length = get_tar_size(header->size);
            found_node->ops = &initrd_ops;
            found_node->fs_private = header; // Store pointer to tar header
            if (header->typeflag == '5') {
                found_node->flags = VFS_DIRECTORY;
            } else {
                found_node->flags = VFS_FILE;
            }
            return found_node;
        }
        size_t size = get_tar_size(header->size);
        header = (tar_header_t*)((u8*)header + 512 + ((size + 511) & ~511));
    }
    return NULL; // Not found
}

static fs_ops_t initrd_ops = {
    .read = initrd_read,
    .finddir = initrd_finddir,
    // open, close, write are NULL as this is a read-only fs
};

error_t initrd_init(struct multiboot_info* mbi) {
    if (!(mbi->flags & MULTIBOOT_INFO_MODS)) {
        KWARN("No initrd module found by bootloader.");
        return E_NOENT;
    }

    multiboot_module_t* mod = (multiboot_module_t*)mbi->mods_addr;
    initrd_data = (u8*)mod->mod_start;
    initrd_size = mod->mod_end - mod->mod_start;

    KINFO("Found initrd at 0x%x, size %u bytes", initrd_data, initrd_size);

    // Mount the initrd at the VFS root
    vfs_mount("/", &initrd_ops, NULL);

    return SUCCESS;
}