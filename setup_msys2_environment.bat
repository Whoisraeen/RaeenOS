@echo off
REM MSYS2 Environment Setup for RaeenOS
REM This script helps set up MSYS2 with all required tools

setlocal enabledelayedexpansion

echo ╔══════════════════════════════════════════════════════════════╗
echo ║                MSYS2 Setup for RaeenOS                      ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

REM Check if MSYS2 is installed
set MSYS2_PATH=C:\msys64
if not exist "%MSYS2_PATH%" (
    set MSYS2_PATH=C:\msys2
)
if not exist "%MSYS2_PATH%" (
    echo [ERROR] MSYS2 not found in standard locations
    echo [INFO] Please install MSYS2 from: https://www.msys2.org/
    echo [INFO] Or specify your MSYS2 installation path
    pause
    exit /b 1
)

echo [INFO] Found MSYS2 at: %MSYS2_PATH%
echo.

REM Create MSYS2 setup script
echo [INFO] Creating MSYS2 setup script...

echo #!/bin/bash > "%MSYS2_PATH%\setup_raeenos.sh"
echo # RaeenOS MSYS2 Setup Script >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "Setting up MSYS2 environment for RaeenOS..." >> "%MSYS2_PATH%\setup_raeenos.sh"
echo. >> "%MSYS2_PATH%\setup_raeenos.sh"
echo # Update package database >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "[INFO] Updating package database..." >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -Sy --noconfirm >> "%MSYS2_PATH%\setup_raeenos.sh"
echo. >> "%MSYS2_PATH%\setup_raeenos.sh"
echo # Install base development tools >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "[INFO] Installing base development tools..." >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -S --noconfirm base-devel >> "%MSYS2_PATH%\setup_raeenos.sh"
echo. >> "%MSYS2_PATH%\setup_raeenos.sh"
echo # Install GCC toolchain >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "[INFO] Installing GCC toolchain..." >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -S --noconfirm mingw-w64-x86_64-gcc >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -S --noconfirm mingw-w64-x86_64-binutils >> "%MSYS2_PATH%\setup_raeenos.sh"
echo. >> "%MSYS2_PATH%\setup_raeenos.sh"
echo # Install NASM >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "[INFO] Installing NASM..." >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -S --noconfirm nasm >> "%MSYS2_PATH%\setup_raeenos.sh"
echo. >> "%MSYS2_PATH%\setup_raeenos.sh"
echo # Install GRUB and ISO tools >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "[INFO] Installing GRUB and ISO creation tools..." >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -S --noconfirm grub >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -S --noconfirm xorriso >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -S --noconfirm mtools >> "%MSYS2_PATH%\setup_raeenos.sh"
echo. >> "%MSYS2_PATH%\setup_raeenos.sh"
echo # Install additional useful tools >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "[INFO] Installing additional tools..." >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -S --noconfirm make >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -S --noconfirm git >> "%MSYS2_PATH%\setup_raeenos.sh"
echo pacman -S --noconfirm vim >> "%MSYS2_PATH%\setup_raeenos.sh"
echo. >> "%MSYS2_PATH%\setup_raeenos.sh"
echo # Verify installations >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "[INFO] Verifying installations..." >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "GCC version:" >> "%MSYS2_PATH%\setup_raeenos.sh"
echo gcc --version >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "NASM version:" >> "%MSYS2_PATH%\setup_raeenos.sh"
echo nasm -v >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "GRUB version:" >> "%MSYS2_PATH%\setup_raeenos.sh"
echo grub-mkrescue --version >> "%MSYS2_PATH%\setup_raeenos.sh"
echo. >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "[SUCCESS] MSYS2 environment setup complete!" >> "%MSYS2_PATH%\setup_raeenos.sh"
echo echo "You can now build RaeenOS using the provided scripts." >> "%MSYS2_PATH%\setup_raeenos.sh"

echo [SUCCESS] Setup script created: %MSYS2_PATH%\setup_raeenos.sh
echo.

REM Create Windows batch file to launch MSYS2 with proper environment
echo [INFO] Creating MSYS2 launcher script...

echo @echo off > "launch_msys2_raeenos.bat"
echo REM Launch MSYS2 with RaeenOS environment >> "launch_msys2_raeenos.bat"
echo echo Starting MSYS2 for RaeenOS development... >> "launch_msys2_raeenos.bat"
echo echo. >> "launch_msys2_raeenos.bat"
echo echo [INFO] This will open MSYS2 terminal in the RaeenOS directory >> "launch_msys2_raeenos.bat"
echo echo [INFO] Run the following commands to set up the environment: >> "launch_msys2_raeenos.bat"
echo echo. >> "launch_msys2_raeenos.bat"
echo echo   1. bash /setup_raeenos.sh >> "launch_msys2_raeenos.bat"
echo echo   2. cd /d/RaeenOS >> "launch_msys2_raeenos.bat"
echo echo   3. ./build_fixed.bat >> "launch_msys2_raeenos.bat"
echo echo. >> "launch_msys2_raeenos.bat"
echo pause >> "launch_msys2_raeenos.bat"
echo start "" "%MSYS2_PATH%\msys2.exe" >> "launch_msys2_raeenos.bat"

echo [SUCCESS] Launcher created: launch_msys2_raeenos.bat
echo.

echo ╔══════════════════════════════════════════════════════════════╗
echo ║                    SETUP INSTRUCTIONS                       ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.
echo [STEP 1] Run the MSYS2 launcher:
echo   .\launch_msys2_raeenos.bat
echo.
echo [STEP 2] In the MSYS2 terminal, run:
echo   bash /setup_raeenos.sh
echo.
echo [STEP 3] Navigate to RaeenOS directory:
echo   cd /d/RaeenOS
echo.
echo [STEP 4] Fix the pacman command (you had a typo):
echo   pacman -S grub xorriso mtools
echo   (Note: Use -S not _S)
echo.
echo [STEP 5] Build the kernel:
echo   ./build_fixed.bat
echo.
echo [STEP 6] Create ISO:
echo   ./tools/create_iso.sh
echo.

echo [TROUBLESHOOTING]
echo - If pacman commands fail, try: pacman -Syu first
echo - If paths don't work, use Windows-style paths: /c/msys64/...
echo - For permission issues, run MSYS2 as administrator
echo.

echo [SUCCESS] MSYS2 setup files created successfully!
echo [INFO] You can now use MSYS2 to build RaeenOS