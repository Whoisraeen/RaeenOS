#include "include/string.h"
#include "include/types.h"

// Simple string implementation for kernel compilation

// Basic string functions
size_t strlen(const char* str) {
    if (!str) return 0;
    
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    if (!dest || !src) return dest;
    
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    if (!dest || !src) return dest;
    
    char* d = dest;
    while (n-- && (*d++ = *src++));
    while (n--) *d++ = '\0';
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    if (!s1 || !s2) return 0;
    
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
    return n == SIZE_MAX ? 0 : *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strcat(char* dest, const char* src) {
    if (!dest || !src) return dest;
    
    char* d = dest;
    while (*d) d++;  // Find end of dest
    while ((*d++ = *src++));
    return dest;
}

char* strchr(const char* str, int c) {
    if (!str) return NULL;
    
    while (*str) {
        if (*str == c) return (char*)str;
        str++;
    }
    return c == '\0' ? (char*)str : NULL;
}

// Memory functions
void* memset(void* ptr, int value, size_t size) {
    if (!ptr) return ptr;
    
    unsigned char* p = (unsigned char*)ptr;
    while (size--) *p++ = (unsigned char)value;
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t size) {
    if (!dest || !src) return dest;
    
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (size--) *d++ = *s++;
    return dest;
}

void* memmove(void* dest, const void* src, size_t size) {
    if (!dest || !src || dest == src) return dest;
    
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        // Copy forward
        while (size--) *d++ = *s++;
    } else {
        // Copy backward
        d += size;
        s += size;
        while (size--) *--d = *--s;
    }
    return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t size) {
    if (!ptr1 || !ptr2) return 0;
    
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    while (size--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

// Simple sprintf implementation (very basic)
static void reverse_string(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static int int_to_string(int num, char* str, int base) {
    int i = 0;
    bool is_negative = false;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }
    
    if (num < 0 && base == 10) {
        is_negative = true;
        num = -num;
    }
    
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

int sprintf(char* buffer, const char* format, ...) {
    if (!buffer || !format) return 0;
    
    const char* p = format;
    char* buf = buffer;
    int written = 0;
    
    // Very simple implementation - just handle %s, %d, %x
    while (*p) {
        if (*p != '%') {
            *buf++ = *p++;
            written++;
        } else {
            p++; // Skip %
            if (*p == 's') {
                // String
                const char* str = "NULL"; // Default
                // In a real implementation, we'd use va_list here
                while (*str) {
                    *buf++ = *str++;
                    written++;
                }
            } else if (*p == 'd') {
                // Integer - simplified
                *buf++ = '0';
                written++;
            } else if (*p == 'x') {
                // Hex - simplified
                *buf++ = '0';
                written++;
            } else {
                *buf++ = *p;
                written++;
            }
            p++;
        }
    }
    
    *buf = '\0';
    return written;
}

int snprintf(char* buffer, size_t size, const char* format, ...) {
    if (!buffer || !format || size == 0) return 0;
    
    // Very simplified - just copy format for now
    size_t len = strlen(format);
    if (len >= size) len = size - 1;
    
    memcpy(buffer, format, len);
    buffer[len] = '\0';
    
    return len;
}