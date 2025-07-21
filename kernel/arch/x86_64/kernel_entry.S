; RaeenOS Kernel Entry Point
; This is the entry point called by the bootloader

global _start
extern kernel_main
extern _bss_start
extern _bss_end

section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002                    ; Multiboot magic number
    dd 0x00000003                    ; Flags (memory info + boot device)
    dd -(0x1BADB002 + 0x00000003)    ; Checksum

section .text
_start:
    ; Disable interrupts
    cli
    
    ; Set up stack
    mov rsp, 0x90000
    
    ; Clear BSS section
    mov rdi, _bss_start
    mov rcx, _bss_end
    sub rcx, rdi
    xor rax, rax
    rep stosb
    
    ; Set up registers for kernel_main
    mov rdi, rbx                     ; Multiboot info structure
    mov rsi, rax                     ; Multiboot magic number
    
    ; Call kernel main
    call kernel_main
    
    ; Should never return, but just in case
    cli
    hlt 