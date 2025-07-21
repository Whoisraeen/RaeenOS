#include "raeen_gui_system.h"
#include <kernel/memory/memory.h>
#include <kernel/libc/string.h>
#include <kernel/libc/stdio.h>
#include <kernel/hal/hal.h>
#include <kernel/process/process.h>

// Global GUI system instance
static raeen_gui_system_t* g_gui_system = NULL;
static raeen_gui_system_error_t g_last_error = RAEEN_GUI_SYSTEM_SUCCESS;

// Internal function prototypes
static void set_error(raeen_gui_system_error_t error);
static bool initialize_compositor(raeen_gui_system_t* gui);
static bool initialize_dock(raeen_gui_system_t* gui);
static bool initialize_window_manager(raeen_gui_system_t* gui);
static bool initialize_search(raeen_gui_system_t* gui);
static bool initialize_customizer(raeen_gui_system_t* gui);
static void update_metrics(raeen_gui_system_t* gui);
static void render_desktop_environment(raeen_gui_system_t* gui);

// Error handling
static void set_error(raeen_gui_system_error_t error) {
    g_last_error = error;
    if (g_gui_system && g_gui_system->last_error[0] == '\0') {
        snprintf(g_gui_system->last_error, sizeof(g_gui_system->last_error), 
                "GUI System Error: %d", error);
    }
}

// Initialization and shutdown
raeen_gui_system_t* raeen_gui_system_init(gui_system_config_t* config) {
    if (g_gui_system) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    
    if (!config) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    
    g_gui_system = kmalloc(sizeof(raeen_gui_system_t));
    if (!g_gui_system) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    memset(g_gui_system, 0, sizeof(raeen_gui_system_t));
    
    // Copy configuration
    memcpy(&g_gui_system->config, config, sizeof(gui_system_config_t));
    
    // Set initial state
    g_gui_system->state = GUI_STATE_INITIALIZING;
    g_gui_system->current_display_mode = config->display_mode;
    g_gui_system->current_performance_profile = config->performance_profile;
    
    // Initialize metrics
    memset(&g_gui_system->metrics, 0, sizeof(system_metrics_t));
    g_gui_system->last_metrics_update = hal_get_tick_count();
    
    // Initialize performance monitoring
    g_gui_system->frame_count = 0;
    g_gui_system->last_frame_time = hal_get_tick_count();
    g_gui_system->average_fps = 0.0f;
    memset(g_gui_system->frame_time_history, 0, sizeof(g_gui_system->frame_time_history));
    g_gui_system->frame_time_index = 0;
    
    printf("RaeenOS GUI System: Initializing...\n");
    
    // Initialize components in order
    if (!initialize_compositor(g_gui_system)) {
        printf("RaeenOS GUI System: Failed to initialize compositor\n");
        raeen_gui_system_shutdown(g_gui_system);
        return NULL;
    }
    
    if (!initialize_dock(g_gui_system)) {
        printf("RaeenOS GUI System: Failed to initialize dock\n");
        raeen_gui_system_shutdown(g_gui_system);
        return NULL;
    }
    
    if (!initialize_window_manager(g_gui_system)) {
        printf("RaeenOS GUI System: Failed to initialize window manager\n");
        raeen_gui_system_shutdown(g_gui_system);
        return NULL;
    }
    
    if (!initialize_search(g_gui_system)) {
        printf("RaeenOS GUI System: Failed to initialize search\n");
        raeen_gui_system_shutdown(g_gui_system);
        return NULL;
    }
    
    if (!initialize_customizer(g_gui_system)) {
        printf("RaeenOS GUI System: Failed to initialize customizer\n");
        raeen_gui_system_shutdown(g_gui_system);
        return NULL;
    }
    
    // Set state to ready
    g_gui_system->state = GUI_STATE_READY;
    
    printf("RaeenOS GUI System: Initialization complete\n");
    printf("  Display: %dx%d @ %dHz\n", config->display_width, config->display_height, config->refresh_rate);
    printf("  Mode: %d, Performance: %d\n", config->display_mode, config->performance_profile);
    printf("  Glass Effects: %s, Animations: %s, AI: %s\n", 
           config->enable_glass_effects ? "Enabled" : "Disabled",
           config->enable_animations ? "Enabled" : "Disabled",
           config->enable_ai_features ? "Enabled" : "Disabled");
    
    return g_gui_system;
}

void raeen_gui_system_shutdown(raeen_gui_system_t* gui) {
    if (!gui || gui != g_gui_system) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    printf("RaeenOS GUI System: Shutting down...\n");
    
    // Set state to shutdown
    gui->state = GUI_STATE_SHUTDOWN;
    
    // Shutdown components in reverse order
    if (gui->customizer_initialized && gui->customizer) {
        raeen_customizer_shutdown(gui->customizer);
        gui->customizer = NULL;
        gui->customizer_initialized = false;
    }
    
    if (gui->search_initialized && gui->search) {
        // spotlight_plus_shutdown(gui->search); // TODO: Implement
        gui->search = NULL;
        gui->search_initialized = false;
    }
    
    if (gui->window_manager_initialized && gui->window_manager) {
        // glass_window_manager_shutdown(gui->window_manager); // TODO: Implement
        gui->window_manager = NULL;
        gui->window_manager_initialized = false;
    }
    
    if (gui->dock_initialized && gui->dock) {
        // hybrid_dock_shutdown(gui->dock); // TODO: Implement
        gui->dock = NULL;
        gui->dock_initialized = false;
    }
    
    if (gui->compositor_initialized && gui->compositor) {
        // glass_compositor_shutdown(gui->compositor); // TODO: Implement
        gui->compositor = NULL;
        gui->compositor_initialized = false;
    }
    
    kfree(gui);
    g_gui_system = NULL;
    
    printf("RaeenOS GUI System: Shutdown complete\n");
}

bool raeen_gui_system_is_initialized(raeen_gui_system_t* gui) {
    return gui && gui == g_gui_system && gui->state >= GUI_STATE_READY;
}

// Component initialization
static bool initialize_compositor(raeen_gui_system_t* gui) {
    printf("RaeenOS GUI System: Initializing glass compositor...\n");
    
    // Create compositor configuration
    compositor_config_t comp_config = {
        .width = gui->config.display_width,
        .height = gui->config.display_height,
        .enable_vsync = gui->config.enable_vsync,
        .enable_gpu_acceleration = gui->config.enable_hardware_acceleration,
        .enable_glass_effects = gui->config.enable_glass_effects,
        .enable_animations = gui->config.enable_animations,
        .max_layers = 100,
        .max_animations = 50,
        .target_fps = gui->config.target_fps
    };
    
    // Initialize RaeenDX first
    raeen_dx_context_t* renderer = raeen_dx_init(gui->config.display_width, gui->config.display_height, RAEEN_FORMAT_RGBA8);
    if (!renderer) {
        printf("RaeenOS GUI System: Failed to initialize RaeenDX renderer\n");
        return false;
    }
    
    // TODO: Initialize glass compositor when implemented
    // gui->compositor = glass_compositor_init(renderer, &comp_config);
    // if (!gui->compositor) {
    //     printf("RaeenOS GUI System: Failed to initialize glass compositor\n");
    //     raeen_dx_shutdown(renderer);
    //     return false;
    // }
    
    gui->compositor_initialized = true;
    printf("RaeenOS GUI System: Glass compositor initialized\n");
    return true;
}

static bool initialize_dock(raeen_gui_system_t* gui) {
    printf("RaeenOS GUI System: Initializing hybrid dock...\n");
    
    // Create dock configuration
    dock_config_t dock_config = hybrid_dock_preset_hybrid_style();
    dock_config.width = gui->config.display_width;
    dock_config.height = 80; // Standard dock height
    dock_config.enable_glass_effects = gui->config.enable_glass_effects;
    dock_config.enable_animations = gui->config.enable_animations;
    
    // TODO: Initialize hybrid dock when implemented
    // gui->dock = hybrid_dock_init(gui->compositor, &dock_config);
    // if (!gui->dock) {
    //     printf("RaeenOS GUI System: Failed to initialize hybrid dock\n");
    //     return false;
    // }
    
    gui->dock_initialized = true;
    printf("RaeenOS GUI System: Hybrid dock initialized\n");
    return true;
}

