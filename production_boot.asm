; RaeenOS Production Kernel Multiboot Header and Boot Entry Point

MBOOT_PAGE_ALIGN    equ 1<<0    
MBOOT_MEM_INFO      equ 1<<1    
MBOOT_HEADER_MAGIC  equ 0x1BADB002 
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC   
    dd MBOOT_HEADER_FLAGS   
    dd MBOOT_CHECKSUM       

section .text
align 4

global _start
extern kernel_main

_start:
    mov esp, production_stack_top   ; Set up stack pointer
    
    push eax                        ; Push Multiboot magic number (0x2BADB002)  
    push ebx                        ; Push Multiboot info structure
    
    call kernel_main                ; Call the PRODUCTION kernel main function
    
.hang:
    cli                             
    hlt                             
    jmp .hang                       

section .bss
align 4
production_stack_bottom:
    resb 16384                      ; 16 KiB stack for production kernel
production_stack_top: