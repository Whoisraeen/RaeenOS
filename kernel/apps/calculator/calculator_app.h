#ifndef CALCULATOR_APP_H
#define CALCULATOR_APP_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/gui/window/glass_window_manager.h>

// Calculator App - Advanced calculator for RaeenOS
// Provides scientific, programming, and financial calculations

// Calculator modes
typedef enum {
    CALCULATOR_MODE_STANDARD = 0,    // Standard calculator
    CALCULATOR_MODE_SCIENTIFIC,      // Scientific calculator
    CALCULATOR_MODE_PROGRAMMER,      // Programmer calculator
    CALCULATOR_MODE_FINANCIAL,       // Financial calculator
    CALCULATOR_MODE_STATISTICS,      // Statistics calculator
    CALCULATOR_MODE_CONVERSION,      // Unit conversion
    CALCULATOR_MODE_GRAPHING,        // Graphing calculator
    CALCULATOR_MODE_GEOMETRY,        // Geometry calculator
    CALCULATOR_MODE_PHYSICS,         // Physics calculator
    CALCULATOR_MODE_CHEMISTRY,       // Chemistry calculator
} calculator_mode_t;

// Number systems
typedef enum {
    NUMBER_SYSTEM_DECIMAL = 0,       // Decimal (base 10)
    NUMBER_SYSTEM_BINARY,            // Binary (base 2)
    NUMBER_SYSTEM_OCTAL,             // Octal (base 8)
    NUMBER_SYSTEM_HEXADECIMAL,       // Hexadecimal (base 16)
    NUMBER_SYSTEM_ROMAN,             // Roman numerals
} number_system_t;

// Angle units
typedef enum {
    ANGLE_UNIT_DEGREES = 0,          // Degrees
    ANGLE_UNIT_RADIANS,              // Radians
    ANGLE_UNIT_GRADIANS,             // Gradians
} angle_unit_t;

// Calculator operations
typedef enum {
    OPERATION_NONE = 0,              // No operation
    OPERATION_ADD,                   // Addition
    OPERATION_SUBTRACT,              // Subtraction
    OPERATION_MULTIPLY,              // Multiplication
    OPERATION_DIVIDE,                // Division
    OPERATION_MODULO,                // Modulo
    OPERATION_POWER,                 // Power
    OPERATION_SQUARE_ROOT,           // Square root
    OPERATION_CUBE_ROOT,             // Cube root
    OPERATION_FACTORIAL,             // Factorial
    OPERATION_SINE,                  // Sine
    OPERATION_COSINE,                // Cosine
    OPERATION_TANGENT,               // Tangent
    OPERATION_ARC_SINE,              // Arc sine
    OPERATION_ARC_COSINE,            // Arc cosine
    OPERATION_ARC_TANGENT,           // Arc tangent
    OPERATION_LOGARITHM,             // Natural logarithm
    OPERATION_LOGARITHM_10,          // Base-10 logarithm
    OPERATION_LOGARITHM_2,           // Base-2 logarithm
    OPERATION_EXPONENTIAL,           // Exponential
    OPERATION_ABSOLUTE,              // Absolute value
    OPERATION_NEGATE,                // Negation
    OPERATION_RECIPROCAL,            // Reciprocal
    OPERATION_PERCENTAGE,            // Percentage
    OPERATION_AND,                   // Bitwise AND
    OPERATION_OR,                    // Bitwise OR
    OPERATION_XOR,                   // Bitwise XOR
    OPERATION_NOT,                   // Bitwise NOT
    OPERATION_LEFT_SHIFT,            // Left shift
    OPERATION_RIGHT_SHIFT,           // Right shift
    OPERATION_ROTATE_LEFT,           // Rotate left
    OPERATION_ROTATE_RIGHT,          // Rotate right
} calculator_operation_t;

// Calculator state
typedef struct {
    double display_value;            // Current display value
    double memory_value;             // Memory value
    double previous_value;           // Previous value
    calculator_operation_t operation; // Current operation
    bool operation_pending;          // Operation pending
    bool memory_active;              // Memory active
    bool error_state;                // Error state
    char error_message[256];         // Error message
    number_system_t number_system;   // Current number system
    angle_unit_t angle_unit;         // Current angle unit
    calculator_mode_t mode;          // Current mode
    bool degrees_mode;               // Degrees mode
    bool inverse_mode;               // Inverse mode
    bool hyperbolic_mode;            // Hyperbolic mode
    uint32_t precision;              // Display precision
    bool scientific_notation;        // Scientific notation
    uint32_t history_count;          // History count
    uint32_t max_history;            // Maximum history
} calculator_state_t;

