#ifndef SPOTLIGHT_PLUS_H
#define SPOTLIGHT_PLUS_H

#include "../rendering/glass_compositor.h"
#include <stdint.h>
#include <stdbool.h>

// Spotlight+ - Advanced search system for RaeenOS
// Combines macOS Spotlight with Windows Search, enhanced with AI

// Search result types
typedef enum {
    SEARCH_RESULT_APP = 0,       // Application
    SEARCH_RESULT_FILE,          // File
    SEARCH_RESULT_FOLDER,        // Folder
    SEARCH_RESULT_SETTING,       // System setting
    SEARCH_RESULT_CONTACT,       // Contact
    SEARCH_RESULT_EMAIL,         // Email
    SEARCH_RESULT_WEB,           // Web result
    SEARCH_RESULT_CALCULATOR,    // Calculator result
    SEARCH_RESULT_WEATHER,       // Weather
    SEARCH_RESULT_NEWS,          // News
    SEARCH_RESULT_CUSTOM         // Custom result
} search_result_type_t;

// Search categories
typedef enum {
    SEARCH_CATEGORY_ALL = 0,     // All categories
    SEARCH_CATEGORY_APPS,        // Applications
    SEARCH_CATEGORY_FILES,       // Files and folders
    SEARCH_CATEGORY_SETTINGS,    // System settings
    SEARCH_CATEGORY_CONTACTS,    // Contacts and people
    SEARCH_CATEGORY_EMAILS,      // Emails
    SEARCH_CATEGORY_WEB,         // Web results
    SEARCH_CATEGORY_CALCULATOR,  // Calculator
    SEARCH_CATEGORY_WEATHER,     // Weather
    SEARCH_CATEGORY_NEWS,        // News
    SEARCH_CATEGORY_RECENT       // Recent items
} search_category_t;

// Search result
typedef struct {
    uint32_t id;
    search_result_type_t type;
    search_category_t category;
    char title[256];
    char subtitle[256];
    char description[512];
    char path[512];
    char icon_path[256];
    raeen_texture_t* icon_texture;
    float relevance_score;
    uint64_t last_accessed;
    uint64_t file_size;
    bool is_recent;
    bool is_favorite;
    void* user_data;
} search_result_t;

// Search query
typedef struct {
    char query[512];
    search_category_t category;
    bool include_files;
    bool include_apps;
    bool include_settings;
    bool include_web;
    bool include_recent;
    bool use_ai_enhancement;
    uint32_t max_results;
    uint32_t offset;
} search_query_t;

// AI enhancement types
typedef enum {
    AI_ENHANCEMENT_NONE = 0,
    AI_ENHANCEMENT_SEMANTIC,     // Semantic search
    AI_ENHANCEMENT_CONTEXTUAL,   // Context-aware search
    AI_ENHANCEMENT_PREDICTIVE,   // Predictive search
    AI_ENHANCEMENT_NATURAL,      // Natural language processing
    AI_ENHANCEMENT_VOICE         // Voice search
} ai_enhancement_type_t;

// Search index entry
typedef struct {
    uint32_t id;
    char path[512];
    char title[256];
    char content[1024];
    search_result_type_t type;
    search_category_t category;
    uint64_t last_modified;
    uint64_t last_accessed;
    uint32_t access_count;
    float relevance_score;
    bool indexed;
} search_index_entry_t;

// Spotlight+ configuration
typedef struct {
    bool enable_ai_enhancement;
    bool enable_voice_search;
    bool enable_predictive_search;
    bool enable_contextual_search;
    bool enable_web_search;
    bool enable_recent_search;
    bool enable_favorites;
    uint32_t max_results_per_category;
    uint32_t max_total_results;
    float min_relevance_score;
    uint32_t index_update_interval;
    bool auto_index_new_files;
    bool index_file_contents;
    uint32_t max_index_size;
} spotlight_plus_config_t;

// Spotlight+ context
typedef struct {
    spotlight_plus_config_t config;
    glass_compositor_t* compositor;
    search_index_entry_t* index;
    uint32_t index_count;
    uint32_t max_index_entries;
    search_result_t* recent_results;
    uint32_t recent_count;
    search_result_t* favorite_results;
    uint32_t favorite_count;
    bool initialized;
    bool indexing;
    uint64_t last_index_update;
    uint32_t next_result_id;
    uint32_t next_index_id;
} spotlight_plus_t;

// Function prototypes

