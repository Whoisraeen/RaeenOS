#!/bin/bash

# RaeenOS ISO Creation Script
# Creates a bootable ISO image for RaeenOS

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build"
ISO_DIR="${BUILD_DIR}/iso"
GRUB_DIR="${ISO_DIR}/boot/grub"

# Configuration
OS_NAME="RaeenOS"
OS_VERSION="1.0.0"
ISO_NAME="${OS_NAME}-${OS_VERSION}.iso"
KERNEL_NAME="raeenos_kernel"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_banner() {
    echo -e "${BLUE}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║                  RaeenOS ISO Creation Tool                  ║${NC}"
    echo -e "${BLUE}║                         Version 1.0                         ║${NC}"
    echo -e "${BLUE}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo
}

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_dependencies() {
    log_info "Checking dependencies..."
    
    local missing_deps=()
    
    if ! command -v grub-mkrescue &> /dev/null; then
        missing_deps+=("grub-mkrescue")
    fi
    
    if ! command -v xorriso &> /dev/null; then
        missing_deps+=("xorriso")
    fi
    
    if ! command -v mtools &> /dev/null; then
        missing_deps+=("mtools")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        log_error "Missing dependencies: ${missing_deps[*]}"
        echo "Please install the missing dependencies:"
        echo "  Ubuntu/Debian: sudo apt-get install grub-pc-bin xorriso mtools"
        echo "  Fedora/RHEL: sudo dnf install grub2-tools xorriso mtools"
        echo "  Arch: sudo pacman -S grub xorriso mtools"
        exit 1
    fi
    
    log_info "All dependencies found."
}

create_directory_structure() {
    log_info "Creating directory structure..."
    
    mkdir -p "${ISO_DIR}"
    mkdir -p "${GRUB_DIR}"
    mkdir -p "${ISO_DIR}/boot"
    mkdir -p "${ISO_DIR}/modules"
    mkdir -p "${ISO_DIR}/drivers"
    mkdir -p "${ISO_DIR}/apps"
    mkdir -p "${ISO_DIR}/data"
    
    log_info "Directory structure created."
}

copy_kernel() {
    log_info "Copying kernel..."
    
    local kernel_path="${BUILD_DIR}/kernel/${KERNEL_NAME}"
    
    if [ ! -f "${kernel_path}" ]; then
        log_error "Kernel not found at ${kernel_path}"
        log_error "Please build the kernel first: make kernel"
        exit 1
    fi
    
    cp "${kernel_path}" "${ISO_DIR}/boot/"
    log_info "Kernel copied to ISO."
}

copy_bootloader() {
    log_info "Copying bootloader..."
    
    local bootloader_path="${BUILD_DIR}/bootloader/bootloader.bin"
    
    if [ ! -f "${bootloader_path}" ]; then
        log_warn "Bootloader not found at ${bootloader_path}"
        log_warn "Using GRUB as primary bootloader"
    else
        cp "${bootloader_path}" "${ISO_DIR}/boot/"
        log_info "Custom bootloader copied to ISO."
    fi
}

create_grub_config() {
    log_info "Creating GRUB configuration..."
    
    cat > "${GRUB_DIR}/grub.cfg" << EOF
# GRUB Configuration for RaeenOS
set timeout=5
set default=0

# Set the root
insmod part_msdos
insmod ext2
set root='hd0,msdos1'

# Menu entries
menuentry "${OS_NAME} ${OS_VERSION}" {
    multiboot /boot/${KERNEL_NAME}
    boot
}

menuentry "${OS_NAME} ${OS_VERSION} (Safe Mode)" {
    multiboot /boot/${KERNEL_NAME} safe_mode
    boot
}

menuentry "${OS_NAME} ${OS_VERSION} (Debug Mode)" {
    multiboot /boot/${KERNEL_NAME} debug
    boot
}

menuentry "Memory Test" {
    linux16 /boot/memtest86+.bin
}

menuentry "Reboot" {
    reboot
}

menuentry "Shutdown" {
    halt
}
EOF
    
    log_info "GRUB configuration created."
}

