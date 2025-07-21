#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_PATH 256
#define MAX_FILES 1000
#define MAX_FILENAME 256

typedef struct {
    char name[MAX_FILENAME];
    char path[MAX_PATH];
    bool is_directory;
    size_t size;
    time_t modified;
    mode_t permissions;
} file_entry_t;

typedef struct {
    char current_path[MAX_PATH];
    file_entry_t files[MAX_FILES];
    int file_count;
    int selected_index;
    int scroll_offset;
} file_manager_t;

// Initialize file manager
void file_manager_init(file_manager_t* fm) {
    strcpy(fm->current_path, "/");
    fm->file_count = 0;
    fm->selected_index = 0;
    fm->scroll_offset = 0;
}

// Get file information
int file_manager_get_file_info(const char* path, file_entry_t* entry) {
    struct stat st;
    
    if (stat(path, &st) != 0) {
        return -1;
    }
    
    strncpy(entry->path, path, MAX_PATH - 1);
    entry->path[MAX_PATH - 1] = '\0';
    
    // Extract filename from path
    const char* filename = strrchr(path, '/');
    if (filename) {
        filename++; // Skip the '/'
    } else {
        filename = path;
    }
    
    strncpy(entry->name, filename, MAX_FILENAME - 1);
    entry->name[MAX_FILENAME - 1] = '\0';
    
    entry->is_directory = S_ISDIR(st.st_mode);
    entry->size = st.st_size;
    entry->modified = st.st_mtime;
    entry->permissions = st.st_mode;
    
    return 0;
}

// Load directory contents
int file_manager_load_directory(file_manager_t* fm) {
    DIR* dir = opendir(fm->current_path);
    if (!dir) {
        printf("Error: Cannot open directory '%s'\n", fm->current_path);
        return -1;
    }
    
    fm->file_count = 0;
    fm->selected_index = 0;
    fm->scroll_offset = 0;
    
    // Add parent directory entry
    if (strcmp(fm->current_path, "/") != 0) {
        strcpy(fm->files[0].name, "..");
        strcpy(fm->files[0].path, fm->current_path);
        fm->files[0].is_directory = true;
        fm->files[0].size = 0;
        fm->files[0].modified = 0;
        fm->files[0].permissions = 0755;
        fm->file_count++;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && fm->file_count < MAX_FILES) {
        // Skip . and .. entries (we handle .. manually)
        if (strcmp(entry->d_name, ".") == 0) {
            continue;
        }
        
        // Build full path
        char full_path[MAX_PATH];
        snprintf(full_path, MAX_PATH, "%s/%s", fm->current_path, entry->d_name);
        
        // Get file information
        if (file_manager_get_file_info(full_path, &fm->files[fm->file_count]) == 0) {
            fm->file_count++;
        }
    }
    
    closedir(dir);
    return 0;
}

// Format file size
void file_manager_format_size(size_t size, char* buffer, size_t buffer_size) {
    if (size < 1024) {
        snprintf(buffer, buffer_size, "%zu B", size);
    } else if (size < 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f KB", size / 1024.0);
    } else if (size < 1024 * 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f MB", size / (1024.0 * 1024.0));
    } else {
        snprintf(buffer, buffer_size, "%.1f GB", size / (1024.0 * 1024.0 * 1024.0));
    }
}

// Format permissions
void file_manager_format_permissions(mode_t mode, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%c%c%c%c%c%c%c%c%c%c",
             S_ISDIR(mode) ? 'd' : '-',
             (mode & S_IRUSR) ? 'r' : '-',
             (mode & S_IWUSR) ? 'w' : '-',
             (mode & S_IXUSR) ? 'x' : '-',
             (mode & S_IRGRP) ? 'r' : '-',
             (mode & S_IWGRP) ? 'w' : '-',
             (mode & S_IXGRP) ? 'x' : '-',
             (mode & S_IROTH) ? 'r' : '-',
             (mode & S_IWOTH) ? 'w' : '-',
             (mode & S_IXOTH) ? 'x' : '-');
}

// Display file manager
void file_manager_display(file_manager_t* fm) {
    printf("\033[2J\033[H"); // Clear screen
    
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  RaeenOS File Manager v1.0                   ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Current Path: %-45s ║\n", fm->current_path);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Permissions  Size        Modified    Name                     ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    
    int display_count = 0;
    int max_display = 20; // Number of lines to display
    
    for (int i = fm->scroll_offset; i < fm->file_count && display_count < max_display; i++) {
        file_entry_t* entry = &fm->files[i];
        
        // Highlight selected item
        if (i == fm->selected_index) {
            printf("║ ");
        } else {
            printf("║ ");
        }
        
        // Format permissions
        char perm_str[11];
        file_manager_format_permissions(entry->permissions, perm_str, sizeof(perm_str));
        
        // Format size
        char size_str[12];
        if (entry->is_directory) {
            strcpy(size_str, "<DIR>");
        } else {
            file_manager_format_size(entry->size, size_str, sizeof(size_str));
        }
        
        // Format date
        char date_str[12];
        if (entry->modified > 0) {
            struct tm* tm_info = localtime(&entry->modified);
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);
        } else {
            strcpy(date_str, "N/A");
        }
        
        // Display entry
        printf("%-10s %-11s %-11s %-25s", perm_str, size_str, date_str, entry->name);
        
        if (i == fm->selected_index) {
            printf(" ←");
        }
        
        printf(" ║\n");
        display_count++;
    }
    
    // Fill remaining lines
    for (int i = display_count; i < max_display; i++) {
        printf("║                                                              ║\n");
    }
    
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Commands: Enter=Open, Backspace=Up, q=Quit, h=Help          ║\n");
    printf("║ Files: %d selected, %d total                                 ║\n", 
           fm->selected_index + 1, fm->file_count);
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

