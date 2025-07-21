; RaeenOS Stage 1 Bootloader
; Copyright (c) 2025 RaeenOS Project
; Multi-stage bootloader compatible with BIOS and UEFI

[org 0x7c00]
[bits 16]

; Bootloader entry point
_start:
    ; Initialize segment registers
    cli                     ; Disable interrupts during setup
    mov ax, 0x0000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00         ; Set up stack
    sti                     ; Re-enable interrupts

    ; Save boot drive number
    mov [boot_drive], dl

    ; Display boot message
    mov si, boot_msg
    call print_string

    ; Check for extended BIOS functions
    call check_extensions

    ; Load Stage 2 bootloader
    call load_stage2

    ; Jump to Stage 2
    jmp 0x0000:0x7e00

; Check for extended BIOS functions
check_extensions:
    mov ah, 0x41
    mov bx, 0x55aa
    int 0x13
    jc .no_extensions
    
    cmp bx, 0xaa55
    jne .no_extensions
    
    test cx, 0x01          ; Check if extended disk access is supported
    jz .no_extensions
    
    mov byte [extended_bios], 1
.no_extensions:
    ret

; Load Stage 2 bootloader from disk
load_stage2:
    mov si, loading_msg
    call print_string

    ; Read sectors from disk
    mov ah, 0x02           ; BIOS read sectors function
    mov al, 15             ; Number of sectors to read (Stage 2 is 15 sectors)
    mov ch, 0              ; Cylinder 0
    mov cl, 2              ; Sector 2 (Stage 2 starts at sector 2)
    mov dh, 0              ; Head 0
    mov dl, [boot_drive]   ; Drive number
    mov bx, 0x7e00         ; Load to address 0x7e00
    int 0x13
    
    jc disk_error          ; Jump if error (carry flag set)
    
    ; Verify Stage 2 signature
    mov ax, [0x7e00]
    cmp ax, 0x4F52         ; "RO" signature for RaeenOS
    jne signature_error
    
    mov si, success_msg
    call print_string
    ret

; Print string function
print_string:
    lodsb                   ; Load next character
    or al, al              ; Check if character is null
    jz .done
    mov ah, 0x0e           ; BIOS teletype function
    int 0x10
    jmp print_string
.done:
    ret

; Error handlers
disk_error:
    mov si, disk_error_msg
    call print_string
    jmp halt_system

signature_error:
    mov si, signature_error_msg
    call print_string
    jmp halt_system

halt_system:
    mov si, halt_msg
    call print_string
    cli
    hlt
    jmp halt_system

; Data section
boot_msg:          db 'RaeenOS Bootloader v1.0', 0x0d, 0x0a, 0
loading_msg:       db 'Loading Stage 2...', 0x0d, 0x0a, 0
success_msg:       db 'Stage 2 loaded successfully', 0x0d, 0x0a, 0
disk_error_msg:    db 'Disk read error!', 0x0d, 0x0a, 0
signature_error_msg: db 'Invalid Stage 2 signature!', 0x0d, 0x0a, 0
halt_msg:          db 'System halted.', 0x0d, 0x0a, 0

; Variables
boot_drive:        db 0
extended_bios:     db 0

; Boot signature
times 510-($-$$) db 0
dw 0xaa55 