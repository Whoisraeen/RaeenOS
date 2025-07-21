#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Test Framework - Comprehensive testing for RaeenOS
// Provides unit tests, integration tests, and system-level tests

// Test result types
typedef enum {
    TEST_RESULT_PASS = 0,            // Test passed
    TEST_RESULT_FAIL,                // Test failed
    TEST_RESULT_SKIP,                // Test skipped
    TEST_RESULT_ERROR,               // Test error
    TEST_RESULT_TIMEOUT,             // Test timeout
} test_result_t;

// Test severity levels
typedef enum {
    TEST_SEVERITY_LOW = 0,           // Low severity
    TEST_SEVERITY_MEDIUM,            // Medium severity
    TEST_SEVERITY_HIGH,              // High severity
    TEST_SEVERITY_CRITICAL,          // Critical severity
} test_severity_t;

// Test categories
typedef enum {
    TEST_CATEGORY_UNIT = 0,          // Unit tests
    TEST_CATEGORY_INTEGRATION,       // Integration tests
    TEST_CATEGORY_SYSTEM,            // System tests
    TEST_CATEGORY_PERFORMANCE,       // Performance tests
    TEST_CATEGORY_SECURITY,          // Security tests
    TEST_CATEGORY_COMPATIBILITY,     // Compatibility tests
    TEST_CATEGORY_STRESS,            // Stress tests
    TEST_CATEGORY_REGRESSION,        // Regression tests
} test_category_t;

// Test function signature
typedef test_result_t (*test_function_t)(void* context);

// Test case
typedef struct {
    char name[128];                  // Test name
    char description[256];           // Test description
    test_function_t function;        // Test function
    test_category_t category;        // Test category
    test_severity_t severity;        // Test severity
    uint32_t timeout_ms;             // Timeout in milliseconds
    bool is_enabled;                 // Is test enabled
    bool is_automated;               // Is test automated
    void* context;                   // Test context
} test_case_t;

// Test suite
typedef struct {
    char name[128];                  // Suite name
    char description[256];           // Suite description
    test_case_t* tests;              // Test cases
    uint32_t test_count;             // Number of tests
    uint32_t max_tests;              // Maximum tests
    test_category_t category;        // Suite category
    bool is_enabled;                 // Is suite enabled
    void* suite_context;             // Suite context
} test_suite_t;

// Test result
typedef struct {
    test_case_t* test_case;          // Test case
    test_result_t result;            // Test result
    char message[512];               // Result message
    uint64_t start_time;             // Start time
    uint64_t end_time;               // End time
    uint64_t duration_ms;            // Duration in milliseconds
    uint32_t memory_usage;           // Memory usage in KB
    uint32_t cpu_usage;              // CPU usage percentage
    void* additional_data;           // Additional result data
} test_result_info_t;

// Test statistics
typedef struct {
    uint32_t total_tests;            // Total tests
    uint32_t passed_tests;           // Passed tests
    uint32_t failed_tests;           // Failed tests
    uint32_t skipped_tests;          // Skipped tests
    uint32_t error_tests;            // Error tests
    uint32_t timeout_tests;          // Timeout tests
    uint64_t total_duration_ms;      // Total duration
    uint64_t average_duration_ms;    // Average duration
    uint64_t min_duration_ms;        // Minimum duration
    uint64_t max_duration_ms;        // Maximum duration
    uint32_t total_memory_usage;     // Total memory usage
    uint32_t average_memory_usage;   // Average memory usage
    uint32_t total_cpu_usage;        // Total CPU usage
    uint32_t average_cpu_usage;      // Average CPU usage
} test_statistics_t;

// Test framework configuration
typedef struct {
    bool enable_unit_tests;          // Enable unit tests
    bool enable_integration_tests;   // Enable integration tests
    bool enable_system_tests;        // Enable system tests
    bool enable_performance_tests;   // Enable performance tests
    bool enable_security_tests;      // Enable security tests
    bool enable_compatibility_tests; // Enable compatibility tests
    bool enable_stress_tests;        // Enable stress tests
    bool enable_regression_tests;    // Enable regression tests
    bool enable_parallel_execution;  // Enable parallel execution
    bool enable_memory_tracking;     // Enable memory tracking
    bool enable_cpu_tracking;        // Enable CPU tracking
    bool enable_coverage_reporting;  // Enable coverage reporting
    bool enable_xml_output;          // Enable XML output
    bool enable_json_output;         // Enable JSON output
    bool enable_html_output;         // Enable HTML output
    uint32_t max_parallel_tests;     // Maximum parallel tests
    uint32_t default_timeout_ms;     // Default timeout
    uint32_t max_memory_usage_mb;    // Maximum memory usage
    uint32_t max_cpu_usage_percent;  // Maximum CPU usage
    char output_directory[256];      // Output directory
    char log_file[256];              // Log file path
    char coverage_file[256];         // Coverage file path
    char report_file[256];           // Report file path
} test_framework_config_t;

// Test framework context
typedef struct {
    test_framework_config_t config;
    test_suite_t* suites;
    uint32_t suite_count;
    uint32_t max_suites;
    test_result_info_t* results;
    uint32_t result_count;
    uint32_t max_results;
    test_statistics_t statistics;
    bool initialized;
    bool running;
    uint64_t start_time;
    uint64_t end_time;
    FILE* log_file;
    FILE* coverage_file;
    FILE* report_file;
} test_framework_t;

// Function prototypes

// Initialization and shutdown
test_framework_t* test_framework_init(test_framework_config_t* config);
void test_framework_shutdown(test_framework_t* framework);
bool test_framework_is_initialized(test_framework_t* framework);

// Test suite management
uint32_t test_framework_add_suite(test_framework_t* framework, test_suite_t* suite);
void test_framework_remove_suite(test_framework_t* framework, uint32_t suite_id);
test_suite_t* test_framework_get_suite(test_framework_t* framework, uint32_t suite_id);
test_suite_t* test_framework_get_suites(test_framework_t* framework, uint32_t* count);
uint32_t test_framework_get_suite_count(test_framework_t* framework);
void test_framework_enable_suite(test_framework_t* framework, uint32_t suite_id, bool enable);
bool test_framework_is_suite_enabled(test_framework_t* framework, uint32_t suite_id);

// Test case management
uint32_t test_suite_add_test(test_suite_t* suite, test_case_t* test_case);
void test_suite_remove_test(test_suite_t* suite, uint32_t test_id);
test_case_t* test_suite_get_test(test_suite_t* suite, uint32_t test_id);
test_case_t* test_suite_get_tests(test_suite_t* suite, uint32_t* count);
uint32_t test_suite_get_test_count(test_suite_t* suite);
void test_suite_enable_test(test_suite_t* suite, uint32_t test_id, bool enable);
bool test_suite_is_test_enabled(test_suite_t* suite, uint32_t test_id);

// Test execution
bool test_framework_run_all_tests(test_framework_t* framework);
bool test_framework_run_suite(test_framework_t* framework, uint32_t suite_id);
bool test_framework_run_test(test_framework_t* framework, uint32_t suite_id, uint32_t test_id);
bool test_framework_run_category(test_framework_t* framework, test_category_t category);
bool test_framework_run_severity(test_framework_t* framework, test_severity_t severity);
test_result_t test_framework_execute_test(test_framework_t* framework, test_case_t* test_case);

// Test results
test_result_info_t* test_framework_get_results(test_framework_t* framework, uint32_t* count);
test_result_info_t* test_framework_get_result(test_framework_t* framework, uint32_t suite_id, uint32_t test_id);
test_statistics_t* test_framework_get_statistics(test_framework_t* framework);
void test_framework_clear_results(test_framework_t* framework);
bool test_framework_has_failures(test_framework_t* framework);
uint32_t test_framework_get_failure_count(test_framework_t* framework);

