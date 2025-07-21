#ifndef RAEEN_CUSTOMIZER_H
#define RAEEN_CUSTOMIZER_H

#include "../rendering/glass_compositor.h"
#include <stdint.h>
#include <stdbool.h>

// Raeen Customizer - Unified theming and customization system
// Provides AI-assisted theme generation and comprehensive customization

// Theme types
typedef enum {
    THEME_TYPE_LIGHT = 0,        // Light theme
    THEME_TYPE_DARK,             // Dark theme
    THEME_TYPE_AUTO,             // Auto (follows system)
    THEME_TYPE_CUSTOM,           // Custom theme
    THEME_TYPE_AI_GENERATED      // AI-generated theme
} theme_type_t;

// Color scheme
typedef struct {
    uint32_t primary_color;      // Primary accent color
    uint32_t secondary_color;    // Secondary accent color
    uint32_t background_color;   // Background color
    uint32_t surface_color;      // Surface color
    uint32_t text_color;         // Text color
    uint32_t text_secondary;     // Secondary text color
    uint32_t border_color;       // Border color
    uint32_t shadow_color;       // Shadow color
    uint32_t success_color;      // Success/positive color
    uint32_t warning_color;      // Warning color
    uint32_t error_color;        // Error/negative color
    uint32_t info_color;         // Info color
} color_scheme_t;

// Glass effect scheme
typedef struct {
    float blur_radius;           // Blur radius
    float transparency;          // Transparency level
    float border_width;          // Border width
    uint32_t border_color;       // Border color
    float corner_radius;         // Corner radius
    bool enable_shadow;          // Enable shadows
    float shadow_offset_x;       // Shadow X offset
    float shadow_offset_y;       // Shadow Y offset
    float shadow_blur;           // Shadow blur radius
    uint32_t shadow_color;       // Shadow color
    bool enable_reflection;      // Enable reflections
    float reflection_strength;   // Reflection strength
} glass_effect_scheme_t;

// Animation scheme
typedef struct {
    bool enable_animations;      // Enable animations
    float animation_duration;    // Animation duration
    float animation_easing;      // Animation easing
    bool enable_spring_effects;  // Enable spring effects
    float spring_stiffness;      // Spring stiffness
    float spring_damping;        // Spring damping
    bool enable_parallax;        // Enable parallax effects
    float parallax_strength;     // Parallax strength
} animation_scheme_t;

// Typography scheme
typedef struct {
    char font_family[64];        // Font family
    float font_size_base;        // Base font size
    float font_size_small;       // Small font size
    float font_size_large;       // Large font size
    float font_weight_normal;    // Normal font weight
    float font_weight_bold;      // Bold font weight
    float line_height;           // Line height
    float letter_spacing;        // Letter spacing
    bool enable_antialiasing;    // Enable font antialiasing
    bool enable_subpixel;        // Enable subpixel rendering
} typography_scheme_t;

// Layout scheme
typedef struct {
    float padding_small;         // Small padding
    float padding_medium;        // Medium padding
    float padding_large;         // Large padding
    float margin_small;          // Small margin
    float margin_medium;         // Medium margin
    float margin_large;          // Large margin
    float border_radius_small;   // Small border radius
    float border_radius_medium;  // Medium border radius
    float border_radius_large;   // Large border radius
    float spacing_small;         // Small spacing
    float spacing_medium;        // Medium spacing
    float spacing_large;         // Large spacing
} layout_scheme_t;

// Complete theme
typedef struct {
    uint32_t id;
    char name[64];
    char description[256];
    theme_type_t type;
    color_scheme_t colors;
    glass_effect_scheme_t glass_effects;
    animation_scheme_t animations;
    typography_scheme_t typography;
    layout_scheme_t layout;
    bool is_default;
    bool is_custom;
    uint64_t created_time;
    uint64_t modified_time;
} theme_t;

// AI theme generation parameters
typedef struct {
    char style_description[512]; // Natural language style description
    char mood[64];               // Mood (e.g., "professional", "playful", "minimal")
    char color_preference[64];   // Color preference (e.g., "warm", "cool", "neutral")
    bool include_glass_effects;  // Include glass effects
    bool include_animations;     // Include animations
    float creativity_level;      // Creativity level (0.0 - 1.0)
    bool use_reference_image;    // Use reference image
    char reference_image_path[256]; // Path to reference image
} ai_theme_params_t;

// Customization target
typedef enum {
    CUSTOMIZATION_TARGET_GLOBAL = 0,     // Global theme
    CUSTOMIZATION_TARGET_DOCK,           // Dock/taskbar
    CUSTOMIZATION_TARGET_WINDOWS,        // Windows
    CUSTOMIZATION_TARGET_MENUS,          // Menus
    CUSTOMIZATION_TARGET_BUTTONS,        // Buttons
    CUSTOMIZATION_TARGET_INPUTS,         // Input fields
    CUSTOMIZATION_TARGET_NOTIFICATIONS,  // Notifications
    CUSTOMIZATION_TARGET_DESKTOP,        // Desktop
    CUSTOMIZATION_TARGET_CURSOR,         // Cursor
    CUSTOMIZATION_TARGET_FONTS,          // Fonts
    CUSTOMIZATION_TARGET_ICONS           // Icons
} customization_target_t;

// Customization action
typedef struct {
    uint32_t id;
    customization_target_t target;
    char property[64];           // Property to customize
    char value[256];             // New value
    bool applied;
    uint64_t timestamp;
} customization_action_t;

// Raeen Customizer configuration
typedef struct {
    bool enable_ai_generation;
    bool enable_theme_previews;
    bool enable_auto_save;
    bool enable_theme_sharing;
    bool enable_advanced_customization;
    uint32_t max_custom_themes;
    uint32_t auto_save_interval;
    char default_theme_path[256];
    char themes_directory[256];
} customizer_config_t;

// Raeen Customizer context
typedef struct {
    customizer_config_t config;
    glass_compositor_t* compositor;
    theme_t* themes;
    uint32_t theme_count;
    uint32_t max_themes;
    theme_t* current_theme;
    theme_t* default_theme;
    customization_action_t* customization_history;
    uint32_t customization_count;
    uint32_t max_customizations;
    bool initialized;
    bool ui_visible;
    uint32_t next_theme_id;
    uint32_t next_customization_id;
} raeen_customizer_t;

// Function prototypes

// Initialization and shutdown
raeen_customizer_t* raeen_customizer_init(glass_compositor_t* compositor, customizer_config_t* config);
void raeen_customizer_shutdown(raeen_customizer_t* customizer);

// Theme management
uint32_t raeen_customizer_create_theme(raeen_customizer_t* customizer, const char* name, const char* description);
void raeen_customizer_destroy_theme(raeen_customizer_t* customizer, uint32_t theme_id);
void raeen_customizer_duplicate_theme(raeen_customizer_t* customizer, uint32_t theme_id, const char* new_name);
void raeen_customizer_apply_theme(raeen_customizer_t* customizer, uint32_t theme_id);
void raeen_customizer_set_default_theme(raeen_customizer_t* customizer, uint32_t theme_id);

// Theme customization
void raeen_customizer_set_color_scheme(raeen_customizer_t* customizer, uint32_t theme_id, color_scheme_t* colors);
void raeen_customizer_set_glass_effects(raeen_customizer_t* customizer, uint32_t theme_id, glass_effect_scheme_t* effects);
void raeen_customizer_set_animations(raeen_customizer_t* customizer, uint32_t theme_id, animation_scheme_t* animations);
void raeen_customizer_set_typography(raeen_customizer_t* customizer, uint32_t theme_id, typography_scheme_t* typography);
void raeen_customizer_set_layout(raeen_customizer_t* customizer, uint32_t theme_id, layout_scheme_t* layout);

// AI theme generation
uint32_t raeen_customizer_generate_ai_theme(raeen_customizer_t* customizer, ai_theme_params_t* params);
void raeen_customizer_refine_ai_theme(raeen_customizer_t* customizer, uint32_t theme_id, const char* refinement_description);
void raeen_customizer_evolve_ai_theme(raeen_customizer_t* customizer, uint32_t theme_id, float evolution_factor);
theme_t* raeen_customizer_get_ai_suggestions(raeen_customizer_t* customizer, const char* base_description, uint32_t* suggestion_count);

// Theme import/export
bool raeen_customizer_export_theme(raeen_customizer_t* customizer, uint32_t theme_id, const char* file_path);
uint32_t raeen_customizer_import_theme(raeen_customizer_t* customizer, const char* file_path);
bool raeen_customizer_share_theme(raeen_customizer_t* customizer, uint32_t theme_id, const char* share_url);
uint32_t raeen_customizer_download_theme(raeen_customizer_t* customizer, const char* theme_url);

// Real-time customization
void raeen_customizer_customize_color(raeen_customizer_t* customizer, customization_target_t target, const char* property, uint32_t color);
void raeen_customizer_customize_glass_effect(raeen_customizer_t* customizer, customization_target_t target, const char* property, float value);
void raeen_customizer_customize_animation(raeen_customizer_t* customizer, customization_target_t target, const char* property, float value);
void raeen_customizer_customize_typography(raeen_customizer_t* customizer, customization_target_t target, const char* property, const char* value);
void raeen_customizer_customize_layout(raeen_customizer_t* customizer, customization_target_t target, const char* property, float value);

// Customization history
void raeen_customizer_undo_last_customization(raeen_customizer_t* customizer);
void raeen_customizer_redo_last_customization(raeen_customizer_t* customizer);
void raeen_customizer_clear_customization_history(raeen_customizer_t* customizer);
customization_action_t* raeen_customizer_get_customization_history(raeen_customizer_t* customizer, uint32_t* count);

// Theme previews
void raeen_customizer_generate_theme_preview(raeen_customizer_t* customizer, uint32_t theme_id, raeen_texture_t** preview_texture);
void raeen_customizer_show_theme_preview(raeen_customizer_t* customizer, uint32_t theme_id);
void raeen_customizer_hide_theme_preview(raeen_customizer_t* customizer);

// UI management
void raeen_customizer_show_ui(raeen_customizer_t* customizer);
void raeen_customizer_hide_ui(raeen_customizer_t* customizer);
void raeen_customizer_toggle_ui(raeen_customizer_t* customizer);
void raeen_customizer_render_ui(raeen_customizer_t* customizer);
void raeen_customizer_render_theme_editor(raeen_customizer_t* customizer);
void raeen_customizer_render_color_picker(raeen_customizer_t* customizer);
void raeen_customizer_render_glass_editor(raeen_customizer_t* customizer);
void raeen_customizer_render_animation_editor(raeen_customizer_t* customizer);

// Input handling
bool raeen_customizer_handle_mouse_click(raeen_customizer_t* customizer, float x, float y, bool left_click);
bool raeen_customizer_handle_mouse_move(raeen_customizer_t* customizer, float x, float y);
bool raeen_customizer_handle_mouse_wheel(raeen_customizer_t* customizer, float x, float y, float delta);
bool raeen_customizer_handle_key_press(raeen_customizer_t* customizer, uint32_t key_code);
bool raeen_customizer_handle_text_input(raeen_customizer_t* customizer, const char* text);

// Theme information
uint32_t raeen_customizer_get_theme_count(raeen_customizer_t* customizer);
theme_t* raeen_customizer_get_theme(raeen_customizer_t* customizer, uint32_t theme_id);
theme_t* raeen_customizer_get_current_theme(raeen_customizer_t* customizer);
theme_t* raeen_customizer_get_default_theme(raeen_customizer_t* customizer);
bool raeen_customizer_is_ui_visible(raeen_customizer_t* customizer);

// Utility functions
void raeen_customizer_reset_to_default(raeen_customizer_t* customizer);
void raeen_customizer_auto_save_themes(raeen_customizer_t* customizer);
void raeen_customizer_load_themes_from_directory(raeen_customizer_t* customizer, const char* directory);
void raeen_customizer_validate_theme(raeen_customizer_t* customizer, theme_t* theme);
void raeen_customizer_optimize_theme(raeen_customizer_t* customizer, theme_t* theme);

// Preset themes
uint32_t raeen_customizer_create_macos_theme(raeen_customizer_t* customizer);
uint32_t raeen_customizer_create_windows_theme(raeen_customizer_t* customizer);
uint32_t raeen_customizer_create_hybrid_theme(raeen_customizer_t* customizer);
uint32_t raeen_customizer_create_minimal_theme(raeen_customizer_t* customizer);
uint32_t raeen_customizer_create_dark_theme(raeen_customizer_t* customizer);
uint32_t raeen_customizer_create_light_theme(raeen_customizer_t* customizer);

// Callbacks
typedef void (*theme_changed_callback_t)(raeen_customizer_t* customizer, theme_t* old_theme, theme_t* new_theme, void* user_data);
typedef void (*customization_applied_callback_t)(raeen_customizer_t* customizer, customization_action_t* action, void* user_data);
typedef void (*ai_generation_complete_callback_t)(raeen_customizer_t* customizer, uint32_t theme_id, void* user_data);

void raeen_customizer_set_theme_changed_callback(raeen_customizer_t* customizer, theme_changed_callback_t callback, void* user_data);
void raeen_customizer_set_customization_applied_callback(raeen_customizer_t* customizer, customization_applied_callback_t callback, void* user_data);
void raeen_customizer_set_ai_generation_complete_callback(raeen_customizer_t* customizer, ai_generation_complete_callback_t callback, void* user_data);

// Preset configurations
customizer_config_t raeen_customizer_preset_basic_style(void);
customizer_config_t raeen_customizer_preset_advanced_style(void);
customizer_config_t raeen_customizer_preset_ai_enhanced_style(void);

// Error handling
typedef enum {
    RAEEN_CUSTOMIZER_SUCCESS = 0,
    RAEEN_CUSTOMIZER_ERROR_INVALID_CONTEXT,
    RAEEN_CUSTOMIZER_ERROR_INVALID_THEME,
    RAEEN_CUSTOMIZER_ERROR_OUT_OF_MEMORY,
    RAEEN_CUSTOMIZER_ERROR_AI_FAILED,
    RAEEN_CUSTOMIZER_ERROR_IMPORT_FAILED,
    RAEEN_CUSTOMIZER_ERROR_EXPORT_FAILED,
} raeen_customizer_error_t;

raeen_customizer_error_t raeen_customizer_get_last_error(void);
const char* raeen_customizer_get_error_string(raeen_customizer_error_t error);

#endif // RAEEN_CUSTOMIZER_H 