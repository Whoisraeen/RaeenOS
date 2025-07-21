@echo off 
REM RaeenOS Fixed Build Script 
 
setlocal enabledelayedexpansion 
 
set PROJECT_ROOT=%~dp0 
set BUILD_DIR=%PROJECT_ROOT%build 
set KERNEL_DIR=%PROJECT_ROOT%kernel 
 
echo [INFO] Building RaeenOS with bootability fixes... 
 
REM Check for required tools 
where gcc >nul 2>&1 
if errorlevel 1 ( 
    echo [ERROR] GCC not found 
    exit /b 1 
) 
 
where nasm >nul 2>&1 
if errorlevel 1 ( 
    echo [ERROR] NASM not found 
    exit /b 1 
) 
 
REM Create build directories 
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%" 
if not exist "%BUILD_DIR%\kernel" mkdir "%BUILD_DIR%\kernel" 
 
REM Compile assembly entry point 
echo [INFO] Compiling kernel entry point... 
nasm -f elf64 "%KERNEL_DIR%\arch\x86_64\kernel_entry.S" -o "%BUILD_DIR%\kernel\kernel_entry.o" 
if errorlevel 1 ( 
    echo [ERROR] Failed to compile kernel entry point 
    exit /b 1 
) 
 
REM Compile kernel main 
echo [INFO] Compiling kernel main... 
gcc -c -m64 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra -I"%KERNEL_DIR%" -I"%KERNEL_DIR%\include" -I"%KERNEL_DIR%\core\include" "%KERNEL_DIR%\core\kernel_main.c" -o "%BUILD_DIR%\kernel\kernel_main.o" 
if errorlevel 1 ( 
    echo [ERROR] Failed to compile kernel main 
    exit /b 1 
) 
 
REM Link kernel 
echo [INFO] Linking kernel... 
ld -T "%KERNEL_DIR%\kernel_fixed.ld" -o "%BUILD_DIR%\kernel.bin" "%BUILD_DIR%\kernel\kernel_entry.o" "%BUILD_DIR%\kernel\kernel_main.o" 
if errorlevel 1 ( 
    echo [ERROR] Failed to link kernel 
    exit /b 1 
) 
 
echo [SUCCESS] Kernel built successfully: %BUILD_DIR%\kernel.bin 
exit /b 0 
