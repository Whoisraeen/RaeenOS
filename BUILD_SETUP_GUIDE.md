# RaeenOS Immediate Build Setup Guide

## 🚀 **CRITICAL: Build Environment Setup**

Based on your comprehensive feature requirements, here's the immediate action plan to get RaeenOS building and running:

## **Step 1: Install MSYS2 Build Environment**

### Download and Install MSYS2
```powershell
# Download MSYS2 installer
Invoke-WebRequest -Uri "https://github.com/msys2/msys2-installer/releases/download/2024-01-13/msys2-x86_64-20240113.exe" -OutFile "msys2-installer.exe"

# Run installer (follow GUI prompts)
.\msys2-installer.exe
```

### Configure MSYS2 Environment
```bash
# Open MSYS2 terminal and update package database
pacman -Syu

# Install essential build tools
pacman -S --needed base-devel mingw-w64-x86_64-toolchain

# Install cross-compilation tools
pacman -S --needed mingw-w64-x86_64-gcc
pacman -S --needed mingw-w64-x86_64-binutils
pacman -S --needed mingw-w64-x86_64-gdb

# Install assembler and build tools
pacman -S nasm
pacman -S make
pacman -S git

# Install QEMU for testing
pacman -S mingw-w64-x86_64-qemu
```

## **Step 2: Configure Cross-Compilation**

### Create Toolchain Configuration
```bash
# Create toolchain script: setup-toolchain.sh
cat > setup-toolchain.sh << 'EOF'
#!/bin/bash

export TARGET=x86_64-elf
export PREFIX=/opt/cross
export PATH="$PREFIX/bin:$PATH"

# GCC Configuration
export CC=$TARGET-gcc
export CXX=$TARGET-g++
export AS=$TARGET-as
export LD=$TARGET-ld
export AR=$TARGET-ar
export OBJCOPY=$TARGET-objcopy
export OBJDUMP=$TARGET-objdump
export STRIP=$TARGET-strip

# Build flags
export CFLAGS="-ffreestanding -O2 -Wall -Wextra"
export CXXFLAGS="-ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti"
export LDFLAGS="-nostdlib"

echo "Cross-compilation toolchain configured for $TARGET"
EOF

chmod +x setup-toolchain.sh
source setup-toolchain.sh
```

## **Step 3: Enhanced Build Script**