// Initialization and shutdown
spotlight_plus_t* spotlight_plus_init(glass_compositor_t* compositor, spotlight_plus_config_t* config);
void spotlight_plus_shutdown(spotlight_plus_t* spotlight);
void spotlight_plus_resize(spotlight_plus_t* spotlight, uint32_t width, uint32_t height);

// Search functionality
search_result_t* spotlight_plus_search(spotlight_plus_t* spotlight, search_query_t* query, uint32_t* result_count);
search_result_t* spotlight_plus_search_instant(spotlight_plus_t* spotlight, const char* query, uint32_t* result_count);
search_result_t* spotlight_plus_search_category(spotlight_plus_t* spotlight, search_category_t category, uint32_t* result_count);
search_result_t* spotlight_plus_search_recent(spotlight_plus_t* spotlight, uint32_t* result_count);
search_result_t* spotlight_plus_search_favorites(spotlight_plus_t* spotlight, uint32_t* result_count);

// AI-enhanced search
search_result_t* spotlight_plus_search_ai(spotlight_plus_t* spotlight, const char* natural_query, uint32_t* result_count);
search_result_t* spotlight_plus_search_semantic(spotlight_plus_t* spotlight, const char* query, uint32_t* result_count);
search_result_t* spotlight_plus_search_contextual(spotlight_plus_t* spotlight, const char* query, void* context, uint32_t* result_count);
search_result_t* spotlight_plus_search_predictive(spotlight_plus_t* spotlight, const char* partial_query, uint32_t* result_count);

// Indexing
void spotlight_plus_build_index(spotlight_plus_t* spotlight);
void spotlight_plus_update_index(spotlight_plus_t* spotlight);
void spotlight_plus_add_to_index(spotlight_plus_t* spotlight, const char* path, const char* title, const char* content, search_result_type_t type);
void spotlight_plus_remove_from_index(spotlight_plus_t* spotlight, const char* path);
void spotlight_plus_clear_index(spotlight_plus_t* spotlight);
bool spotlight_plus_is_indexing(spotlight_plus_t* spotlight);

// Result management
void spotlight_plus_add_recent_result(spotlight_plus_t* spotlight, search_result_t* result);
void spotlight_plus_add_favorite_result(spotlight_plus_t* spotlight, search_result_t* result);
void spotlight_plus_remove_favorite_result(spotlight_plus_t* spotlight, uint32_t result_id);
void spotlight_plus_clear_recent_results(spotlight_plus_t* spotlight);
void spotlight_plus_clear_favorite_results(spotlight_plus_t* spotlight);

// Result actions
void spotlight_plus_launch_result(spotlight_plus_t* spotlight, search_result_t* result);
void spotlight_plus_open_result(spotlight_plus_t* spotlight, search_result_t* result);
void spotlight_plus_show_result_info(spotlight_plus_t* spotlight, search_result_t* result);
void spotlight_plus_copy_result_path(spotlight_plus_t* spotlight, search_result_t* result);
void spotlight_plus_share_result(spotlight_plus_t* spotlight, search_result_t* result);

// UI rendering
void spotlight_plus_show_search_ui(spotlight_plus_t* spotlight);
void spotlight_plus_hide_search_ui(spotlight_plus_t* spotlight);
void spotlight_plus_toggle_search_ui(spotlight_plus_t* spotlight);
void spotlight_plus_render_search_ui(spotlight_plus_t* spotlight);
void spotlight_plus_render_results(spotlight_plus_t* spotlight, search_result_t* results, uint32_t result_count);
void spotlight_plus_render_categories(spotlight_plus_t* spotlight);

// Input handling
bool spotlight_plus_handle_key_press(spotlight_plus_t* spotlight, uint32_t key_code);
bool spotlight_plus_handle_text_input(spotlight_plus_t* spotlight, const char* text);
bool spotlight_plus_handle_mouse_click(spotlight_plus_t* spotlight, float x, float y, bool left_click);
bool spotlight_plus_handle_mouse_move(spotlight_plus_t* spotlight, float x, float y);
bool spotlight_plus_handle_mouse_wheel(spotlight_plus_t* spotlight, float x, float y, float delta);

