; RaeenOS Kernel Entry Point
; This file provides the multiboot-compliant kernel entry point

[bits 32]
[extern kernel_main]
[extern multiboot_info_ptr]
[extern multiboot_magic_ptr]

; Multiboot header
section .multiboot
align 4
    dd 0x1BADB002           ; Multiboot magic number
    dd 0x00000003           ; Flags (memory info + boot device)
    dd -(0x1BADB002 + 0x00000003)  ; Checksum

; Kernel entry point
section .text
global _start
_start:
    ; Disable interrupts
    cli
    
    ; Save multiboot information
    mov [multiboot_magic_ptr], eax
    mov [multiboot_info_ptr], ebx
    
    ; Set up stack
    mov esp, kernel_stack_top
    
    ; Clear EFLAGS
    push 0
    popf
    
    ; Enable SSE (required for modern C code)
    mov eax, cr0
    and ax, 0xFFFB          ; Clear coprocessor emulation
    or ax, 0x0002           ; Set coprocessor monitoring
    mov cr0, eax
    
    mov eax, cr4
    or ax, 0x0600           ; Set OSFXSR and OSXMMEXCPT
    mov cr4, eax
    
    ; Call kernel main function
    call kernel_main
    
    ; If kernel_main returns, halt
    cli
    hlt
    jmp $

; Kernel stack
section .bss
align 16
kernel_stack_bottom:
    resb 16384              ; 16 KB stack
kernel_stack_top:

; Storage for multiboot information
section .data
global multiboot_magic_ptr
global multiboot_info_ptr
multiboot_magic_ptr: dd 0
multiboot_info_ptr: dd 0