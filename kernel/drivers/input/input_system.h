#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memory.h>

// Input System - Advanced input handling for RaeenOS
// Provides keyboard, mouse, touch, and other input device support

// Input device types
typedef enum {
    INPUT_DEVICE_KEYBOARD = 0,       // Keyboard
    INPUT_DEVICE_MOUSE,              // Mouse
    INPUT_DEVICE_TOUCHPAD,           // Touchpad
    INPUT_DEVICE_TOUCHSCREEN,        // Touchscreen
    INPUT_DEVICE_GAMEPAD,            // Gamepad/Controller
    INPUT_DEVICE_JOYSTICK,           // Joystick
    INPUT_DEVICE_WHEEL,              // Steering wheel
    INPUT_DEVICE_PEN,                // Pen/Tablet
    INPUT_DEVICE_TOUCH,              // Touch input
    INPUT_DEVICE_GESTURE,            // Gesture recognition
    INPUT_DEVICE_VOICE,              // Voice input
    INPUT_DEVICE_EYE_TRACKING,       // Eye tracking
    INPUT_DEVICE_BRAIN_COMPUTER,     // Brain-computer interface
} input_device_type_t;

// Input event types
typedef enum {
    INPUT_EVENT_KEY_DOWN = 0,        // Key pressed
    INPUT_EVENT_KEY_UP,              // Key released
    INPUT_EVENT_KEY_REPEAT,          // Key repeat
    INPUT_EVENT_MOUSE_MOVE,          // Mouse movement
    INPUT_EVENT_MOUSE_BUTTON_DOWN,   // Mouse button pressed
    INPUT_EVENT_MOUSE_BUTTON_UP,     // Mouse button released
    INPUT_EVENT_MOUSE_WHEEL,         // Mouse wheel
    INPUT_EVENT_TOUCH_DOWN,          // Touch down
    INPUT_EVENT_TOUCH_UP,            // Touch up
    INPUT_EVENT_TOUCH_MOVE,          // Touch move
    INPUT_EVENT_GESTURE,             // Gesture
    INPUT_EVENT_VOICE,               // Voice input
    INPUT_EVENT_GAMEPAD_BUTTON,      // Gamepad button
    INPUT_EVENT_GAMEPAD_AXIS,        // Gamepad axis
    INPUT_EVENT_PEN_DOWN,            // Pen down
    INPUT_EVENT_PEN_UP,              // Pen up
    INPUT_EVENT_PEN_MOVE,            // Pen move
    INPUT_EVENT_PEN_PRESSURE,        // Pen pressure
} input_event_type_t;

// Keyboard keys
typedef enum {
    KEY_NONE = 0,
    KEY_ESCAPE = 1,
    KEY_F1 = 2, KEY_F2 = 3, KEY_F3 = 4, KEY_F4 = 5,
    KEY_F5 = 6, KEY_F6 = 7, KEY_F7 = 8, KEY_F8 = 9,
    KEY_F9 = 10, KEY_F10 = 11, KEY_F11 = 12, KEY_F12 = 13,
    KEY_PRINT_SCREEN = 14, KEY_SCROLL_LOCK = 15, KEY_PAUSE = 16,
    KEY_BACKTICK = 17, KEY_1 = 18, KEY_2 = 19, KEY_3 = 20,
    KEY_4 = 21, KEY_5 = 22, KEY_6 = 23, KEY_7 = 24,
    KEY_8 = 25, KEY_9 = 26, KEY_0 = 27, KEY_MINUS = 28,
    KEY_EQUALS = 29, KEY_BACKSPACE = 30, KEY_TAB = 31,
    KEY_Q = 32, KEY_W = 33, KEY_E = 34, KEY_R = 35,
    KEY_T = 36, KEY_Y = 37, KEY_U = 38, KEY_I = 39,
    KEY_O = 40, KEY_P = 41, KEY_LEFT_BRACKET = 42, KEY_RIGHT_BRACKET = 43,
    KEY_BACKSLASH = 44, KEY_CAPS_LOCK = 45, KEY_A = 46, KEY_S = 47,
    KEY_D = 48, KEY_F = 49, KEY_G = 50, KEY_H = 51,
    KEY_J = 52, KEY_K = 53, KEY_L = 54, KEY_SEMICOLON = 55,
    KEY_APOSTROPHE = 56, KEY_ENTER = 57, KEY_LEFT_SHIFT = 58,
    KEY_Z = 59, KEY_X = 60, KEY_C = 61, KEY_V = 62,
    KEY_B = 63, KEY_N = 64, KEY_M = 65, KEY_COMMA = 66,
    KEY_PERIOD = 67, KEY_SLASH = 68, KEY_RIGHT_SHIFT = 69,
    KEY_LEFT_CTRL = 70, KEY_LEFT_ALT = 71, KEY_SPACE = 72,
    KEY_RIGHT_ALT = 73, KEY_RIGHT_CTRL = 74, KEY_INSERT = 75,
    KEY_HOME = 76, KEY_PAGE_UP = 77, KEY_DELETE = 78,
    KEY_END = 79, KEY_PAGE_DOWN = 80, KEY_UP = 81,
    KEY_LEFT = 82, KEY_DOWN = 83, KEY_RIGHT = 84,
    KEY_NUM_LOCK = 85, KEY_NUMPAD_DIVIDE = 86, KEY_NUMPAD_MULTIPLY = 87,
    KEY_NUMPAD_SUBTRACT = 88, KEY_NUMPAD_7 = 89, KEY_NUMPAD_8 = 90,
    KEY_NUMPAD_9 = 91, KEY_NUMPAD_ADD = 92, KEY_NUMPAD_4 = 93,
    KEY_NUMPAD_5 = 94, KEY_NUMPAD_6 = 95, KEY_NUMPAD_1 = 96,
    KEY_NUMPAD_2 = 97, KEY_NUMPAD_3 = 98, KEY_NUMPAD_ENTER = 99,
    KEY_NUMPAD_0 = 100, KEY_NUMPAD_DECIMAL = 101,
    KEY_MENU = 102, KEY_LEFT_WINDOWS = 103, KEY_RIGHT_WINDOWS = 104,
    KEY_VOLUME_MUTE = 105, KEY_VOLUME_DOWN = 106, KEY_VOLUME_UP = 107,
    KEY_MEDIA_PLAY = 108, KEY_MEDIA_STOP = 109, KEY_MEDIA_PREV = 110,
    KEY_MEDIA_NEXT = 111, KEY_BRIGHTNESS_DOWN = 112, KEY_BRIGHTNESS_UP = 113,
    KEY_MAX = 114,
} keyboard_key_t;

// Mouse buttons
typedef enum {
    MOUSE_BUTTON_LEFT = 0,           // Left button
    MOUSE_BUTTON_RIGHT,              // Right button
    MOUSE_BUTTON_MIDDLE,             // Middle button
    MOUSE_BUTTON_X1,                 // X1 button (back)
    MOUSE_BUTTON_X2,                 // X2 button (forward)
    MOUSE_BUTTON_MAX = 5,
} mouse_button_t;

// Gamepad buttons
typedef enum {
    GAMEPAD_BUTTON_A = 0,            // A button
    GAMEPAD_BUTTON_B,                // B button
    GAMEPAD_BUTTON_X,                // X button
    GAMEPAD_BUTTON_Y,                // Y button
    GAMEPAD_BUTTON_LB,               // Left bumper
    GAMEPAD_BUTTON_RB,               // Right bumper
    GAMEPAD_BUTTON_BACK,             // Back button
    GAMEPAD_BUTTON_START,            // Start button
    GAMEPAD_BUTTON_LEFT_STICK,       // Left stick press
    GAMEPAD_BUTTON_RIGHT_STICK,      // Right stick press
    GAMEPAD_BUTTON_DPAD_UP,          // D-pad up
    GAMEPAD_BUTTON_DPAD_DOWN,        // D-pad down
    GAMEPAD_BUTTON_DPAD_LEFT,        // D-pad left
    GAMEPAD_BUTTON_DPAD_RIGHT,       // D-pad right
    GAMEPAD_BUTTON_MAX = 14,
} gamepad_button_t;

// Gamepad axes
typedef enum {
    GAMEPAD_AXIS_LEFT_X = 0,         // Left stick X
    GAMEPAD_AXIS_LEFT_Y,             // Left stick Y
    GAMEPAD_AXIS_RIGHT_X,            // Right stick X
    GAMEPAD_AXIS_RIGHT_Y,            // Right stick Y
    GAMEPAD_AXIS_LEFT_TRIGGER,       // Left trigger
    GAMEPAD_AXIS_RIGHT_TRIGGER,      // Right trigger
    GAMEPAD_AXIS_MAX = 6,
} gamepad_axis_t;

