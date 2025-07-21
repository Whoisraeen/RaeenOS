#ifndef MODERN_UX_H
#define MODERN_UX_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// Modern UX constants
#define MAX_NOTIFICATIONS 100
#define MAX_SEARCH_RESULTS 50
#define MAX_VOICE_COMMANDS 200
#define MAX_ACCESSIBILITY_FEATURES 20
#define MAX_CLOUD_SERVICES 10
#define MAX_DEVICES 20
#define MAX_CLIPBOARD_ITEMS 10
#define MAX_WORKSPACES 10
#define MAX_APPS_IN_DOCK 20
#define MAX_START_MENU_ITEMS 100

// UI themes
typedef enum {
    UI_THEME_LIGHT = 0,
    UI_THEME_DARK = 1,
    UI_THEME_AUTO = 2,
    UI_THEME_CUSTOM = 3
} ui_theme_t;

// Notification types
typedef enum {
    NOTIFICATION_INFO = 0,
    NOTIFICATION_SUCCESS = 1,
    NOTIFICATION_WARNING = 2,
    NOTIFICATION_ERROR = 3,
    NOTIFICATION_UPDATE = 4,
    NOTIFICATION_SECURITY = 5
} notification_type_t;

// Search result types
typedef enum {
    SEARCH_RESULT_APP = 0,
    SEARCH_RESULT_FILE = 1,
    SEARCH_RESULT_SETTING = 2,
    SEARCH_RESULT_CONTACT = 3,
    SEARCH_RESULT_WEB = 4,
    SEARCH_RESULT_ACTION = 5
} search_result_type_t;

// Voice command types
typedef enum {
    VOICE_COMMAND_APP_LAUNCH = 0,
    VOICE_COMMAND_SYSTEM_CONTROL = 1,
    VOICE_COMMAND_SEARCH = 2,
    VOICE_COMMAND_NAVIGATION = 3,
    VOICE_COMMAND_CREATION = 4,
    VOICE_COMMAND_CUSTOM = 5
} voice_command_type_t;

// Accessibility feature types
typedef enum {
    ACCESSIBILITY_SCREEN_READER = 0,
    ACCESSIBILITY_MAGNIFIER = 1,
    ACCESSIBILITY_HIGH_CONTRAST = 2,
    ACCESSIBILITY_COLOR_BLIND = 3,
    ACCESSIBILITY_KEYBOARD_NAVIGATION = 4,
    ACCESSIBILITY_VOICE_CONTROL = 5,
    ACCESSIBILITY_CLOSED_CAPTIONS = 6,
    ACCESSIBILITY_SOUND_INDICATORS = 7
} accessibility_feature_t;

// Cloud service types
typedef enum {
    CLOUD_SERVICE_STORAGE = 0,
    CLOUD_SERVICE_SYNC = 1,
    CLOUD_SERVICE_BACKUP = 2,
    CLOUD_SERVICE_SHARING = 3,
    CLOUD_SERVICE_COLLABORATION = 4
} cloud_service_type_t;

// Device types
typedef enum {
    DEVICE_TYPE_PHONE = 0,
    DEVICE_TYPE_TABLET = 1,
    DEVICE_TYPE_LAPTOP = 2,
    DEVICE_TYPE_DESKTOP = 3,
    DEVICE_TYPE_SMARTWATCH = 4,
    DEVICE_TYPE_SMART_SPEAKER = 5
} device_type_t;

// Clipboard item types
typedef enum {
    CLIPBOARD_TEXT = 0,
    CLIPBOARD_IMAGE = 1,
    CLIPBOARD_FILE = 2,
    CLIPBOARD_LINK = 3,
    CLIPBOARD_FORMAT = 4
} clipboard_item_type_t;

// Workspace types
typedef enum {
    WORKSPACE_TYPE_DESKTOP = 0,
    WORKSPACE_TYPE_GAMING = 1,
    WORKSPACE_TYPE_CREATIVE = 2,
    WORKSPACE_TYPE_PRODUCTIVITY = 3,
    WORKSPACE_TYPE_ENTERTAINMENT = 4,
    WORKSPACE_TYPE_CUSTOM = 5
} workspace_type_t;

