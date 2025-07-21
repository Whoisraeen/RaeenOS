# RaeenOS MSYS2 Quick Reference

## Issues You Encountered and Solutions

### 1. Pacman Command Error
**Problem:** `pacman _S grub xorriso mtools` - error: no operation specified
**Solution:** Use `-S` (dash) not `_S` (underscore)
```bash
pacman -S grub xorriso mtools
```

### 2. Directory Navigation
**Problem:** `cd d/RaeenOS/` - No such file or directory
**Solution:** Use absolute path with forward slash
```bash
cd /d/RaeenOS/
```

### 3. Missing Dependencies
**Problem:** Missing grub-mkrescue, xorriso, mtools
**Solution:** Install all required packages:
```bash
# Update package database first
pacman -Sy

# Install required packages
pacman -S --noconfirm base-devel
pacman -S --noconfirm mingw-w64-x86_64-gcc
pacman -S --noconfirm mingw-w64-x86_64-binutils
pacman -S --noconfirm nasm
pacman -S --noconfirm grub
pacman -S --noconfirm xorriso
pacman -S --noconfirm mtools
pacman -S --noconfirm make
```

## Quick Start Commands for MSYS2

### Step 1: Open MSYS2 Terminal
```bash
# Navigate to RaeenOS directory
cd /d/RaeenOS/

# Make the helper script executable
chmod +x msys2_build_helper.sh

# Run the interactive build helper
./msys2_build_helper.sh
```

### Step 2: Install Dependencies (Option 1 in menu)
The script will automatically install all required tools.

### Step 3: Build Kernel (Option 4 in menu)
This will compile the kernel using the fixed build process.

### Step 4: Create ISO (Option 5 in menu)
This will create a bootable ISO file.

## Manual Commands (if you prefer)

### Install Dependencies
```bash
pacman -Sy --noconfirm
pacman -S --noconfirm base-devel mingw-w64-x86_64-gcc mingw-w64-x86_64-binutils nasm grub xorriso mtools make
```

### Build Kernel
```bash
# Create build directories
mkdir -p build/kernel

# Compile assembly entry point
nasm -f elf64 "kernel/arch/x86_64/kernel_entry.S" -o "build/kernel/kernel_entry.o"

# Compile kernel main
gcc -c -m64 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra \
    -I"kernel" -I"kernel/include" -I"kernel/core/include" \
    "kernel/core/kernel_main.c" -o "build/kernel/kernel_main.o"

# Link kernel
ld -T "kernel/kernel_fixed.ld" -o "build/kernel.bin" \
    "build/kernel/kernel_entry.o" "build/kernel/kernel_main.o"
```

### Create Bootable ISO
```bash
# Create ISO directory structure
mkdir -p "build/iso/boot/grub"

# Copy kernel
cp "build/kernel.bin" "build/iso/boot/"

# Create GRUB config
cat > "build/iso/boot/grub/grub.cfg" << EOF
menuentry "RaeenOS" {
    multiboot /boot/kernel.bin
}
EOF

# Create ISO
grub-mkrescue -o "build/raeenos.iso" "build/iso"
```

## Troubleshooting

### If pacman fails with signature errors:
```bash
pacman-key --refresh-keys
pacman -Sy archlinux-keyring
```

### If you get permission errors:
```bash
# Run MSYS2 as administrator, or:
sudo pacman -S <package>
```

### If paths don't work:
- Use forward slashes: `/d/RaeenOS/`
- Use absolute paths from root
- Windows drives are mounted as `/c/`, `/d/`, etc.

### If build fails:
1. Check that all dependencies are installed
2. Verify you're in the correct directory
3. Check file permissions
4. Use the interactive build helper for guided process

## Files Created for You

1. `setup_msys2_environment.bat` - Windows script to set up MSYS2
2. `msys2_build_helper.sh` - Interactive build helper for MSYS2
3. `launch_msys2_raeenos.bat` - Quick launcher for MSYS2 with RaeenOS environment

## Next Steps

1. Run `./msys2_build_helper.sh` in MSYS2
2. Choose option 1 to install dependencies
3. Choose option 6 for full build (kernel + ISO)
4. Test the resulting ISO in a virtual machine