// Gesture types
typedef enum {
    GESTURE_TAP = 0,                 // Single tap
    GESTURE_DOUBLE_TAP,              // Double tap
    GESTURE_LONG_PRESS,              // Long press
    GESTURE_SWIPE_LEFT,              // Swipe left
    GESTURE_SWIPE_RIGHT,             // Swipe right
    GESTURE_SWIPE_UP,                // Swipe up
    GESTURE_SWIPE_DOWN,              // Swipe down
    GESTURE_PINCH_IN,                // Pinch in
    GESTURE_PINCH_OUT,               // Pinch out
    GESTURE_ROTATE,                  // Rotate
    GESTURE_MAX = 10,
} gesture_type_t;

// Input device information
typedef struct {
    uint32_t id;
    char name[64];                   // Device name
    char manufacturer[32];            // Manufacturer
    char model[32];                  // Model
    char serial[32];                 // Serial number
    input_device_type_t type;        // Device type
    bool is_connected;               // Is connected
    bool is_enabled;                 // Is enabled
    bool is_primary;                 // Is primary device
    uint32_t capabilities;           // Device capabilities
    void* device_data;               // Device-specific data
} input_device_t;

// Input event
typedef struct {
    uint32_t id;
    input_event_type_t type;         // Event type
    uint32_t device_id;              // Source device
    uint64_t timestamp;              // Event timestamp
    uint32_t modifiers;              // Modifier keys
    union {
        struct {
            keyboard_key_t key;      // Key code
            bool is_repeat;          // Is key repeat
        } keyboard;
        struct {
            float x, y;              // Position
            float delta_x, delta_y;  // Movement delta
            mouse_button_t button;   // Button
            float wheel_delta;       // Wheel delta
        } mouse;
        struct {
            float x, y;              // Position
            float pressure;          // Pressure
            uint32_t touch_id;       // Touch ID
        } touch;
        struct {
            gesture_type_t gesture;  // Gesture type
            float x, y;              // Position
            float scale;             // Scale factor
            float rotation;          // Rotation angle
        } gesture;
        struct {
            gamepad_button_t button; // Button
            bool is_pressed;         // Is pressed
        } gamepad_button;
        struct {
            gamepad_axis_t axis;     // Axis
            float value;             // Axis value
        } gamepad_axis;
        struct {
            char* text;              // Voice text
            float confidence;        // Confidence level
        } voice;
    } data;
} input_event_t;

// Input mapping
typedef struct {
    uint32_t id;
    char name[64];                   // Mapping name
    input_device_type_t device_type; // Device type
    uint32_t source_event;           // Source event
    uint32_t target_action;          // Target action
    bool is_enabled;                 // Is enabled
    void* mapping_data;              // Mapping-specific data
} input_mapping_t;

// Input system configuration
typedef struct {
    bool enable_keyboard;            // Enable keyboard input
    bool enable_mouse;               // Enable mouse input
    bool enable_touch;               // Enable touch input
    bool enable_gesture;             // Enable gesture recognition
    bool enable_voice;               // Enable voice input
    bool enable_gamepad;             // Enable gamepad input
    bool enable_pen;                 // Enable pen input
    bool enable_eye_tracking;        // Enable eye tracking
    bool enable_brain_computer;      // Enable brain-computer interface
    uint32_t max_devices;            // Maximum input devices
    uint32_t max_events;             // Maximum events in queue
    uint32_t event_timeout;          // Event timeout in ms
    bool enable_input_mapping;       // Enable input mapping
    bool enable_macro_recording;     // Enable macro recording
    bool enable_auto_repeat;         // Enable auto repeat
    uint32_t repeat_delay;           // Repeat delay in ms
    uint32_t repeat_rate;            // Repeat rate in ms
} input_system_config_t;