// Notification
typedef struct notification {
    uint32_t notification_id;                    // Notification identifier
    char title[128];                             // Notification title
    char message[512];                           // Notification message
    notification_type_t type;                    // Notification type
    bool active;                                 // Notification active
    bool dismissed;                              // Notification dismissed
    uint64_t created_time;                       // Creation time
    uint64_t expiry_time;                        // Expiry time
    uint32_t priority;                           // Priority (0-10)
    bool actionable;                             // Actionable notification
    char action_text[64];                        // Action text
    void* action_data;                           // Action data
    void* notification_data;                     // Notification-specific data
} notification_t;

// Search result
typedef struct search_result {
    uint32_t result_id;                          // Result identifier
    char title[128];                             // Result title
    char description[256];                       // Result description
    char path[512];                              // Result path
    search_result_type_t type;                   // Result type
    float relevance;                             // Relevance score (0.0-1.0)
    uint64_t last_accessed;                      // Last accessed time
    uint64_t file_size;                          // File size (if applicable)
    char icon_path[256];                         // Icon path
    bool executable;                             // Executable result
    void* result_data;                           // Result-specific data
} search_result_t;

// Voice command
typedef struct voice_command {
    uint32_t command_id;                         // Command identifier
    char name[64];                               // Command name
    char description[256];                       // Command description
    char trigger_phrase[128];                    // Trigger phrase
    voice_command_type_t type;                   // Command type
    bool enabled;                                // Command enabled
    bool active;                                 // Command active
    uint32_t usage_count;                        // Usage count
    uint64_t last_used;                          // Last used time
    void* action_function;                       // Action function pointer
    void* command_data;                          // Command-specific data
} voice_command_t;

// Accessibility feature
typedef struct accessibility_feature {
    uint32_t feature_id;                         // Feature identifier
    char name[64];                               // Feature name
    char description[256];                       // Feature description
    accessibility_feature_t type;                // Feature type
    bool enabled;                                // Feature enabled
    bool active;                                 // Feature active
    uint32_t intensity;                          // Feature intensity (0-100)
    uint32_t speed;                              // Feature speed (0-100)
    char settings[512];                          // Feature settings
    void* feature_data;                          // Feature-specific data
} accessibility_feature_t;

// Cloud service
typedef struct cloud_service {
    uint32_t service_id;                         // Service identifier
    char name[64];                               // Service name
    char description[256];                       // Service description
    cloud_service_type_t type;                   // Service type
    char server_url[256];                        // Server URL
    bool connected;                              // Connected
    bool authenticated;                          // Authenticated
    uint64_t storage_used;                       // Storage used (bytes)
    uint64_t storage_total;                      // Total storage (bytes)
    uint64_t last_sync;                          // Last sync time
    bool auto_sync;                              // Auto sync enabled
    void* service_data;                          // Service-specific data
} cloud_service_t;

// Device
typedef struct device {
    uint32_t device_id;                          // Device identifier
    char name[64];                               // Device name
    char model[64];                              // Device model
    device_type_t type;                          // Device type
    char os_version[32];                         // OS version
    bool connected;                              // Connected
    bool authenticated;                          // Authenticated
    uint64_t last_seen;                          // Last seen time
    uint32_t battery_level;                      // Battery level (0-100)
    bool charging;                               // Charging
    char ip_address[16];                         // IP address
    void* device_data;                           // Device-specific data
} device_t;

// Clipboard item
typedef struct clipboard_item {
    uint32_t item_id;                            // Item identifier
    clipboard_item_type_t type;                  // Item type
    char title[128];                             // Item title
    char content[1024];                          // Item content
    uint64_t created_time;                       // Creation time
    uint64_t last_used;                          // Last used time
    bool synced;                                 // Synced across devices
    uint32_t size;                               // Item size
    void* item_data;                             // Item-specific data
} clipboard_item_t;

// Workspace
typedef struct workspace {
    uint32_t workspace_id;                       // Workspace identifier
    char name[64];                               // Workspace name
    char description[256];                       // Workspace description
    workspace_type_t type;                       // Workspace type
    bool active;                                 // Workspace active
    bool visible;                                // Workspace visible
    uint32_t app_count;                          // Number of apps
    uint32_t window_count;                       // Number of windows
    uint64_t created_time;                       // Creation time
    uint64_t last_used;                          // Last used time
    void* workspace_data;                        // Workspace-specific data
} workspace_t;

// Start menu item
typedef struct start_menu_item {
    uint32_t item_id;                            // Item identifier
    char name[64];                               // Item name
    char description[256];                       // Item description
    char executable[512];                        // Executable path
    char icon_path[256];                         // Icon path
    bool pinned;                                 // Pinned to start menu
    bool recently_used;                          // Recently used
    uint64_t last_used;                          // Last used time
    uint32_t usage_count;                        // Usage count
    void* item_data;                             // Item-specific data
} start_menu_item_t;

// Dock item
typedef struct dock_item {
    uint32_t item_id;                            // Item identifier
    char name[64];                               // Item name
    char description[256];                       // Item description
    char executable[512];                        // Executable path
    char icon_path[256];                         // Icon path
    bool running;                                // Application running
    bool pinned;                                 // Pinned to dock
    uint32_t badge_count;                        // Badge count
    char badge_text[16];                         // Badge text
    uint64_t last_used;                          // Last used time
    void* item_data;                             // Item-specific data
} dock_item_t;

// Modern UX system
typedef struct modern_ux_system {
    spinlock_t lock;                             // System lock
    bool initialized;                            // Initialization flag
    
    // Theme management
    ui_theme_t current_theme;                    // Current theme
    bool auto_theme;                             // Auto theme switching
    uint32_t theme_switch_time;                  // Theme switch time (hour)
    
    // Notification management
    notification_t notifications[MAX_NOTIFICATIONS]; // Notifications
    uint32_t notification_count;                 // Number of notifications
    bool notification_center_enabled;            // Notification center enabled
    bool notification_sounds_enabled;            // Notification sounds enabled
    bool notification_toasts_enabled;            // Notification toasts enabled
    
    // Search management
    search_result_t search_results[MAX_SEARCH_RESULTS]; // Search results
    uint32_t search_result_count;                // Number of search results
    bool search_spotlight_enabled;               // Search spotlight enabled
    bool search_indexing_enabled;                // Search indexing enabled
    bool search_web_enabled;                     // Web search enabled
    char last_search_query[256];                 // Last search query
    
    // Voice management
    voice_command_t voice_commands[MAX_VOICE_COMMANDS]; // Voice commands
    uint32_t voice_command_count;                // Number of voice commands
    bool voice_assistant_enabled;                // Voice assistant enabled
    bool voice_recognition_enabled;              // Voice recognition enabled
    bool voice_synthesis_enabled;                // Voice synthesis enabled
    char wake_word[32];                          // Wake word
    
    // Accessibility management
    accessibility_feature_t accessibility_features[MAX_ACCESSIBILITY_FEATURES]; // Accessibility features
    uint32_t accessibility_feature_count;        // Number of accessibility features
    bool accessibility_enabled;                  // Accessibility enabled
    bool high_contrast_enabled;                  // High contrast enabled
    bool screen_reader_enabled;                  // Screen reader enabled
    bool magnifier_enabled;                      // Magnifier enabled
    
    // Cloud management
    cloud_service_t cloud_services[MAX_CLOUD_SERVICES]; // Cloud services
    uint32_t cloud_service_count;                // Number of cloud services
    bool cloud_sync_enabled;                     // Cloud sync enabled
    bool cloud_backup_enabled;                   // Cloud backup enabled
    bool cloud_sharing_enabled;                  // Cloud sharing enabled
    
    // Device management
    device_t devices[MAX_DEVICES];               // Connected devices
    uint32_t device_count;                       // Number of devices
    bool device_sync_enabled;                    // Device sync enabled
    bool universal_clipboard_enabled;            // Universal clipboard enabled
    bool handoff_enabled;                        // Handoff enabled
    
    // Clipboard management
    clipboard_item_t clipboard_items[MAX_CLIPBOARD_ITEMS]; // Clipboard items
    uint32_t clipboard_item_count;               // Number of clipboard items
    uint32_t clipboard_index;                    // Current clipboard index
    bool clipboard_sync_enabled;                 // Clipboard sync enabled
    bool clipboard_history_enabled;              // Clipboard history enabled
    
    // Workspace management
    workspace_t workspaces[MAX_WORKSPACES];      // Workspaces
    uint32_t workspace_count;                    // Number of workspaces
    workspace_t* active_workspace;               // Active workspace
    bool workspace_switching_enabled;            // Workspace switching enabled
    bool workspace_animations_enabled;           // Workspace animations enabled
    
    // Start menu management
    start_menu_item_t start_menu_items[MAX_START_MENU_ITEMS]; // Start menu items
    uint32_t start_menu_item_count;              // Number of start menu items
    bool start_menu_enabled;                     // Start menu enabled
    bool start_menu_search_enabled;              // Start menu search enabled
    bool start_menu_recent_enabled;              // Start menu recent enabled
    
    // Dock management
    dock_item_t dock_items[MAX_APPS_IN_DOCK];    // Dock items
    uint32_t dock_item_count;                    // Number of dock items
    bool dock_enabled;                           // Dock enabled
    bool dock_auto_hide;                         // Dock auto hide
    bool dock_animations_enabled;                // Dock animations enabled
    uint32_t dock_position;                      // Dock position (0=bottom, 1=left, 2=right)
    
    // Mission control
    bool mission_control_enabled;                // Mission control enabled
    bool mission_control_gestures_enabled;       // Mission control gestures enabled
    bool mission_control_hot_corners_enabled;    // Mission control hot corners enabled
    
    // Statistics
    uint64_t notifications_shown;                // Notifications shown
    uint64_t searches_performed;                 // Searches performed
    uint64_t voice_commands_executed;            // Voice commands executed
    uint64_t accessibility_features_used;        // Accessibility features used
    uint64_t cloud_syncs_performed;              // Cloud syncs performed
    uint64_t device_connections;                 // Device connections
    uint64_t clipboard_operations;               // Clipboard operations
    uint64_t workspace_switches;                 // Workspace switches
    uint64_t last_update;                        // Last update time
} modern_ux_system_t;