// Calculator history entry
typedef struct {
    char expression[256];            // Expression
    double result;                   // Result
    uint64_t timestamp;              // Timestamp
    calculator_mode_t mode;          // Mode when calculated
} calculator_history_entry_t;

// Calculator configuration
typedef struct {
    uint32_t window_width;           // Window width
    uint32_t window_height;          // Window height
    bool enable_history;             // Enable calculation history
    bool enable_memory;              // Enable memory functions
    bool enable_scientific;          // Enable scientific functions
    bool enable_programmer;          // Enable programmer functions
    bool enable_financial;           // Enable financial functions
    bool enable_statistics;          // Enable statistics functions
    bool enable_conversion;          // Enable unit conversion
    bool enable_graphing;            // Enable graphing
    bool enable_geometry;            // Enable geometry functions
    bool enable_physics;             // Enable physics functions
    bool enable_chemistry;           // Enable chemistry functions
    uint32_t max_history_entries;    // Maximum history entries
    uint32_t default_precision;      // Default precision
    bool default_scientific_notation; // Default scientific notation
    angle_unit_t default_angle_unit; // Default angle unit
    number_system_t default_number_system; // Default number system
} calculator_config_t;

// Calculator context
typedef struct {
    calculator_config_t config;
    glass_window_manager_t* window_manager;
    uint32_t window_id;
    calculator_state_t state;
    calculator_history_entry_t* history;
    uint32_t history_count;
    uint32_t max_history;
    bool initialized;
    bool is_visible;
    bool is_focused;
    uint64_t last_update_time;
} calculator_app_t;

// Function prototypes

// Initialization and shutdown
calculator_app_t* calculator_app_init(glass_window_manager_t* window_manager, calculator_config_t* config);
void calculator_app_shutdown(calculator_app_t* calculator);
bool calculator_app_is_initialized(calculator_app_t* calculator);

// Window management
void calculator_app_show(calculator_app_t* calculator);
void calculator_app_hide(calculator_app_t* calculator);
bool calculator_app_is_visible(calculator_app_t* calculator);
void calculator_app_set_size(calculator_app_t* calculator, uint32_t width, uint32_t height);
void calculator_app_get_size(calculator_app_t* calculator, uint32_t* width, uint32_t* height);
void calculator_app_set_position(calculator_app_t* calculator, uint32_t x, uint32_t y);
void calculator_app_get_position(calculator_app_t* calculator, uint32_t* x, uint32_t* y);

// Calculator operations
double calculator_app_calculate(calculator_app_t* calculator, double a, double b, calculator_operation_t operation);
double calculator_app_calculate_single(calculator_app_t* calculator, double a, calculator_operation_t operation);
void calculator_app_clear(calculator_app_t* calculator);
void calculator_app_clear_entry(calculator_app_t* calculator);
void calculator_app_backspace(calculator_app_t* calculator);
void calculator_app_enter_digit(calculator_app_t* calculator, uint8_t digit);
void calculator_app_enter_decimal(calculator_app_t* calculator);
void calculator_app_enter_operation(calculator_app_t* calculator, calculator_operation_t operation);
void calculator_app_equals(calculator_app_t* calculator);
void calculator_app_negate(calculator_app_t* calculator);
void calculator_app_percentage(calculator_app_t* calculator);

// Memory operations
void calculator_app_memory_store(calculator_app_t* calculator);
void calculator_app_memory_recall(calculator_app_t* calculator);
void calculator_app_memory_add(calculator_app_t* calculator);
void calculator_app_memory_subtract(calculator_app_t* calculator);
void calculator_app_memory_clear(calculator_app_t* calculator);
double calculator_app_get_memory_value(calculator_app_t* calculator);
bool calculator_app_is_memory_active(calculator_app_t* calculator);

// Mode switching
void calculator_app_set_mode(calculator_app_t* calculator, calculator_mode_t mode);
calculator_mode_t calculator_app_get_mode(calculator_app_t* calculator);
bool calculator_app_is_mode_supported(calculator_app_t* calculator, calculator_mode_t mode);
void calculator_app_switch_to_standard(calculator_app_t* calculator);
void calculator_app_switch_to_scientific(calculator_app_t* calculator);
void calculator_app_switch_to_programmer(calculator_app_t* calculator);
void calculator_app_switch_to_financial(calculator_app_t* calculator);
void calculator_app_switch_to_statistics(calculator_app_t* calculator);

