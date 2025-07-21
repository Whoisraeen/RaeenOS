; RaeenOS Bootloader
; Multi-stage bootloader with x86/x64 support
; Stage 1: Boot sector (512 bytes)

[BITS 16]
[ORG 0x7C00]

; Multiboot header
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000003
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Memory layout
STACK_BASE          equ 0x9000
STAGE2_LOAD_ADDR    equ 0x8000
KERNEL_LOAD_ADDR    equ 0x100000

start:
    ; Clear interrupts and set up segments
    cli
    cld
    
    ; Set up stack
    mov ax, 0
    mov ss, ax
    mov sp, STACK_BASE
    
    ; Set up data segments
    mov ds, ax
    mov es, ax
    
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Print boot message
    mov si, boot_msg
    call print_string
    
    ; Reset disk system
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    ; Load stage 2 bootloader
    mov si, loading_msg
    call print_string
    
    ; Read stage 2 from disk (16 sectors = 8KB)
    mov ah, 0x02    ; Read sectors
    mov al, 16      ; Number of sectors to read
    mov ch, 0       ; Cylinder
    mov cl, 2       ; Sector (sector 1 is boot sector)
    mov dh, 0       ; Head
    mov dl, [boot_drive] ; Drive
    mov bx, STAGE2_LOAD_ADDR ; Buffer
    int 0x13
    jc disk_error
    
    ; Jump to stage 2
    jmp STAGE2_LOAD_ADDR

disk_error:
    mov si, disk_error_msg
    call print_string
    jmp hang

print_string:
    pusha
    mov ah, 0x0E    ; BIOS teletype function
.loop:
    lodsb
    or al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

hang:
    cli
    hlt
    jmp hang

; Data section
boot_msg        db 'RaeenOS Bootloader v0.1', 0x0D, 0x0A, 0
loading_msg     db 'Loading stage 2...', 0x0D, 0x0A, 0
disk_error_msg  db 'Disk read error!', 0x0D, 0x0A, 0
boot_drive      db 0

; Pad to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55

; Stage 2 bootloader starts here
stage2_start:
[BITS 16]

    ; Print stage 2 message
    mov si, stage2_msg
    call print_string
    
    ; Enable A20 line
    call enable_a20
    
    ; Check for x64 support
    call check_x64_support
    
    ; Load GDT
    lgdt [gdt_descriptor]
    
    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Jump to 32-bit code
    jmp 0x08:protected_mode_start

enable_a20:
    ; Fast A20 enable via keyboard controller
    mov al, 0xDD
    out 0x64, al
    ret

check_x64_support:
    ; Check if CPUID is supported
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21    ; Flip ID bit
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .no_x64
    
    ; Check for extended CPUID
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_x64
    
    ; Check for x64 support
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29   ; Long mode bit
    jz .no_x64
    
    mov si, x64_supported_msg
    call print_string
    ret

.no_x64:
    mov si, no_x64_msg
    call print_string
    jmp hang

[BITS 32]
protected_mode_start:
    ; Set up 32-bit segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Print protected mode message
    mov esi, protected_mode_msg
    call print_string_32
    
    ; Set up paging for long mode
    call setup_paging
    
    ; Load long mode GDT
    lgdt [gdt64_descriptor]
    
    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    ; Set long mode bit in EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ; Enable paging and enter long mode
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    
    ; Jump to 64-bit code
    jmp 0x08:long_mode_start

setup_paging:
    ; Set up identity paging for first 2MB
    ; PML4 at 0x1000, PDPT at 0x2000, PD at 0x3000
    
    ; Clear page tables
    mov edi, 0x1000
    mov ecx, 0x3000
    xor eax, eax
    rep stosd
    
    ; Set up PML4
    mov dword [0x1000], 0x2003  ; PML4[0] -> PDPT (present, writable)
    
    ; Set up PDPT
    mov dword [0x2000], 0x3003  ; PDPT[0] -> PD (present, writable)
    
    ; Set up PD (2MB pages)
    mov dword [0x3000], 0x83    ; PD[0] -> 0x0 (present, writable, 2MB page)
    
    ; Set CR3 to PML4
    mov eax, 0x1000
    mov cr3, eax
    
    ret

print_string_32:
    mov edi, 0xB8000    ; VGA text buffer
    mov ah, 0x07        ; White on black
.loop:
    lodsb
    or al, al
    jz .done
    stosw
    jmp .loop
.done:
    ret

[BITS 64]
long_mode_start:
    ; Set up 64-bit segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x90000
    
    ; Print long mode message
    mov rsi, long_mode_msg
    call print_string_64
    
    ; Load kernel
    call load_kernel
    
    ; Jump to kernel
    mov rax, KERNEL_LOAD_ADDR
    jmp rax

load_kernel:
    ; TODO: Implement kernel loading from disk/filesystem
    ; For now, just return
    ret

print_string_64:
    mov rdi, 0xB8000
    mov ah, 0x0F
.loop:
    lodsb
    or al, al
    jz .done
    stosw
    jmp .loop
.done:
    ret

; GDT for 32-bit mode
gdt_start:
    ; Null descriptor
    dq 0

    ; Code segment (32-bit)
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10011010b    ; Access: present, DPL=0, code, executable, readable
    db 11001111b    ; Flags: 4KB granularity, 32-bit
    db 0x00         ; Base high

    ; Data segment (32-bit)
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10010010b    ; Access: present, DPL=0, data, writable
    db 11001111b    ; Flags: 4KB granularity, 32-bit
    db 0x00         ; Base high

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; GDT for 64-bit mode
gdt64_start:
    ; Null descriptor
    dq 0

    ; Code segment (64-bit)
    dw 0xFFFF       ; Limit (ignored in 64-bit)
    dw 0x0000       ; Base (ignored in 64-bit)
    db 0x00         ; Base (ignored in 64-bit)
    db 10011010b    ; Access: present, DPL=0, code, executable, readable
    db 10101111b    ; Flags: long mode, 4KB granularity
    db 0x00         ; Base (ignored in 64-bit)

    ; Data segment (64-bit)
    dw 0xFFFF       ; Limit (ignored in 64-bit)
    dw 0x0000       ; Base (ignored in 64-bit)
    db 0x00         ; Base (ignored in 64-bit)
    db 10010010b    ; Access: present, DPL=0, data, writable
    db 10101111b    ; Flags: long mode, 4KB granularity
    db 0x00         ; Base (ignored in 64-bit)

gdt64_end:

gdt64_descriptor:
    dw gdt64_end - gdt64_start - 1
    dq gdt64_start

; Messages
stage2_msg          db 'Stage 2 bootloader loaded', 0x0D, 0x0A, 0
x64_supported_msg   db 'x64 support detected', 0x0D, 0x0A, 0
no_x64_msg          db 'x64 not supported!', 0x0D, 0x0A, 0
protected_mode_msg  db 'Entered 32-bit protected mode', 0
long_mode_msg       db 'Entered 64-bit long mode', 0

; Pad stage 2 to sector boundary
times 2048-($-stage2_start) db 0