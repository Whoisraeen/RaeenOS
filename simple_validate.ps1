# RaeenOS Simple Validation Script
# This script validates the OS codebase structure and provides analysis

Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Blue
Write-Host "║                    RaeenOS Validation Suite                 ║" -ForegroundColor Blue
Write-Host "║                         Version 1.0                         ║" -ForegroundColor Blue
Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Blue
Write-Host ""

# Initialize counters
$TotalTests = 0
$PassedTests = 0

function Test-Component {
    param($Name, $Path, $Type = "Directory")
    
    $global:TotalTests++
    
    if (Test-Path $Path) {
        $global:PassedTests++
        Write-Host "  [PASS] $Name" -ForegroundColor Green
        return $true
    } else {
        Write-Host "  [FAIL] $Name" -ForegroundColor Red
        return $false
    }
}

function Get-FileStats {
    param($Path)
    
    if (Test-Path $Path) {
        $files = Get-ChildItem $Path -Recurse -File -ErrorAction SilentlyContinue
        $totalLines = 0
        
        foreach ($file in $files) {
            try {
                $lines = (Get-Content $file.FullName -ErrorAction SilentlyContinue).Count
                if ($lines) { $totalLines += $lines }
            } catch {
                # Skip files that can't be read
            }
        }
        
        return @{
            FileCount = $files.Count
            LineCount = $totalLines
        }
    }
    
    return @{ FileCount = 0; LineCount = 0 }
}

# Test 1: Project Structure
Write-Host "[1/6] Validating project structure..." -ForegroundColor Cyan

$RequiredDirs = @("bootloader", "kernel", "tools", "ui", "docs")
$RequiredFiles = @("README.md", "Makefile", "build.bat")

foreach ($dir in $RequiredDirs) {
    Test-Component "Directory: $dir" $dir
}

foreach ($file in $RequiredFiles) {
    Test-Component "File: $file" $file "File"
}

# Test 2: Bootloader Components
Write-Host "[2/6] Analyzing bootloader..." -ForegroundColor Cyan

$BootloaderFiles = @(
    "bootloader/boot.asm",
    "bootloader/stage1/stage1.asm", 
    "bootloader/stage2/stage2.asm",
    "bootloader/multiboot.c"
)

foreach ($file in $BootloaderFiles) {
    if (Test-Component "Bootloader: $file" $file "File") {
        $lines = (Get-Content $file -ErrorAction SilentlyContinue).Count
        Write-Host "    Lines: $lines" -ForegroundColor Gray
    }
}

# Test 3: Kernel Analysis
Write-Host "[3/6] Analyzing kernel components..." -ForegroundColor Cyan

$KernelDirs = @(
    "kernel/core",
    "kernel/memory", 
    "kernel/hal",
    "kernel/security",
    "kernel/process"
)

$KernelStats = @{}
foreach ($dir in $KernelDirs) {
    if (Test-Component "Kernel: $dir" $dir) {
        $stats = Get-FileStats $dir
        $KernelStats[$dir] = $stats
        Write-Host "    Files: $($stats.FileCount), Lines: $($stats.LineCount)" -ForegroundColor Gray
    }
}

# Test 4: Additional Kernel Modules
Write-Host "[4/6] Checking advanced kernel features..." -ForegroundColor Cyan

$AdvancedDirs = @(
    "kernel/drivers",
    "kernel/filesystem", 
    "kernel/graphics",
    "kernel/network",
    "kernel/ai",
    "kernel/security"
)

foreach ($dir in $AdvancedDirs) {
    if (Test-Path $dir) {
        $stats = Get-FileStats $dir
        Test-Component "Advanced: $dir" $dir
        Write-Host "    Files: $($stats.FileCount), Lines: $($stats.LineCount)" -ForegroundColor Gray
    }
}

# Test 5: Tools Analysis
Write-Host "[5/6] Analyzing development tools..." -ForegroundColor Cyan

$ToolsStats = Get-FileStats "tools"
Write-Host "  [INFO] Tools directory contains $($ToolsStats.FileCount) files with $($ToolsStats.LineCount) lines" -ForegroundColor Yellow

$ToolFiles = Get-ChildItem "tools" -Recurse -Include "*.c", "*.sh", "*.bat" -ErrorAction SilentlyContinue
foreach ($tool in $ToolFiles) {
    $lines = (Get-Content $tool.FullName -ErrorAction SilentlyContinue).Count
    Write-Host "    Tool: $($tool.Name) - $lines lines" -ForegroundColor Gray
}

# Test 6: Overall Codebase Statistics
Write-Host "[6/6] Calculating overall statistics..." -ForegroundColor Cyan

$AllFiles = Get-ChildItem -Recurse -Include "*.c", "*.h", "*.asm", "*.cpp" -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notlike "*build*" }

$TotalFiles = $AllFiles.Count
$TotalLines = 0
$CFiles = 0
$AsmFiles = 0
$HeaderFiles = 0

foreach ($file in $AllFiles) {
    try {
        $lines = (Get-Content $file.FullName -ErrorAction SilentlyContinue).Count
        if ($lines) { $TotalLines += $lines }
        
        switch ($file.Extension.ToLower()) {
            ".c" { $CFiles++ }
            ".cpp" { $CFiles++ }
            ".asm" { $AsmFiles++ }
            ".h" { $HeaderFiles++ }
        }
    } catch {
        # Skip problematic files
    }
}

Write-Host "  [INFO] Total codebase: $TotalFiles files, $TotalLines lines" -ForegroundColor Yellow
Write-Host "    - C/C++ files: $CFiles" -ForegroundColor Gray
Write-Host "    - Assembly files: $AsmFiles" -ForegroundColor Gray
Write-Host "    - Header files: $HeaderFiles" -ForegroundColor Gray

# Calculate and display results
$OverallScore = if ($TotalTests -gt 0) { [math]::Round(($PassedTests / $TotalTests) * 100, 2) } else { 0 }

Write-Host ""
Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Blue
Write-Host "║                      Validation Summary                      ║" -ForegroundColor Blue
Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Blue

Write-Host "Total Tests: $TotalTests"
Write-Host "Passed: $PassedTests" -ForegroundColor Green
Write-Host "Failed: $($TotalTests - $PassedTests)" -ForegroundColor Red
Write-Host "Overall Score: $OverallScore%" -ForegroundColor Yellow

if ($OverallScore -ge 80) {
    Write-Host "[EXCELLENT] RaeenOS codebase is well-structured!" -ForegroundColor Green
} elseif ($OverallScore -ge 60) {
    Write-Host "[GOOD] Good progress, some improvements needed." -ForegroundColor Yellow
} else {
    Write-Host "[NEEDS WORK] Significant work required." -ForegroundColor Red
}

Write-Host ""
Write-Host "RaeenOS Validation completed!" -ForegroundColor Cyan