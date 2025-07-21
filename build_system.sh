#!/bin/bash

# RaeenOS Build System
# This script builds the complete RaeenOS operating system

set -e

# Configuration
KERNEL_NAME="raeenos-kernel"
BOOTLOADER_NAME="raeenos-bootloader"
ISO_NAME="raeenos.iso"
BUILD_DIR="build"
TOOLS_DIR="tools"
GRUB_DIR="grub"
INITRD_DIR="initrd"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check dependencies
check_dependencies() {
    print_status "Checking build dependencies..."
    
    local missing_deps=()
    
    # Check for required tools
    command -v gcc >/dev/null 2>&1 || missing_deps+=("gcc")
    command -v ld >/dev/null 2>&1 || missing_deps+=("ld")
    command -v nasm >/dev/null 2>&1 || missing_deps+=("nasm")
    command -v grub-mkrescue >/dev/null 2>&1 || missing_deps+=("grub-mkrescue")
    command -v xorriso >/dev/null 2>&1 || missing_deps+=("xorriso")
    command -v mtools >/dev/null 2>&1 || missing_deps+=("mtools")
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_error "Please install the missing packages and try again."
        exit 1
    fi
    
    print_success "All dependencies found"
}

# Create build directories
create_directories() {
    print_status "Creating build directories..."
    
    mkdir -p $BUILD_DIR
    mkdir -p $BUILD_DIR/boot
    mkdir -p $BUILD_DIR/boot/grub
    mkdir -p $BUILD_DIR/initrd
    mkdir -p $BUILD_DIR/apps
    mkdir -p $BUILD_DIR/lib
    mkdir -p $BUILD_DIR/bin
    mkdir -p $BUILD_DIR/sbin
    mkdir -p $BUILD_DIR/etc
    mkdir -p $BUILD_DIR/usr
    mkdir -p $BUILD_DIR/usr/bin
    mkdir -p $BUILD_DIR/usr/sbin
    mkdir -p $BUILD_DIR/usr/lib
    mkdir -p $BUILD_DIR/usr/share
    mkdir -p $BUILD_DIR/var
    mkdir -p $BUILD_DIR/tmp
    mkdir -p $BUILD_DIR/proc
    mkdir -p $BUILD_DIR/sys
    mkdir -p $BUILD_DIR/dev
    mkdir -p $BUILD_DIR/home
    
    print_success "Build directories created"
}

# Build bootloader
build_bootloader() {
    print_status "Building bootloader..."
    
    cd bootloader
    
    # Build stage1
    print_status "Building bootloader stage1..."
    nasm -f bin stage1/stage1.asm -o ../$BUILD_DIR/stage1.bin
    
    # Build stage2
    print_status "Building bootloader stage2..."
    nasm -f bin stage2/stage2.asm -o ../$BUILD_DIR/stage2.bin
    
    # Build multiboot header
    print_status "Building multiboot header..."
    gcc -m32 -c multiboot.c -o ../$BUILD_DIR/multiboot.o
    ld -m elf_i386 -T multiboot.ld ../$BUILD_DIR/multiboot.o -o ../$BUILD_DIR/multiboot.bin
    objcopy -O binary ../$BUILD_DIR/multiboot.bin ../$BUILD_DIR/multiboot.bin
    
    cd ..
    
    print_success "Bootloader built successfully"
}