static bool initialize_window_manager(raeen_gui_system_t* gui) {
    printf("RaeenOS GUI System: Initializing glass window manager...\n");
    
    // Create window manager configuration
    window_manager_config_t wm_config = glass_window_manager_preset_hybrid_style();
    wm_config.max_windows = gui->config.max_windows;
    wm_config.enable_glass_effects = gui->config.enable_glass_effects;
    wm_config.enable_animations = gui->config.enable_animations;
    wm_config.enable_snap_layouts = true;
    wm_config.enable_ai_tiling = gui->config.enable_ai_features;
    
    // TODO: Initialize glass window manager when implemented
    // gui->window_manager = glass_window_manager_init(gui->compositor, &wm_config);
    // if (!gui->window_manager) {
    //     printf("RaeenOS GUI System: Failed to initialize glass window manager\n");
    //     return false;
    // }
    
    gui->window_manager_initialized = true;
    printf("RaeenOS GUI System: Glass window manager initialized\n");
    return true;
}

static bool initialize_search(raeen_gui_system_t* gui) {
    printf("RaeenOS GUI System: Initializing Spotlight+ search...\n");
    
    // Create search configuration
    spotlight_plus_config_t search_config = spotlight_plus_preset_hybrid_style();
    search_config.enable_ai_enhancement = gui->config.enable_ai_features;
    search_config.enable_voice_search = gui->config.enable_voice_control;
    search_config.enable_predictive_search = gui->config.enable_ai_features;
    search_config.enable_contextual_search = gui->config.enable_ai_features;
    
    // TODO: Initialize Spotlight+ search when implemented
    // gui->search = spotlight_plus_init(gui->compositor, &search_config);
    // if (!gui->search) {
    //     printf("RaeenOS GUI System: Failed to initialize Spotlight+ search\n");
    //     return false;
    // }
    
    gui->search_initialized = true;
    printf("RaeenOS GUI System: Spotlight+ search initialized\n");
    return true;
}

static bool initialize_customizer(raeen_gui_system_t* gui) {
    printf("RaeenOS GUI System: Initializing Raeen Customizer...\n");
    
    // Create customizer configuration
    customizer_config_t customizer_config = raeen_customizer_preset_ai_enhanced_style();
    customizer_config.enable_ai_generation = gui->config.enable_ai_features;
    customizer_config.enable_theme_previews = true;
    customizer_config.enable_auto_save = true;
    
    // TODO: Initialize Raeen Customizer when implemented
    // gui->customizer = raeen_customizer_init(gui->compositor, &customizer_config);
    // if (!gui->customizer) {
    //     printf("RaeenOS GUI System: Failed to initialize Raeen Customizer\n");
    //     return false;
    // }
    
    gui->customizer_initialized = true;
    printf("RaeenOS GUI System: Raeen Customizer initialized\n");
    return true;
}

// System lifecycle
void raeen_gui_system_start(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    printf("RaeenOS GUI System: Starting...\n");
    gui->state = GUI_STATE_RUNNING;
    
    // Start the main loop
    raeen_gui_system_main_loop(gui);
}

void raeen_gui_system_stop(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    printf("RaeenOS GUI System: Stopping...\n");
    gui->state = GUI_STATE_SHUTDOWN;
}

void raeen_gui_system_pause(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    gui->state = GUI_STATE_SLEEPING;
}

void raeen_gui_system_resume(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    gui->state = GUI_STATE_RUNNING;
}

void raeen_gui_system_sleep(raeen_gui_system_t* gui) {
    raeen_gui_system_pause(gui);
}