copy_modules() {
    log_info "Copying kernel modules..."
    
    local modules_dir="${BUILD_DIR}/modules"
    
    if [ -d "${modules_dir}" ]; then
        cp -r "${modules_dir}"/* "${ISO_DIR}/modules/" 2>/dev/null || true
        log_info "Kernel modules copied."
    else
        log_warn "No kernel modules found."
    fi
}

copy_drivers() {
    log_info "Copying drivers..."
    
    local drivers_dir="${BUILD_DIR}/drivers"
    
    if [ -d "${drivers_dir}" ]; then
        cp -r "${drivers_dir}"/* "${ISO_DIR}/drivers/" 2>/dev/null || true
        log_info "Drivers copied."
    else
        log_warn "No drivers found."
    fi
}

copy_applications() {
    log_info "Copying applications..."
    
    local apps_dir="${BUILD_DIR}/apps"
    
    if [ -d "${apps_dir}" ]; then
        cp -r "${apps_dir}"/* "${ISO_DIR}/apps/" 2>/dev/null || true
        log_info "Applications copied."
    else
        log_warn "No applications found."
    fi
}

create_initrd() {
    log_info "Creating initial ramdisk..."
    
    local initrd_dir="${BUILD_DIR}/initrd"
    local initrd_file="${ISO_DIR}/boot/initrd.img"
    
    if [ -d "${initrd_dir}" ]; then
        # Create initrd archive
        (cd "${initrd_dir}" && find . | cpio -o -H newc | gzip > "${initrd_file}")
        log_info "Initial ramdisk created."
    else
        log_warn "No initrd directory found."
    fi
}

create_version_info() {
    log_info "Creating version information..."
    
    cat > "${ISO_DIR}/VERSION" << EOF
${OS_NAME} ${OS_VERSION}
Build Date: $(date)
Build Host: $(hostname)
Git Commit: $(git rev-parse HEAD 2>/dev/null || echo "Unknown")
Kernel: ${KERNEL_NAME}
Architecture: x86_64
EOF
    
    log_info "Version information created."
}

create_readme() {
    log_info "Creating README..."
    
    cat > "${ISO_DIR}/README.txt" << EOF
Welcome to ${OS_NAME} ${OS_VERSION}!

This is a bootable ISO image of ${OS_NAME}, a modern operating system
designed for performance, security, and user experience.

Features:
- Hybrid kernel architecture
- Advanced memory management
- Modern graphics subsystem
- Comprehensive security framework
- AI-enhanced user experience
- Cross-platform compatibility (x86/x64)

Boot Instructions:
1. Burn this ISO to a CD/DVD or create a bootable USB drive
2. Boot from the media
3. Select "${OS_NAME} ${OS_VERSION}" from the boot menu
4. Enjoy exploring ${OS_NAME}!

For more information, visit: https://github.com/RaeenOS/RaeenOS

Build Information:
- Version: ${OS_VERSION}
- Build Date: $(date)
- Architecture: x86_64

Troubleshooting:
- If the system fails to boot, try "Safe Mode"
- For debugging, use "Debug Mode"
- Check hardware compatibility requirements

© 2024 RaeenOS Project. All rights reserved.
EOF
    
    log_info "README created."
}

create_iso() {
    log_info "Creating ISO image..."
    
    local iso_path="${BUILD_DIR}/${ISO_NAME}"
    
    # Remove existing ISO
    if [ -f "${iso_path}" ]; then
        rm "${iso_path}"
    fi
    
    # Create ISO using grub-mkrescue
    grub-mkrescue -o "${iso_path}" "${ISO_DIR}" \
        --volid="${OS_NAME}_${OS_VERSION}" \
        --application-id="${OS_NAME}" \
        --publisher="RaeenOS Project" \
        --preparer="RaeenOS Build System" \
        --copyright="Copyright 2024 RaeenOS Project"
    
    if [ $? -eq 0 ]; then
        log_info "ISO image created successfully: ${iso_path}"
        
        # Show ISO information
        local iso_size=$(du -h "${iso_path}" | cut -f1)
        log_info "ISO size: ${iso_size}"
        
        # Verify ISO
        if command -v file &> /dev/null; then
            file "${iso_path}"
        fi
        
        return 0
    else
        log_error "Failed to create ISO image"
        return 1
    fi
}

test_iso() {
    log_info "Testing ISO with QEMU..."
    
    local iso_path="${BUILD_DIR}/${ISO_NAME}"
    
    if [ ! -f "${iso_path}" ]; then
        log_error "ISO not found: ${iso_path}"
        return 1
    fi
    
    if ! command -v qemu-system-x86_64 &> /dev/null; then
        log_warn "QEMU not found. Skipping ISO test."
        return 0
    fi
    
    log_info "Starting QEMU test (press Ctrl+Alt+G to release mouse, Ctrl+Alt+Q to quit)..."
    
    qemu-system-x86_64 \
        -cdrom "${iso_path}" \
        -m 512M \
        -boot d \
        -vga std \
        -netdev user,id=net0 \
        -device rtl8139,netdev=net0 \
        -monitor stdio \
        -no-reboot \
        -no-shutdown
}

cleanup() {
    log_info "Cleaning up temporary files..."
    
    if [ -d "${ISO_DIR}" ]; then
        rm -rf "${ISO_DIR}"
        log_info "Temporary ISO directory removed."
    fi
}

show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "Options:"
    echo "  -h, --help      Show this help message"
    echo "  -c, --clean     Clean up before creating ISO"
    echo "  -t, --test      Test ISO with QEMU after creation"
    echo "  -k, --keep      Keep temporary files"
    echo "  -v, --verbose   Verbose output"
    echo "  -o, --output    Output ISO filename"
    echo
    echo "Examples:"
    echo "  $0                    # Create ISO"
    echo "  $0 --test            # Create and test ISO"
    echo "  $0 --clean --test    # Clean, create, and test ISO"
    echo
}

main() {
    local clean_first=false
    local test_iso_flag=false
    local keep_temp=false
    local verbose=false
    local custom_output=""
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                clean_first=true
                shift
                ;;
            -t|--test)
                test_iso_flag=true
                shift
                ;;
            -k|--keep)
                keep_temp=true
                shift
                ;;
            -v|--verbose)
                verbose=true
                set -x
                shift
                ;;
            -o|--output)
                custom_output="$2"
                shift 2
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Set custom output name if provided
    if [ -n "${custom_output}" ]; then
        ISO_NAME="${custom_output}"
    fi
    
    print_banner
    
    # Clean if requested
    if [ "${clean_first}" = true ]; then
        cleanup
    fi
    
    # Check dependencies
    check_dependencies
    
    # Create ISO
    create_directory_structure
    copy_kernel
    copy_bootloader
    create_grub_config
    copy_modules
    copy_drivers
    copy_applications
    create_initrd
    create_version_info
    create_readme
    
    if create_iso; then
        log_info "ISO creation completed successfully!"
        
        # Test if requested
        if [ "${test_iso_flag}" = true ]; then
            test_iso
        fi
        
        # Cleanup unless keeping temp files
        if [ "${keep_temp}" = false ]; then
            cleanup
        fi
        
        log_info "Done!"
        exit 0
    else
        log_error "ISO creation failed!"
        exit 1
    fi
}

# Run main function with all arguments
main "$@"