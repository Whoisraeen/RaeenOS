#ifndef WEB_BROWSER_H
#define WEB_BROWSER_H

#include "../../gui/rendering/glass_compositor.h"
#include "../../gui/window/glass_window_manager.h"
#include <stdint.h>
#include <stdbool.h>

// Web Browser - Modern web browser for RaeenOS
// Provides security, performance, and compatibility features

// Browser engines
typedef enum {
    BROWSER_ENGINE_WEBKIT = 0,   // WebKit (Safari-like)
    BROWSER_ENGINE_GECKO,        // Gecko (Firefox-like)
    BROWSER_ENGINE_BLINK,        // Blink (Chrome-like)
    BROWSER_ENGINE_CUSTOM        // Custom engine
} browser_engine_t;

// Security levels
typedef enum {
    SECURITY_LEVEL_LOW = 0,      // Low security (allow all)
    SECURITY_LEVEL_MEDIUM,       // Medium security (default)
    SECURITY_LEVEL_HIGH,         // High security (strict)
    SECURITY_LEVEL_ULTRA         // Ultra security (paranoid)
} security_level_t;

// Privacy modes
typedef enum {
    PRIVACY_MODE_NORMAL = 0,     // Normal browsing
    PRIVACY_MODE_INCOGNITO,      // Incognito/private browsing
    PRIVACY_MODE_TOR,            // Tor network browsing
    PRIVACY_MODE_VPN             // VPN browsing
} privacy_mode_t;

// Tab states
typedef enum {
    TAB_STATE_LOADING = 0,       // Page is loading
    TAB_STATE_READY,             // Page is ready
    TAB_STATE_ERROR,             // Error occurred
    TAB_STATE_CRASHED,           // Tab crashed
    TAB_STATE_SUSPENDED          // Tab is suspended
} tab_state_t;

// Browser tab
typedef struct {
    uint32_t id;
    char title[256];             // Page title
    char url[512];               // Current URL
    char favicon_path[256];      // Favicon path
    tab_state_t state;           // Tab state
    bool is_active;              // Is active tab
    bool is_pinned;              // Is pinned tab
    bool is_muted;               // Is muted
    float progress;              // Loading progress (0.0 - 1.0)
    uint64_t last_accessed;      // Last access time
    void* page_data;             // Page-specific data
    raeen_texture_t* thumbnail;  // Tab thumbnail
    raeen_texture_t* favicon;    // Tab favicon
} browser_tab_t;

// Browser window
typedef struct {
    uint32_t id;
    char title[256];             // Window title
    raeen_rect_t bounds;         // Window bounds
    browser_tab_t* tabs;         // Array of tabs
    uint32_t tab_count;          // Number of tabs
    uint32_t max_tabs;           // Maximum tabs
    uint32_t active_tab;         // Active tab index
    bool is_fullscreen;          // Is fullscreen
    bool is_private;             // Is private window
    void* window_data;           // Window-specific data
} browser_window_t;

// Browser history entry
typedef struct {
    char title[256];             // Page title
    char url[512];               // URL
    char favicon_path[256];      // Favicon path
    uint64_t visit_time;         // Visit timestamp
    uint32_t visit_count;        // Number of visits
    bool is_bookmarked;          // Is bookmarked
} history_entry_t;

// Browser bookmark
typedef struct {
    uint32_t id;
    char title[256];             // Bookmark title
    char url[512];               // Bookmark URL
    char description[512];       // Bookmark description
    char folder[64];             // Bookmark folder
    uint64_t created_time;       // Creation time
    uint64_t last_visited;       // Last visit time
    raeen_texture_t* favicon;    // Bookmark favicon
} browser_bookmark_t;

// Browser settings
typedef struct {
    browser_engine_t engine;     // Browser engine
    security_level_t security;   // Security level
    privacy_mode_t privacy;      // Privacy mode
    char homepage[512];          // Homepage URL
    char search_engine[256];     // Default search engine
    bool enable_javascript;      // Enable JavaScript
    bool enable_plugins;         // Enable plugins
    bool enable_cookies;         // Enable cookies
    bool enable_popups;          // Enable popups
    bool enable_images;          // Enable images
    bool enable_auto_fill;       // Enable auto-fill
    bool enable_password_save;   // Enable password saving
    bool enable_location;        // Enable location services
    bool enable_notifications;   // Enable notifications
    bool enable_do_not_track;    // Enable Do Not Track
    uint32_t max_tabs;           // Maximum tabs per window
    uint32_t max_history;        // Maximum history entries
    uint32_t cache_size_mb;      // Cache size in MB
    uint32_t download_path[512]; // Download path
} browser_settings_t;