// Assertions and utilities
void test_assert_true(bool condition, const char* message);
void test_assert_false(bool condition, const char* message);
void test_assert_equal(int expected, int actual, const char* message);
void test_assert_not_equal(int expected, int actual, const char* message);
void test_assert_string_equal(const char* expected, const char* actual, const char* message);
void test_assert_string_not_equal(const char* expected, const char* actual, const char* message);
void test_assert_null(void* pointer, const char* message);
void test_assert_not_null(void* pointer, const char* message);
void test_assert_memory_equal(const void* expected, const void* actual, size_t size, const char* message);
void test_assert_memory_not_equal(const void* expected, const void* actual, size_t size, const char* message);
void test_assert_float_equal(float expected, float actual, float tolerance, const char* message);
void test_assert_double_equal(double expected, double actual, double tolerance, const char* message);

// Test utilities
void test_setup(test_framework_t* framework);
void test_teardown(test_framework_t* framework);
void test_log(test_framework_t* framework, const char* format, ...);
void test_debug(test_framework_t* framework, const char* format, ...);
void test_warning(test_framework_t* framework, const char* format, ...);
void test_error(test_framework_t* framework, const char* format, ...);
uint64_t test_get_time_ms(void);
uint64_t test_get_memory_usage_kb(void);
uint32_t test_get_cpu_usage_percent(void);

// Test macros
#define TEST_ASSERT_TRUE(condition) test_assert_true(condition, #condition)
#define TEST_ASSERT_FALSE(condition) test_assert_false(condition, #condition)
#define TEST_ASSERT_EQUAL(expected, actual) test_assert_equal(expected, actual, #expected " == " #actual)
#define TEST_ASSERT_NOT_EQUAL(expected, actual) test_assert_not_equal(expected, actual, #expected " != " #actual)
#define TEST_ASSERT_STRING_EQUAL(expected, actual) test_assert_string_equal(expected, actual, #expected " == " #actual)
#define TEST_ASSERT_STRING_NOT_EQUAL(expected, actual) test_assert_string_not_equal(expected, actual, #expected " != " #actual)
#define TEST_ASSERT_NULL(pointer) test_assert_null(pointer, #pointer " is NULL")
#define TEST_ASSERT_NOT_NULL(pointer) test_assert_not_null(pointer, #pointer " is not NULL")
#define TEST_ASSERT_MEMORY_EQUAL(expected, actual, size) test_assert_memory_equal(expected, actual, size, #expected " == " #actual)
#define TEST_ASSERT_MEMORY_NOT_EQUAL(expected, actual, size) test_assert_memory_not_equal(expected, actual, size, #expected " != " #actual)
#define TEST_ASSERT_FLOAT_EQUAL(expected, actual, tolerance) test_assert_float_equal(expected, actual, tolerance, #expected " == " #actual)
#define TEST_ASSERT_DOUBLE_EQUAL(expected, actual, tolerance) test_assert_double_equal(expected, actual, tolerance, #expected " == " #actual)

// Test case definition macros
#define TEST_CASE(name, description, func, category, severity) \
    { name, description, func, category, severity, 5000, true, true, NULL }

#define TEST_SUITE(name, description, category) \
    { name, description, NULL, 0, 100, category, true, NULL }

// Test function wrapper
#define TEST_FUNCTION(name) \
    test_result_t name(void* context)

// Test registration
#define REGISTER_TEST(suite, test) \
    test_suite_add_test(&suite, &test)

#define REGISTER_SUITE(framework, suite) \
    test_framework_add_suite(framework, &suite)

// Reporting
bool test_framework_generate_report(test_framework_t* framework, const char* format);
bool test_framework_save_results(test_framework_t* framework, const char* filename);
bool test_framework_load_results(test_framework_t* framework, const char* filename);
bool test_framework_export_xml(test_framework_t* framework, const char* filename);
bool test_framework_export_json(test_framework_t* framework, const char* filename);
bool test_framework_export_html(test_framework_t* framework, const char* filename);

// Coverage reporting
bool test_framework_start_coverage(test_framework_t* framework);
bool test_framework_stop_coverage(test_framework_t* framework);
bool test_framework_generate_coverage_report(test_framework_t* framework, const char* filename);
uint32_t test_framework_get_coverage_percentage(test_framework_t* framework);

