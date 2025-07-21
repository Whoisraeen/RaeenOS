#ifndef STRING_H
#define STRING_H

#include "types.h"
#include <stdarg.h>

// String manipulation functions
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcat(char* dest, const char* src);
char* strchr(const char* str, int c);
char* strrchr(const char* str, int c);
char* strstr(const char* haystack, const char* needle);

// Memory manipulation functions
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t num);
void* memmove(void* dest, const void* src, size_t num);
int memcmp(const void* ptr1, const void* ptr2, size_t num);
void* memchr(const void* ptr, int value, size_t num);

// Formatted output functions
int vsnprintf(char* buffer, size_t size, const char* format, va_list args);
int snprintf(char* buffer, size_t size, const char* format, ...);
int sprintf(char* buffer, const char* format, ...);

// Additional utility functions
char* strtok(char* str, const char* delim);
char* strtok_r(char* str, const char* delim, char** saveptr);
long strtol(const char* str, char** endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);

#endif // STRING_H