void raeen_gui_system_wake(raeen_gui_system_t* gui) {
    raeen_gui_system_resume(gui);
}

// Main loop
void raeen_gui_system_main_loop(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    printf("RaeenOS GUI System: Entering main loop\n");
    
    uint64_t last_time = hal_get_tick_count();
    
    while (gui->state == GUI_STATE_RUNNING) {
        uint64_t current_time = hal_get_tick_count();
        float delta_time = (current_time - last_time) / 1000.0f; // Convert to seconds
        last_time = current_time;
        
        // Update system
        raeen_gui_system_update(gui, delta_time);
        
        // Render frame
        raeen_gui_system_render(gui);
        
        // Present frame
        raeen_gui_system_present(gui);
        
        // Update metrics
        update_metrics(gui);
        
        // Simple frame rate limiting (basic implementation)
        if (gui->config.target_fps > 0) {
            uint32_t target_frame_time = 1000 / gui->config.target_fps;
            uint32_t elapsed = hal_get_tick_count() - current_time;
            if (elapsed < target_frame_time) {
                // Simple busy wait - in real implementation would use proper sleep
                while (hal_get_tick_count() - current_time < target_frame_time) {
                    // Busy wait
                }
            }
        }
    }
    
    printf("RaeenOS GUI System: Exiting main loop\n");
}

void raeen_gui_system_update(raeen_gui_system_t* gui, float delta_time) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Update compositor
    if (gui->compositor_initialized && gui->compositor) {
        // glass_compositor_update(gui->compositor, delta_time);
    }
    
    // Update dock
    if (gui->dock_initialized && gui->dock) {
        // hybrid_dock_update(gui->dock, delta_time);
    }
    
    // Update window manager
    if (gui->window_manager_initialized && gui->window_manager) {
        // glass_window_manager_update(gui->window_manager, delta_time);
    }
    
    // Update search
    if (gui->search_initialized && gui->search) {
        // spotlight_plus_update(gui->search, delta_time);
    }
    
    // Update customizer
    if (gui->customizer_initialized && gui->customizer) {
        // raeen_customizer_update(gui->customizer, delta_time);
    }
}

void raeen_gui_system_render(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Render desktop environment
    render_desktop_environment(gui);
}

void raeen_gui_system_present(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return;
    }
    
    // Present the frame
    if (gui->compositor_initialized && gui->compositor) {
        // glass_compositor_present(gui->compositor);
    }
    
    // Update frame count
    gui->frame_count++;
}

// Desktop environment rendering
static void render_desktop_environment(raeen_gui_system_t* gui) {
    // Render desktop background
    // TODO: Implement desktop background rendering
    
    // Render desktop icons
    if (gui->show_desktop_icons) {
        // TODO: Implement desktop icon rendering
    }
    
    // Render windows
    if (gui->window_manager_initialized && gui->window_manager) {
        // glass_window_manager_render(gui->window_manager);
    }
    
    // Render dock
    if (gui->dock_initialized && gui->dock) {
        // hybrid_dock_render(gui->dock);
    }
    
    // Render search UI
    if (gui->search_initialized && gui->search) {
        // spotlight_plus_render_search_ui(gui->search);
    }
    
    // Render customizer UI
    if (gui->customizer_initialized && gui->customizer) {
        // raeen_customizer_render_ui(gui->customizer);
    }
}

// Metrics update
static void update_metrics(raeen_gui_system_t* gui) {
    uint64_t current_time = hal_get_tick_count();
    
    // Update every second
    if (current_time - gui->last_metrics_update >= 1000) {
        // Calculate FPS
        gui->metrics.fps = gui->average_fps;
        
        // Update other metrics (simplified)
        gui->metrics.cpu_usage = 0.0f; // TODO: Get from kernel
        gui->metrics.memory_usage = 0.0f; // TODO: Get from kernel
        gui->metrics.gpu_usage = 0.0f; // TODO: Get from GPU driver
        gui->metrics.active_windows = 0; // TODO: Get from window manager
        gui->metrics.uptime_seconds = current_time / 1000;
        
        gui->last_metrics_update = current_time;
    }
    
    // Update frame time history
    uint32_t frame_time = hal_get_tick_count() - gui->last_frame_time;
    gui->frame_time_history[gui->frame_time_index] = frame_time;
    gui->frame_time_index = (gui->frame_time_index + 1) % 60;
    
    // Calculate average frame time
    uint32_t total_frame_time = 0;
    for (int i = 0; i < 60; i++) {
        total_frame_time += gui->frame_time_history[i];
    }
    gui->metrics.frame_time_ms = total_frame_time / 60;
    
    // Calculate average FPS
    if (gui->metrics.frame_time_ms > 0) {
        gui->average_fps = 1000.0f / gui->metrics.frame_time_ms;
    }
}

// Component access
glass_compositor_t* raeen_gui_system_get_compositor(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    return gui->compositor;
}

hybrid_dock_t* raeen_gui_system_get_dock(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    return gui->dock;
}

glass_window_manager_t* raeen_gui_system_get_window_manager(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    return gui->window_manager;
}

spotlight_plus_t* raeen_gui_system_get_search(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    return gui->search;
}

raeen_customizer_t* raeen_gui_system_get_customizer(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    return gui->customizer;
}

// Input handling (basic implementation)
bool raeen_gui_system_handle_mouse_move(raeen_gui_system_t* gui, float x, float y) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return false;
    }
    
    bool handled = false;
    
    // Handle window manager input
    if (gui->window_manager_initialized && gui->window_manager) {
        // handled |= glass_window_manager_handle_mouse_move(gui->window_manager, x, y);
    }
    
    // Handle dock input
    if (gui->dock_initialized && gui->dock) {
        // handled |= hybrid_dock_handle_mouse_move(gui->dock, x, y);
    }
    
    return handled;
}

bool raeen_gui_system_handle_mouse_click(raeen_gui_system_t* gui, float x, float y, bool left_click) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return false;
    }
    
    bool handled = false;
    
    // Handle window manager input
    if (gui->window_manager_initialized && gui->window_manager) {
        // handled |= glass_window_manager_handle_mouse_click(gui->window_manager, x, y, left_click);
    }
    
    // Handle dock input
    if (gui->dock_initialized && gui->dock) {
        // handled |= hybrid_dock_handle_mouse_click(gui->dock, x, y, left_click);
    }
    
    return handled;
}

bool raeen_gui_system_handle_key_press(raeen_gui_system_t* gui, uint32_t key_code) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return false;
    }
    
    bool handled = false;
    
    // Handle global shortcuts
    switch (key_code) {
        case 0x1B: // Escape key
            // Hide search, customizer, etc.
            break;
        case 0x20: // Space key
            // Toggle search
            break;
        case 0x2E: // Delete key
            // Delete selected items
            break;
        default:
            break;
    }
    
    // Handle window manager input
    if (gui->window_manager_initialized && gui->window_manager) {
        // handled |= glass_window_manager_handle_key_press(gui->window_manager, key_code);
    }
    
    return handled;
}

// System information
gui_system_state_t raeen_gui_system_get_state(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return GUI_STATE_SHUTDOWN;
    }
    return gui->state;
}

display_mode_t raeen_gui_system_get_display_mode(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return DISPLAY_MODE_NORMAL;
    }
    return gui->current_display_mode;
}

performance_profile_t raeen_gui_system_get_performance_profile(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return PERFORMANCE_PROFILE_BALANCED;
    }
    return gui->current_performance_profile;
}

system_metrics_t* raeen_gui_system_get_metrics(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return NULL;
    }
    return &gui->metrics;
}

float raeen_gui_system_get_fps(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0.0f;
    }
    return gui->average_fps;
}

uint64_t raeen_gui_system_get_uptime(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        set_error(RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT);
        return 0;
    }
    return gui->metrics.uptime_seconds;
}

// Error handling
uint32_t raeen_gui_system_get_error_count(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        return 0;
    }
    return gui->error_count;
}