// Function declarations

// System initialization
int modern_ux_init(void);
void modern_ux_shutdown(void);
modern_ux_system_t* modern_ux_get_system(void);

// Theme management
int theme_set(ui_theme_t theme);
int theme_auto_enable(bool enabled);
int theme_switch_time_set(uint32_t hour);
ui_theme_t theme_get_current(void);

// Notification management
notification_t* notification_create(const char* title, const char* message, notification_type_t type);
int notification_destroy(uint32_t notification_id);
int notification_show(uint32_t notification_id);
int notification_dismiss(uint32_t notification_id);
int notification_center_enable(bool enabled);
int notification_sounds_enable(bool enabled);
int notification_toasts_enable(bool enabled);
notification_t* notification_find(uint32_t notification_id);

// Search management
int search_perform(const char* query);
int search_clear_results(void);
int search_spotlight_enable(bool enabled);
int search_indexing_enable(bool enabled);
int search_web_enable(bool enabled);
search_result_t* search_result_find(uint32_t result_id);
search_result_t* search_result_find_by_title(const char* title);

// Voice management
voice_command_t* voice_command_add(const char* name, const char* trigger_phrase, voice_command_type_t type);
int voice_command_remove(uint32_t command_id);
int voice_command_enable(uint32_t command_id, bool enabled);
int voice_assistant_enable(bool enabled);
int voice_recognition_enable(bool enabled);
int voice_synthesis_enable(bool enabled);
int wake_word_set(const char* wake_word);
voice_command_t* voice_command_find(uint32_t command_id);
voice_command_t* voice_command_find_by_phrase(const char* trigger_phrase);

