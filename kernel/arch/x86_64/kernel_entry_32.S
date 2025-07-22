; RaeenOS Kernel Entry Point - 32-bit Multiboot Compatible
; This is the entry point called by the bootloader

global _start
extern kernel_main
extern _bss_start
extern _bss_end

section .text
bits 32
_start:
    ; Disable interrupts
    cli
    
    ; Set up stack (32-bit)
    mov esp, 0x90000
    
    ; Clear BSS section (32-bit)
    mov edi, _bss_start
    mov ecx, _bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb
    
    ; Set up arguments for kernel_main (32-bit calling convention)
    ; EAX contains multiboot magic, EBX contains multiboot info pointer
    push eax                    ; Second argument: multiboot magic number
    push ebx                    ; First argument: multiboot info structure
    
    ; Verify multiboot magic number
    cmp eax, 0x2BADB002         ; Expected multiboot magic from bootloader
    jne halt_system             ; Halt if wrong magic number
    
    ; Call kernel main
    call kernel_main
    
    ; Clean up stack
    add esp, 8
    
    ; Should never return, but just in case
halt_system:
    cli
    hlt
    jmp halt_system