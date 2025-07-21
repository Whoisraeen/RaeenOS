#ifndef COCOA_API_H
#define COCOA_API_H

#include "types.h"
#include <stddef.h>

// macOS types
typedef void* id;
typedef void* SEL;
typedef void* Class;
typedef u32 NSUInteger;
typedef s32 NSInteger;
typedef u8 BOOL;
typedef double CGFloat;
typedef struct {
    CGFloat x, y;
} NSPoint;
typedef struct {
    CGFloat width, height;
} NSSize;
typedef struct {
    NSPoint origin;
    NSSize size;
} NSRect;

// macOS constants
#define YES 1
#define NO 0
#define nil NULL

// Objective-C runtime types
typedef struct {
    Class isa;
} NSObject;

typedef struct {
    Class isa;
    id window;
    id view;
    id delegate;
} NSApplication;

typedef struct {
    Class isa;
    NSRect frame;
    id title;
    id delegate;
} NSWindow;

typedef struct {
    Class isa;
    NSRect bounds;
    id backgroundColor;
} NSView;

// Cocoa API function types
typedef id (*objc_msgSend_t)(id self, SEL op, ...);
typedef id (*objc_getClass_t)(const char* name);
typedef SEL (*sel_registerName_t)(const char* str);
typedef id (*alloc_t)(id self, SEL op);
typedef id (*init_t)(id self, SEL op);
typedef void (*release_t)(id self, SEL op);
typedef void (*autorelease_t)(id self, SEL op);

// Cocoa API function pointers
extern objc_msgSend_t objc_msgSend;
extern objc_getClass_t objc_getClass;
extern sel_registerName_t sel_registerName;
extern alloc_t alloc;
extern init_t init;
extern release_t release;
extern autorelease_t autorelease;

// Cocoa API initialization
error_t cocoa_api_init(void);
void cocoa_api_shutdown(void);

// Cocoa API function implementations
id cocoa_objc_msgSend(id self, SEL op, ...);
id cocoa_objc_getClass(const char* name);
SEL cocoa_sel_registerName(const char* str);
id cocoa_alloc(id self, SEL op);
id cocoa_init(id self, SEL op);
void cocoa_release(id self, SEL op);
void cocoa_autorelease(id self, SEL op);

// NSApplication functions
id cocoa_NSApplication_sharedApplication(void);
void cocoa_NSApplication_run(id app);
void cocoa_NSApplication_terminate(id app, id sender);

// NSWindow functions
id cocoa_NSWindow_alloc(void);
id cocoa_NSWindow_initWithContentRect(id window, NSRect contentRect, 
                                    u32 styleMask, u32 backing, BOOL defer);
void cocoa_NSWindow_setTitle(id window, id title);
void cocoa_NSWindow_makeKeyAndOrderFront(id window, id sender);
void cocoa_NSWindow_close(id window);

// NSView functions
id cocoa_NSView_alloc(void);
id cocoa_NSView_initWithFrame(id view, NSRect frameRect);
void cocoa_NSView_setBackgroundColor(id view, id color);
void cocoa_NSView_drawRect(id view, NSRect dirtyRect);

// NSColor functions
id cocoa_NSColor_redColor(void);
id cocoa_NSColor_greenColor(void);
id cocoa_NSColor_blueColor(void);
id cocoa_NSColor_whiteColor(void);
id cocoa_NSColor_blackColor(void);

// NSString functions
id cocoa_NSString_stringWithUTF8String(const char* nullTerminatedCString);
const char* cocoa_NSString_UTF8String(id string);

// NSRect functions
NSRect cocoa_NSMakeRect(CGFloat x, CGFloat y, CGFloat w, CGFloat h);
NSPoint cocoa_NSMakePoint(CGFloat x, CGFloat y);
NSSize cocoa_NSMakeSize(CGFloat w, CGFloat h);

#endif // COCOA_API_H 