@echo off
REM RaeenOS Build Script for Windows
REM This script builds RaeenOS components

setlocal enabledelayedexpansion

REM Configuration
set PROJECT_ROOT=%~dp0
set BUILD_DIR=%PROJECT_ROOT%build
set TARGET_ARCH=x64
set BUILD_TYPE=debug

REM Colors (if supported)
set COLOR_RED=[91m
set COLOR_GREEN=[92m
set COLOR_YELLOW=[93m
set COLOR_BLUE=[94m
set COLOR_CYAN=[96m
set COLOR_RESET=[0m

echo %COLOR_BLUE%╔══════════════════════════════════════════════════════════════╗%COLOR_RESET%
echo %COLOR_BLUE%║                    RaeenOS Build Script                     ║%COLOR_RESET%
echo %COLOR_BLUE%║                         Version 1.0                         ║%COLOR_RESET%
echo %COLOR_BLUE%╚══════════════════════════════════════════════════════════════╝%COLOR_RESET%
echo.

REM Check for required tools
echo %COLOR_CYAN%[INFO]%COLOR_RESET% Checking for required build tools...

where gcc >nul 2>&1
if errorlevel 1 (
    echo %COLOR_RED%[ERROR]%COLOR_RESET% GCC not found. Please install MinGW-w64 or MSYS2.
    goto :missing_tools
)

where nasm >nul 2>&1
if errorlevel 1 (
    echo %COLOR_RED%[ERROR]%COLOR_RESET% NASM not found. Please install NASM assembler.
    goto :missing_tools
)

echo %COLOR_GREEN%[INFO]%COLOR_RESET% Required tools found.

REM Parse command line arguments
:parse_args
if "%1"=="" goto :start_build
if "%1"=="clean" (
    set DO_CLEAN=1
    shift
    goto :parse_args
)
if "%1"=="bootloader" (
    set BUILD_BOOTLOADER=1
    shift
    goto :parse_args
)
if "%1"=="kernel" (
    set BUILD_KERNEL=1
    shift
    goto :parse_args
)
if "%1"=="tools" (
    set BUILD_TOOLS=1
    shift
    goto :parse_args
)
if "%1"=="all" (
    set BUILD_ALL=1
    shift
    goto :parse_args
)
if "%1"=="test" (
    set RUN_TESTS=1
    shift
    goto :parse_args
)
if "%1"=="help" (
    goto :show_help
)
shift
goto :parse_args

:start_build
REM Default to build all if no specific target
if not defined BUILD_BOOTLOADER if not defined BUILD_KERNEL if not defined BUILD_TOOLS (
    set BUILD_ALL=1
)

REM Clean if requested
if defined DO_CLEAN (
    echo %COLOR_CYAN%[INFO]%COLOR_RESET% Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
    echo %COLOR_GREEN%[INFO]%COLOR_RESET% Clean completed.
)

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Build bootloader
if defined BUILD_BOOTLOADER (
    call :build_bootloader
) else if defined BUILD_ALL (
    call :build_bootloader
)

REM Build kernel
if defined BUILD_KERNEL (
    call :build_kernel
) else if defined BUILD_ALL (
    call :build_kernel
)

REM Build tools
if defined BUILD_TOOLS (
    call :build_tools
) else if defined BUILD_ALL (
    call :build_tools
)

REM Run tests
if defined RUN_TESTS (
    call :run_tests
)

echo.
echo %COLOR_GREEN%[INFO]%COLOR_RESET% Build completed successfully!
goto :end

:build_bootloader
echo %COLOR_CYAN%[INFO]%COLOR_RESET% Building bootloader...
cd /d "%PROJECT_ROOT%bootloader"

if not exist "%BUILD_DIR%\bootloader" mkdir "%BUILD_DIR%\bootloader"

REM Build boot.asm
nasm -f bin boot.asm -o "%BUILD_DIR%\bootloader\boot.bin"
if errorlevel 1 (
    echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to build boot.asm
    exit /b 1
)

REM Build stage1.asm
nasm -f bin stage1\stage1.asm -o "%BUILD_DIR%\bootloader\stage1.bin"
if errorlevel 1 (
    echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to build stage1.asm
    exit /b 1
)

REM Build stage2.asm
nasm -f bin stage2\stage2.asm -o "%BUILD_DIR%\bootloader\stage2.bin"
if errorlevel 1 (
    echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to build stage2.asm
    exit /b 1
)

REM Build multiboot.c
gcc -c -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs multiboot.c -o "%BUILD_DIR%\bootloader\multiboot.o"
if errorlevel 1 (
    echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to build multiboot.c
    exit /b 1
)

REM Create bootloader.bin
copy /b "%BUILD_DIR%\bootloader\boot.bin" + "%BUILD_DIR%\bootloader\stage2.bin" "%BUILD_DIR%\bootloader\bootloader.bin" >nul
if errorlevel 1 (
    echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to create bootloader.bin
    exit /b 1
)

echo %COLOR_GREEN%[INFO]%COLOR_RESET% Bootloader built successfully.
cd /d "%PROJECT_ROOT%"
exit /b 0

:build_kernel
echo %COLOR_CYAN%[INFO]%COLOR_RESET% Building kernel...
cd /d "%PROJECT_ROOT%kernel"

REM Create kernel build directory structure
if not exist "%BUILD_DIR%\kernel" mkdir "%BUILD_DIR%\kernel"
if not exist "%BUILD_DIR%\kernel\core" mkdir "%BUILD_DIR%\kernel\core"

REM Set compiler flags
set CFLAGS=-std=c11 -Wall -Wextra -nostdlib -fno-builtin -fno-stack-protector -ffreestanding -fno-pie -fno-pic -mno-red-zone -mcmodel=kernel -m64 -DTARGET_X64
set INCLUDES=-I. -Iinclude -Icore/include

if "%BUILD_TYPE%"=="debug" (
    set CFLAGS=!CFLAGS! -g -O0 -DDEBUG
) else (
    set CFLAGS=!CFLAGS! -O2 -DNDEBUG
)

REM Compile kernel_main.c
gcc !CFLAGS! !INCLUDES! -c core\kernel_main.c -o "%BUILD_DIR%\kernel\core\kernel_main.o"
if errorlevel 1 (
    echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to compile kernel_main.c
    exit /b 1
)

REM Create a minimal kernel for now (since we don't have all components yet)
echo %COLOR_YELLOW%[WARN]%COLOR_RESET% Creating minimal kernel for testing...

REM Check if linker script exists
if not exist kernel.ld (
    echo %COLOR_YELLOW%[WARN]%COLOR_RESET% Creating minimal linker script...
    echo ENTRY(_start) > kernel.ld
    echo SECTIONS { >> kernel.ld
    echo     . = 0x100000; >> kernel.ld
    echo     .text : { *(.text) } >> kernel.ld
    echo     .data : { *(.data) } >> kernel.ld
    echo     .bss : { *(.bss) } >> kernel.ld
    echo } >> kernel.ld
)

REM Link kernel
ld -T kernel.ld -o "%BUILD_DIR%\kernel\raeenos_kernel" "%BUILD_DIR%\kernel\core\kernel_main.o"
if errorlevel 1 (
    echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to link kernel
    exit /b 1
)

echo %COLOR_GREEN%[INFO]%COLOR_RESET% Kernel built successfully.
cd /d "%PROJECT_ROOT%"
exit /b 0

:build_tools
echo %COLOR_CYAN%[INFO]%COLOR_RESET% Building tools...
cd /d "%PROJECT_ROOT%tools"

REM Create tools build directory
if not exist "%BUILD_DIR%\tools" mkdir "%BUILD_DIR%\tools"

REM Build mkfs tool
if exist mkfs_raeenos.c (
    gcc -o "%BUILD_DIR%\tools\mkfs_raeenos.exe" mkfs_raeenos.c
    if errorlevel 1 (
        echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to build mkfs tool
        exit /b 1
    )
    echo %COLOR_GREEN%[INFO]%COLOR_RESET% mkfs tool built successfully.
)

REM Build debugger
if exist raeenos_debugger.c (
    gcc -o "%BUILD_DIR%\tools\raeenos_debugger.exe" raeenos_debugger.c
    if errorlevel 1 (
        echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to build debugger
        exit /b 1
    )
    echo %COLOR_GREEN%[INFO]%COLOR_RESET% Debugger built successfully.
)

REM Build profiler
if exist profiler\performance_profiler.c (
    gcc -o "%BUILD_DIR%\tools\performance_profiler.exe" profiler\performance_profiler.c
    if errorlevel 1 (
        echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to build profiler
        exit /b 1
    )
    echo %COLOR_GREEN%[INFO]%COLOR_RESET% Profiler built successfully.
)

REM Build emulator
if exist emulator\hardware_emulator.c (
    gcc -o "%BUILD_DIR%\tools\hardware_emulator.exe" emulator\hardware_emulator.c
    if errorlevel 1 (
        echo %COLOR_RED%[ERROR]%COLOR_RESET% Failed to build emulator
        exit /b 1
    )
    echo %COLOR_GREEN%[INFO]%COLOR_RESET% Hardware emulator built successfully.
)

echo %COLOR_GREEN%[INFO]%COLOR_RESET% Tools built successfully.
cd /d "%PROJECT_ROOT%"
exit /b 0

:run_tests
echo %COLOR_CYAN%[INFO]%COLOR_RESET% Running basic tests...

REM Test bootloader
if exist "%BUILD_DIR%\bootloader\bootloader.bin" (
    echo %COLOR_GREEN%[PASS]%COLOR_RESET% Bootloader exists
) else (
    echo %COLOR_RED%[FAIL]%COLOR_RESET% Bootloader not found
)

REM Test kernel
if exist "%BUILD_DIR%\kernel\raeenos_kernel" (
    echo %COLOR_GREEN%[PASS]%COLOR_RESET% Kernel exists
) else (
    echo %COLOR_RED%[FAIL]%COLOR_RESET% Kernel not found
)

REM Test tools
if exist "%BUILD_DIR%\tools\mkfs_raeenos.exe" (
    echo %COLOR_GREEN%[PASS]%COLOR_RESET% mkfs tool exists
) else (
    echo %COLOR_YELLOW%[SKIP]%COLOR_RESET% mkfs tool not found
)

echo %COLOR_GREEN%[INFO]%COLOR_RESET% Basic tests completed.
exit /b 0

:missing_tools
echo.
echo %COLOR_YELLOW%[INFO]%COLOR_RESET% To install the required tools on Windows:
echo.
echo %COLOR_CYAN%Option 1: MSYS2 (Recommended)%COLOR_RESET%
echo   1. Download and install MSYS2 from https://www.msys2.org/
echo   2. Open MSYS2 terminal and run:
echo      pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-nasm
echo   3. Add C:\msys64\mingw64\bin to your PATH
echo.
echo %COLOR_CYAN%Option 2: Standalone Tools%COLOR_RESET%
echo   1. Download MinGW-w64 from https://www.mingw-w64.org/
echo   2. Download NASM from https://www.nasm.us/
echo   3. Add all tools to your PATH
echo.
goto :end

:show_help
echo Usage: %0 [OPTIONS] [TARGETS]
echo.
echo Options:
echo   clean       Clean build directory
echo   help        Show this help message
echo.
echo Targets:
echo   bootloader  Build bootloader only
echo   kernel      Build kernel only
echo   tools       Build tools only
echo   test        Run tests
echo   all         Build everything (default)
echo.
echo Examples:
echo   %0                    Build everything
echo   %0 clean all          Clean and build everything
echo   %0 kernel test        Build kernel and run tests
echo   %0 bootloader kernel  Build bootloader and kernel only
echo.
goto :end

:end
endlocal