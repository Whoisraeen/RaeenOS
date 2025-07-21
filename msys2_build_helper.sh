#!/bin/bash
# RaeenOS MSYS2 Build Script
# This script provides the correct commands for building RaeenOS in MSYS2

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                RaeenOS MSYS2 Build Helper                   ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install missing packages
install_dependencies() {
    echo "[INFO] Installing required dependencies..."
    
    # Update package database
    echo "[INFO] Updating package database..."
    pacman -Sy --noconfirm
    
    # Install base development tools
    echo "[INFO] Installing base development tools..."
    pacman -S --noconfirm base-devel
    
    # Install GCC toolchain
    echo "[INFO] Installing GCC toolchain..."
    pacman -S --noconfirm mingw-w64-x86_64-gcc
    pacman -S --noconfirm mingw-w64-x86_64-binutils
    
    # Install NASM
    echo "[INFO] Installing NASM..."
    pacman -S --noconfirm nasm
    
    # Install GRUB and ISO tools
    echo "[INFO] Installing GRUB and ISO creation tools..."
    
    pacman -S --noconfirm xorriso
    pacman -S --noconfirm mtools
    
    # Install additional tools
    echo "[INFO] Installing additional tools..."
    pacman -S --noconfirm make
    pacman -S --noconfirm git
    
    echo "[SUCCESS] Dependencies installed!"
}

# Function to verify tools
verify_tools() {
    echo "[INFO] Verifying installed tools..."
    
    local missing_tools=()
    
    if ! command_exists gcc; then
        missing_tools+=("gcc")
    fi
    
    if ! command_exists nasm; then
        missing_tools+=("nasm")
    fi
    
    if ! command_exists grub-mkrescue; then
        missing_tools+=("grub-mkrescue")
    fi
    
    if ! command_exists xorriso; then
        missing_tools+=("xorriso")
    fi
    
    if ! command_exists mtools; then
        missing_tools+=("mtools")
    fi
    
    if [ ${#missing_tools[@]} -eq 0 ]; then
        echo "[SUCCESS] All required tools are available!"
        return 0
    else
        echo "[ERROR] Missing tools: ${missing_tools[*]}"
        return 1
    fi
}

# Function to show tool versions
show_versions() {
    echo "[INFO] Tool versions:"
    echo "GCC: $(gcc --version | head -n1)"
    echo "NASM: $(nasm -v)"
    echo "GRUB: $(grub-mkrescue --version | head -n1)"
    echo "Xorriso: $(xorriso -version 2>&1 | head -n1)"
}

# Function to build RaeenOS
build_raeenos() {
    echo "[INFO] Building RaeenOS..."
    
    # Check if we're in the right directory
    if [ ! -f "build_fixed.bat" ]; then
        echo "[ERROR] Not in RaeenOS directory or build_fixed.bat not found"
        echo "[INFO] Please run: cd /d/RaeenOS"
        return 1
    fi
    
    # Convert Windows batch script to bash commands
    echo "[INFO] Compiling kernel entry point..."
    nasm -f elf64 "kernel/arch/x86_64/kernel_entry.S" -o "build/kernel/kernel_entry.o"
    
    if [ $? -ne 0 ]; then
        echo "[ERROR] Failed to compile kernel entry point"
        return 1
    fi
    
    echo "[INFO] Compiling kernel main..."
    gcc -c -m64 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra \
        -I"kernel" -I"kernel/include" -I"kernel/core/include" \
        "kernel/core/kernel_main.c" -o "build/kernel/kernel_main.o"
    
    if [ $? -ne 0 ]; then
        echo "[ERROR] Failed to compile kernel main"
        return 1
    fi
    
    echo "[INFO] Linking kernel..."
    ld -T "kernel/kernel_fixed.ld" -o "build/kernel.bin" \
        "build/kernel/kernel_entry.o" "build/kernel/kernel_main.o"
    
    if [ $? -ne 0 ]; then
        echo "[ERROR] Failed to link kernel"
        return 1
    fi
    
    echo "[SUCCESS] Kernel built successfully: build/kernel.bin"
    return 0
}

# Function to create ISO
create_iso() {
    echo "[INFO] Creating bootable ISO..."
    
    if [ ! -f "build/kernel.bin" ]; then
        echo "[ERROR] Kernel binary not found. Please build the kernel first."
        return 1
    fi
    
    # Create ISO directory structure
    mkdir -p "build/iso/boot/grub"
    
    # Copy kernel
    cp "build/kernel.bin" "build/iso/boot/"
    
    # Create GRUB configuration
    cat > "build/iso/boot/grub/grub.cfg" << EOF
menuentry "RaeenOS" {
    multiboot /boot/kernel.bin
}
EOF
    
    # Create ISO
    grub-mkrescue -o "build/raeenos.iso" "build/iso"
    
    if [ $? -eq 0 ]; then
        echo "[SUCCESS] ISO created: build/raeenos.iso"
        return 0
    else
        echo "[ERROR] Failed to create ISO"
        return 1
    fi
}

# Main menu
show_menu() {
    echo
    echo "Choose an option:"
    echo "1. Install dependencies"
    echo "2. Verify tools"
    echo "3. Show tool versions"
    echo "4. Build RaeenOS kernel"
    echo "5. Create bootable ISO"
    echo "6. Full build (kernel + ISO)"
    echo "7. Exit"
    echo
    read -p "Enter your choice (1-7): " choice
    
    case $choice in
        1)
            install_dependencies
            ;;
        2)
            verify_tools
            ;;
        3)
            show_versions
            ;;
        4)
            # Create build directories
            mkdir -p build/kernel
            build_raeenos
            ;;
        5)
            create_iso
            ;;
        6)
            mkdir -p build/kernel
            if build_raeenos; then
                create_iso
            fi
            ;;
        7)
            echo "Goodbye!"
            exit 0
            ;;
        *)
            echo "[ERROR] Invalid choice"
            ;;
    esac
}

# Main execution
echo "[INFO] RaeenOS MSYS2 Build Helper"
echo "[INFO] Current directory: $(pwd)"
echo

# Check if we're in MSYS2
if [ -z "$MSYSTEM" ]; then
    echo "[WARNING] This script is designed for MSYS2 environment"
fi

# Show menu in loop
while true; do
    show_menu
    echo
    read -p "Press Enter to continue..."
    echo
done