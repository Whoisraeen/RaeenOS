@echo off
REM RaeenOS Build Environment Setup Script
REM This script helps set up the build environment for Windows

setlocal enabledelayedexpansion

echo ╔══════════════════════════════════════════════════════════════╗
echo ║              RaeenOS Build Environment Setup                ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

echo [INFO] This script will guide you through setting up the build environment.
echo [INFO] You have several options for installing GCC and NASM:
echo.
echo [OPTION 1] MSYS2 (Recommended)
echo   - Complete development environment
echo   - Easy package management
echo   - Integrated terminal
echo.
echo [OPTION 2] Standalone Tools
echo   - MinGW-w64 for GCC
echo   - NASM standalone installer
echo   - Manual PATH configuration
echo.
echo [OPTION 3] Chocolatey (Package Manager)
echo   - Automated installation
echo   - Easy updates
echo   - Requires Chocolatey to be installed
echo.

:menu
echo Please choose an option:
echo [1] Install via MSYS2 (Recommended)
echo [2] Install Standalone Tools
echo [3] Install via Chocolatey
echo [4] Check Current Environment
echo [5] Exit
echo.
set /p choice="Enter your choice (1-5): "

if "%choice%"=="1" goto :msys2_install
if "%choice%"=="2" goto :standalone_install
if "%choice%"=="3" goto :chocolatey_install
if "%choice%"=="4" goto :check_environment
if "%choice%"=="5" goto :exit
echo [ERROR] Invalid choice. Please try again.
goto :menu

:msys2_install
echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                    MSYS2 Installation                       ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.
echo [INFO] To install MSYS2:
echo.
echo 1. Download MSYS2 installer from: https://www.msys2.org/
echo 2. Run the installer and follow the setup wizard
echo 3. After installation, open MSYS2 terminal and run:
echo    pacman -Syu
echo 4. Install development tools:
echo    pacman -S mingw-w64-x86_64-gcc
echo    pacman -S mingw-w64-x86_64-gdb
echo    pacman -S nasm
echo    pacman -S make
echo.
echo 5. Add to Windows PATH:
echo    C:\msys64\mingw64\bin
echo    C:\msys64\usr\bin
echo.
echo [INFO] Would you like to open the MSYS2 website?
set /p open_site="Open MSYS2 website? (y/n): "
if /i "%open_site%"=="y" start https://www.msys2.org/
echo.
echo [INFO] After installing MSYS2, run this script again to verify.
pause
goto :menu

:standalone_install
echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                 Standalone Tools Installation                ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.
echo [INFO] To install standalone tools:
echo.
echo 1. Download MinGW-w64:
echo    https://www.mingw-w64.org/downloads/
echo    Recommended: w64devkit or TDM-GCC
echo.
echo 2. Download NASM:
echo    https://www.nasm.us/pub/nasm/releasebuilds/
echo    Get the Windows 64-bit version
echo.
echo 3. Extract both to C:\tools\ (or your preferred location)
echo.
echo 4. Add to Windows PATH:
echo    C:\tools\mingw64\bin
echo    C:\tools\nasm
echo.
echo [INFO] Would you like to open the download pages?
set /p open_downloads="Open download pages? (y/n): "
if /i "%open_downloads%"=="y" (
    start https://www.mingw-w64.org/downloads/
    start https://www.nasm.us/pub/nasm/releasebuilds/
)
echo.
echo [INFO] After installing tools, run this script again to verify.
pause
goto :menu

:chocolatey_install
echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                 Chocolatey Installation                     ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.
echo [INFO] Installing via Chocolatey (requires admin privileges):
echo.

REM Check if chocolatey is installed
where choco >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Chocolatey not found. Please install Chocolatey first:
    echo https://chocolatey.org/install
    echo.
    set /p install_choco="Open Chocolatey installation page? (y/n): "
    if /i "!install_choco!"=="y" start https://chocolatey.org/install
    pause
    goto :menu
)

echo [INFO] Chocolatey found. Installing build tools...
echo.
echo [INFO] This will run the following commands:
echo   choco install mingw -y
echo   choco install nasm -y
echo.
set /p proceed="Proceed with installation? (y/n): "
if /i "%proceed%"=="y" (
    echo [INFO] Installing MinGW-w64...
    choco install mingw -y
    echo [INFO] Installing NASM...
    choco install nasm -y
    echo [SUCCESS] Installation completed!
    echo [INFO] Please restart your command prompt to refresh PATH.
) else (
    echo [INFO] Installation cancelled.
)
pause
goto :menu

:check_environment
echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                Environment Check                             ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

echo [INFO] Checking for required tools...
echo.

REM Check GCC
where gcc >nul 2>&1
if errorlevel 1 (
    echo [ERROR] ✗ GCC not found
    set gcc_found=false
) else (
    echo [SUCCESS] ✓ GCC found:
    gcc --version | findstr "gcc"
    set gcc_found=true
)

echo.

REM Check NASM
where nasm >nul 2>&1
if errorlevel 1 (
    echo [ERROR] ✗ NASM not found
    set nasm_found=false
) else (
    echo [SUCCESS] ✓ NASM found:
    nasm -v
    set nasm_found=true
)

echo.

REM Check Make (optional)
where make >nul 2>&1
if errorlevel 1 (
    echo [WARNING] ✗ Make not found (optional)
    set make_found=false
) else (
    echo [SUCCESS] ✓ Make found:
    make --version | findstr "GNU Make"
    set make_found=true
)

echo.

if "%gcc_found%"=="true" if "%nasm_found%"=="true" (
    echo ╔══════════════════════════════════════════════════════════════╗
    echo ║                 ENVIRONMENT READY                           ║
    echo ╚══════════════════════════════════════════════════════════════╝
    echo.
    echo [SUCCESS] Build environment is ready!
    echo [INFO] You can now compile the RaeenOS kernel using:
    echo   .\build.bat kernel
    echo.
    echo [INFO] Or run the integration test:
    echo   .\test_kernel_integration.bat
    echo.
    set /p test_now="Run kernel integration test now? (y/n): "
    if /i "!test_now!"=="y" (
        echo.
        echo [INFO] Starting kernel integration test...
        call "%~dp0test_kernel_integration.bat"
    )
) else (
    echo ╔══════════════════════════════════════════════════════════════╗
    echo ║                ENVIRONMENT NOT READY                        ║
    echo ╚══════════════════════════════════════════════════════════════╝
    echo.
    echo [ERROR] Build environment is not complete.
    echo [INFO] Please install missing tools using one of the options above.
)

pause
goto :menu

:exit
echo.
echo [INFO] Setup script exiting.
echo [INFO] For detailed instructions, see BUILD_ENVIRONMENT_SETUP.md
exit /b 0