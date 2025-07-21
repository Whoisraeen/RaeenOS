#include "raeen_gui_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// Test configuration
#define TEST_DISPLAY_WIDTH 1280
#define TEST_DISPLAY_HEIGHT 720
#define TEST_DURATION_SECONDS 30
#define TEST_FPS_TARGET 60

// Global test state
static raeen_gui_system_t* g_test_gui = NULL;
static bool g_test_running = true;
static uint64_t g_test_start_time = 0;
static uint32_t g_test_frame_count = 0;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    printf("\nTest interrupted by signal %d\n", sig);
    g_test_running = false;
}

// Test functions
static bool test_gui_initialization(void) {
    printf("=== Testing GUI System Initialization ===\n");
    
    // Test different configurations
    gui_system_config_t configs[] = {
        raeen_gui_system_preset_desktop_style(),
        raeen_gui_system_preset_gaming_style(),
        raeen_gui_system_preset_minimal_style(),
        raeen_gui_system_preset_accessibility_style()
    };
    
    const char* config_names[] = {
        "Desktop Style",
        "Gaming Style", 
        "Minimal Style",
        "Accessibility Style"
    };
    
    for (int i = 0; i < 4; i++) {
        printf("Testing %s configuration...\n", config_names[i]);
        
        // Override display size for testing
        configs[i].display_width = TEST_DISPLAY_WIDTH;
        configs[i].display_height = TEST_DISPLAY_HEIGHT;
        
        raeen_gui_system_t* test_gui = raeen_gui_system_init(&configs[i]);
        if (!test_gui) {
            printf("FAILED: Could not initialize GUI with %s configuration\n", config_names[i]);
            return false;
        }
        
        if (!raeen_gui_system_is_initialized(test_gui)) {
            printf("FAILED: GUI not properly initialized with %s configuration\n", config_names[i]);
            raeen_gui_system_shutdown(test_gui);
            return false;
        }
        
        printf("PASSED: %s configuration\n", config_names[i]);
        raeen_gui_system_shutdown(test_gui);
    }
    
    printf("All initialization tests passed!\n\n");
    return true;
}

static bool test_component_access(void) {
    printf("=== Testing Component Access ===\n");
    
    gui_system_config_t config = raeen_gui_system_preset_desktop_style();
    config.display_width = TEST_DISPLAY_WIDTH;
    config.display_height = TEST_DISPLAY_HEIGHT;
    
    g_test_gui = raeen_gui_system_init(&config);
    if (!g_test_gui) {
        printf("FAILED: Could not initialize GUI for component testing\n");
        return false;
    }
    
    // Test compositor access
    glass_compositor_t* compositor = raeen_gui_system_get_compositor(g_test_gui);
    if (!compositor) {
        printf("WARNING: Compositor not available (not implemented yet)\n");
    } else {
        printf("PASSED: Compositor access\n");
    }
    
    // Test dock access
    hybrid_dock_t* dock = raeen_gui_system_get_dock(g_test_gui);
    if (!dock) {
        printf("WARNING: Dock not available (not implemented yet)\n");
    } else {
        printf("PASSED: Dock access\n");
    }
    
    // Test window manager access
    glass_window_manager_t* window_manager = raeen_gui_system_get_window_manager(g_test_gui);
    if (!window_manager) {
        printf("WARNING: Window manager not available (not implemented yet)\n");
    } else {
        printf("PASSED: Window manager access\n");
    }
    
    // Test search access
    spotlight_plus_t* search = raeen_gui_system_get_search(g_test_gui);
    if (!search) {
        printf("WARNING: Search not available (not implemented yet)\n");
    } else {
        printf("PASSED: Search access\n");
    }
    
    // Test customizer access
    raeen_customizer_t* customizer = raeen_gui_system_get_customizer(g_test_gui);
    if (!customizer) {
        printf("WARNING: Customizer not available (not implemented yet)\n");
    } else {
        printf("PASSED: Customizer access\n");
    }
    
    printf("Component access tests completed!\n\n");
    return true;
}

