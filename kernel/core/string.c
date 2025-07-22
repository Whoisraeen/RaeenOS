#include "types.h"
#include <stdarg.h>

// String manipulation functions
size_t strlen(const char* str) {
    if (!str) return 0;
    
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    if (!dest || !src) return dest;
    
    char* orig_dest = dest;
    while ((*dest++ = *src++));
    return orig_dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    if (!dest || !src) return dest;
    
    char* orig_dest = dest;
    while (n-- && (*dest++ = *src++));
    while (n--) *dest++ = '\0';
    return orig_dest;
}

int strcmp(const char* s1, const char* s2) {
    if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);
    
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    if (!s1 || !s2 || n == 0) return 0;
    
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (n == SIZE_MAX) ? 0 : *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strcat(char* dest, const char* src) {
    if (!dest || !src) return dest;
    
    char* orig_dest = dest;
    dest += strlen(dest);
    strcpy(dest, src);
    return orig_dest;
}

// Memory manipulation functions
void* memset(void* ptr, int value, size_t num) {
    if (!ptr) return ptr;
    
    unsigned char* p = (unsigned char*)ptr;
    unsigned char val = (unsigned char)value;
    
    while (num--) {
        *p++ = val;
    }
    
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t num) {
    if (!dest || !src) return dest;
    
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    while (num--) {
        *d++ = *s++;
    }
    
    return dest;
}

void* memmove(void* dest, const void* src, size_t num) {
    if (!dest || !src) return dest;
    
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        // Copy forward
        while (num--) {
            *d++ = *s++;
        }
    } else {
        // Copy backward
        d += num;
        s += num;
        while (num--) {
            *(--d) = *(--s);
        }
    }
    
    return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num) {
    if (!ptr1 || !ptr2) return 0;
    
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    while (num--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    
    return 0;
}

// Simple printf implementation
static void print_char(char c) {
    // This should call the HAL console function
    hal_console_print("%c", c);
}

static void print_string(const char* str) {
    if (!str) str = "(null)";
    while (*str) {
        print_char(*str++);
    }
}

static void print_number(long long num, int base, bool uppercase, int width, char pad_char) {
    char buffer[32];
    int pos = 0;
    bool negative = false;
    
    if (num < 0 && base == 10) {
        negative = true;
        num = -num;
    }
    
    if (num == 0) {
        buffer[pos++] = '0';
    } else {
        const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
        while (num > 0) {
            buffer[pos++] = digits[num % base];
            num /= base;
        }
    }
    
    if (negative) {
        buffer[pos++] = '-';
    }
    
    // Pad if necessary
    while (pos < width) {
        buffer[pos++] = pad_char;
    }
    
    // Print in reverse order
    while (pos > 0) {
        print_char(buffer[--pos]);
    }
}

int vsnprintf(char* buffer, size_t size, const char* format, va_list args) {
    if (!buffer || !format || size == 0) return 0;
    
    size_t pos = 0;
    const char* fmt = format;
    
    while (*fmt && pos < size - 1) {
        if (*fmt != '%') {
            buffer[pos++] = *fmt++;
            continue;
        }
        
        fmt++; // Skip '%'
        
        // Parse format specifier
        bool left_align = false;
        bool show_sign = false;
        bool pad_zero = false;
        int width = 0;
        
        // Parse flags
        while (*fmt) {
            if (*fmt == '-') left_align = true;
            else if (*fmt == '+') show_sign = true;
            else if (*fmt == '0') pad_zero = true;
            else break;
            fmt++;
        }
        
        // Parse width
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }
        
        // Parse specifier
        switch (*fmt++) {
            case 'c': {
                char c = (char)va_arg(args, int);
                if (pos < size - 1) buffer[pos++] = c;
                break;
            }
            case 's': {
                const char* str = va_arg(args, const char*);
                if (!str) str = "(null)";
                while (*str && pos < size - 1) {
                    buffer[pos++] = *str++;
                }
                break;
            }
            case 'd':
            case 'i': {
                int num = va_arg(args, int);
                char temp[32];
                int temp_pos = 0;
                bool negative = false;
                
                if (num < 0) {
                    negative = true;
                    num = -num;
                }
                
                if (num == 0) {
                    temp[temp_pos++] = '0';
                } else {
                    while (num > 0) {
                        temp[temp_pos++] = '0' + (num % 10);
                        num /= 10;
                    }
                }
                
                if (negative && pos < size - 1) {
                    buffer[pos++] = '-';
                }
                
                while (temp_pos > 0 && pos < size - 1) {
                    buffer[pos++] = temp[--temp_pos];
                }
                break;
            }
            case 'u': {
                unsigned int num = va_arg(args, unsigned int);
                char temp[32];
                int temp_pos = 0;
                
                if (num == 0) {
                    temp[temp_pos++] = '0';
                } else {
                    while (num > 0) {
                        temp[temp_pos++] = '0' + (num % 10);
                        num /= 10;
                    }
                }
                
                while (temp_pos > 0 && pos < size - 1) {
                    buffer[pos++] = temp[--temp_pos];
                }
                break;
            }
            case 'x':
            case 'X': {
                unsigned int num = va_arg(args, unsigned int);
                char temp[32];
                int temp_pos = 0;
                bool uppercase = (*fmt - 1) == 'X';
                
                if (num == 0) {
                    temp[temp_pos++] = '0';
                } else {
                    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
                    while (num > 0) {
                        temp[temp_pos++] = digits[num % 16];
                        num /= 16;
                    }
                }
                
                while (temp_pos > 0 && pos < size - 1) {
                    buffer[pos++] = temp[--temp_pos];
                }
                break;
            }
            case 'p': {
                void* ptr = va_arg(args, void*);
                uintptr_t addr = (uintptr_t)ptr;
                
                // Add "0x" prefix
                if (pos < size - 2) {
                    buffer[pos++] = '0';
                    buffer[pos++] = 'x';
                }
                
                char temp[32];
                int temp_pos = 0;
                
                if (addr == 0) {
                    temp[temp_pos++] = '0';
                } else {
                    while (addr > 0) {
                        temp[temp_pos++] = "0123456789abcdef"[addr % 16];
                        addr /= 16;
                    }
                }
                
                while (temp_pos > 0 && pos < size - 1) {
                    buffer[pos++] = temp[--temp_pos];
                }
                break;
            }
            case '%':
                if (pos < size - 1) buffer[pos++] = '%';
                break;
            default:
                // Unknown specifier, just copy it
                if (pos < size - 1) buffer[pos++] = *(fmt - 1);
                break;
        }
    }
    
    buffer[pos] = '\0';
    return pos;
}

int snprintf(char* buffer, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}