### Update build.bat for Better Error Handling
```batch
@echo off
setlocal enabledelayedexpansion

echo ========================================
echo RaeenOS Enhanced Build System
echo ========================================

:: Check for required tools
call :check_tool gcc "GCC Compiler"
call :check_tool nasm "NASM Assembler"
call :check_tool ld "GNU Linker"
call :check_tool make "GNU Make"

if !TOOLS_MISSING! == 1 (
    echo.
    echo ERROR: Missing required build tools!
    echo Please install MSYS2 and required packages.
    echo See BUILD_SETUP_GUIDE.md for instructions.
    exit /b 1
)

:: Set build configuration
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=debug

echo Build Type: %BUILD_TYPE%
echo.

:: Create build directories
if not exist build mkdir build
if not exist build\kernel mkdir build\kernel
if not exist build\bootloader mkdir build\bootloader
if not exist build\iso mkdir build\iso

:: Build based on target
if "%1"=="clean" goto :clean
if "%1"=="bootloader" goto :build_bootloader
if "%1"=="kernel" goto :build_kernel
if "%1"=="iso" goto :build_iso
if "%1"=="test" goto :test
if "%1"=="all" goto :build_all

:build_all
echo Building complete RaeenOS system...
call :build_bootloader
if !ERRORLEVEL! neq 0 exit /b 1
call :build_kernel
if !ERRORLEVEL! neq 0 exit /b 1
call :build_iso
if !ERRORLEVEL! neq 0 exit /b 1
goto :success

:build_bootloader
echo Building bootloader...
nasm -f bin bootloader\boot.asm -o build\bootloader\boot.bin
if !ERRORLEVEL! neq 0 (
    echo ERROR: Bootloader build failed!
    exit /b 1
)
echo Bootloader built successfully.
goto :eof

:build_kernel
echo Building kernel...

:: Compile kernel objects
set KERNEL_OBJECTS=
for /r kernel %%f in (*.c) do (
    set "source=%%f"
    set "object=build\kernel\%%~nf.o"
    echo Compiling !source!...
    gcc -c !source! -o !object! -Ikernel\include -ffreestanding -O2 -Wall -Wextra
    if !ERRORLEVEL! neq 0 (
        echo ERROR: Failed to compile !source!
        exit /b 1
    )
    set "KERNEL_OBJECTS=!KERNEL_OBJECTS! !object!"
)

:: Compile assembly files
for /r kernel %%f in (*.asm) do (
    set "source=%%f"
    set "object=build\kernel\%%~nf.o"
    echo Assembling !source!...
    nasm -f elf64 !source! -o !object!
    if !ERRORLEVEL! neq 0 (
        echo ERROR: Failed to assemble !source!
        exit /b 1
    )
    set "KERNEL_OBJECTS=!KERNEL_OBJECTS! !object!"
)

:: Link kernel
echo Linking kernel...
ld -T kernel\kernel.ld !KERNEL_OBJECTS! -o build\kernel\kernel.bin
if !ERRORLEVEL! neq 0 (
    echo ERROR: Kernel linking failed!
    exit /b 1
)
echo Kernel built successfully.
goto :eof

:build_iso
echo Creating bootable ISO...
if not exist build\iso\boot mkdir build\iso\boot
if not exist build\iso\boot\grub mkdir build\iso\boot\grub

copy build\bootloader\boot.bin build\iso\boot\
copy build\kernel\kernel.bin build\iso\boot\
copy bootloader\grub.cfg build\iso\boot\grub\

:: Create ISO using grub-mkrescue (if available)
grub-mkrescue -o build\raeenos.iso build\iso\
if !ERRORLEVEL! neq 0 (
    echo WARNING: grub-mkrescue not available, creating simple ISO
    :: Fallback to simple ISO creation
    echo Creating simple bootable image...
    copy /b build\bootloader\boot.bin + build\kernel\kernel.bin build\raeenos.img
)
echo ISO created successfully.
goto :eof

:test
echo Running RaeenOS tests...
if exist build\raeenos.iso (
    echo Starting QEMU test...
    qemu-system-x86_64 -cdrom build\raeenos.iso -m 512M -serial stdio
) else if exist build\raeenos.img (
    echo Starting QEMU test with image...
    qemu-system-x86_64 -drive format=raw,file=build\raeenos.img -m 512M -serial stdio
) else (
    echo ERROR: No bootable image found. Run 'build.bat all' first.
    exit /b 1
)
goto :eof

:clean
echo Cleaning build directory...
if exist build rmdir /s /q build
echo Clean complete.
goto :eof

:check_tool
where %1 >nul 2>&1
if !ERRORLEVEL! neq 0 (
    echo ERROR: %2 not found in PATH
    set TOOLS_MISSING=1
) else (
    echo OK: %2 found
)
goto :eof

:success
echo.
echo ========================================
echo RaeenOS Build Complete!
echo ========================================
echo.
echo Next steps:
echo 1. Test with: build.bat test
echo 2. Create ISO: build.bat iso
echo 3. Run in QEMU: qemu-system-x86_64 -cdrom build\raeenos.iso
echo.
goto :eof
```

## **Step 4: Create Development Environment**

### VS Code Configuration
```json
// .vscode/settings.json
{
    "C_Cpp.default.compilerPath": "C:/msys64/mingw64/bin/gcc.exe",
    "C_Cpp.default.cStandard": "c11",
    "C_Cpp.default.cppStandard": "c++17",
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/kernel/include",
        "${workspaceFolder}/kernel",
        "C:/msys64/mingw64/include"
    ],
    "C_Cpp.default.defines": [
        "_DEBUG",
        "KERNEL_BUILD",
        "__x86_64__"
    ],
    "files.associations": {
        "*.h": "c",
        "*.c": "c",
        "*.asm": "nasm"
    }
}
```