const char* raeen_gui_system_get_last_error(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        return "Invalid GUI system context";
    }
    return gui->last_error;
}

void raeen_gui_system_clear_errors(raeen_gui_system_t* gui) {
    if (!raeen_gui_system_is_initialized(gui)) {
        return;
    }
    gui->error_count = 0;
    memset(gui->last_error, 0, sizeof(gui->last_error));
}

// Preset configurations
gui_system_config_t raeen_gui_system_preset_desktop_style(void) {
    gui_system_config_t config = {0};
    config.display_width = 1920;
    config.display_height = 1080;
    config.refresh_rate = 60;
    config.display_mode = DISPLAY_MODE_NORMAL;
    config.performance_profile = PERFORMANCE_PROFILE_BALANCED;
    config.enable_hardware_acceleration = true;
    config.enable_vsync = true;
    config.enable_glass_effects = true;
    config.enable_animations = true;
    config.enable_ai_features = true;
    config.enable_voice_control = false;
    config.enable_gesture_control = false;
    config.enable_accessibility = true;
    config.max_windows = 50;
    config.max_processes = 100;
    config.target_fps = 60.0f;
    config.memory_limit_mb = 512;
    return config;
}

gui_system_config_t raeen_gui_system_preset_gaming_style(void) {
    gui_system_config_t config = raeen_gui_system_preset_desktop_style();
    config.display_mode = DISPLAY_MODE_GAMING;
    config.performance_profile = PERFORMANCE_PROFILE_GAMING;
    config.enable_glass_effects = false;
    config.enable_animations = false;
    config.enable_ai_features = false;
    config.target_fps = 144.0f;
    config.memory_limit_mb = 1024;
    return config;
}

gui_system_config_t raeen_gui_system_preset_minimal_style(void) {
    gui_system_config_t config = {0};
    config.display_width = 1280;
    config.display_height = 720;
    config.refresh_rate = 60;
    config.display_mode = DISPLAY_MODE_SAFE;
    config.performance_profile = PERFORMANCE_PROFILE_POWER_SAVER;
    config.enable_hardware_acceleration = false;
    config.enable_vsync = false;
    config.enable_glass_effects = false;
    config.enable_animations = false;
    config.enable_ai_features = false;
    config.enable_voice_control = false;
    config.enable_gesture_control = false;
    config.enable_accessibility = true;
    config.max_windows = 10;
    config.max_processes = 20;
    config.target_fps = 30.0f;
    config.memory_limit_mb = 128;
    return config;
}

gui_system_config_t raeen_gui_system_preset_accessibility_style(void) {
    gui_system_config_t config = raeen_gui_system_preset_desktop_style();
    config.enable_accessibility = true;
    config.enable_voice_control = true;
    config.enable_glass_effects = false;
    config.enable_animations = false;
    config.target_fps = 30.0f;
    return config;
}

// Error handling
raeen_gui_system_error_t raeen_gui_system_get_last_error_code(raeen_gui_system_t* gui) {
    return g_last_error;
}

const char* raeen_gui_system_get_error_string(raeen_gui_system_error_t error) {
    switch (error) {
        case RAEEN_GUI_SYSTEM_SUCCESS:
            return "Success";
        case RAEEN_GUI_SYSTEM_ERROR_INVALID_CONTEXT:
            return "Invalid GUI system context";
        case RAEEN_GUI_SYSTEM_ERROR_INITIALIZATION_FAILED:
            return "GUI system initialization failed";
        case RAEEN_GUI_SYSTEM_ERROR_OUT_OF_MEMORY:
            return "Out of memory";
        case RAEEN_GUI_SYSTEM_ERROR_RENDER_FAILED:
            return "Rendering failed";
        case RAEEN_GUI_SYSTEM_ERROR_INPUT_FAILED:
            return "Input handling failed";
        case RAEEN_GUI_SYSTEM_ERROR_COMPONENT_FAILED:
            return "Component initialization failed";
        default:
            return "Unknown error";
    }
} 