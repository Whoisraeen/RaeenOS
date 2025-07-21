#ifndef DISPLAY_SYSTEM_H
#define DISPLAY_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memory.h>

// Display System - Advanced display management for RaeenOS
// Provides multi-monitor support, HDR, and modern display technologies

// Display technologies
typedef enum {
    DISPLAY_TECHNOLOGY_LCD = 0,      // LCD display
    DISPLAY_TECHNOLOGY_OLED,         // OLED display
    DISPLAY_TECHNOLOGY_QLED,         // QLED display
    DISPLAY_TECHNOLOGY_MICROLED,     // MicroLED display
    DISPLAY_TECHNOLOGY_CRT,          // CRT display (legacy)
    DISPLAY_TECHNOLOGY_PLASMA,       // Plasma display (legacy)
} display_technology_t;

// Display interfaces
typedef enum {
    DISPLAY_INTERFACE_VGA = 0,       // VGA
    DISPLAY_INTERFACE_DVI,           // DVI
    DISPLAY_INTERFACE_HDMI,          // HDMI
    DISPLAY_INTERFACE_DISPLAYPORT,   // DisplayPort
    DISPLAY_INTERFACE_USB_C,         // USB-C
    DISPLAY_INTERFACE_THUNDERBOLT,   // Thunderbolt
    DISPLAY_INTERFACE_WIRELESS,      // Wireless display
} display_interface_t;

// Display resolutions
typedef enum {
    RESOLUTION_640x480 = 0,          // VGA
    RESOLUTION_800x600,              // SVGA
    RESOLUTION_1024x768,             // XGA
    RESOLUTION_1280x720,             // HD
    RESOLUTION_1280x1024,            // SXGA
    RESOLUTION_1366x768,             // HD
    RESOLUTION_1440x900,             // WXGA+
    RESOLUTION_1600x900,             // HD+
    RESOLUTION_1680x1050,            // WSXGA+
    RESOLUTION_1920x1080,            // Full HD
    RESOLUTION_1920x1200,            // WUXGA
    RESOLUTION_2560x1440,            // QHD
    RESOLUTION_2560x1600,            // WQXGA
    RESOLUTION_3440x1440,            // UWQHD
    RESOLUTION_3840x2160,            // 4K UHD
    RESOLUTION_5120x2880,            // 5K
    RESOLUTION_7680x4320,            // 8K UHD
} display_resolution_t;

// Refresh rates
typedef enum {
    REFRESH_RATE_30 = 30,            // 30 Hz
    REFRESH_RATE_50 = 50,            // 50 Hz
    REFRESH_RATE_60 = 60,            // 60 Hz
    REFRESH_RATE_75 = 75,            // 75 Hz
    REFRESH_RATE_90 = 90,            // 90 Hz
    REFRESH_RATE_120 = 120,          // 120 Hz
    REFRESH_RATE_144 = 144,          // 144 Hz
    REFRESH_RATE_165 = 165,          // 165 Hz
    REFRESH_RATE_240 = 240,          // 240 Hz
    REFRESH_RATE_360 = 360,          // 360 Hz
} refresh_rate_t;

// Color formats
typedef enum {
    COLOR_FORMAT_RGB_565 = 0,        // 16-bit RGB
    COLOR_FORMAT_RGB_888,            // 24-bit RGB
    COLOR_FORMAT_RGBA_8888,          // 32-bit RGBA
    COLOR_FORMAT_BGR_888,            // 24-bit BGR
    COLOR_FORMAT_BGRA_8888,          // 32-bit BGRA
    COLOR_FORMAT_YUV_420,            // YUV 4:2:0
    COLOR_FORMAT_YUV_422,            // YUV 4:2:2
    COLOR_FORMAT_YUV_444,            // YUV 4:4:4
} color_format_t;

// HDR standards
typedef enum {
    HDR_STANDARD_NONE = 0,           // No HDR
    HDR_STANDARD_HDR10,              // HDR10
    HDR_STANDARD_HDR10_PLUS,         // HDR10+
    HDR_STANDARD_DOLBY_VISION,       // Dolby Vision
    HDR_STANDARD_HLG,                // HLG
    HDR_STANDARD_ADVANCED_HDR,       // Advanced HDR
} hdr_standard_t;

// Display modes
typedef enum {
    DISPLAY_MODE_EXTENDED = 0,       // Extended desktop
    DISPLAY_MODE_MIRRORED,           // Mirrored displays
    DISPLAY_MODE_SINGLE,             // Single display
    DISPLAY_MODE_PRESENTATION,       // Presentation mode
    DISPLAY_MODE_GAMING,             // Gaming mode
    DISPLAY_MODE_CREATIVE,           // Creative work mode
} display_mode_t;

// Display information
typedef struct {
    uint32_t id;
    char name[64];                   // Display name
    char manufacturer[32];            // Manufacturer
    char model[32];                  // Model number
    char serial[32];                 // Serial number
    display_technology_t technology; // Display technology
    display_interface_t interface;   // Connection interface
    uint32_t width, height;          // Native resolution
    refresh_rate_t refresh_rate;     // Native refresh rate
    color_format_t color_format;     // Color format
    uint32_t color_depth;            // Color depth in bits
    hdr_standard_t hdr_standard;     // HDR support
    float brightness;                // Brightness in nits
    float contrast_ratio;            // Contrast ratio
    uint32_t response_time;          // Response time in ms
    bool is_primary;                 // Is primary display
    bool is_active;                  // Is currently active
    bool is_connected;               // Is connected
    bool is_enabled;                 // Is enabled
    uint32_t x_offset, y_offset;     // Position offset
    uint32_t rotation;               // Rotation (0, 90, 180, 270)
    float scaling;                   // Display scaling
    void* driver_data;               // Driver-specific data
} display_info_t;

// Display mode
typedef struct {
    uint32_t width, height;          // Resolution
    refresh_rate_t refresh_rate;     // Refresh rate
    color_format_t color_format;     // Color format
    uint32_t color_depth;            // Color depth
    bool is_interlaced;              // Is interlaced
    bool is_stereo;                  // Is stereo
    uint32_t pixel_clock;            // Pixel clock in kHz
    uint32_t horizontal_sync;        // Horizontal sync
    uint32_t vertical_sync;          // Vertical sync
    uint32_t horizontal_front_porch; // Horizontal front porch
    uint32_t horizontal_back_porch;  // Horizontal back porch
    uint32_t vertical_front_porch;   // Vertical front porch
    uint32_t vertical_back_porch;    // Vertical back porch
} display_mode_t;

// Display configuration
typedef struct {
    display_mode_t mode;             // Display mode
    uint32_t x_offset, y_offset;     // Position
    uint32_t rotation;               // Rotation
    float scaling;                   // Scaling
    bool is_primary;                 // Is primary
    bool is_enabled;                 // Is enabled
    bool enable_hdr;                 // Enable HDR
    bool enable_vrr;                 // Enable VRR
    bool enable_adaptive_sync;       // Enable adaptive sync
    uint32_t brightness;             // Brightness (0-100)
    uint32_t contrast;               // Contrast (0-100)
    uint32_t gamma;                  // Gamma correction
    uint32_t color_temperature;      // Color temperature
} display_config_t;

// Multi-monitor configuration
typedef struct {
    display_mode_t mode;             // Multi-monitor mode
    display_info_t* displays;        // Array of displays
    uint32_t display_count;          // Number of displays
    uint32_t max_displays;           // Maximum displays
    uint32_t primary_display;        // Primary display ID
    bool enable_spanning;            // Enable display spanning
    bool enable_mirroring;           // Enable display mirroring
    uint32_t total_width;            // Total virtual width
    uint32_t total_height;           // Total virtual height
} multi_monitor_config_t;

// Display system configuration
typedef struct {
    bool enable_multi_monitor;       // Enable multi-monitor
    bool enable_hdr;                 // Enable HDR
    bool enable_vrr;                 // Enable VRR
    bool enable_adaptive_sync;       // Enable adaptive sync
    bool enable_display_scaling;     // Enable display scaling
    bool enable_color_management;    // Enable color management
    bool enable_display_calibration; // Enable display calibration
    uint32_t max_displays;           // Maximum displays
    uint32_t default_refresh_rate;   // Default refresh rate
    uint32_t default_color_depth;    // Default color depth
    float default_scaling;           // Default scaling
    uint32_t hotplug_timeout;        // Hotplug timeout in ms
} display_system_config_t;