// Configuration
void spotlight_plus_set_ai_enhancement(spotlight_plus_t* spotlight, bool enable);
void spotlight_plus_set_voice_search(spotlight_plus_t* spotlight, bool enable);
void spotlight_plus_set_predictive_search(spotlight_plus_t* spotlight, bool enable);
void spotlight_plus_set_contextual_search(spotlight_plus_t* spotlight, bool enable);
void spotlight_plus_set_web_search(spotlight_plus_t* spotlight, bool enable);
void spotlight_plus_set_max_results(spotlight_plus_t* spotlight, uint32_t max_results);
void spotlight_plus_set_min_relevance(spotlight_plus_t* spotlight, float min_relevance);

// Voice search
void spotlight_plus_start_voice_search(spotlight_plus_t* spotlight);
void spotlight_plus_stop_voice_search(spotlight_plus_t* spotlight);
bool spotlight_plus_is_voice_search_active(spotlight_plus_t* spotlight);
void spotlight_plus_process_voice_input(spotlight_plus_t* spotlight, const char* voice_text);

// Calculator integration
search_result_t* spotlight_plus_calculate(spotlight_plus_t* spotlight, const char* expression);
bool spotlight_plus_is_calculation_query(spotlight_plus_t* spotlight, const char* query);

// Weather integration
search_result_t* spotlight_plus_get_weather(spotlight_plus_t* spotlight, const char* location);
bool spotlight_plus_is_weather_query(spotlight_plus_t* spotlight, const char* query);

// Web search integration
search_result_t* spotlight_plus_web_search(spotlight_plus_t* spotlight, const char* query, uint32_t* result_count);
bool spotlight_plus_is_web_query(spotlight_plus_t* spotlight, const char* query);

// Information
uint32_t spotlight_plus_get_index_count(spotlight_plus_t* spotlight);
uint32_t spotlight_plus_get_recent_count(spotlight_plus_t* spotlight);
uint32_t spotlight_plus_get_favorite_count(spotlight_plus_t* spotlight);
bool spotlight_plus_is_visible(spotlight_plus_t* spotlight);
uint64_t spotlight_plus_get_last_index_update(spotlight_plus_t* spotlight);

// Utility functions
void spotlight_plus_refresh_results(spotlight_plus_t* spotlight);
void spotlight_plus_sort_results_by_relevance(search_result_t* results, uint32_t result_count);
void spotlight_plus_sort_results_by_date(search_result_t* results, uint32_t result_count);
void spotlight_plus_sort_results_by_name(search_result_t* results, uint32_t result_count);
void spotlight_plus_filter_results_by_type(search_result_t* results, uint32_t* result_count, search_result_type_t type);
void spotlight_plus_filter_results_by_category(search_result_t* results, uint32_t* result_count, search_category_t category);

// Callbacks
typedef void (*search_result_selected_callback_t)(spotlight_plus_t* spotlight, search_result_t* result, void* user_data);
typedef void (*search_query_changed_callback_t)(spotlight_plus_t* spotlight, const char* query, void* user_data);
typedef void (*search_category_changed_callback_t)(spotlight_plus_t* spotlight, search_category_t category, void* user_data);

void spotlight_plus_set_result_selected_callback(spotlight_plus_t* spotlight, search_result_selected_callback_t callback, void* user_data);
void spotlight_plus_set_query_changed_callback(spotlight_plus_t* spotlight, search_query_changed_callback_t callback, void* user_data);
void spotlight_plus_set_category_changed_callback(spotlight_plus_t* spotlight, search_category_changed_callback_t callback, void* user_data);

// Preset configurations
spotlight_plus_config_t spotlight_plus_preset_macos_style(void);
spotlight_plus_config_t spotlight_plus_preset_windows_style(void);
spotlight_plus_config_t spotlight_plus_preset_hybrid_style(void);
spotlight_plus_config_t spotlight_plus_preset_ai_enhanced_style(void);

// Error handling
typedef enum {
    SPOTLIGHT_PLUS_SUCCESS = 0,
    SPOTLIGHT_PLUS_ERROR_INVALID_CONTEXT,
    SPOTLIGHT_PLUS_ERROR_INVALID_QUERY,
    SPOTLIGHT_PLUS_ERROR_OUT_OF_MEMORY,
    SPOTLIGHT_PLUS_ERROR_INDEX_FAILED,
    SPOTLIGHT_PLUS_ERROR_AI_FAILED,
    SPOTLIGHT_PLUS_ERROR_VOICE_FAILED,
} spotlight_plus_error_t;

spotlight_plus_error_t spotlight_plus_get_last_error(void);
const char* spotlight_plus_get_error_string(spotlight_plus_error_t error);

#endif // SPOTLIGHT_PLUS_H 