; RaeenOS Multiboot Header and Boot Entry Point

MBOOT_PAGE_ALIGN    equ 1<<0    ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1    ; Provide memory map
MBOOT_HEADER_MAGIC  equ 0x1BADB002 ; Multiboot Magic value
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC   ; Magic number
    dd MBOOT_HEADER_FLAGS   ; Flags
    dd MBOOT_CHECKSUM       ; Checksum

section .text
align 4

global _start
extern kernel_main

_start:
    mov esp, kernel_stack_top   ; Set up stack pointer
    
    push eax                    ; Push Multiboot magic number
    push ebx                    ; Push Multiboot info structure
    
    call kernel_main            ; Call our kernel main function
    
    ; If kernel_main returns, halt
.hang:
    cli                         ; Clear interrupt flag
    hlt                         ; Halt processor
    jmp .hang                   ; Jump here forever

section .bss
align 4
kernel_stack_bottom:
    resb 16384                  ; 16 KiB stack
kernel_stack_top: