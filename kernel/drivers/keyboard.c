#include "drivers/keyboard.h"
#include "kernel.h"
#include "hal/hal.h"
#include <string.h>

// Keyboard constants
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_COMMAND_PORT 0x64
#define KEYBOARD_BUFFER_SIZE 256

// Keyboard commands
#define KEYBOARD_CMD_SET_LED 0xED
#define KEYBOARD_CMD_ECHO 0xEE
#define KEYBOARD_CMD_SET_SCANCODE 0xF0
#define KEYBOARD_CMD_IDENTIFY 0xF2
#define KEYBOARD_CMD_SET_TYPEMATIC 0xF3
#define KEYBOARD_CMD_ENABLE 0xF4
#define KEYBOARD_CMD_RESET 0xFF

// Keyboard responses
#define KEYBOARD_RESPONSE_ACK 0xFA
#define KEYBOARD_RESPONSE_RESEND 0xFE
#define KEYBOARD_RESPONSE_ERROR 0xFC

// Scan code sets
#define SCANCODE_SET_1 0x01
#define SCANCODE_SET_2 0x02
#define SCANCODE_SET_3 0x03

// Special keys
#define KEY_ESCAPE 0x01
#define KEY_BACKSPACE 0x0E
#define KEY_TAB 0x0F
#define KEY_ENTER 0x1C
#define KEY_LEFT_SHIFT 0x2A
#define KEY_RIGHT_SHIFT 0x36
#define KEY_LEFT_CTRL 0x1D
#define KEY_LEFT_ALT 0x38
#define KEY_CAPS_LOCK 0x3A
#define KEY_F1 0x3B
#define KEY_F2 0x3C
#define KEY_F3 0x3D
#define KEY_F4 0x3E
#define KEY_F5 0x3F
#define KEY_F6 0x40
#define KEY_F7 0x41
#define KEY_F8 0x42
#define KEY_F9 0x43
#define KEY_F10 0x44
#define KEY_F11 0x57
#define KEY_F12 0x58
#define KEY_NUM_LOCK 0x45
#define KEY_SCROLL_LOCK 0x46
#define KEY_HOME 0x47
#define KEY_UP 0x48
#define KEY_PAGE_UP 0x49
#define KEY_LEFT 0x4B
#define KEY_RIGHT 0x4D
#define KEY_END 0x4F
#define KEY_DOWN 0x50
#define KEY_PAGE_DOWN 0x51
#define KEY_INSERT 0x52
#define KEY_DELETE 0x53

// Global keyboard state
static keyboard_state_t keyboard_state = {0};
static bool keyboard_initialized = false;
static uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static size_t keyboard_buffer_head = 0;
static size_t keyboard_buffer_tail = 0;
static size_t keyboard_buffer_count = 0;

// ASCII table for scan code set 1
static const char ascii_table[128] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Shifted ASCII table
static const char ascii_table_shift[128] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Initialize keyboard driver
error_t keyboard_init(void) {
    if (keyboard_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing keyboard driver...");
    
    // Initialize keyboard state
    memset(&keyboard_state, 0, sizeof(keyboard_state));
    keyboard_state.caps_lock = false;
    keyboard_state.num_lock = false;
    keyboard_state.scroll_lock = false;
    keyboard_state.left_shift = false;
    keyboard_state.right_shift = false;
    keyboard_state.left_ctrl = false;
    keyboard_state.left_alt = false;
    keyboard_state.extended = false;
    
    // Initialize keyboard buffer
    memset(keyboard_buffer, 0, sizeof(keyboard_buffer));
    keyboard_buffer_head = 0;
    keyboard_buffer_tail = 0;
    keyboard_buffer_count = 0;
    
    // Reset keyboard
    keyboard_reset();
    
    // Enable keyboard
    keyboard_enable();
    
    keyboard_initialized = true;
    KINFO("Keyboard driver initialized successfully");
    
    return SUCCESS;
}

// Reset keyboard
void keyboard_reset(void) {
    // Send reset command
    keyboard_send_command(KEYBOARD_CMD_RESET);
    
    // Wait for ACK
    uint8_t response = keyboard_read_response();
    if (response == KEYBOARD_RESPONSE_ACK) {
        KDEBUG("Keyboard reset acknowledged");
    } else {
        KWARN("Keyboard reset failed: 0x%02x", response);
    }
}

// Enable keyboard
void keyboard_enable(void) {
    // Send enable command
    keyboard_send_command(KEYBOARD_CMD_ENABLE);
    
    // Wait for ACK
    uint8_t response = keyboard_read_response();
    if (response == KEYBOARD_RESPONSE_ACK) {
        KDEBUG("Keyboard enabled");
    } else {
        KWARN("Keyboard enable failed: 0x%02x", response);
    }
}

// Send command to keyboard
void keyboard_send_command(uint8_t command) {
    // Wait for input buffer to be empty
    while (hal_inb(KEYBOARD_COMMAND_PORT) & 0x02);
    
    // Send command
    hal_outb(KEYBOARD_COMMAND_PORT, command);
}

// Read response from keyboard
uint8_t keyboard_read_response(void) {
    // Wait for output buffer to be full
    while (!(hal_inb(KEYBOARD_COMMAND_PORT) & 0x01));
    
    // Read response
    return hal_inb(KEYBOARD_DATA_PORT);
}

// Read scan code from keyboard
uint8_t keyboard_read_scancode(void) {
    // Check if output buffer is full
    if (!(hal_inb(KEYBOARD_COMMAND_PORT) & 0x01)) {
        return 0; // No data available
    }
    
    // Read scan code
    return hal_inb(KEYBOARD_DATA_PORT);
}

// Process scan code
void keyboard_process_scancode(uint8_t scancode) {
    KDEBUG("Processing scan code: 0x%02x", scancode);
    
    // Check for extended key prefix
    if (scancode == 0xE0) {
        keyboard_state.extended = true;
        return;
    }
    
    // Check for key release
    bool key_released = (scancode & 0x80) != 0;
    uint8_t key_code = scancode & 0x7F;
    
    // Handle special keys
    switch (key_code) {
        case KEY_LEFT_SHIFT:
            keyboard_state.left_shift = !key_released;
            break;
        case KEY_RIGHT_SHIFT:
            keyboard_state.right_shift = !key_released;
            break;
        case KEY_LEFT_CTRL:
            keyboard_state.left_ctrl = !key_released;
            break;
        case KEY_LEFT_ALT:
            keyboard_state.left_alt = !key_released;
            break;
        case KEY_CAPS_LOCK:
            if (!key_released) {
                keyboard_state.caps_lock = !keyboard_state.caps_lock;
            }
            break;
        case KEY_NUM_LOCK:
            if (!key_released) {
                keyboard_state.num_lock = !keyboard_state.num_lock;
            }
            break;
        case KEY_SCROLL_LOCK:
            if (!key_released) {
                keyboard_state.scroll_lock = !keyboard_state.scroll_lock;
            }
            break;
        default:
            // Handle regular keys
            if (!key_released && key_code < 128) {
                char ascii = keyboard_scancode_to_ascii(key_code);
                if (ascii != 0) {
                    keyboard_buffer_add(ascii);
                }
            }
            break;
    }
    
    // Reset extended flag
    keyboard_state.extended = false;
}

// Convert scan code to ASCII
char keyboard_scancode_to_ascii(uint8_t scancode) {
    if (scancode >= 128) {
        return 0;
    }
    
    // Check if shift is pressed
    bool shift = keyboard_state.left_shift || keyboard_state.right_shift;
    
    // Get ASCII character
    char ascii = shift ? ascii_table_shift[scancode] : ascii_table[scancode];
    
    // Apply caps lock
    if (keyboard_state.caps_lock && ascii >= 'a' && ascii <= 'z') {
        ascii = ascii - 'a' + 'A';
    }
    
    return ascii;
}

// Add character to keyboard buffer
void keyboard_buffer_add(char c) {
    if (keyboard_buffer_count >= KEYBOARD_BUFFER_SIZE) {
        return; // Buffer full
    }
    
    keyboard_buffer[keyboard_buffer_tail] = c;
    keyboard_buffer_tail = (keyboard_buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    keyboard_buffer_count++;
}

// Read character from keyboard buffer
int keyboard_read_char(void) {
    if (keyboard_buffer_count == 0) {
        return -1; // No data available
    }
    
    char c = keyboard_buffer[keyboard_buffer_head];
    keyboard_buffer_head = (keyboard_buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    keyboard_buffer_count--;
    
    return c;
}

// Check if keyboard buffer has data
bool keyboard_has_data(void) {
    return keyboard_buffer_count > 0;
}

// Get keyboard buffer size
size_t keyboard_buffer_size(void) {
    return keyboard_buffer_count;
}

// Clear keyboard buffer
void keyboard_clear_buffer(void) {
    keyboard_buffer_head = 0;
    keyboard_buffer_tail = 0;
    keyboard_buffer_count = 0;
}

// Get keyboard state
keyboard_state_t* keyboard_get_state(void) {
    return &keyboard_state;
}

// Set keyboard LEDs
void keyboard_set_leds(bool caps_lock, bool num_lock, bool scroll_lock) {
    uint8_t leds = 0;
    if (caps_lock) leds |= 0x04;
    if (num_lock) leds |= 0x02;
    if (scroll_lock) leds |= 0x01;
    
    // Send LED command
    keyboard_send_command(KEYBOARD_CMD_SET_LED);
    
    // Wait for ACK
    uint8_t response = keyboard_read_response();
    if (response == KEYBOARD_RESPONSE_ACK) {
        // Send LED data
        hal_outb(KEYBOARD_DATA_PORT, leds);
        
        // Wait for ACK
        response = keyboard_read_response();
        if (response == KEYBOARD_RESPONSE_ACK) {
            KDEBUG("Keyboard LEDs set: 0x%02x", leds);
        }
    }
}

// Handle keyboard interrupt
void keyboard_interrupt_handler(void) {
    uint8_t scancode = keyboard_read_scancode();
    if (scancode != 0) {
        keyboard_process_scancode(scancode);
    }
}

// Check if keyboard is initialized
bool keyboard_is_initialized(void) {
    return keyboard_initialized;
}

// Update keyboard state (called periodically)
void keyboard_update(void) {
    // Process any pending scan codes
    uint8_t scancode;
    while ((scancode = keyboard_read_scancode()) != 0) {
        keyboard_process_scancode(scancode);
    }
    
    // Update LEDs based on state
    keyboard_set_leds(keyboard_state.caps_lock, 
                     keyboard_state.num_lock, 
                     keyboard_state.scroll_lock);
} 