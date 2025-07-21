# RaeenOS Test Script
# This script validates the RaeenOS codebase and checks for common issues

Write-Host "=== RaeenOS Codebase Validation Test ===" -ForegroundColor Green
Write-Host ""

# Test 1: Check file structure
Write-Host "1. Checking file structure..." -ForegroundColor Yellow
$required_dirs = @(
    "bootloader",
    "kernel",
    "kernel/core",
    "kernel/drivers",
    "kernel/filesystem",
    "kernel/memory",
    "kernel/process",
    "kernel/network",
    "kernel/gui",
    "kernel/security"
)

$missing_dirs = @()
foreach ($dir in $required_dirs) {
    if (-not (Test-Path $dir)) {
        $missing_dirs += $dir
    }
}

if ($missing_dirs.Count -eq 0) {
    Write-Host "   ✓ All required directories present" -ForegroundColor Green
} else {
    Write-Host "   ✗ Missing directories: $($missing_dirs -join ', ')" -ForegroundColor Red
}

# Test 2: Check critical files
Write-Host "2. Checking critical files..." -ForegroundColor Yellow
$critical_files = @(
    "bootloader/boot.asm",
    "bootloader/stage2/stage2.asm",
    "bootloader/Makefile",
    "kernel/kernel.ld",
    "kernel/Makefile",
    "kernel/arch/x86_64/kernel_entry.S",
    "kernel/core/kernel_main.c",
    "kernel/core/include/kernel.h",
    "kernel/drivers/ata.c",
    "kernel/drivers/ahci.c",
    "kernel/drivers/nvme.c",
    "kernel/drivers/usb.c",
    "kernel/filesystem/fat32.c",
    "kernel/filesystem/ntfs.c",
    "kernel/filesystem/ext4.c",
    "kernel/network/tcpip.c",
    "kernel/drivers/gpu.c",
    "kernel/drivers/network.c"
)

$missing_files = @()
foreach ($file in $critical_files) {
    if (-not (Test-Path $file)) {
        $missing_files += $file
    }
}

if ($missing_files.Count -eq 0) {
    Write-Host "   ✓ All critical files present" -ForegroundColor Green
} else {
    Write-Host "   ✗ Missing files: $($missing_files.Count) files" -ForegroundColor Red
    foreach ($file in $missing_files) {
        Write-Host "     - $file" -ForegroundColor Red
    }
}

# Test 3: Check for syntax errors in assembly files
Write-Host "3. Checking assembly syntax..." -ForegroundColor Yellow
$asm_files = @(
    "bootloader/boot.asm",
    "bootloader/stage2/stage2.asm",
    "kernel/arch/x86_64/kernel_entry.S"
)

$asm_errors = @()
foreach ($file in $asm_files) {
    if (Test-Path $file) {
        $content = Get-Content $file -Raw
        # Check for common assembly syntax issues
        if ($content -match "\[org 0x[0-9A-Fa-f]+\]" -and $content -match "\[bits 16\]") {
            # Basic structure looks good
        } else {
            $asm_errors += "$file - Basic structure issues"
        }
        
        # Check for missing labels
        if ($content -match "kernel_load_error" -and $content -notmatch "kernel_load_error:") {
            $asm_errors += "$file - Missing kernel_load_error label"
        }
    }
}

if ($asm_errors.Count -eq 0) {
    Write-Host "   ✓ Assembly files look syntactically correct" -ForegroundColor Green
} else {
    Write-Host "   ✗ Assembly syntax issues found:" -ForegroundColor Red
    foreach ($error in $asm_errors) {
        Write-Host "     - $error" -ForegroundColor Red
    }
}

# Test 4: Check C source code structure
Write-Host "4. Checking C source code structure..." -ForegroundColor Yellow
$c_files = Get-ChildItem -Path "kernel" -Recurse -Filter "*.c" | Select-Object -First 10
$c_errors = @()

foreach ($file in $c_files) {
    $content = Get-Content $file.FullName -Raw
    if ($content -match "int main" -or $content -match "void main") {
        $c_errors += "$($file.Name) - Should not have main function"
    }
    
    # Check for proper includes
    if ($content -notmatch "#include" -and $content.Length -gt 100) {
        $c_errors += "$($file.Name) - Missing includes"
    }
}