// Input system context
typedef struct {
    input_system_config_t config;
    input_device_t* devices;
    uint32_t device_count;
    uint32_t max_devices;
    input_event_t* event_queue;
    uint32_t event_count;
    uint32_t max_events;
    uint32_t event_head;
    uint32_t event_tail;
    input_mapping_t* mappings;
    uint32_t mapping_count;
    uint32_t max_mappings;
    bool initialized;
    uint32_t next_device_id;
    uint32_t next_event_id;
    uint32_t next_mapping_id;
    uint64_t last_update_time;
} input_system_t;

// Function prototypes

// Initialization and shutdown
input_system_t* input_system_init(input_system_config_t* config);
void input_system_shutdown(input_system_t* input);
bool input_system_is_initialized(input_system_t* input);

// Device management
uint32_t input_system_add_device(input_system_t* input, input_device_t* device);
void input_system_remove_device(input_system_t* input, uint32_t device_id);
input_device_t* input_system_get_device(input_system_t* input, uint32_t device_id);
input_device_t* input_system_get_devices(input_system_t* input, uint32_t* count);
uint32_t input_system_get_device_count(input_system_t* input);
input_device_t* input_system_get_devices_by_type(input_system_t* input, input_device_type_t type, uint32_t* count);
input_device_t* input_system_get_primary_device(input_system_t* input, input_device_type_t type);

// Event handling
void input_system_push_event(input_system_t* input, input_event_t* event);
input_event_t* input_system_pop_event(input_system_t* input);
input_event_t* input_system_peek_event(input_system_t* input);
uint32_t input_system_get_event_count(input_system_t* input);
void input_system_clear_events(input_system_t* input);
bool input_system_has_events(input_system_t* input);

// Keyboard input
void input_system_send_key_event(input_system_t* input, uint32_t device_id, keyboard_key_t key, bool is_pressed, bool is_repeat);
bool input_system_is_key_pressed(input_system_t* input, keyboard_key_t key);
bool input_system_is_modifier_pressed(input_system_t* input, uint32_t modifier);
uint32_t input_system_get_pressed_keys(input_system_t* input, keyboard_key_t* keys, uint32_t max_keys);
void input_system_set_key_repeat(input_system_t* input, bool enable, uint32_t delay, uint32_t rate);

// Mouse input
void input_system_send_mouse_move(input_system_t* input, uint32_t device_id, float x, float y, float delta_x, float delta_y);
void input_system_send_mouse_button(input_system_t* input, uint32_t device_id, mouse_button_t button, bool is_pressed);
void input_system_send_mouse_wheel(input_system_t* input, uint32_t device_id, float delta);
void input_system_get_mouse_position(input_system_t* input, uint32_t device_id, float* x, float* y);
bool input_system_is_mouse_button_pressed(input_system_t* input, mouse_button_t button);
void input_system_set_mouse_sensitivity(input_system_t* input, uint32_t device_id, float sensitivity);

// Touch input
void input_system_send_touch_event(input_system_t* input, uint32_t device_id, input_event_type_t type, float x, float y, float pressure, uint32_t touch_id);
void input_system_get_touch_position(input_system_t* input, uint32_t device_id, uint32_t touch_id, float* x, float* y);
uint32_t input_system_get_active_touches(input_system_t* input, uint32_t device_id, uint32_t* touch_ids, uint32_t max_touches);

// Gesture recognition
void input_system_send_gesture_event(input_system_t* input, uint32_t device_id, gesture_type_t gesture, float x, float y, float scale, float rotation);
bool input_system_enable_gesture_recognition(input_system_t* input, uint32_t device_id, bool enable);
bool input_system_is_gesture_enabled(input_system_t* input, uint32_t device_id);

// Gamepad input
void input_system_send_gamepad_button(input_system_t* input, uint32_t device_id, gamepad_button_t button, bool is_pressed);
void input_system_send_gamepad_axis(input_system_t* input, uint32_t device_id, gamepad_axis_t axis, float value);
bool input_system_is_gamepad_button_pressed(input_system_t* input, uint32_t device_id, gamepad_button_t button);
float input_system_get_gamepad_axis_value(input_system_t* input, uint32_t device_id, gamepad_axis_t axis);

// Voice input
void input_system_send_voice_event(input_system_t* input, uint32_t device_id, const char* text, float confidence);
bool input_system_enable_voice_recognition(input_system_t* input, uint32_t device_id, bool enable);
bool input_system_is_voice_enabled(input_system_t* input, uint32_t device_id);