// Performance testing
bool test_framework_start_performance_monitoring(test_framework_t* framework);
bool test_framework_stop_performance_monitoring(test_framework_t* framework);
bool test_framework_measure_performance(test_framework_t* framework, const char* test_name, test_function_t function, void* context);
uint64_t test_framework_get_performance_result(test_framework_t* framework, const char* test_name);

// Stress testing
bool test_framework_run_stress_test(test_framework_t* framework, test_function_t function, void* context, uint32_t iterations, uint32_t timeout_ms);
bool test_framework_run_load_test(test_framework_t* framework, test_function_t function, void* context, uint32_t concurrent_users, uint32_t duration_ms);

// Security testing
bool test_framework_run_security_test(test_framework_t* framework, test_function_t function, void* context);
bool test_framework_run_penetration_test(test_framework_t* framework, const char* target, uint32_t timeout_ms);
bool test_framework_run_vulnerability_scan(test_framework_t* framework, const char* target);

// Compatibility testing
bool test_framework_run_compatibility_test(test_framework_t* framework, test_function_t function, void* context);
bool test_framework_test_windows_compatibility(test_framework_t* framework);
bool test_framework_test_macos_compatibility(test_framework_t* framework);
bool test_framework_test_linux_compatibility(test_framework_t* framework);

// Information
uint32_t test_framework_get_total_test_count(test_framework_t* framework);
uint32_t test_framework_get_enabled_test_count(test_framework_t* framework);
uint32_t test_framework_get_disabled_test_count(test_framework_t* framework);
bool test_framework_is_running(test_framework_t* framework);
uint64_t test_framework_get_execution_time(test_framework_t* framework);

// Utility functions
char* test_framework_get_result_name(test_result_t result);
char* test_framework_get_severity_name(test_severity_t severity);
char* test_framework_get_category_name(test_category_t category);
bool test_framework_is_result_success(test_result_t result);
bool test_framework_is_result_failure(test_result_t result);

// Callbacks
typedef void (*test_start_callback_t)(test_framework_t* framework, test_case_t* test_case, void* user_data);
typedef void (*test_complete_callback_t)(test_framework_t* framework, test_result_info_t* result, void* user_data);
typedef void (*test_suite_start_callback_t)(test_framework_t* framework, test_suite_t* suite, void* user_data);
typedef void (*test_suite_complete_callback_t)(test_framework_t* framework, test_suite_t* suite, test_statistics_t* stats, void* user_data);

void test_framework_set_test_start_callback(test_framework_t* framework, test_start_callback_t callback, void* user_data);
void test_framework_set_test_complete_callback(test_framework_t* framework, test_complete_callback_t callback, void* user_data);
void test_framework_set_suite_start_callback(test_framework_t* framework, test_suite_start_callback_t callback, void* user_data);
void test_framework_set_suite_complete_callback(test_framework_t* framework, test_suite_complete_callback_t callback, void* user_data);

// Preset configurations
test_framework_config_t test_framework_preset_development_style(void);
test_framework_config_t test_framework_preset_ci_style(void);
test_framework_config_t test_framework_preset_production_style(void);
test_framework_config_t test_framework_preset_coverage_style(void);

// Error handling
typedef enum {
    TEST_FRAMEWORK_SUCCESS = 0,
    TEST_FRAMEWORK_ERROR_INVALID_CONTEXT,
    TEST_FRAMEWORK_ERROR_INVALID_SUITE,
    TEST_FRAMEWORK_ERROR_INVALID_TEST,
    TEST_FRAMEWORK_ERROR_EXECUTION_FAILED,
    TEST_FRAMEWORK_ERROR_TIMEOUT,
    TEST_FRAMEWORK_ERROR_OUT_OF_MEMORY,
    TEST_FRAMEWORK_ERROR_FILE_IO,
    TEST_FRAMEWORK_ERROR_INVALID_CONFIG,
} test_framework_error_t;

test_framework_error_t test_framework_get_last_error(void);
const char* test_framework_get_error_string(test_framework_error_t error);

#endif // TEST_FRAMEWORK_H 