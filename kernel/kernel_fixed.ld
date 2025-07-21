/* RaeenOS Kernel Linker Script - Fixed for Bootability */ 
 
OUTPUT_FORMAT("elf64-x86-64") 
OUTPUT_ARCH("i386:x86-64") 
ENTRY(_start) 
 
SECTIONS 
{ 
    /* Kernel loaded at 1MB physical address */ 
    . = 0x100000; 
 
    /* Multiboot header must be first */ 
    .multiboot ALIGN(4) : 
    { 
        *(.multiboot) 
    } 
 
    /* Text section */ 
    .text ALIGN(4K) : 
    { 
        *(.text) 
        *(.text.*) 
    } 
 
    /* Read-only data */ 
    .rodata ALIGN(4K) : 
    { 
        *(.rodata) 
        *(.rodata.*) 
    } 
 
    /* Data section */ 
    .data ALIGN(4K) : 
    { 
        *(.data) 
        *(.data.*) 
    } 
 
    /* BSS section */ 
    .bss ALIGN(4K) : 
    { 
        _bss_start = .; 
        *(.bss) 
        *(.bss.*) 
        *(COMMON) 
        _bss_end = .; 
    } 
 
    /* Discard debug sections */ 
    /DISCARD/ : 
    { 
        *(.comment) 
        *(.note) 
        *(.note.*) 
        *(.eh_frame) 
        *(.eh_frame_hdr) 
    } 
} 
