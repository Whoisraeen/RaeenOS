#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "../../gui/rendering/glass_compositor.h"
#include "../../gui/window/glass_window_manager.h"
#include <stdint.h>
#include <stdbool.h>

// File Manager - Advanced file management for RaeenOS
// Provides modern file operations, search, and organization features

// File types
typedef enum {
    FILE_TYPE_UNKNOWN = 0,       // Unknown file type
    FILE_TYPE_FOLDER,            // Directory/folder
    FILE_TYPE_DOCUMENT,          // Document file
    FILE_TYPE_IMAGE,             // Image file
    FILE_TYPE_VIDEO,             // Video file
    FILE_TYPE_AUDIO,             // Audio file
    FILE_TYPE_ARCHIVE,           // Archive/compressed file
    FILE_TYPE_EXECUTABLE,        // Executable file
    FILE_TYPE_SYSTEM,            // System file
    FILE_TYPE_HIDDEN,            // Hidden file
    FILE_TYPE_SYMLINK,           // Symbolic link
    FILE_TYPE_SHORTCUT,          // Shortcut/link
} file_type_t;

// File attributes
typedef enum {
    FILE_ATTR_READONLY = 0x01,   // Read-only
    FILE_ATTR_HIDDEN = 0x02,     // Hidden
    FILE_ATTR_SYSTEM = 0x04,     // System file
    FILE_ATTR_ARCHIVE = 0x08,    // Archive
    FILE_ATTR_COMPRESSED = 0x10, // Compressed
    FILE_ATTR_ENCRYPTED = 0x20,  // Encrypted
    FILE_ATTR_OFFLINE = 0x40,    // Offline
    FILE_ATTR_TEMPORARY = 0x80,  // Temporary
} file_attributes_t;

// File permissions
typedef enum {
    FILE_PERM_READ = 0x01,       // Read permission
    FILE_PERM_WRITE = 0x02,      // Write permission
    FILE_PERM_EXECUTE = 0x04,    // Execute permission
    FILE_PERM_DELETE = 0x08,     // Delete permission
    FILE_PERM_MODIFY = 0x10,     // Modify permission
    FILE_PERM_OWNER = 0x20,      // Owner permission
} file_permissions_t;

// View modes
typedef enum {
    VIEW_MODE_ICONS = 0,         // Large icons
    VIEW_MODE_LIST,              // List view
    VIEW_MODE_DETAILS,           // Details view
    VIEW_MODE_TILES,             // Tiles view
    VIEW_MODE_THUMBNAILS,        // Thumbnails
    VIEW_MODE_PREVIEW,           // Preview pane
} view_mode_t;

// Sort modes
typedef enum {
    SORT_MODE_NAME = 0,          // Sort by name
    SORT_MODE_TYPE,              // Sort by type
    SORT_MODE_SIZE,              // Sort by size
    SORT_MODE_DATE_MODIFIED,     // Sort by modification date
    SORT_MODE_DATE_CREATED,      // Sort by creation date
    SORT_MODE_DATE_ACCESSED,     // Sort by access date
    SORT_MODE_EXTENSION,         // Sort by extension
    SORT_MODE_OWNER,             // Sort by owner
} sort_mode_t;

// File information
typedef struct {
    char name[256];              // File name
    char path[512];              // Full path
    char extension[16];          // File extension
    file_type_t type;            // File type
    uint64_t size;               // File size in bytes
    uint64_t size_on_disk;       // Size on disk
    uint32_t attributes;         // File attributes
    uint32_t permissions;        // File permissions
    uint64_t date_created;       // Creation date
    uint64_t date_modified;      // Modification date
    uint64_t date_accessed;      // Access date
    char owner[64];              // File owner
    char group[64];              // File group
    bool is_selected;            // Is selected
    bool is_visible;             // Is visible
    raeen_texture_t* icon;       // File icon
    raeen_texture_t* thumbnail;  // File thumbnail
    void* user_data;             // User-specific data
} file_info_t;

// Directory information
typedef struct {
    char path[512];              // Directory path
    char name[256];              // Directory name
    uint32_t file_count;         // Number of files
    uint32_t folder_count;       // Number of folders
    uint64_t total_size;         // Total size of contents
    uint64_t date_created;       // Creation date
    uint64_t date_modified;      // Modification date
    uint32_t attributes;         // Directory attributes
    uint32_t permissions;        // Directory permissions
    char owner[64];              // Directory owner
    char group[64];              // Directory group
    bool is_expanded;            // Is expanded in tree view
    bool is_selected;            // Is selected
    file_info_t* files;          // Array of files
    uint32_t file_array_size;    // Size of file array
} directory_info_t;

// Search criteria
typedef struct {
    char query[256];             // Search query
    char location[512];          // Search location
    bool search_subfolders;      // Search in subfolders
    bool case_sensitive;         // Case sensitive search
    bool use_regex;              // Use regular expressions
    file_type_t* file_types;     // File types to search
    uint32_t file_type_count;    // Number of file types
    uint64_t min_size;           // Minimum file size
    uint64_t max_size;           // Maximum file size
    uint64_t date_from;          // Date range from
    uint64_t date_to;            // Date range to
    char* content_pattern;       // Content pattern
    bool search_content;         // Search file contents
} search_criteria_t;

// File operation types
typedef enum {
    FILE_OP_COPY = 0,            // Copy files
    FILE_OP_MOVE,                // Move files
    FILE_OP_DELETE,              // Delete files
    FILE_OP_RENAME,              // Rename file
    FILE_OP_CREATE_FOLDER,       // Create folder
    FILE_OP_CREATE_FILE,         // Create file
    FILE_OP_COMPRESS,            // Compress files
    FILE_OP_EXTRACT,             // Extract archive
    FILE_OP_ENCRYPT,             // Encrypt files
    FILE_OP_DECRYPT,             // Decrypt files
    FILE_OP_BACKUP,              // Backup files
    FILE_OP_RESTORE,             // Restore files
} file_operation_t;

// File operation progress
typedef struct {
    file_operation_t type;       // Operation type
    char source[512];            // Source path
    char destination[512];       // Destination path
    uint64_t total_size;         // Total size to process
    uint64_t processed_size;     // Processed size
    uint32_t total_files;        // Total files to process
    uint32_t processed_files;    // Processed files
    bool is_cancelled;           // Is operation cancelled
    bool is_paused;              // Is operation paused
    float progress;              // Progress percentage
    char current_file[256];      // Current file being processed
    uint64_t start_time;         // Operation start time
    uint64_t estimated_time;     // Estimated completion time
} file_operation_progress_t;

// File manager configuration
typedef struct {
    uint32_t window_width, window_height;
    view_mode_t default_view_mode;
    sort_mode_t default_sort_mode;
    bool show_hidden_files;
    bool show_system_files;
    bool show_file_extensions;
    bool show_file_sizes;
    bool show_file_dates;
    bool show_file_attributes;
    bool enable_thumbnails;
    bool enable_preview;
    bool enable_search;
    bool enable_drag_drop;
    bool enable_context_menus;
    bool enable_keyboard_shortcuts;
    uint32_t max_recent_files;
    uint32_t max_search_results;
    uint32_t thumbnail_size;
    uint32_t icon_size;
} file_manager_config_t;

// File manager context
typedef struct {
    file_manager_config_t config;
    glass_compositor_t* compositor;
    glass_window_manager_t* window_manager;
    uint32_t window_id;
    char current_path[512];      // Current directory
    directory_info_t* current_directory;
    file_info_t* selected_files;
    uint32_t selected_count;
    uint32_t max_selected;
    view_mode_t view_mode;
    sort_mode_t sort_mode;
    bool show_hidden;
    bool show_system;
    search_criteria_t* search_criteria;
    uint32_t search_result_count;
    file_info_t* search_results;
    uint32_t max_search_results;
    file_operation_progress_t* current_operation;
    bool initialized;
    uint64_t last_refresh_time;
    uint32_t next_file_id;
} file_manager_t;

// Function prototypes

// Initialization and shutdown
file_manager_t* file_manager_init(glass_compositor_t* compositor, 
                                 glass_window_manager_t* window_manager,
                                 file_manager_config_t* config);
void file_manager_shutdown(file_manager_t* fm);
bool file_manager_is_initialized(file_manager_t* fm);

// Window management
void file_manager_show(file_manager_t* fm);
void file_manager_hide(file_manager_t* fm);
void file_manager_minimize(file_manager_t* fm);
void file_manager_maximize(file_manager_t* fm);
void file_manager_restore(file_manager_t* fm);
void file_manager_close(file_manager_t* fm);

// Navigation
bool file_manager_navigate_to(file_manager_t* fm, const char* path);
bool file_manager_navigate_up(file_manager_t* fm);
bool file_manager_navigate_back(file_manager_t* fm);
bool file_manager_navigate_forward(file_manager_t* fm);
bool file_manager_navigate_home(file_manager_t* fm);
bool file_manager_navigate_desktop(file_manager_t* fm);
bool file_manager_navigate_documents(file_manager_t* fm);
bool file_manager_navigate_downloads(file_manager_t* fm);
bool file_manager_navigate_pictures(file_manager_t* fm);
bool file_manager_navigate_music(file_manager_t* fm);
bool file_manager_navigate_videos(file_manager_t* fm);
const char* file_manager_get_current_path(file_manager_t* fm);
directory_info_t* file_manager_get_current_directory(file_manager_t* fm);