### Build Tasks
```json
// .vscode/tasks.json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build RaeenOS",
            "type": "shell",
            "command": "build.bat",
            "args": ["all"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        },
        {
            "label": "Test RaeenOS",
            "type": "shell",
            "command": "build.bat",
            "args": ["test"],
            "group": "test"
        },
        {
            "label": "Clean Build",
            "type": "shell",
            "command": "build.bat",
            "args": ["clean"],
            "group": "build"
        }
    ]
}
```

## **Step 5: Immediate Testing**

### Quick Validation Script
```powershell
# test-build.ps1
Write-Host "RaeenOS Build Validation" -ForegroundColor Green

# Test 1: Check tools
Write-Host "`nChecking build tools..." -ForegroundColor Yellow
$tools = @("gcc", "nasm", "ld", "make")
$missing = @()

foreach ($tool in $tools) {
    try {
        $null = Get-Command $tool -ErrorAction Stop
        Write-Host "✓ $tool found" -ForegroundColor Green
    } catch {
        Write-Host "✗ $tool missing" -ForegroundColor Red
        $missing += $tool
    }
}

if ($missing.Count -gt 0) {
    Write-Host "`nMissing tools: $($missing -join ', ')" -ForegroundColor Red
    Write-Host "Please install MSYS2 and required packages." -ForegroundColor Yellow
    exit 1
}

# Test 2: Try building
Write-Host "`nAttempting to build RaeenOS..." -ForegroundColor Yellow
& .\build.bat clean
$result = & .\build.bat all

if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Build successful!" -ForegroundColor Green
    
    # Test 3: Check outputs
    if (Test-Path "build\raeenos.iso") {
        Write-Host "✓ ISO created successfully" -ForegroundColor Green
    } elseif (Test-Path "build\raeenos.img") {
        Write-Host "✓ Boot image created successfully" -ForegroundColor Green
    }
    
    Write-Host "`nRaeenOS is ready for testing!" -ForegroundColor Green
    Write-Host "Run: build.bat test" -ForegroundColor Cyan
} else {
    Write-Host "✗ Build failed" -ForegroundColor Red
    Write-Host "Check error messages above" -ForegroundColor Yellow
}
```

## **Step 6: Feature Implementation Priority**

### Immediate (This Week)
1. **Get kernel compiling** - Fix any compilation errors
2. **Create bootable image** - Ensure boot sequence works
3. **Basic I/O** - Console output, keyboard input
4. **Memory management** - Validate PMM/VMM functionality

### Week 2-3: Core Features
1. **File system** - Basic VFS operations
2. **Process management** - Process creation/termination
3. **Device drivers** - Essential hardware support
4. **Network stack** - Basic TCP/IP

### Week 4-8: User Interface
1. **GUI framework** - Window management
2. **Desktop environment** - Taskbar, windows, menus
3. **Applications** - File manager, text editor, terminal
4. **Package manager** - Software installation

## **Step 7: Quality Assurance**

### Automated Testing
```bash
# Create test suite
mkdir tests
cat > tests/run_tests.sh << 'EOF'
#!/bin/bash

echo "Running RaeenOS Test Suite"

# Unit tests
echo "Running unit tests..."
make test-unit

# Integration tests
echo "Running integration tests..."
make test-integration

# Boot test
echo "Testing boot sequence..."
timeout 30 qemu-system-x86_64 -cdrom build/raeenos.iso -nographic -serial mon:stdio

echo "All tests completed"
EOF
```

## **🎯 Success Criteria**

### Week 1 Goals
- [ ] MSYS2 installed and configured
- [ ] Cross-compilation toolchain working
- [ ] Kernel compiles without errors
- [ ] Bootable image created
- [ ] Basic boot test in QEMU successful

### Week 2 Goals
- [ ] All core subsystems initializing
- [ ] Memory management functional
- [ ] Basic device drivers working
- [ ] Console I/O operational

### Week 4 Goals
- [ ] GUI framework operational
- [ ] Basic applications running
- [ ] File system operations working
- [ ] Network connectivity established

This setup guide will get your RaeenOS from the current state to a fully buildable and testable operating system. The comprehensive feature list you provided shows excellent planning - now it's time to make it reality!