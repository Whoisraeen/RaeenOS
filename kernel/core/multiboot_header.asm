; RaeenOS Multiboot Header
; This header allows GRUB to load our kernel

section .multiboot
align 4

; Multiboot header constants
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000003  ; Page align + memory info
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Multiboot header
dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM

; Optional header fields
dd 0x00000000  ; header_addr
dd 0x00000000  ; load_addr
dd 0x00000000  ; load_end_addr
dd 0x00000000  ; bss_end_addr
dd 0x00000000  ; entry_addr

; Video mode
dd 0x00000000  ; mode_type
dd 1024        ; width
dd 768         ; height
dd 32          ; depth 