// Number system operations
void calculator_app_set_number_system(calculator_app_t* calculator, number_system_t system);
number_system_t calculator_app_get_number_system(calculator_app_t* calculator);
char* calculator_app_convert_number_system(calculator_app_t* calculator, double value, number_system_t from, number_system_t to);
uint64_t calculator_app_decimal_to_binary(calculator_app_t* calculator, uint64_t decimal);
uint64_t calculator_app_binary_to_decimal(calculator_app_t* calculator, uint64_t binary);
char* calculator_app_decimal_to_hexadecimal(calculator_app_t* calculator, uint64_t decimal);
uint64_t calculator_app_hexadecimal_to_decimal(calculator_app_t* calculator, const char* hex);
char* calculator_app_decimal_to_octal(calculator_app_t* calculator, uint64_t decimal);
uint64_t calculator_app_octal_to_decimal(calculator_app_t* calculator, uint64_t octal);

// Scientific functions
double calculator_app_sine(calculator_app_t* calculator, double angle);
double calculator_app_cosine(calculator_app_t* calculator, double angle);
double calculator_app_tangent(calculator_app_t* calculator, double angle);
double calculator_app_arc_sine(calculator_app_t* calculator, double value);
double calculator_app_arc_cosine(calculator_app_t* calculator, double value);
double calculator_app_arc_tangent(calculator_app_t* calculator, double value);
double calculator_app_hyperbolic_sine(calculator_app_t* calculator, double value);
double calculator_app_hyperbolic_cosine(calculator_app_t* calculator, double value);
double calculator_app_hyperbolic_tangent(calculator_app_t* calculator, double value);
double calculator_app_logarithm(calculator_app_t* calculator, double value);
double calculator_app_logarithm_10(calculator_app_t* calculator, double value);
double calculator_app_logarithm_2(calculator_app_t* calculator, double value);
double calculator_app_exponential(calculator_app_t* calculator, double value);
double calculator_app_power(calculator_app_t* calculator, double base, double exponent);
double calculator_app_square_root(calculator_app_t* calculator, double value);
double calculator_app_cube_root(calculator_app_t* calculator, double value);
double calculator_app_factorial(calculator_app_t* calculator, uint32_t n);
double calculator_app_absolute(calculator_app_t* calculator, double value);

// Angle unit conversion
void calculator_app_set_angle_unit(calculator_app_t* calculator, angle_unit_t unit);
angle_unit_t calculator_app_get_angle_unit(calculator_app_t* calculator);
double calculator_app_degrees_to_radians(calculator_app_t* calculator, double degrees);
double calculator_app_radians_to_degrees(calculator_app_t* calculator, double radians);
double calculator_app_degrees_to_gradians(calculator_app_t* calculator, double degrees);
double calculator_app_gradians_to_degrees(calculator_app_t* calculator, double gradians);

// Financial functions
double calculator_app_present_value(calculator_app_t* calculator, double future_value, double rate, uint32_t periods);
double calculator_app_future_value(calculator_app_t* calculator, double present_value, double rate, uint32_t periods);
double calculator_app_payment(calculator_app_t* calculator, double principal, double rate, uint32_t periods);
double calculator_app_interest_rate(calculator_app_t* calculator, double present_value, double future_value, uint32_t periods);
double calculator_app_number_of_periods(calculator_app_t* calculator, double present_value, double future_value, double rate);
double calculator_app_net_present_value(calculator_app_t* calculator, double* cash_flows, uint32_t count, double rate);
double calculator_app_internal_rate_of_return(calculator_app_t* calculator, double* cash_flows, uint32_t count);

// Statistics functions
double calculator_app_mean(calculator_app_t* calculator, double* values, uint32_t count);
double calculator_app_median(calculator_app_t* calculator, double* values, uint32_t count);
double calculator_app_mode(calculator_app_t* calculator, double* values, uint32_t count);
double calculator_app_standard_deviation(calculator_app_t* calculator, double* values, uint32_t count);
double calculator_app_variance(calculator_app_t* calculator, double* values, uint32_t count);
double calculator_app_correlation(calculator_app_t* calculator, double* x_values, double* y_values, uint32_t count);
double calculator_app_linear_regression(calculator_app_t* calculator, double* x_values, double* y_values, uint32_t count, double* slope, double* intercept);

// Unit conversion
double calculator_app_convert_length(calculator_app_t* calculator, double value, const char* from_unit, const char* to_unit);
double calculator_app_convert_weight(calculator_app_t* calculator, double value, const char* from_unit, const char* to_unit);
double calculator_app_convert_volume(calculator_app_t* calculator, double value, const char* from_unit, const char* to_unit);
double calculator_app_convert_temperature(calculator_app_t* calculator, double value, const char* from_unit, const char* to_unit);
double calculator_app_convert_pressure(calculator_app_t* calculator, double value, const char* from_unit, const char* to_unit);
double calculator_app_convert_energy(calculator_app_t* calculator, double value, const char* from_unit, const char* to_unit);
double calculator_app_convert_power(calculator_app_t* calculator, double value, const char* from_unit, const char* to_unit);
double calculator_app_convert_speed(calculator_app_t* calculator, double value, const char* from_unit, const char* to_unit);

// History management
void calculator_app_add_to_history(calculator_app_t* calculator, const char* expression, double result);
calculator_history_entry_t* calculator_app_get_history(calculator_app_t* calculator, uint32_t* count);
void calculator_app_clear_history(calculator_app_t* calculator);
void calculator_app_remove_history_entry(calculator_app_t* calculator, uint32_t index);
bool calculator_app_save_history(calculator_app_t* calculator, const char* filename);
bool calculator_app_load_history(calculator_app_t* calculator, const char* filename);

// Display and formatting
void calculator_app_set_precision(calculator_app_t* calculator, uint32_t precision);
uint32_t calculator_app_get_precision(calculator_app_t* calculator);
void calculator_app_set_scientific_notation(calculator_app_t* calculator, bool enable);
bool calculator_app_is_scientific_notation(calculator_app_t* calculator);
char* calculator_app_format_number(calculator_app_t* calculator, double value);
char* calculator_app_format_expression(calculator_app_t* calculator, const char* expression, double result);

// Error handling
bool calculator_app_has_error(calculator_app_t* calculator);
const char* calculator_app_get_error_message(calculator_app_t* calculator);
void calculator_app_clear_error(calculator_app_t* calculator);
bool calculator_app_is_valid_expression(calculator_app_t* calculator, const char* expression);

// Information
double calculator_app_get_display_value(calculator_app_t* calculator);
double calculator_app_get_previous_value(calculator_app_t* calculator);
calculator_operation_t calculator_app_get_current_operation(calculator_app_t* calculator);
bool calculator_app_is_operation_pending(calculator_app_t* calculator);
uint32_t calculator_app_get_history_count(calculator_app_t* calculator);
bool calculator_app_is_focused(calculator_app_t* calculator);

// Utility functions
char* calculator_app_get_mode_name(calculator_mode_t mode);
char* calculator_app_get_number_system_name(number_system_t system);
char* calculator_app_get_angle_unit_name(angle_unit_t unit);
char* calculator_app_get_operation_name(calculator_operation_t operation);
bool calculator_app_is_mode_supported(calculator_mode_t mode);
bool calculator_app_is_number_system_supported(number_system_t system);
bool calculator_app_is_operation_supported(calculator_operation_t operation);
double calculator_app_round_to_precision(double value, uint32_t precision);

// Callbacks
typedef void (*calculator_result_callback_t)(calculator_app_t* calculator, double result, void* user_data);
typedef void (*calculator_error_callback_t)(calculator_app_t* calculator, const char* error, void* user_data);
typedef void (*calculator_mode_change_callback_t)(calculator_app_t* calculator, calculator_mode_t old_mode, calculator_mode_t new_mode, void* user_data);

void calculator_app_set_result_callback(calculator_app_t* calculator, calculator_result_callback_t callback, void* user_data);
void calculator_app_set_error_callback(calculator_app_t* calculator, calculator_error_callback_t callback, void* user_data);
void calculator_app_set_mode_change_callback(calculator_app_t* calculator, calculator_mode_change_callback_t callback, void* user_data);

// Preset configurations
calculator_config_t calculator_app_preset_normal_style(void);
calculator_config_t calculator_app_preset_scientific_style(void);
calculator_config_t calculator_app_preset_programmer_style(void);
calculator_config_t calculator_app_preset_financial_style(void);

// Error handling
typedef enum {
    CALCULATOR_APP_SUCCESS = 0,
    CALCULATOR_APP_ERROR_INVALID_CONTEXT,
    CALCULATOR_APP_ERROR_INVALID_OPERATION,
    CALCULATOR_APP_ERROR_DIVISION_BY_ZERO,
    CALCULATOR_APP_ERROR_OVERFLOW,
    CALCULATOR_APP_ERROR_UNDERFLOW,
    CALCULATOR_APP_ERROR_INVALID_INPUT,
    CALCULATOR_APP_ERROR_OUT_OF_MEMORY,
    CALCULATOR_APP_ERROR_NOT_SUPPORTED,
} calculator_app_error_t;

calculator_app_error_t calculator_app_get_last_error(void);
const char* calculator_app_get_error_string(calculator_app_error_t error);

#endif // CALCULATOR_APP_H 