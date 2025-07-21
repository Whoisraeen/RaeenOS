@echo off
REM RaeenOS Boot Sequence Test Script
REM This script helps test the kernel boot sequence using QEMU or other emulators

setlocal enabledelayedexpansion

set PROJECT_ROOT=%~dp0
set BUILD_DIR=%PROJECT_ROOT%build
set KERNEL_BIN=%BUILD_DIR%\kernel.bin

echo ╔══════════════════════════════════════════════════════════════╗
echo ║                RaeenOS Boot Sequence Test                   ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

REM Check if kernel binary exists
if not exist "%KERNEL_BIN%" (
    echo [ERROR] Kernel binary not found: %KERNEL_BIN%
    echo [INFO] Please compile the kernel first using: .\build.bat kernel
    goto :error_exit
)

echo [INFO] Kernel binary found: %KERNEL_BIN%
for %%A in ("%KERNEL_BIN%") do set KERNEL_SIZE=%%~zA
echo [INFO] Kernel size: %KERNEL_SIZE% bytes
echo.

:menu
echo Boot testing options:
echo [1] Test with QEMU (x86_64)
echo [2] Test with QEMU (i386)
echo [3] Create VirtualBox VM script
echo [4] Create VMware VM script
echo [5] Boot sequence analysis
echo [6] Exit
echo.
set /p choice="Enter your choice (1-6): "

if "%choice%"=="1" goto :qemu_x64
if "%choice%"=="2" goto :qemu_i386
if "%choice%"=="3" goto :virtualbox_script
if "%choice%"=="4" goto :vmware_script
if "%choice%"=="5" goto :boot_analysis
if "%choice%"=="6" goto :exit
echo [ERROR] Invalid choice. Please try again.
goto :menu

:qemu_x64
echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                    QEMU x86_64 Test                         ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

REM Check if QEMU is available
where qemu-system-x86_64 >nul 2>&1
if errorlevel 1 (
    echo [ERROR] QEMU x86_64 not found in PATH
    echo [INFO] Please install QEMU from: https://www.qemu.org/download/
    echo [INFO] Or use: choco install qemu
    set /p open_qemu="Open QEMU download page? (y/n): "
    if /i "!open_qemu!"=="y" start https://www.qemu.org/download/
    pause
    goto :menu
)

echo [INFO] Starting QEMU x86_64 with RaeenOS kernel...
echo [INFO] QEMU parameters:
echo   - Architecture: x86_64
echo   - Memory: 512MB
echo   - Kernel: %KERNEL_BIN%
echo   - Serial output: stdio
echo   - Monitor: stdio
echo.
echo [INFO] Press Ctrl+Alt+G to release mouse, Ctrl+Alt+2 for monitor
echo [INFO] Use 'quit' in monitor to exit QEMU
echo.
pause

qemu-system-x86_64 -kernel "%KERNEL_BIN%" -m 512 -serial stdio -monitor stdio -no-reboot -no-shutdown

echo.
echo [INFO] QEMU session ended.
pause
goto :menu

:qemu_i386
echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                     QEMU i386 Test                          ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

REM Check if QEMU is available
where qemu-system-i386 >nul 2>&1
if errorlevel 1 (
    echo [ERROR] QEMU i386 not found in PATH
    echo [INFO] Please install QEMU from: https://www.qemu.org/download/
    pause
    goto :menu
)

echo [INFO] Starting QEMU i386 with RaeenOS kernel...
echo [INFO] QEMU parameters:
echo   - Architecture: i386
echo   - Memory: 256MB
echo   - Kernel: %KERNEL_BIN%
echo   - Serial output: stdio
echo.
pause

qemu-system-i386 -kernel "%KERNEL_BIN%" -m 256 -serial stdio -monitor stdio -no-reboot -no-shutdown

echo.
echo [INFO] QEMU session ended.
pause
goto :menu

:virtualbox_script
echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                  VirtualBox VM Creation                     ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

set VM_NAME=RaeenOS-Test
set VM_SCRIPT=%PROJECT_ROOT%create_virtualbox_vm.bat

echo [INFO] Creating VirtualBox VM script...