// File operations
bool file_manager_copy_files(file_manager_t* fm, const char** source_paths, uint32_t count, const char* dest_path);
bool file_manager_move_files(file_manager_t* fm, const char** source_paths, uint32_t count, const char* dest_path);
bool file_manager_delete_files(file_manager_t* fm, const char** paths, uint32_t count);
bool file_manager_rename_file(file_manager_t* fm, const char* old_path, const char* new_name);
bool file_manager_create_folder(file_manager_t* fm, const char* name);
bool file_manager_create_file(file_manager_t* fm, const char* name);
bool file_manager_compress_files(file_manager_t* fm, const char** source_paths, uint32_t count, const char* archive_path);
bool file_manager_extract_archive(file_manager_t* fm, const char* archive_path, const char* dest_path);
bool file_manager_encrypt_files(file_manager_t* fm, const char** source_paths, uint32_t count, const char* password);
bool file_manager_decrypt_files(file_manager_t* fm, const char** source_paths, uint32_t count, const char* password);

// File selection
void file_manager_select_file(file_manager_t* fm, const char* path, bool select);
void file_manager_select_files(file_manager_t* fm, const char** paths, uint32_t count, bool select);
void file_manager_select_all(file_manager_t* fm);
void file_manager_deselect_all(file_manager_t* fm);
void file_manager_invert_selection(file_manager_t* fm);
file_info_t* file_manager_get_selected_files(file_manager_t* fm, uint32_t* count);
uint32_t file_manager_get_selected_count(file_manager_t* fm);

// View and display
void file_manager_set_view_mode(file_manager_t* fm, view_mode_t mode);
view_mode_t file_manager_get_view_mode(file_manager_t* fm);
void file_manager_set_sort_mode(file_manager_t* fm, sort_mode_t mode);
sort_mode_t file_manager_get_sort_mode(file_manager_t* fm);
void file_manager_toggle_hidden_files(file_manager_t* fm);
void file_manager_toggle_system_files(file_manager_t* fm);
void file_manager_toggle_file_extensions(file_manager_t* fm);
void file_manager_refresh(file_manager_t* fm);
void file_manager_auto_refresh(file_manager_t* fm, bool enable);

// Search functionality
bool file_manager_start_search(file_manager_t* fm, search_criteria_t* criteria);
bool file_manager_stop_search(file_manager_t* fm);
bool file_manager_is_searching(file_manager_t* fm);
file_info_t* file_manager_get_search_results(file_manager_t* fm, uint32_t* count);
void file_manager_clear_search_results(file_manager_t* fm);
bool file_manager_search_by_name(file_manager_t* fm, const char* query);
bool file_manager_search_by_content(file_manager_t* fm, const char* query);
bool file_manager_search_by_type(file_manager_t* fm, file_type_t type);
bool file_manager_search_by_size(file_manager_t* fm, uint64_t min_size, uint64_t max_size);
bool file_manager_search_by_date(file_manager_t* fm, uint64_t from_date, uint64_t to_date);

// File information
file_info_t* file_manager_get_file_info(file_manager_t* fm, const char* path);
bool file_manager_get_file_size(file_manager_t* fm, const char* path, uint64_t* size);
bool file_manager_get_file_type(file_manager_t* fm, const char* path, file_type_t* type);
bool file_manager_get_file_attributes(file_manager_t* fm, const char* path, uint32_t* attributes);
bool file_manager_set_file_attributes(file_manager_t* fm, const char* path, uint32_t attributes);
bool file_manager_get_file_permissions(file_manager_t* fm, const char* path, uint32_t* permissions);
bool file_manager_set_file_permissions(file_manager_t* fm, const char* path, uint32_t permissions);

// Thumbnails and previews
raeen_texture_t* file_manager_get_file_thumbnail(file_manager_t* fm, const char* path);
raeen_texture_t* file_manager_get_file_icon(file_manager_t* fm, const char* path);
bool file_manager_generate_thumbnail(file_manager_t* fm, const char* path);
void file_manager_clear_thumbnail_cache(file_manager_t* fm);
bool file_manager_show_file_preview(file_manager_t* fm, const char* path);

// Drag and drop
bool file_manager_start_drag(file_manager_t* fm, const char** paths, uint32_t count);
bool file_manager_handle_drop(file_manager_t* fm, const char** paths, uint32_t count, const char* dest_path);
bool file_manager_is_dragging(file_manager_t* fm);
void file_manager_cancel_drag(file_manager_t* fm);

// Context menus
void file_manager_show_context_menu(file_manager_t* fm, float x, float y);
void file_manager_show_file_context_menu(file_manager_t* fm, const char* path, float x, float y);
void file_manager_show_selection_context_menu(file_manager_t* fm, float x, float y);

// Keyboard shortcuts
bool file_manager_handle_key_press(file_manager_t* fm, uint32_t key_code);
bool file_manager_handle_shortcut(file_manager_t* fm, uint32_t key_code, uint32_t modifiers);

// Progress monitoring
file_operation_progress_t* file_manager_get_operation_progress(file_manager_t* fm);
bool file_manager_cancel_operation(file_manager_t* fm);
bool file_manager_pause_operation(file_manager_t* fm);
bool file_manager_resume_operation(file_manager_t* fm);
float file_manager_get_operation_progress_percentage(file_manager_t* fm);

// Recent files
void file_manager_add_recent_file(file_manager_t* fm, const char* path);
char** file_manager_get_recent_files(file_manager_t* fm, uint32_t* count);
void file_manager_clear_recent_files(file_manager_t* fm);

// Favorites
void file_manager_add_favorite(file_manager_t* fm, const char* path, const char* name);
void file_manager_remove_favorite(file_manager_t* fm, const char* path);
char** file_manager_get_favorites(file_manager_t* fm, uint32_t* count);

// Rendering
void file_manager_render(file_manager_t* fm);
void file_manager_render_file_list(file_manager_t* fm);
void file_manager_render_toolbar(file_manager_t* fm);
void file_manager_render_status_bar(file_manager_t* fm);
void file_manager_render_progress_dialog(file_manager_t* fm);

// Input handling
bool file_manager_handle_mouse_move(file_manager_t* fm, float x, float y);
bool file_manager_handle_mouse_click(file_manager_t* fm, float x, float y, bool left_click);
bool file_manager_handle_mouse_double_click(file_manager_t* fm, float x, float y);
bool file_manager_handle_mouse_drag(file_manager_t* fm, float x, float y, bool start_drag);
bool file_manager_handle_mouse_wheel(file_manager_t* fm, float x, float y, float delta);

// Utility functions
bool file_manager_is_path_valid(const char* path);
bool file_manager_is_file_readable(const char* path);
bool file_manager_is_file_writable(const char* path);
bool file_manager_is_file_executable(const char* path);
char* file_manager_get_file_extension(const char* path);
char* file_manager_get_file_name(const char* path);
char* file_manager_get_directory_name(const char* path);
char* file_manager_combine_paths(const char* path1, const char* path2);
bool file_manager_path_exists(const char* path);
bool file_manager_is_directory(const char* path);
bool file_manager_is_file(const char* path);
uint64_t file_manager_get_directory_size(const char* path);
uint32_t file_manager_get_file_count(const char* path);

// Callbacks
typedef void (*file_operation_callback_t)(file_manager_t* fm, file_operation_t operation, const char* path, void* user_data);
typedef void (*file_selection_callback_t)(file_manager_t* fm, const char* path, bool selected, void* user_data);
typedef void (*file_navigation_callback_t)(file_manager_t* fm, const char* old_path, const char* new_path, void* user_data);

void file_manager_set_operation_callback(file_manager_t* fm, file_operation_callback_t callback, void* user_data);
void file_manager_set_selection_callback(file_manager_t* fm, file_selection_callback_t callback, void* user_data);
void file_manager_set_navigation_callback(file_manager_t* fm, file_navigation_callback_t callback, void* user_data);

// Preset configurations
file_manager_config_t file_manager_preset_normal_style(void);
file_manager_config_t file_manager_preset_compact_style(void);
file_manager_config_t file_manager_preset_detailed_style(void);

// Error handling
typedef enum {
    FILE_MANAGER_SUCCESS = 0,
    FILE_MANAGER_ERROR_INVALID_CONTEXT,
    FILE_MANAGER_ERROR_INVALID_PATH,
    FILE_MANAGER_ERROR_FILE_NOT_FOUND,
    FILE_MANAGER_ERROR_ACCESS_DENIED,
    FILE_MANAGER_ERROR_OUT_OF_MEMORY,
    FILE_MANAGER_ERROR_OPERATION_FAILED,
    FILE_MANAGER_ERROR_INVALID_OPERATION,
} file_manager_error_t;

file_manager_error_t file_manager_get_last_error(void);
const char* file_manager_get_error_string(file_manager_error_t error);

#endif // FILE_MANAGER_H 