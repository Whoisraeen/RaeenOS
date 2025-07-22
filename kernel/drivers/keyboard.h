#ifndef RAEENOS_KEYBOARD_H
#define RAEENOS_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// Add missing type
typedef uint32_t spinlock_t;

// Keyboard controller ports
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_COMMAND_PORT 0x64

// Keyboard controller commands
#define KEYBOARD_CMD_READ_CONFIG   0x20
#define KEYBOARD_CMD_WRITE_CONFIG  0x60
#define KEYBOARD_CMD_DISABLE_PORT2 0xA7
#define KEYBOARD_CMD_ENABLE_PORT2  0xA8
#define KEYBOARD_CMD_TEST_PORT2    0xA9
#define KEYBOARD_CMD_TEST_CONTROLLER 0xAA
#define KEYBOARD_CMD_TEST_PORT1    0xAB
#define KEYBOARD_CMD_DISABLE_PORT1 0xAD
#define KEYBOARD_CMD_ENABLE_PORT1  0xAE
#define KEYBOARD_CMD_READ_OUTPUT   0xD0
#define KEYBOARD_CMD_WRITE_OUTPUT  0xD1

// Keyboard configuration bits
#define KEYBOARD_CONFIG_INTERRUPT  0x01
#define KEYBOARD_CONFIG_CLOCK      0x02
#define KEYBOARD_CONFIG_TRANSLATE  0x40

// Keyboard status bits
#define KEYBOARD_STATUS_OUTPUT_FULL  0x01
#define KEYBOARD_STATUS_INPUT_FULL   0x02
#define KEYBOARD_STATUS_SYSTEM_FLAG  0x04
#define KEYBOARD_STATUS_COMMAND_DATA 0x08
#define KEYBOARD_STATUS_LOCKED       0x10
#define KEYBOARD_STATUS_AUX_OUTPUT   0x20
#define KEYBOARD_STATUS_TIMEOUT      0x40
#define KEYBOARD_STATUS_PARITY_ERROR 0x80

// Special key codes
#define KEY_ESCAPE        0x01
#define KEY_BACKSPACE     0x0E
#define KEY_TAB           0x0F
#define KEY_ENTER         0x1C
#define KEY_CTRL_LEFT     0x1D
#define KEY_SHIFT_LEFT    0x2A
#define KEY_SHIFT_RIGHT   0x36
#define KEY_ALT_LEFT      0x38
#define KEY_CAPS_LOCK     0x3A
#define KEY_F1            0x3B
#define KEY_F2            0x3C
#define KEY_F3            0x3D
#define KEY_F4            0x3E
#define KEY_F5            0x3F
#define KEY_F6            0x40
#define KEY_F7            0x41
#define KEY_F8            0x42
#define KEY_F9            0x43
#define KEY_F10           0x44
#define KEY_NUM_LOCK      0x45
#define KEY_SCROLL_LOCK   0x46
#define KEY_HOME          0x47
#define KEY_UP            0x48
#define KEY_PAGE_UP       0x49
#define KEY_LEFT          0x4B
#define KEY_RIGHT         0x4D
#define KEY_END           0x4F
#define KEY_DOWN          0x50
#define KEY_PAGE_DOWN     0x51
#define KEY_INSERT        0x52
#define KEY_DELETE        0x53
#define KEY_F11           0x57
#define KEY_F12           0x58

// Key event types
typedef enum {
    KEY_EVENT_PRESS,
    KEY_EVENT_RELEASE,
    KEY_EVENT_REPEAT
} key_event_type_t;

// Key event structure
typedef struct key_event {
    uint8_t scancode;           // Raw scancode
    uint8_t ascii;              // ASCII character (if applicable)
    uint16_t keycode;           // Processed keycode
    key_event_type_t type;      // Event type
    uint64_t timestamp;         // Event timestamp
    bool ctrl;                  // Ctrl modifier
    bool shift;                 // Shift modifier
    bool alt;                   // Alt modifier
    bool caps_lock;             // Caps lock state
    bool num_lock;              // Num lock state
} key_event_t;

// Keyboard buffer structure
typedef struct keyboard_buffer {
    key_event_t events[256];    // Circular buffer of events
    uint32_t head;              // Buffer head
    uint32_t tail;              // Buffer tail
    uint32_t count;             // Number of events in buffer
    spinlock_t lock;            // Buffer lock
} keyboard_buffer_t;

// Keyboard state structure
typedef struct keyboard_state {
    bool initialized;           // Initialization flag
    bool enabled;              // Keyboard enabled flag
    bool translate;            // Scancode translation enabled
    
    // Modifier states
    bool ctrl_left;            // Left Ctrl pressed
    bool ctrl_right;           // Right Ctrl pressed
    bool shift_left;           // Left Shift pressed
    bool shift_right;          // Right Shift pressed
    bool alt_left;             // Left Alt pressed
    bool alt_right;            // Right Alt pressed
    bool caps_lock;            // Caps Lock state
    bool num_lock;             // Num Lock state
    bool scroll_lock;          // Scroll Lock state
    
    // Event buffer
    keyboard_buffer_t buffer;  // Key event buffer
    
    // Statistics
    uint64_t total_events;     // Total events processed
    uint64_t buffer_overflows; // Buffer overflow count
} keyboard_state_t;

// Function prototypes
int keyboard_init(void);
void keyboard_shutdown(void);

// Keyboard control
int keyboard_enable(void);
int keyboard_disable(void);
int keyboard_reset(void);
int keyboard_set_leds(uint8_t leds);

// Event handling
int keyboard_read_event(key_event_t* event);
int keyboard_peek_event(key_event_t* event);
int keyboard_clear_buffer(void);
bool keyboard_has_events(void);

// Input functions
int keyboard_read_char(char* ch);
int keyboard_read_string(char* buffer, size_t size);
int keyboard_read_line(char* buffer, size_t size);

// State queries
bool keyboard_is_ctrl_pressed(void);
bool keyboard_is_shift_pressed(void);
bool keyboard_is_alt_pressed(void);
bool keyboard_is_caps_lock_on(void);
bool keyboard_is_num_lock_on(void);

// Utility functions
uint8_t keyboard_scancode_to_ascii(uint8_t scancode);
char* keyboard_get_key_name(uint16_t keycode);
void keyboard_dump_state(void);

// Interrupt handler
void keyboard_interrupt_handler(void);

// Buffer management
int keyboard_buffer_put(key_event_t* event);
int keyboard_buffer_get(key_event_t* event);
uint32_t keyboard_buffer_count(void);

#endif // RAEENOS_KEYBOARD_H 