; RaeenOS Bootloader Stage 2
; This stage loads the kernel from disk and sets up the environment

[org 0x8000]
[bits 16]

; Constants
KERNEL_LOAD_ADDRESS equ 0x100000    ; 1MB - standard kernel load address
KERNEL_SECTOR_START equ 2048        ; Kernel starts at sector 2048 (1MB into disk)
MAX_KERNEL_SECTORS equ 512          ; Maximum 256KB kernel size

; Stage 2 entry point
stage2_start:
    ; Set up segments
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x7C00                  ; Stack grows downward from 0x7C00

    ; Save boot drive number
    mov [boot_drive], dl

    ; Print welcome message
    mov si, welcome_msg
    call print_string

    ; Check if we're in a supported video mode
    call check_video_mode

    ; Get memory map
    call get_memory_map

    ; Load kernel from disk
    call load_kernel

    ; Check if kernel was loaded successfully
    cmp word [kernel_load_status], 0
    jne kernel_load_error

    ; Set up multiboot information structure
    call setup_multiboot_info

    ; Switch to protected mode
    call switch_to_protected_mode

    ; Jump to kernel entry in protected mode
    jmp 0x08:protected_mode_kernel_entry

; Kernel load error handler
kernel_load_error:
    mov si, kernel_load_error_msg
    call print_string
    ; Halt the system
    cli
    hlt

; Print string function
print_string:
    pusha
    mov ah, 0x0E                    ; BIOS teletype function
.loop:
    lodsb                           ; Load next character
    test al, al                     ; Check if null terminator
    jz .done
    int 0x10                        ; BIOS interrupt
    jmp .loop
.done:
    popa
    ret

; Check video mode
check_video_mode:
    pusha
    
    ; Get current video mode
    mov ah, 0x0F
    int 0x10
    mov [video_mode], al
    
    ; Check if we have VESA support
    mov ax, 0x4F00
    mov di, vesa_info
    int 0x10
    
    cmp ax, 0x004F
    jne .no_vesa
    
    ; VESA is available
    mov byte [vesa_available], 1
    
    ; Try to set 1024x768x32 mode
    mov ax, 0x4F02
    mov bx, 0x118                    ; Mode 0x118 = 1024x768x32
    int 0x10
    
    cmp ax, 0x004F
    je .vesa_success
    
.no_vesa:
    ; Fall back to standard VGA
    mov ax, 0x0013                   ; 320x200x256 mode
    int 0x10
    
.vesa_success:
    popa
    ret

; Get memory map using INT 15h, AX=E820h
get_memory_map:
    pusha
    
    mov di, memory_map              ; Destination buffer
    mov ebx, 0                      ; Continuation value
    mov edx, 0x534D4150             ; 'SMAP' signature
    
.loop:
    mov eax, 0xE820                 ; Function number
    mov ecx, 24                     ; Buffer size
    int 0x15                        ; BIOS interrupt
    
    jc .done                        ; Carry flag set on error
    
    cmp eax, 0x534D4150             ; Check signature
    jne .done
    
    add di, 24                      ; Move to next entry
    inc word [memory_map_entries]   ; Count entries
    
    test ebx, ebx                   ; Check if more entries
    jnz .loop
    
.done:
    popa
    ret

; Load kernel from disk
load_kernel:
    pusha
    
    mov si, loading_kernel_msg
    call print_string
    
    ; Reset disk system
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc .disk_error
    
    ; Load kernel sectors
    mov ax, KERNEL_SECTOR_START     ; Starting sector
    mov bx, KERNEL_LOAD_ADDRESS     ; Load address
    mov cx, MAX_KERNEL_SECTORS      ; Number of sectors to load
    
.load_loop:
    push ax
    push bx
    push cx
    
    ; Convert LBA to CHS
    call lba_to_chs
    
    ; Read sector
    mov ah, 0x02                    ; Read sectors function
    mov al, 1                       ; Read 1 sector
    mov ch, [cylinder]              ; Cylinder
    mov cl, [sector]                ; Sector
    mov dh, [head]                  ; Head
    mov dl, [boot_drive]            ; Drive
    int 0x13
    
    jc .read_error
    
    ; Move to next sector
    pop cx
    pop bx
    pop ax
    
    add bx, 512                     ; Next memory address
    inc ax                          ; Next sector
    
    dec cx                          ; Decrement counter
    jnz .load_loop
    
    ; Kernel loaded successfully
    mov word [kernel_load_status], 0
    mov si, kernel_loaded_msg
    call print_string
    
    popa
    ret

.read_error:
    pop cx
    pop bx
    pop ax
    mov word [kernel_load_status], 1
    mov si, read_error_msg
    call print_string
    popa
    ret

.disk_error:
    mov word [kernel_load_status], 2
    mov si, disk_error_msg
    call print_string
    popa
    ret

; Convert LBA to CHS addressing
lba_to_chs:
    pusha
    
    ; LBA is in AX
    ; Drive geometry: 16 heads, 63 sectors per track
    mov bx, 16 * 63                 ; Sectors per cylinder
    
    ; Calculate cylinder
    xor dx, dx
    div bx                          ; AX = cylinder, DX = sector within cylinder
    mov [cylinder], al
    
    ; Calculate head and sector
    mov ax, dx
    mov bl, 63                      ; Sectors per track
    div bl                          ; AL = head, AH = sector (0-based)
    mov [head], al
    inc ah                          ; Make sector 1-based
    mov [sector], ah
    
    popa
    ret

; Set up multiboot information structure
setup_multiboot_info:
    pusha
    
    ; Clear multiboot info structure
    mov di, multiboot_info
    mov cx, 256
    xor al, al
    rep stosb
    
    ; Set up basic multiboot info
    mov dword [multiboot_info + 0], 0x1BADB002     ; Magic number
    mov dword [multiboot_info + 4], 0x00000003     ; Flags (memory info + boot device)
    mov dword [multiboot_info + 8], 0x00000000     ; Checksum
    mov dword [multiboot_info + 12], 0x00000000    ; Header addr
    mov dword [multiboot_info + 16], 0x00000000    ; Load addr
    mov dword [multiboot_info + 20], 0x00000000    ; Load end addr
    mov dword [multiboot_info + 24], 0x00000000    ; BSS end addr
    mov dword [multiboot_info + 28], KERNEL_LOAD_ADDRESS  ; Entry addr
    
    ; Set boot device
    mov al, [boot_drive]
    mov [multiboot_info + 32], al
    
    ; Set command line (empty for now)
    mov dword [multiboot_info + 36], 0x00000000
    
    ; Set memory map
    mov dword [multiboot_info + 40], memory_map
    mov ax, [memory_map_entries]
    mov [multiboot_info + 44], ax
    
    ; Set video info
    mov al, [video_mode]
    mov [multiboot_info + 48], al
    
    popa
    ret

; Switch to protected mode
switch_to_protected_mode:
    pusha
    
    mov si, switching_mode_msg
    call print_string
    
    ; Disable interrupts
    cli
    
    ; Load GDT
    lgdt [gdt_descriptor]
    
    ; Enable A20 line
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Set PE bit in CR0
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Far jump to flush pipeline
    jmp 0x08:protected_mode_entry

; Protected mode entry point
[bits 32]
protected_mode_entry:
    ; Set up segment registers
    mov ax, 0x10                    ; Data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up stack
    mov esp, 0x90000                ; Stack at 0x90000
    
    ; Jump to kernel with multiboot protocol
    mov eax, 0x2BADB002             ; Multiboot magic number
    mov ebx, multiboot_info         ; Multiboot info structure
    mov ecx, KERNEL_LOAD_ADDRESS    ; Kernel entry point
    jmp ecx

; Protected mode kernel entry point
protected_mode_kernel_entry:
    ; Set up registers for multiboot protocol
    mov eax, 0x2BADB002             ; Multiboot magic number
    mov ebx, multiboot_info         ; Multiboot info structure
    
    ; Jump to kernel entry point
    jmp KERNEL_LOAD_ADDRESS
    
    ; Should never return, but just in case
    cli
    hlt

; Data section
welcome_msg db 'RaeenOS Bootloader Stage 2', 13, 10, 0
loading_kernel_msg db 'Loading kernel...', 13, 10, 0
kernel_loaded_msg db 'Kernel loaded successfully', 13, 10, 0
kernel_load_error_msg db 'Error: Failed to load kernel', 13, 10, 0
read_error_msg db 'Error: Failed to read kernel from disk', 13, 10, 0
disk_error_msg db 'Error: Disk system failure', 13, 10, 0
switching_mode_msg db 'Switching to protected mode...', 13, 10, 0

; Variables
boot_drive db 0
video_mode db 0
vesa_available db 0
kernel_load_status dw 0
memory_map_entries dw 0

; Disk geometry variables
cylinder db 0
head db 0
sector db 0

; GDT (Global Descriptor Table)
gdt:
    ; Null descriptor
    dq 0x0000000000000000
    
    ; Code descriptor (0x08)
    dw 0xFFFF                       ; Limit 0:15
    dw 0x0000                       ; Base 0:15
    db 0x00                         ; Base 16:23
    db 10011010b                    ; Access byte
    db 11001111b                    ; Flags + Limit 16:19
    db 0x00                         ; Base 24:31
    
    ; Data descriptor (0x10)
    dw 0xFFFF                       ; Limit 0:15
    dw 0x0000                       ; Base 0:15
    db 0x00                         ; Base 16:23
    db 10010010b                    ; Access byte
    db 11001111b                    ; Flags + Limit 16:19
    db 0x00                         ; Base 24:31

gdt_descriptor:
    dw gdt_descriptor - gdt - 1     ; GDT size
    dd gdt                          ; GDT address

; Memory map buffer (24 bytes per entry)
memory_map times 24 * 128 db 0      ; Space for 128 memory map entries

; Multiboot information structure
multiboot_info times 256 db 0       ; Multiboot info structure

; VESA information structure
vesa_info times 512 db 0            ; VESA information

; Pad to 8KB
times 8192 - ($ - $$) db 0 