// Browser configuration
typedef struct {
    uint32_t window_width, window_height;
    browser_engine_t default_engine;
    security_level_t default_security;
    privacy_mode_t default_privacy;
    bool enable_hardware_acceleration;
    bool enable_sandboxing;
    bool enable_process_isolation;
    bool enable_memory_optimization;
    bool enable_battery_optimization;
    uint32_t max_windows;
    uint32_t max_tabs_per_window;
    uint32_t max_history_entries;
    uint32_t max_bookmarks;
    uint32_t max_downloads;
    uint32_t cache_size_mb;
    uint32_t memory_limit_mb;
} browser_config_t;

// Browser context
typedef struct {
    browser_config_t config;
    glass_compositor_t* compositor;
    glass_window_manager_t* window_manager;
    browser_window_t* windows;
    uint32_t window_count;
    uint32_t max_windows;
    browser_settings_t settings;
    history_entry_t* history;
    uint32_t history_count;
    uint32_t max_history;
    browser_bookmark_t* bookmarks;
    uint32_t bookmark_count;
    uint32_t max_bookmarks;
    bool initialized;
    uint32_t next_tab_id;
    uint32_t next_window_id;
    uint32_t next_bookmark_id;
    uint64_t last_cleanup_time;
} web_browser_t;

// Function prototypes

// Initialization and shutdown
web_browser_t* web_browser_init(glass_compositor_t* compositor,
                               glass_window_manager_t* window_manager,
                               browser_config_t* config);
void web_browser_shutdown(web_browser_t* browser);
bool web_browser_is_initialized(web_browser_t* browser);

// Window management
uint32_t web_browser_create_window(web_browser_t* browser, const char* title);
void web_browser_destroy_window(web_browser_t* browser, uint32_t window_id);
void web_browser_show_window(web_browser_t* browser, uint32_t window_id);
void web_browser_hide_window(web_browser_t* browser, uint32_t window_id);
void web_browser_minimize_window(web_browser_t* browser, uint32_t window_id);
void web_browser_maximize_window(web_browser_t* browser, uint32_t window_id);
void web_browser_restore_window(web_browser_t* browser, uint32_t window_id);
void web_browser_set_fullscreen(web_browser_t* browser, uint32_t window_id, bool fullscreen);
browser_window_t* web_browser_get_window(web_browser_t* browser, uint32_t window_id);
uint32_t web_browser_get_window_count(web_browser_t* browser);