// Accessibility management
accessibility_feature_t* accessibility_feature_add(const char* name, accessibility_feature_t type);
int accessibility_feature_remove(uint32_t feature_id);
int accessibility_feature_enable(uint32_t feature_id, bool enabled);
int accessibility_feature_set_intensity(uint32_t feature_id, uint32_t intensity);
int accessibility_enable(bool enabled);
int high_contrast_enable(bool enabled);
int screen_reader_enable(bool enabled);
int magnifier_enable(bool enabled);
accessibility_feature_t* accessibility_feature_find(uint32_t feature_id);

// Cloud management
cloud_service_t* cloud_service_add(const char* name, cloud_service_type_t type, const char* server_url);
int cloud_service_remove(uint32_t service_id);
int cloud_service_connect(uint32_t service_id);
int cloud_service_disconnect(uint32_t service_id);
int cloud_service_authenticate(uint32_t service_id, const char* username, const char* password);
int cloud_sync_enable(bool enabled);
int cloud_backup_enable(bool enabled);
int cloud_sharing_enable(bool enabled);
cloud_service_t* cloud_service_find(uint32_t service_id);
cloud_service_t* cloud_service_find_by_name(const char* name);

// Device management
int device_enumerate(void);
device_t* device_get_info(uint32_t device_id);
int device_connect(uint32_t device_id);
int device_disconnect(uint32_t device_id);
int device_authenticate(uint32_t device_id, const char* credentials);
int device_sync_enable(bool enabled);
int universal_clipboard_enable(bool enabled);
int handoff_enable(bool enabled);
device_t* device_find(uint32_t device_id);
device_t* device_find_by_name(const char* name);

// Clipboard management
clipboard_item_t* clipboard_item_add(clipboard_item_type_t type, const char* content);
int clipboard_item_remove(uint32_t item_id);
int clipboard_item_use(uint32_t item_id);
int clipboard_sync_enable(bool enabled);
int clipboard_history_enable(bool enabled);
clipboard_item_t* clipboard_item_find(uint32_t item_id);
clipboard_item_t* clipboard_item_get_current(void);

// Workspace management
workspace_t* workspace_create(const char* name, workspace_type_t type);
int workspace_destroy(uint32_t workspace_id);
int workspace_switch(uint32_t workspace_id);
int workspace_show(uint32_t workspace_id);
int workspace_hide(uint32_t workspace_id);
int workspace_switching_enable(bool enabled);
int workspace_animations_enable(bool enabled);
workspace_t* workspace_find(uint32_t workspace_id);
workspace_t* workspace_find_by_name(const char* name);
workspace_t* workspace_get_active(void);

// Start menu management
start_menu_item_t* start_menu_item_add(const char* name, const char* executable);
int start_menu_item_remove(uint32_t item_id);
int start_menu_item_pin(uint32_t item_id, bool pinned);
int start_menu_enable(bool enabled);
int start_menu_search_enable(bool enabled);
int start_menu_recent_enable(bool enabled);
start_menu_item_t* start_menu_item_find(uint32_t item_id);
start_menu_item_t* start_menu_item_find_by_name(const char* name);

// Dock management
dock_item_t* dock_item_add(const char* name, const char* executable);
int dock_item_remove(uint32_t item_id);
int dock_item_pin(uint32_t item_id, bool pinned);
int dock_item_set_badge(uint32_t item_id, uint32_t count, const char* text);
int dock_enable(bool enabled);
int dock_auto_hide_enable(bool enabled);
int dock_animations_enable(bool enabled);
int dock_position_set(uint32_t position);
dock_item_t* dock_item_find(uint32_t item_id);
dock_item_t* dock_item_find_by_name(const char* name);

// Mission control
int mission_control_enable(bool enabled);
int mission_control_gestures_enable(bool enabled);
int mission_control_hot_corners_enable(bool enabled);
int mission_control_show(void);
int mission_control_hide(void);

// Statistics
void modern_ux_get_stats(modern_ux_system_t* stats);
void modern_ux_reset_stats(void);

#endif // MODERN_UX_H 