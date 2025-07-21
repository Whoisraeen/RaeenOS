/* RaeenOS Kernel Linker Script */

OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH("i386:x86-64")
ENTRY(_start)

/* Define the program headers we want so the bootloader gives us the right */
/* MMU permissions */
PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)); /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2));            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)); /* Write + Read */
}

SECTIONS
{
    /* Kernel starts at 1MB physical, mapped at -2GB virtual */
    . = 0xFFFFFFFF80100000;
    
    /* Save the start and end of kernel for memory management */
    _kernel_start = .;
    
    /* Text section */
    .text ALIGN(4K) : AT(ADDR(.text) - 0xFFFFFFFF80000000)
    {
        *(.multiboot)
        *(.text)
        *(.text.*)
    } :text
    
    /* Read-only data */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xFFFFFFFF80000000)
    {
        *(.rodata)
        *(.rodata.*)
    } :rodata
    
    /* Read-write data */
    .data ALIGN(4K) : AT(ADDR(.data) - 0xFFFFFFFF80000000)
    {
        *(.data)
        *(.data.*)
    } :data
    
    /* BSS section (uninitialized data) */
    .bss ALIGN(4K) : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
    {
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        _bss_end = .;
    } :data
    
    /* Mark the end of the kernel */
    _kernel_end = .;
    
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