# Build kernel
build_kernel() {
    print_status "Building kernel..."
    
    cd kernel
    
    # Set compiler flags
    CFLAGS="-std=c11 -Wall -Wextra -nostdlib -fno-builtin -fno-stack-protector -ffreestanding -fno-pie -fno-pic -mno-red-zone -mcmodel=kernel -m64 -DTARGET_X64"
    INCLUDES="-I. -I./core/include -I./hal/include -I./security/include -I./memory/include -I./process/include"
    
    # Create kernel object files
    local kernel_objects=()
    
    # Core kernel files
    kernel_objects+=("core/kernel_main.o")
    kernel_objects+=("core/string.o")
    kernel_objects+=("core/interrupts.o")
    kernel_objects+=("core/exceptions.o")
    kernel_objects+=("core/syscall.o")
    kernel_objects+=("core/performance.o")
    
    # Memory management
    kernel_objects+=("memory/memory_core.o")
    kernel_objects+=("memory/physical_memory.o")
    kernel_objects+=("memory/virtual_memory.o")
    kernel_objects+=("memory/heap.o")
    kernel_objects+=("memory/demand_paging.o")
    kernel_objects+=("memory/vm_advanced.o")
    
    # HAL
    kernel_objects+=("hal/common/hal_common.o")
    kernel_objects+=("hal/x64/hal_x64.o")
    kernel_objects+=("hal/hal.o")
    
    # Process management
    kernel_objects+=("process/process.o")
    kernel_objects+=("process/scheduler.o")
    kernel_objects+=("process/elf_loader.o")
    kernel_objects+=("process/executable_loader.o")
    kernel_objects+=("process/threading.o")
    
    # File systems
    kernel_objects+=("filesystem/vfs.o")
    kernel_objects+=("filesystem/ramfs.o")
    kernel_objects+=("filesystem/ext4.o")
    kernel_objects+=("filesystem/initrd.o")
    
    # Device drivers
    kernel_objects+=("drivers/keyboard.o")
    kernel_objects+=("drivers/vga.o")
    kernel_objects+=("drivers/usb.o")
    kernel_objects+=("drivers/network.o")
    kernel_objects+=("drivers/pci.o")
    kernel_objects+=("drivers/acpi.o")
    kernel_objects+=("drivers/ata.o")
    kernel_objects+=("drivers/device_manager.o")
    
    # Security
    kernel_objects+=("security/security_core.o")
    kernel_objects+=("security/access_control.o")
    kernel_objects+=("security/advanced_security.o")
    kernel_objects+=("security/enterprise_security.o")
    
    # Networking
    kernel_objects+=("network/network.o")
    kernel_objects+=("network/advanced_networking.o")
    kernel_objects+=("network/tcpip.o")
    
    # Graphics and GUI
    kernel_objects+=("graphics/gui.o")
    kernel_objects+=("graphics/window_manager.o")
    
    # System services
    kernel_objects+=("init/init_system.o")
    kernel_objects+=("shell/shell.o")
    
    # Applications
    kernel_objects+=("apps/app_framework.o")
    kernel_objects+=("apps/runtime.o")
    
    # World-class features
    kernel_objects+=("gaming/gaming_system.o")
    kernel_objects+=("creative/creative_suite.o")
    kernel_objects+=("enterprise/enterprise_features.o")
    kernel_objects+=("ux/modern_ux.o")
    kernel_objects+=("performance/intelligent_performance.o")
    kernel_objects+=("compatibility/professional_compatibility.o")
    kernel_objects+=("reliability/system_reliability.o")
    kernel_objects+=("ai/ai_system.o")
    kernel_objects+=("quantum/quantum_computing.o")
    kernel_objects+=("xr/extended_reality.o")
    kernel_objects+=("blockchain/blockchain_system.o")
    kernel_objects+=("clustering/high_availability.o")
    kernel_objects+=("virtualization/advanced_virtualization.o")
    
    # Architecture-specific
    kernel_objects+=("arch/x86_64/exception_handler.o")
    kernel_objects+=("arch/x86_64/gdt.o")
    kernel_objects+=("arch/x86_64/idt.o")
    kernel_objects+=("arch/x86_64/isr_stubs.o")
    kernel_objects+=("arch/x86_64/syscall_entry.o")
    kernel_objects+=("arch/x86_64/syscall.o")
    kernel_objects+=("arch/x86_64/tss.o")
    kernel_objects+=("arch/x86_64/user_mode.o")
    
    # Compile all object files
    for obj in "${kernel_objects[@]}"; do
        local src="${obj%.o}.c"
        if [ -f "$src" ]; then
            print_status "Compiling $src..."
            gcc $CFLAGS $INCLUDES -c "$src" -o "../$BUILD_DIR/$obj"
        fi
    done
    
    # Compile assembly files
    if [ -f "core/interrupt_stubs.asm" ]; then
        print_status "Compiling interrupt stubs..."
        nasm -f elf64 core/interrupt_stubs.asm -o ../$BUILD_DIR/core/interrupt_stubs.o
        kernel_objects+=("core/interrupt_stubs.o")
    fi
    
    if [ -f "core/context_switch.S" ]; then
        print_status "Compiling context switch..."
        gcc -m64 -c core/context_switch.S -o ../$BUILD_DIR/core/context_switch.o
        kernel_objects+=("core/context_switch.o")
    fi
    
    # Link kernel
    print_status "Linking kernel..."
    local link_objects=""
    for obj in "${kernel_objects[@]}"; do
        if [ -f "../$BUILD_DIR/$obj" ]; then
            link_objects="$link_objects ../$BUILD_DIR/$obj"
        fi
    done
    
    ld -T kernel.ld -o ../$BUILD_DIR/$KERNEL_NAME.bin $link_objects
    
    cd ..
    
    print_success "Kernel built successfully"
}