// Display system context
typedef struct {
    display_system_config_t config;
    display_info_t* displays;
    uint32_t display_count;
    uint32_t max_displays;
    multi_monitor_config_t multi_monitor;
    display_info_t* primary_display;
    bool initialized;
    bool multi_monitor_enabled;
    bool hdr_enabled;
    bool vrr_enabled;
    uint32_t next_display_id;
    uint64_t last_update_time;
} display_system_t;

// Function prototypes

// Initialization and shutdown
display_system_t* display_system_init(display_system_config_t* config);
void display_system_shutdown(display_system_t* ds);
bool display_system_is_initialized(display_system_t* ds);

// Display detection and management
uint32_t display_system_add_display(display_system_t* ds, display_info_t* display);
void display_system_remove_display(display_system_t* ds, uint32_t display_id);
display_info_t* display_system_get_display(display_system_t* ds, uint32_t display_id);
display_info_t* display_system_get_displays(display_system_t* ds, uint32_t* count);
uint32_t display_system_get_display_count(display_system_t* ds);
display_info_t* display_system_get_primary_display(display_system_t* ds);
void display_system_set_primary_display(display_system_t* ds, uint32_t display_id);

// Display configuration
void display_system_set_display_mode(display_system_t* ds, uint32_t display_id, display_mode_t* mode);
display_mode_t* display_system_get_display_mode(display_system_t* ds, uint32_t display_id);
void display_system_set_display_position(display_system_t* ds, uint32_t display_id, uint32_t x, uint32_t y);
void display_system_get_display_position(display_system_t* ds, uint32_t display_id, uint32_t* x, uint32_t* y);
void display_system_set_display_rotation(display_system_t* ds, uint32_t display_id, uint32_t rotation);
uint32_t display_system_get_display_rotation(display_system_t* ds, uint32_t display_id);
void display_system_set_display_scaling(display_system_t* ds, uint32_t display_id, float scaling);
float display_system_get_display_scaling(display_system_t* ds, uint32_t display_id);

// Multi-monitor management
void display_system_enable_multi_monitor(display_system_t* ds, bool enable);
bool display_system_is_multi_monitor_enabled(display_system_t* ds);
void display_system_set_multi_monitor_mode(display_system_t* ds, display_mode_t mode);
display_mode_t display_system_get_multi_monitor_mode(display_system_t* ds);
void display_system_enable_display_spanning(display_system_t* ds, bool enable);
bool display_system_is_display_spanning_enabled(display_system_t* ds);
void display_system_enable_display_mirroring(display_system_t* ds, bool enable);
bool display_system_is_display_mirroring_enabled(display_system_t* ds);
multi_monitor_config_t* display_system_get_multi_monitor_config(display_system_t* ds);

// Display modes
display_mode_t* display_system_get_supported_modes(display_system_t* ds, uint32_t display_id, uint32_t* count);
bool display_system_is_mode_supported(display_system_t* ds, uint32_t display_id, display_mode_t* mode);
display_mode_t* display_system_get_native_mode(display_system_t* ds, uint32_t display_id);
display_mode_t* display_system_get_optimal_mode(display_system_t* ds, uint32_t display_id);
display_mode_t* display_system_get_gaming_mode(display_system_t* ds, uint32_t display_id);
display_mode_t* display_system_get_creative_mode(display_system_t* ds, uint32_t display_id);

// HDR and color management
void display_system_enable_hdr(display_system_t* ds, bool enable);
bool display_system_is_hdr_enabled(display_system_t* ds);
hdr_standard_t display_system_get_hdr_standard(display_system_t* ds, uint32_t display_id);
void display_system_set_hdr_standard(display_system_t* ds, uint32_t display_id, hdr_standard_t standard);
void display_system_enable_color_management(display_system_t* ds, bool enable);
bool display_system_is_color_management_enabled(display_system_t* ds);
void display_system_set_color_profile(display_system_t* ds, uint32_t display_id, const char* profile_path);
const char* display_system_get_color_profile(display_system_t* ds, uint32_t display_id);

// VRR and adaptive sync
void display_system_enable_vrr(display_system_t* ds, bool enable);
bool display_system_is_vrr_enabled(display_system_t* ds);
void display_system_enable_adaptive_sync(display_system_t* ds, bool enable);
bool display_system_is_adaptive_sync_enabled(display_system_t* ds);
uint32_t display_system_get_vrr_range(display_system_t* ds, uint32_t display_id, uint32_t* min, uint32_t* max);
bool display_system_is_vrr_supported(display_system_t* ds, uint32_t display_id);