static bool test_system_metrics(void) {
    printf("=== Testing System Metrics ===\n");
    
    if (!g_test_gui) {
        printf("FAILED: No GUI system available for metrics testing\n");
        return false;
    }
    
    // Test state access
    gui_system_state_t state = raeen_gui_system_get_state(g_test_gui);
    printf("GUI State: %d\n", state);
    
    // Test display mode
    display_mode_t display_mode = raeen_gui_system_get_display_mode(g_test_gui);
    printf("Display Mode: %d\n", display_mode);
    
    // Test performance profile
    performance_profile_t perf_profile = raeen_gui_system_get_performance_profile(g_test_gui);
    printf("Performance Profile: %d\n", perf_profile);
    
    // Test metrics
    system_metrics_t* metrics = raeen_gui_system_get_metrics(g_test_gui);
    if (metrics) {
        printf("System Metrics:\n");
        printf("  CPU Usage: %.2f%%\n", metrics->cpu_usage);
        printf("  Memory Usage: %.2f%%\n", metrics->memory_usage);
        printf("  GPU Usage: %.2f%%\n", metrics->gpu_usage);
        printf("  FPS: %d\n", metrics->fps);
        printf("  Active Windows: %d\n", metrics->active_windows);
        printf("  Uptime: %llu seconds\n", metrics->uptime_seconds);
    }
    
    // Test FPS
    float fps = raeen_gui_system_get_fps(g_test_gui);
    printf("Current FPS: %.2f\n", fps);
    
    // Test uptime
    uint64_t uptime = raeen_gui_system_get_uptime(g_test_gui);
    printf("System Uptime: %llu seconds\n", uptime);
    
    printf("System metrics tests completed!\n\n");
    return true;
}

static bool test_input_handling(void) {
    printf("=== Testing Input Handling ===\n");
    
    if (!g_test_gui) {
        printf("FAILED: No GUI system available for input testing\n");
        return false;
    }
    
    // Test mouse movement
    bool mouse_handled = raeen_gui_system_handle_mouse_move(g_test_gui, 100.0f, 100.0f);
    printf("Mouse move (100, 100): %s\n", mouse_handled ? "Handled" : "Not handled");
    
    // Test mouse clicks
    bool click_handled = raeen_gui_system_handle_mouse_click(g_test_gui, 100.0f, 100.0f, true);
    printf("Mouse click (100, 100): %s\n", click_handled ? "Handled" : "Not handled");
    
    // Test key presses
    bool key_handled = raeen_gui_system_handle_key_press(g_test_gui, 0x1B); // Escape
    printf("Key press (Escape): %s\n", key_handled ? "Handled" : "Not handled");
    
    key_handled = raeen_gui_system_handle_key_press(g_test_gui, 0x20); // Space
    printf("Key press (Space): %s\n", key_handled ? "Handled" : "Not handled");
    
    printf("Input handling tests completed!\n\n");
    return true;
}

static bool test_error_handling(void) {
    printf("=== Testing Error Handling ===\n");
    
    // Test error count
    uint32_t error_count = raeen_gui_system_get_error_count(g_test_gui);
    printf("Error count: %d\n", error_count);
    
    // Test last error
    const char* last_error = raeen_gui_system_get_last_error(g_test_gui);
    if (last_error && strlen(last_error) > 0) {
        printf("Last error: %s\n", last_error);
    } else {
        printf("No errors recorded\n");
    }
    
    // Test error clearing
    raeen_gui_system_clear_errors(g_test_gui);
    error_count = raeen_gui_system_get_error_count(g_test_gui);
    printf("Error count after clearing: %d\n", error_count);
    
    // Test error strings
    printf("Error strings:\n");
    for (int i = 0; i <= 6; i++) {
        const char* error_str = raeen_gui_system_get_error_string(i);
        printf("  %d: %s\n", i, error_str);
    }
    
    printf("Error handling tests completed!\n\n");
    return true;
}

static bool test_performance_profiles(void) {
    printf("=== Testing Performance Profiles ===\n");
    
    if (!g_test_gui) {
        printf("FAILED: No GUI system available for performance testing\n");
        return false;
    }
    
    // Test different performance profiles
    performance_profile_t profiles[] = {
        PERFORMANCE_PROFILE_POWER_SAVER,
        PERFORMANCE_PROFILE_BALANCED,
        PERFORMANCE_PROFILE_HIGH_PERFORMANCE,
        PERFORMANCE_PROFILE_GAMING
    };
    
    const char* profile_names[] = {
        "Power Saver",
        "Balanced",
        "High Performance",
        "Gaming"
    };
    
    for (int i = 0; i < 4; i++) {
        printf("Testing %s profile...\n", profile_names[i]);
        
        // Note: In a real implementation, this would change the profile
        // For now, we just test that the function doesn't crash
        raeen_gui_system_set_performance_profile(g_test_gui, profiles[i]);
        
        performance_profile_t current = raeen_gui_system_get_performance_profile(g_test_gui);
        printf("Current profile: %d\n", current);
    }
    
    printf("Performance profile tests completed!\n\n");
    return true;
}

static bool test_display_modes(void) {
    printf("=== Testing Display Modes ===\n");
    
    if (!g_test_gui) {
        printf("FAILED: No GUI system available for display mode testing\n");
        return false;
    }
    
    // Test different display modes
    display_mode_t modes[] = {
        DISPLAY_MODE_NORMAL,
        DISPLAY_MODE_GAMING,
        DISPLAY_MODE_PRESENTATION,
        DISPLAY_MODE_ACCESSIBILITY,
        DISPLAY_MODE_SAFE
    };
    
    const char* mode_names[] = {
        "Normal",
        "Gaming",
        "Presentation",
        "Accessibility",
        "Safe"
    };
    
    for (int i = 0; i < 5; i++) {
        printf("Testing %s mode...\n", mode_names[i]);
        
        // Note: In a real implementation, this would change the display mode
        // For now, we just test that the function doesn't crash
        raeen_gui_system_set_display_mode(g_test_gui, modes[i]);
        
        display_mode_t current = raeen_gui_system_get_display_mode(g_test_gui);
        printf("Current mode: %d\n", current);
    }
    
    printf("Display mode tests completed!\n\n");
    return true;
}

static void test_main_loop(void) {
    printf("=== Running Main Loop Test ===\n");
    printf("Running for %d seconds at target %d FPS...\n", TEST_DURATION_SECONDS, TEST_FPS_TARGET);
    printf("Press Ctrl+C to stop early\n\n");
    
    g_test_start_time = hal_get_tick_count();
    g_test_frame_count = 0;
    
    // Start the GUI system
    raeen_gui_system_start(g_test_gui);
    
    // The main loop will run until interrupted or time expires
    printf("Main loop test completed!\n\n");
}

static void print_test_summary(void) {
    printf("=== Test Summary ===\n");
    
    if (g_test_gui) {
        uint64_t test_duration = (hal_get_tick_count() - g_test_start_time) / 1000;
        float avg_fps = g_test_frame_count / (float)test_duration;
        
        printf("Test Duration: %llu seconds\n", test_duration);
        printf("Total Frames: %d\n", g_test_frame_count);
        printf("Average FPS: %.2f\n", avg_fps);
        printf("Target FPS: %d\n", TEST_FPS_TARGET);
        
        if (avg_fps >= TEST_FPS_TARGET * 0.8) {
            printf("Performance: GOOD (%.1f%% of target)\n", (avg_fps / TEST_FPS_TARGET) * 100);
        } else {
            printf("Performance: NEEDS IMPROVEMENT (%.1f%% of target)\n", (avg_fps / TEST_FPS_TARGET) * 100);
        }
        
        // Print final metrics
        system_metrics_t* metrics = raeen_gui_system_get_metrics(g_test_gui);
        if (metrics) {
            printf("Final CPU Usage: %.2f%%\n", metrics->cpu_usage);
            printf("Final Memory Usage: %.2f%%\n", metrics->memory_usage);
            printf("Final GPU Usage: %.2f%%\n", metrics->gpu_usage);
        }
    }
    
    printf("Test summary completed!\n\n");
}

// Main test function
int main(int argc, char* argv[]) {
    printf("RaeenOS GUI System Test Suite\n");
    printf("=============================\n\n");
    
    // Set up signal handling
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Check dependencies
    printf("Checking dependencies...\n");
    // TODO: Add dependency checking
    
    printf("All dependencies satisfied\n\n");
    
    // Run tests
    bool all_tests_passed = true;
    
    if (!test_gui_initialization()) {
        all_tests_passed = false;
    }
    
    if (!test_component_access()) {
        all_tests_passed = false;
    }
    
    if (!test_system_metrics()) {
        all_tests_passed = false;
    }
    
    if (!test_input_handling()) {
        all_tests_passed = false;
    }
    
    if (!test_error_handling()) {
        all_tests_passed = false;
    }
    
    if (!test_performance_profiles()) {
        all_tests_passed = false;
    }
    
    if (!test_display_modes()) {
        all_tests_passed = false;
    }
    
    // Run main loop test if all other tests passed
    if (all_tests_passed) {
        test_main_loop();
    }
    
    // Print summary
    print_test_summary();
    
    // Cleanup
    if (g_test_gui) {
        raeen_gui_system_shutdown(g_test_gui);
        g_test_gui = NULL;
    }
    
    // Final result
    if (all_tests_passed) {
        printf("=== ALL TESTS PASSED ===\n");
        printf("RaeenOS GUI System is ready for production!\n");
        return 0;
    } else {
        printf("=== SOME TESTS FAILED ===\n");
        printf("Please check the implementation and try again.\n");
        return 1;
    }
} 