# Build initrd
build_initrd() {
    print_status "Building initial RAM disk..."
    
    cd $BUILD_DIR/initrd
    
    # Create basic file structure
    mkdir -p bin sbin etc lib usr/bin usr/sbin usr/lib
    
    # Create basic system files
    echo "RaeenOS v1.0.0" > version
    echo "raeenos" > hostname
    
    # Create basic configuration files
    cat > fstab << EOF
# /etc/fstab
proc    /proc   procfs  defaults    0 0
sys     /sys    sysfs   defaults    0 0
dev     /dev    devfs   defaults    0 0
tmp     /tmp    tmpfs   defaults    0 0
EOF
    
    # Create basic passwd file
    cat > passwd << EOF
root:x:0:0:root:/root:/bin/sh
user:x:1000:1000:User:/home/user:/bin/sh
EOF
    
    # Create basic group file
    cat > group << EOF
root:x:0:
user:x:1000:
EOF
    
    # Create basic services file
    cat > services << EOF
# Network services
ssh     22/tcp
http    80/tcp
https   443/tcp
ftp     21/tcp
EOF
    
    cd ../..
    
    print_success "Initial RAM disk built successfully"
}

# Build applications
build_applications() {
    print_status "Building applications..."
    
    cd apps
    
    # Build basic applications
    local apps=("shell" "ls" "cat" "echo" "ps" "kill" "clear" "help")
    
    for app in "${apps[@]}"; do
        if [ -f "$app.c" ]; then
            print_status "Building $app..."
            gcc -std=c11 -Wall -Wextra -nostdlib -fno-builtin -ffreestanding -m64 \
                -I../kernel/core/include -I../kernel/libc \
                -c "$app.c" -o "../$BUILD_DIR/apps/$app.o"
            
            ld -T ../tools/app.ld -o "../$BUILD_DIR/bin/$app" "../$BUILD_DIR/apps/$app.o"
        fi
    done
    
    cd ..
    
    print_success "Applications built successfully"
}

# Build libc
build_libc() {
    print_status "Building C library..."
    
    cd libc
    
    # Compile libc functions
    local libc_objects=()
    libc_objects+=("stdio.o")
    libc_objects+=("stdlib.o")
    libc_objects+=("string.o")
    libc_objects+=("ctype.o")
    libc_objects+=("math.o")
    libc_objects+=("time.o")
    libc_objects+=("signal.o")
    libc_objects+=("unistd.o")
    libc_objects+=("fcntl.o")
    libc_objects+=("syscall.o")
    
    for obj in "${libc_objects[@]}"; do
        local src="${obj%.o}.c"
        if [ -f "$src" ]; then
            print_status "Compiling $src..."
            gcc -std=c11 -Wall -Wextra -nostdlib -fno-builtin -ffreestanding -m64 \
                -I. -I../kernel/core/include \
                -c "$src" -o "../$BUILD_DIR/lib/$obj"
        fi
    done
    
    # Create libc archive
    cd ../$BUILD_DIR/lib
    ar rcs libc.a *.o
    
    cd ../..
    
    print_success "C library built successfully"
}

# Create bootable ISO
create_iso() {
    print_status "Creating bootable ISO..."
    
    # Copy kernel to boot directory
    cp $BUILD_DIR/$KERNEL_NAME.bin $BUILD_DIR/boot/
    
    # Copy GRUB configuration
    cp bootloader/grub.cfg $BUILD_DIR/boot/grub/
    
    # Create initrd
    cd $BUILD_DIR/initrd
    tar -czf ../boot/initrd.img *
    cd ../..
    
    # Create ISO
    grub-mkrescue -o $ISO_NAME $BUILD_DIR
    
    print_success "Bootable ISO created: $ISO_NAME"
}

# Clean build
clean() {
    print_status "Cleaning build directory..."
    rm -rf $BUILD_DIR
    rm -f $ISO_NAME
    print_success "Build cleaned"
}

# Main build function
main() {
    print_status "Starting RaeenOS build process..."
    
    # Check dependencies
    check_dependencies
    
    # Clean previous build
    if [ "$1" != "clean" ]; then
        clean
    fi
    
    if [ "$1" = "clean" ]; then
        exit 0
    fi
    
    # Create directories
    create_directories
    
    # Build components
    build_bootloader
    build_kernel
    build_libc
    build_applications
    build_initrd
    
    # Create ISO
    create_iso
    
    print_success "RaeenOS build completed successfully!"
    print_success "You can now boot from: $ISO_NAME"
}

# Run main function
main "$@" 