// Display calibration
void display_system_start_calibration(display_system_t* ds, uint32_t display_id);
void display_system_stop_calibration(display_system_t* ds, uint32_t display_id);
bool display_system_is_calibrating(display_system_t* ds, uint32_t display_id);
void display_system_set_brightness(display_system_t* ds, uint32_t display_id, uint32_t brightness);
uint32_t display_system_get_brightness(display_system_t* ds, uint32_t display_id);
void display_system_set_contrast(display_system_t* ds, uint32_t display_id, uint32_t contrast);
uint32_t display_system_get_contrast(display_system_t* ds, uint32_t display_id);
void display_system_set_gamma(display_system_t* ds, uint32_t display_id, uint32_t gamma);
uint32_t display_system_get_gamma(display_system_t* ds, uint32_t display_id);
void display_system_set_color_temperature(display_system_t* ds, uint32_t display_id, uint32_t temperature);
uint32_t display_system_get_color_temperature(display_system_t* ds, uint32_t display_id);

// Hotplug and events
void display_system_handle_hotplug(display_system_t* ds, uint32_t display_id, bool connected);
bool display_system_is_display_connected(display_system_t* ds, uint32_t display_id);
void display_system_detect_displays(display_system_t* ds);
void display_system_refresh_displays(display_system_t* ds);

// Information and statistics
uint32_t display_system_get_total_width(display_system_t* ds);
uint32_t display_system_get_total_height(display_system_t* ds);
uint32_t display_system_get_max_refresh_rate(display_system_t* ds, uint32_t display_id);
uint32_t display_system_get_max_resolution(display_system_t* ds, uint32_t display_id, uint32_t* width, uint32_t* height);
bool display_system_is_display_active(display_system_t* ds, uint32_t display_id);
bool display_system_is_display_enabled(display_system_t* ds, uint32_t display_id);

// Utility functions
bool display_system_is_resolution_supported(uint32_t width, uint32_t height);
bool display_system_is_refresh_rate_supported(refresh_rate_t refresh_rate);
char* display_system_get_technology_name(display_technology_t technology);
char* display_system_get_interface_name(display_interface_t interface);
char* display_system_get_resolution_name(display_resolution_t resolution);
char* display_system_get_hdr_standard_name(hdr_standard_t standard);
uint32_t display_system_calculate_pixel_clock(uint32_t width, uint32_t height, refresh_rate_t refresh_rate);

// Callbacks
typedef void (*display_hotplug_callback_t)(display_system_t* ds, uint32_t display_id, bool connected, void* user_data);
typedef void (*display_mode_change_callback_t)(display_system_t* ds, uint32_t display_id, display_mode_t* old_mode, display_mode_t* new_mode, void* user_data);

void display_system_set_hotplug_callback(display_system_t* ds, display_hotplug_callback_t callback, void* user_data);
void display_system_set_mode_change_callback(display_system_t* ds, display_mode_change_callback_t callback, void* user_data);

// Preset configurations
display_system_config_t display_system_preset_normal_style(void);
display_system_config_t display_system_preset_gaming_style(void);
display_system_config_t display_system_preset_creative_style(void);
display_system_config_t display_system_preset_enterprise_style(void);

// Error handling
typedef enum {
    DISPLAY_SYSTEM_SUCCESS = 0,
    DISPLAY_SYSTEM_ERROR_INVALID_CONTEXT,
    DISPLAY_SYSTEM_ERROR_INVALID_DISPLAY,
    DISPLAY_SYSTEM_ERROR_INVALID_MODE,
    DISPLAY_SYSTEM_ERROR_OUT_OF_MEMORY,
    DISPLAY_SYSTEM_ERROR_DRIVER_FAILED,
    DISPLAY_SYSTEM_ERROR_UNSUPPORTED_MODE,
    DISPLAY_SYSTEM_ERROR_DISPLAY_NOT_FOUND,
    DISPLAY_SYSTEM_ERROR_CALIBRATION_FAILED,
} display_system_error_t;

display_system_error_t display_system_get_last_error(void);
const char* display_system_get_error_string(display_system_error_t error);

#endif // DISPLAY_SYSTEM_H 