// Navigate to directory
int file_manager_navigate(file_manager_t* fm, const char* path) {
    if (!path) {
        return -1;
    }
    
    // Handle relative paths
    char new_path[MAX_PATH];
    if (path[0] == '/') {
        strncpy(new_path, path, MAX_PATH - 1);
    } else {
        snprintf(new_path, MAX_PATH, "%s/%s", fm->current_path, path);
    }
    
    // Normalize path
    // Remove trailing slash
    int len = strlen(new_path);
    if (len > 1 && new_path[len - 1] == '/') {
        new_path[len - 1] = '\0';
    }
    
    // Check if directory exists
    struct stat st;
    if (stat(new_path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        printf("Error: Directory '%s' does not exist\n", new_path);
        return -1;
    }
    
    strncpy(fm->current_path, new_path, MAX_PATH - 1);
    fm->current_path[MAX_PATH - 1] = '\0';
    
    return file_manager_load_directory(fm);
}

// Open file or directory
int file_manager_open(file_manager_t* fm) {
    if (fm->selected_index >= fm->file_count) {
        return -1;
    }
    
    file_entry_t* entry = &fm->files[fm->selected_index];
    
    if (entry->is_directory) {
        if (strcmp(entry->name, "..") == 0) {
            // Go to parent directory
            char* last_slash = strrchr(fm->current_path, '/');
            if (last_slash && last_slash != fm->current_path) {
                *last_slash = '\0';
            } else if (strcmp(fm->current_path, "/") != 0) {
                strcpy(fm->current_path, "/");
            }
        } else {
            // Enter directory
            char new_path[MAX_PATH];
            snprintf(new_path, MAX_PATH, "%s/%s", fm->current_path, entry->name);
            strncpy(fm->current_path, new_path, MAX_PATH - 1);
        }
        
        return file_manager_load_directory(fm);
    } else {
        // Open file (simple text viewer)
        printf("\nOpening file: %s\n", entry->path);
        printf("Press Enter to continue...\n");
        getchar();
        
        // TODO: Implement proper file viewer
        printf("File viewer not implemented yet.\n");
        return 0;
    }
}

// Handle input
int file_manager_handle_input(file_manager_t* fm) {
    char c;
    ssize_t bytes_read = read(STDIN_FILENO, &c, 1);
    
    if (bytes_read <= 0) {
        return 0;
    }
    
    switch (c) {
        case 'q':
        case 'Q':
            return -1; // Quit
            
        case 'h':
        case 'H':
            printf("\nFile Manager Help:\n");
            printf("Arrow keys: Navigate files\n");
            printf("Enter: Open file/directory\n");
            printf("Backspace: Go to parent directory\n");
            printf("q: Quit\n");
            printf("h: Show this help\n");
            printf("Press Enter to continue...\n");
            getchar();
            break;
            
        case 10: // Enter
            file_manager_open(fm);
            break;
            
        case 127: // Backspace
            {
                char* last_slash = strrchr(fm->current_path, '/');
                if (last_slash && last_slash != fm->current_path) {
                    *last_slash = '\0';
                    file_manager_load_directory(fm);
                }
            }
            break;
            
        case 27: // Escape sequence
            {
                char seq[2];
                read(STDIN_FILENO, seq, 2);
                if (seq[0] == '[') {
                    switch (seq[1]) {
                        case 'A': // Up arrow
                            if (fm->selected_index > 0) {
                                fm->selected_index--;
                                if (fm->selected_index < fm->scroll_offset) {
                                    fm->scroll_offset = fm->selected_index;
                                }
                            }
                            break;
                        case 'B': // Down arrow
                            if (fm->selected_index < fm->file_count - 1) {
                                fm->selected_index++;
                                if (fm->selected_index >= fm->scroll_offset + 20) {
                                    fm->scroll_offset = fm->selected_index - 19;
                                }
                            }
                            break;
                    }
                }
            }
            break;
    }
    
    return 0;
}

// Main function
int main(int argc, char** argv) {
    printf("RaeenOS File Manager v1.0\n");
    printf("A simple file manager for RaeenOS\n\n");
    
    file_manager_t fm;
    file_manager_init(&fm);
    
    // Set initial directory
    if (argc > 1) {
        if (file_manager_navigate(&fm, argv[1]) != 0) {
            printf("Failed to navigate to '%s', using root directory\n", argv[1]);
            file_manager_load_directory(&fm);
        }
    } else {
        file_manager_load_directory(&fm);
    }
    
    printf("Starting file manager...\n");
    printf("Press any key to continue...\n");
    getchar();
    
    // Main file manager loop
    while (1) {
        file_manager_display(&fm);
        
        if (file_manager_handle_input(&fm) != 0) {
            break;
        }
    }
    
    printf("File manager closed.\n");
    return 0;
} 