echo @echo off > "%VM_SCRIPT%"
echo REM VirtualBox VM Creation Script for RaeenOS >> "%VM_SCRIPT%"
echo. >> "%VM_SCRIPT%"
echo set VM_NAME=%VM_NAME% >> "%VM_SCRIPT%"
echo set KERNEL_BIN=%KERNEL_BIN% >> "%VM_SCRIPT%"
echo. >> "%VM_SCRIPT%"
echo echo [INFO] Creating VirtualBox VM for RaeenOS... >> "%VM_SCRIPT%"
echo. >> "%VM_SCRIPT%"
echo REM Create VM >> "%VM_SCRIPT%"
echo VBoxManage createvm --name "%%VM_NAME%%" --ostype "Other" --register >> "%VM_SCRIPT%"
echo. >> "%VM_SCRIPT%"
echo REM Configure VM >> "%VM_SCRIPT%"
echo VBoxManage modifyvm "%%VM_NAME%%" --memory 512 --vram 16 >> "%VM_SCRIPT%"
echo VBoxManage modifyvm "%%VM_NAME%%" --cpus 1 --pae off >> "%VM_SCRIPT%"
echo VBoxManage modifyvm "%%VM_NAME%%" --boot1 floppy --boot2 none --boot3 none --boot4 none >> "%VM_SCRIPT%"
echo. >> "%VM_SCRIPT%"
echo echo [INFO] VM created. To boot RaeenOS: >> "%VM_SCRIPT%"
echo echo 1. Copy kernel.bin to a floppy image >> "%VM_SCRIPT%"
echo echo 2. Attach floppy image to VM >> "%VM_SCRIPT%"
echo echo 3. Start VM >> "%VM_SCRIPT%"
echo. >> "%VM_SCRIPT%"
echo echo [WARNING] This requires a bootloader to work properly >> "%VM_SCRIPT%"
echo pause >> "%VM_SCRIPT%"

echo [SUCCESS] VirtualBox script created: %VM_SCRIPT%
echo [INFO] Run the script to create a VirtualBox VM
echo [WARNING] Note: VirtualBox requires a proper bootloader
pause
goto :menu

:vmware_script
echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                   VMware VM Creation                        ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

set VMX_FILE=%PROJECT_ROOT%RaeenOS-Test.vmx

echo [INFO] Creating VMware VM configuration...

echo # VMware VM Configuration for RaeenOS > "%VMX_FILE%"
echo .encoding = "UTF-8" >> "%VMX_FILE%"
echo config.version = "8" >> "%VMX_FILE%"
echo virtualHW.version = "19" >> "%VMX_FILE%"
echo. >> "%VMX_FILE%"
echo # VM Settings >> "%VMX_FILE%"
echo displayName = "RaeenOS Test" >> "%VMX_FILE%"
echo guestOS = "other" >> "%VMX_FILE%"
echo. >> "%VMX_FILE%"
echo # Hardware Configuration >> "%VMX_FILE%"
echo memsize = "512" >> "%VMX_FILE%"
echo numvcpus = "1" >> "%VMX_FILE%"
echo. >> "%VMX_FILE%"
echo # Boot Configuration >> "%VMX_FILE%"
echo floppy0.present = "TRUE" >> "%VMX_FILE%"
echo floppy0.fileType = "file" >> "%VMX_FILE%"
echo floppy0.fileName = "kernel.img" >> "%VMX_FILE%"
echo. >> "%VMX_FILE%"
echo # Other Settings >> "%VMX_FILE%"
echo ethernet0.present = "FALSE" >> "%VMX_FILE%"
echo sound.present = "FALSE" >> "%VMX_FILE%"
echo usb.present = "FALSE" >> "%VMX_FILE%"

echo [SUCCESS] VMware configuration created: %VMX_FILE%
echo [INFO] To use with VMware:
echo   1. Create a floppy image with the kernel
echo   2. Open the .vmx file in VMware
echo   3. Start the VM
echo [WARNING] Note: VMware requires a proper bootloader
pause
goto :menu

:boot_analysis
echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                  Boot Sequence Analysis                     ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

echo [INFO] Analyzing kernel boot sequence...
echo.

REM Check kernel_main.c for initialization order
set KERNEL_MAIN=%PROJECT_ROOT%kernel\core\kernel_main.c

if not exist "%KERNEL_MAIN%" (
    echo [ERROR] kernel_main.c not found
    pause
    goto :menu
)

echo [INFO] Expected boot sequence from kernel_main.c:
echo.

findstr /n "init\|handoff" "%KERNEL_MAIN%" | findstr /v "//" | head -20

echo.
echo [INFO] Key initialization functions that should be called:
echo   1. bootloader_handoff_init() - Initialize handoff
echo   2. hal_init() - Hardware Abstraction Layer
echo   3. memory_init() - Memory management
echo   4. interrupts_init() - Interrupt handling
echo   5. timer_init() - System timer
echo   6. syscall_init() - System calls
echo   7. security_init() - Security subsystem
echo   8. process_init() - Process management
echo   9. scheduler_init() - Task scheduler
echo  10. keyboard_init() - Keyboard driver
echo  11. vga_init() - VGA graphics
echo  12. Additional subsystems...
echo  13. bootloader_handoff_complete() - Complete handoff
echo.

echo [INFO] During boot testing, verify:
echo   ✓ Each initialization function completes successfully
echo   ✓ No kernel panics or crashes
echo   ✓ Memory management is working
echo   ✓ Interrupts are properly configured
echo   ✓ Basic I/O is functional
echo.

pause
goto :menu

:error_exit
echo.
echo [ERROR] Boot testing cannot proceed.
pause
exit /b 1

:exit
echo.
echo [INFO] Boot testing script exiting.
exit /b 0