/* RaeenOS Application Linker Script */

OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH("i386:x86-64")
ENTRY(_start)

SECTIONS
{
    /* Application starts at 0x400000 */
    . = 0x400000;
    
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
    
    /* Read-write data */
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
    
    /* Stack */
    .stack ALIGN(4K) :
    {
        . = . + 0x10000; /* 64KB stack */
        _stack_top = .;
    }
    
    /* Discard unnecessary sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
} 