// Input mapping
uint32_t input_system_add_mapping(input_system_t* input, input_mapping_t* mapping);
void input_system_remove_mapping(input_system_t* input, uint32_t mapping_id);
input_mapping_t* input_system_get_mapping(input_system_t* input, uint32_t mapping_id);
input_mapping_t* input_system_get_mappings(input_system_t* input, uint32_t* count);
void input_system_enable_mapping(input_system_t* input, uint32_t mapping_id, bool enable);
bool input_system_is_mapping_enabled(input_system_t* input, uint32_t mapping_id);

// Macro recording
void input_system_start_macro_recording(input_system_t* input, uint32_t device_id);
void input_system_stop_macro_recording(input_system_t* input, uint32_t device_id);
bool input_system_is_macro_recording(input_system_t* input, uint32_t device_id);
void input_system_save_macro(input_system_t* input, uint32_t device_id, const char* name);
void input_system_load_macro(input_system_t* input, uint32_t device_id, const char* name);
void input_system_play_macro(input_system_t* input, uint32_t device_id, const char* name);

// Device capabilities
bool input_system_has_capability(input_system_t* input, uint32_t device_id, uint32_t capability);
uint32_t input_system_get_capabilities(input_system_t* input, uint32_t device_id);
bool input_system_supports_haptic_feedback(input_system_t* input, uint32_t device_id);
void input_system_send_haptic_feedback(input_system_t* input, uint32_t device_id, uint32_t type, float intensity, uint32_t duration);

// Information
uint32_t input_system_get_connected_device_count(input_system_t* input);
uint32_t input_system_get_enabled_device_count(input_system_t* input);
bool input_system_is_device_connected(input_system_t* input, uint32_t device_id);
bool input_system_is_device_enabled(input_system_t* input, uint32_t device_id);
const char* input_system_get_device_name(input_system_t* input, uint32_t device_id);

// Utility functions
char* input_system_get_key_name(keyboard_key_t key);
char* input_system_get_mouse_button_name(mouse_button_t button);
char* input_system_get_gamepad_button_name(gamepad_button_t button);
char* input_system_get_gamepad_axis_name(gamepad_axis_t axis);
char* input_system_get_gesture_name(gesture_type_t gesture);
char* input_system_get_device_type_name(input_device_type_t type);
bool input_system_is_key_valid(keyboard_key_t key);
bool input_system_is_mouse_button_valid(mouse_button_t button);
bool input_system_is_gamepad_button_valid(gamepad_button_t button);
bool input_system_is_gamepad_axis_valid(gamepad_axis_t axis);

// Callbacks
typedef void (*input_event_callback_t)(input_system_t* input, input_event_t* event, void* user_data);
typedef void (*device_connected_callback_t)(input_system_t* input, uint32_t device_id, void* user_data);
typedef void (*device_disconnected_callback_t)(input_system_t* input, uint32_t device_id, void* user_data);

void input_system_set_event_callback(input_system_t* input, input_event_callback_t callback, void* user_data);
void input_system_set_device_connected_callback(input_system_t* input, device_connected_callback_t callback, void* user_data);
void input_system_set_device_disconnected_callback(input_system_t* input, device_disconnected_callback_t callback, void* user_data);

// Preset configurations
input_system_config_t input_system_preset_normal_style(void);
input_system_config_t input_system_preset_gaming_style(void);
input_system_config_t input_system_preset_touch_style(void);
input_system_config_t input_system_preset_accessibility_style(void);

// Error handling
typedef enum {
    INPUT_SYSTEM_SUCCESS = 0,
    INPUT_SYSTEM_ERROR_INVALID_CONTEXT,
    INPUT_SYSTEM_ERROR_INVALID_DEVICE,
    INPUT_SYSTEM_ERROR_INVALID_EVENT,
    INPUT_SYSTEM_ERROR_OUT_OF_MEMORY,
    INPUT_SYSTEM_ERROR_DEVICE_NOT_FOUND,
    INPUT_SYSTEM_ERROR_EVENT_QUEUE_FULL,
    INPUT_SYSTEM_ERROR_INVALID_MAPPING,
    INPUT_SYSTEM_ERROR_DRIVER_FAILED,
} input_system_error_t;

input_system_error_t input_system_get_last_error(void);
const char* input_system_get_error_string(input_system_error_t error);

#endif // INPUT_SYSTEM_H 