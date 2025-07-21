# RaeenOS Build Environment Setup Guide

## Prerequisites for Windows Development

### Option 1: MSYS2 (Recommended)

1. **Download and Install MSYS2**
   - Go to https://www.msys2.org/
   - Download the installer for your system (x86_64)
   - Run the installer and follow the setup wizard
   - Install to default location: `C:\msys64`

2. **Update MSYS2**
   ```bash
   # Open MSYS2 terminal and run:
   pacman -Syu
   # Close terminal when prompted, then reopen and run:
   pacman -Su
   ```

3. **Install Required Tools**
   ```bash
   # Install GCC toolchain
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-gcc-libs
   pacman -S mingw-w64-x86_64-binutils
   
   # Install NASM assembler
   pacman -S mingw-w64-x86_64-nasm
   
   # Install additional build tools
   pacman -S mingw-w64-x86_64-make
   pacman -S mingw-w64-x86_64-gdb
   ```

4. **Add to Windows PATH**
   - Add `C:\msys64\mingw64\bin` to your Windows PATH environment variable
   - Open System Properties → Advanced → Environment Variables
   - Edit the PATH variable and add the MSYS2 bin directory

### Option 2: Standalone Tools

1. **Install MinGW-w64**
   - Download from https://www.mingw-w64.org/downloads/
   - Choose a distribution like w64devkit or TDM-GCC
   - Install and add to PATH

2. **Install NASM**
   - Download from https://www.nasm.us/pub/nasm/releasebuilds/
   - Get the Windows version (nasm-x.xx-win64.zip)
   - Extract and add to PATH

### Option 3: Visual Studio Build Tools (Alternative)

1. **Install Visual Studio Build Tools**
   - Download from https://visualstudio.microsoft.com/downloads/
   - Install C++ build tools
   - Install Windows SDK

2. **Install NASM separately**
   - Download from https://www.nasm.us/
   - Add to PATH

## Verification

After installation, verify the tools are available:

```cmd
# Check GCC
gcc --version

# Check NASM
nasm -v

# Check Make (if using MSYS2)
make --version
```

## Build Commands

Once the environment is set up:

```cmd
# Navigate to RaeenOS directory
cd d:\RaeenOS

# Clean previous builds
.\build.bat clean

# Build kernel only
.\build.bat kernel

# Build everything
.\build.bat all
```

## Troubleshooting

### Common Issues

1. **"gcc is not recognized"**
   - Ensure MinGW-w64 bin directory is in PATH
   - Restart command prompt after PATH changes

2. **"nasm is not recognized"**
   - Ensure NASM is installed and in PATH
   - Download from official NASM website

3. **Build errors**
   - Check that all required headers are available
   - Verify GCC version compatibility (recommend GCC 9.0+)

### Build Environment Test

Create a simple test to verify your environment:

```c
// test.c
#include <stdio.h>
int main() {
    printf("Build environment working!\n");
    return 0;
}
```

```cmd
gcc test.c -o test.exe
test.exe
```

If this works, your environment is ready for RaeenOS development.