if ($c_errors.Count -eq 0) {
    Write-Host "   ✓ C source code structure looks good" -ForegroundColor Green
} else {
    Write-Host "   ✗ C source code issues found:" -ForegroundColor Red
    foreach ($error in $c_errors) {
        Write-Host "     - $error" -ForegroundColor Red
    }
}

# Test 5: Check for development tools
Write-Host "5. Checking development tools..." -ForegroundColor Yellow
$tools = @{
    "GCC" = "gcc"
    "NASM" = "nasm"
    "Make" = "make"
    "LD" = "ld"
}

$missing_tools = @()
foreach ($tool in $tools.Keys) {
    $command = $tools[$tool]
    try {
        $null = Get-Command $command -ErrorAction Stop
        Write-Host "   ✓ $tool available" -ForegroundColor Green
    } catch {
        $missing_tools += $tool
        Write-Host "   ✗ $tool not found" -ForegroundColor Red
    }
}

# Test 6: Check file sizes and complexity
Write-Host "6. Checking code complexity..." -ForegroundColor Yellow
$large_files = @()
$total_lines = 0

Get-ChildItem -Path "kernel" -Recurse -Filter "*.c" | ForEach-Object {
    $lines = (Get-Content $_.FullName).Count
    $total_lines += $lines
    if ($lines -gt 1000) {
        $large_files += "$($_.Name) ($lines lines)"
    }
}

Write-Host "   Total C source lines: $total_lines" -ForegroundColor Cyan
if ($large_files.Count -gt 0) {
    Write-Host "   Large files found:" -ForegroundColor Yellow
    foreach ($file in $large_files) {
        Write-Host "     - $file" -ForegroundColor Yellow
    }
} else {
    Write-Host "   ✓ No excessively large files" -ForegroundColor Green
}

# Test 7: Check for TODO items
Write-Host "7. Checking for incomplete implementations..." -ForegroundColor Yellow
$todo_files = @()
Get-ChildItem -Path "kernel" -Recurse -Filter "*.c" | ForEach-Object {
    $content = Get-Content $_.FullName -Raw
    if ($content -match "TODO" -or $content -match "FIXME" -or $content -match "NOT IMPLEMENTED") {
        $todo_files += $_.Name
    }
}

if ($todo_files.Count -eq 0) {
    Write-Host "   ✓ No obvious TODO items found" -ForegroundColor Green
} else {
    Write-Host "   ⚠ Files with TODO items: $($todo_files.Count)" -ForegroundColor Yellow
    foreach ($file in $todo_files[0..4]) { # Show first 5
        Write-Host "     - $file" -ForegroundColor Yellow
    }
}

# Summary
Write-Host ""
Write-Host "=== Test Summary ===" -ForegroundColor Green

$total_issues = $missing_dirs.Count + $missing_files.Count + $asm_errors.Count + $c_errors.Count + $missing_tools.Count

if ($total_issues -eq 0) {
    Write-Host "🎉 All tests passed! RaeenOS codebase is ready for compilation." -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Cyan
    Write-Host "1. Install development tools (GCC, NASM, Make)" -ForegroundColor White
    Write-Host "2. Run: cd bootloader && make" -ForegroundColor White
    Write-Host "3. Run: cd kernel && make" -ForegroundColor White
    Write-Host "4. Test with QEMU or real hardware" -ForegroundColor White
} else {
    Write-Host "⚠ Found $total_issues issues that need to be addressed." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Priority fixes:" -ForegroundColor Cyan
    if ($missing_tools.Count -gt 0) {
        Write-Host "1. Install missing development tools" -ForegroundColor White
    }
    if ($missing_files.Count -gt 0) {
        Write-Host "2. Create missing critical files" -ForegroundColor White
    }
    if ($asm_errors.Count -gt 0) {
        Write-Host "3. Fix assembly syntax errors" -ForegroundColor White
    }
}

Write-Host ""
Write-Host "RaeenOS Test Complete!" -ForegroundColor Green 