// Tab management
uint32_t web_browser_create_tab(web_browser_t* browser, uint32_t window_id, const char* url);
void web_browser_destroy_tab(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
void web_browser_activate_tab(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
void web_browser_pin_tab(web_browser_t* browser, uint32_t window_id, uint32_t tab_id, bool pin);
void web_browser_mute_tab(web_browser_t* browser, uint32_t window_id, uint32_t tab_id, bool mute);
void web_browser_reload_tab(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
void web_browser_stop_tab(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
browser_tab_t* web_browser_get_tab(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
browser_tab_t* web_browser_get_active_tab(web_browser_t* browser, uint32_t window_id);
uint32_t web_browser_get_tab_count(web_browser_t* browser, uint32_t window_id);

// Navigation
bool web_browser_navigate(web_browser_t* browser, uint32_t window_id, uint32_t tab_id, const char* url);
bool web_browser_navigate_home(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
bool web_browser_navigate_back(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
bool web_browser_navigate_forward(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
bool web_browser_navigate_refresh(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
bool web_browser_navigate_stop(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
const char* web_browser_get_current_url(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
const char* web_browser_get_current_title(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);

// Search functionality
bool web_browser_search(web_browser_t* browser, uint32_t window_id, uint32_t tab_id, const char* query);
bool web_browser_search_in_page(web_browser_t* browser, uint32_t window_id, uint32_t tab_id, const char* query);
void web_browser_set_search_engine(web_browser_t* browser, const char* search_engine);
const char* web_browser_get_search_engine(web_browser_t* browser);

// History management
void web_browser_add_history_entry(web_browser_t* browser, const char* title, const char* url);
history_entry_t* web_browser_get_history(web_browser_t* browser, uint32_t* count);
void web_browser_clear_history(web_browser_t* browser);
void web_browser_clear_history_range(web_browser_t* browser, uint64_t from_time, uint64_t to_time);
bool web_browser_remove_history_entry(web_browser_t* browser, const char* url);

// Bookmark management
uint32_t web_browser_add_bookmark(web_browser_t* browser, const char* title, const char* url, const char* folder);
void web_browser_remove_bookmark(web_browser_t* browser, uint32_t bookmark_id);
void web_browser_edit_bookmark(web_browser_t* browser, uint32_t bookmark_id, const char* title, const char* url, const char* folder);
browser_bookmark_t* web_browser_get_bookmarks(web_browser_t* browser, uint32_t* count);
browser_bookmark_t* web_browser_get_bookmarks_in_folder(web_browser_t* browser, const char* folder, uint32_t* count);
bool web_browser_is_bookmarked(web_browser_t* browser, const char* url);

// Download management
uint32_t web_browser_start_download(web_browser_t* browser, const char* url, const char* filename);
void web_browser_cancel_download(web_browser_t* browser, uint32_t download_id);
void web_browser_pause_download(web_browser_t* browser, uint32_t download_id);
void web_browser_resume_download(web_browser_t* browser, uint32_t download_id);
void web_browser_set_download_path(web_browser_t* browser, const char* path);
const char* web_browser_get_download_path(web_browser_t* browser);

// Security and privacy
void web_browser_set_security_level(web_browser_t* browser, security_level_t level);
security_level_t web_browser_get_security_level(web_browser_t* browser);
void web_browser_set_privacy_mode(web_browser_t* browser, privacy_mode_t mode);
privacy_mode_t web_browser_get_privacy_mode(web_browser_t* browser);
void web_browser_clear_cookies(web_browser_t* browser);
void web_browser_clear_cache(web_browser_t* browser);
void web_browser_clear_data(web_browser_t* browser);
bool web_browser_is_secure_connection(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);

// Settings management
void web_browser_set_homepage(web_browser_t* browser, const char* url);
const char* web_browser_get_homepage(web_browser_t* browser);
void web_browser_enable_javascript(web_browser_t* browser, bool enable);
bool web_browser_is_javascript_enabled(web_browser_t* browser);
void web_browser_enable_plugins(web_browser_t* browser, bool enable);
bool web_browser_are_plugins_enabled(web_browser_t* browser);
void web_browser_enable_cookies(web_browser_t* browser, bool enable);
bool web_browser_are_cookies_enabled(web_browser_t* browser);
void web_browser_enable_popups(web_browser_t* browser, bool enable);
bool web_browser_are_popups_enabled(web_browser_t* browser);

// Performance optimization
void web_browser_enable_hardware_acceleration(web_browser_t* browser, bool enable);
bool web_browser_is_hardware_acceleration_enabled(web_browser_t* browser);
void web_browser_enable_memory_optimization(web_browser_t* browser, bool enable);
bool web_browser_is_memory_optimization_enabled(web_browser_t* browser);
void web_browser_enable_battery_optimization(web_browser_t* browser, bool enable);
bool web_browser_is_battery_optimization_enabled(web_browser_t* browser);
void web_browser_suspend_inactive_tabs(web_browser_t* browser, bool enable);
bool web_browser_are_inactive_tabs_suspended(web_browser_t* browser);

// Developer tools
void web_browser_open_developer_tools(web_browser_t* browser, uint32_t window_id, uint32_t tab_id);
void web_browser_close_developer_tools(web_browser_t* browser, uint32_t window_id);
bool web_browser_are_developer_tools_open(web_browser_t* browser, uint32_t window_id);
void web_browser_inspect_element(web_browser_t* browser, uint32_t window_id, uint32_t tab_id, float x, float y);
void web_browser_show_console(web_browser_t* browser, uint32_t window_id);
void web_browser_show_network(web_browser_t* browser, uint32_t window_id);
void web_browser_show_sources(web_browser_t* browser, uint32_t window_id);

// Rendering
void web_browser_render(web_browser_t* browser);
void web_browser_render_window(web_browser_t* browser, uint32_t window_id);
void web_browser_render_tab_bar(web_browser_t* browser, uint32_t window_id);
void web_browser_render_address_bar(web_browser_t* browser, uint32_t window_id);
void web_browser_render_toolbar(web_browser_t* browser, uint32_t window_id);
void web_browser_render_status_bar(web_browser_t* browser, uint32_t window_id);

// Input handling
bool web_browser_handle_mouse_move(web_browser_t* browser, uint32_t window_id, float x, float y);
bool web_browser_handle_mouse_click(web_browser_t* browser, uint32_t window_id, float x, float y, bool left_click);
bool web_browser_handle_mouse_wheel(web_browser_t* browser, uint32_t window_id, float x, float y, float delta);
bool web_browser_handle_key_press(web_browser_t* browser, uint32_t window_id, uint32_t key_code);
bool web_browser_handle_text_input(web_browser_t* browser, uint32_t window_id, const char* text);

// Keyboard shortcuts
bool web_browser_handle_shortcut(web_browser_t* browser, uint32_t window_id, uint32_t key_code, uint32_t modifiers);
void web_browser_new_tab_shortcut(web_browser_t* browser, uint32_t window_id);
void web_browser_close_tab_shortcut(web_browser_t* browser, uint32_t window_id);
void web_browser_new_window_shortcut(web_browser_t* browser);
void web_browser_private_window_shortcut(web_browser_t* browser);
void web_browser_find_shortcut(web_browser_t* browser, uint32_t window_id);
void web_browser_bookmark_shortcut(web_browser_t* browser, uint32_t window_id);

// Information
uint32_t web_browser_get_total_tab_count(web_browser_t* browser);
uint32_t web_browser_get_memory_usage(web_browser_t* browser);
float web_browser_get_cpu_usage(web_browser_t* browser);
uint64_t web_browser_get_uptime(web_browser_t* browser);
bool web_browser_is_private_mode(web_browser_t* browser, uint32_t window_id);

// Utility functions
bool web_browser_is_url_valid(const char* url);
char* web_browser_get_domain_from_url(const char* url);
char* web_browser_get_protocol_from_url(const char* url);
bool web_browser_is_secure_protocol(const char* url);
char* web_browser_encode_url(const char* url);
char* web_browser_decode_url(const char* url);
char* web_browser_get_favicon_url(const char* url);

// Callbacks
typedef void (*tab_state_change_callback_t)(web_browser_t* browser, uint32_t window_id, uint32_t tab_id, tab_state_t old_state, tab_state_t new_state, void* user_data);
typedef void (*navigation_callback_t)(web_browser_t* browser, uint32_t window_id, uint32_t tab_id, const char* old_url, const char* new_url, void* user_data);
typedef void (*download_complete_callback_t)(web_browser_t* browser, uint32_t download_id, const char* filename, bool success, void* user_data);

void web_browser_set_tab_state_change_callback(web_browser_t* browser, tab_state_change_callback_t callback, void* user_data);
void web_browser_set_navigation_callback(web_browser_t* browser, navigation_callback_t callback, void* user_data);
void web_browser_set_download_complete_callback(web_browser_t* browser, download_complete_callback_t callback, void* user_data);

// Preset configurations
browser_config_t web_browser_preset_normal_style(void);
browser_config_t web_browser_preset_security_style(void);
browser_config_t web_browser_preset_performance_style(void);
browser_config_t web_browser_preset_privacy_style(void);

// Error handling
typedef enum {
    WEB_BROWSER_SUCCESS = 0,
    WEB_BROWSER_ERROR_INVALID_CONTEXT,
    WEB_BROWSER_ERROR_INVALID_WINDOW,
    WEB_BROWSER_ERROR_INVALID_TAB,
    WEB_BROWSER_ERROR_INVALID_URL,
    WEB_BROWSER_ERROR_NETWORK_FAILED,
    WEB_BROWSER_ERROR_OUT_OF_MEMORY,
    WEB_BROWSER_ERROR_ENGINE_FAILED,
    WEB_BROWSER_ERROR_SECURITY_BLOCKED,
    WEB_BROWSER_ERROR_TIMEOUT,
} web_browser_error_t;

web_browser_error_t web_browser_get_last_error(void);
const char* web_browser_get_error_string(web_browser_error_t error);

#endif // WEB_BROWSER_H 