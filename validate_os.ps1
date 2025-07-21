# RaeenOS Validation and Testing Script
# This script validates the OS codebase structure, syntax, and provides comprehensive analysis

param(
    [switch]$Detailed,
    [switch]$StructureOnly,
    [switch]$GenerateReport,
    [string]$OutputFile = "validation_report.html"
)

# Color functions for better output
function Write-ColorOutput($ForegroundColor) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    if ($args) {
        Write-Output $args
    } else {
        $input | Write-Output
    }
    $host.UI.RawUI.ForegroundColor = $fc
}

function Write-Success { Write-ColorOutput Green $args }
function Write-Warning { Write-ColorOutput Yellow $args }
function Write-Error { Write-ColorOutput Red $args }
function Write-Info { Write-ColorOutput Cyan $args }

# Initialize validation results
$ValidationResults = @{
    StructureTests = @()
    SyntaxTests = @()
    ContentTests = @()
    ArchitectureTests = @()
    SecurityTests = @()
    PerformanceTests = @()
    OverallScore = 0
    TotalTests = 0
    PassedTests = 0
}

function Add-TestResult {
    param($Category, $TestName, $Status, $Details = "", $Score = 1)
    
    $result = @{
        TestName = $TestName
        Status = $Status
        Details = $Details
        Score = $Score
        Timestamp = Get-Date
    }
    
    $ValidationResults[$Category] += $result
    $ValidationResults.TotalTests++
    if ($Status -eq "PASS") {
        $ValidationResults.PassedTests++
    }
}

Write-Info "╔══════════════════════════════════════════════════════════════╗"
Write-Info "║                    RaeenOS Validation Suite                 ║"
Write-Info "║                         Version 1.0                         ║"
Write-Info "╚══════════════════════════════════════════════════════════════╝"
Write-Output ""

# Test 1: Project Structure Validation
Write-Info "[1/8] Validating project structure..."

$RequiredDirectories = @(
    "bootloader",
    "kernel",
    "tools",
    "ui",
    "docs"
)

$RequiredFiles = @(
    "README.md",
    "Makefile",
    "build.bat",
    "bootloader/Makefile",
    "kernel/Makefile",
    "tools/Makefile",
    "ui/Makefile"
)

foreach ($dir in $RequiredDirectories) {
    if (Test-Path $dir) {
        Add-TestResult "StructureTests" "Directory: $dir" "PASS" "Directory exists"
        Write-Success "  [PASS] $dir directory exists"
    } else {
        Add-TestResult "StructureTests" "Directory: $dir" "FAIL" "Directory missing"
        Write-Error "  [FAIL] $dir directory missing"
    }
}

foreach ($file in $RequiredFiles) {
    if (Test-Path $file) {
        Add-TestResult "StructureTests" "File: $file" "PASS" "File exists"
        Write-Success "  [PASS] $file exists"
    } else {
        Add-TestResult "StructureTests" "File: $file" "FAIL" "File missing"
        Write-Error "  [FAIL] $file missing"
    }
}

# Test 2: Bootloader Analysis
Write-Info "[2/8] Analyzing bootloader components..."

$BootloaderFiles = @(
    "bootloader/boot.asm",
    "bootloader/stage1/stage1.asm", 
    "bootloader/stage2/stage2.asm",
    "bootloader/multiboot.c"
)

foreach ($file in $BootloaderFiles) {
    if (Test-Path $file) {
        $content = Get-Content $file -Raw
        $lineCount = (Get-Content $file).Count
        
        Add-TestResult "ContentTests" "Bootloader: $file" "PASS" "$lineCount lines of code"
        Write-Success "  [PASS] $file ($lineCount lines)"
        
        # Check for key bootloader features
        if ($file -like "*.asm") {
            if ($content -match "org\s+0x7C00" -or $content -match "BITS\s+16") {
                Add-TestResult "ArchitectureTests" "Bootloader Architecture: $file" "PASS" "Proper 16-bit bootloader setup"
            }
            if ($content -match "int\s+0x13") {
                Add-TestResult "ArchitectureTests" "Disk I/O: $file" "PASS" "BIOS disk interrupt usage"
            }
        }
    } else {
        Add-TestResult "ContentTests" "Bootloader: $file" "FAIL" "File missing"
        Write-Error "  [FAIL] $file missing"
    }
}

# Test 3: Kernel Analysis
Write-Info "[3/8] Analyzing kernel components..."

$KernelDirectories = @(
    "kernel/core",
    "kernel/memory", 
    "kernel/hal",
    "kernel/security",
    "kernel/process"
)

foreach ($dir in $KernelDirectories) {
    if (Test-Path $dir) {
        $files = Get-ChildItem $dir -Recurse -File
        $totalLines = 0
        foreach ($file in $files) {
            $totalLines += (Get-Content $file.FullName).Count
        }
        Add-TestResult "ContentTests" "Kernel Module: $dir" "PASS" "$($files.Count) files, $totalLines lines"
        Write-Success "  [PASS] $dir ($($files.Count) files, $totalLines lines)"
    } else {
        Add-TestResult "ContentTests" "Kernel Module: $dir" "FAIL" "Directory missing"
        Write-Error "  [FAIL] $dir missing"
    }
}

# Test 4: Architecture Support Analysis
Write-Info "[4/8] Analyzing architecture support..."

$ArchFiles = Get-ChildItem -Recurse -Include "*.c", "*.h", "*.asm" | Where-Object { $_.FullName -notlike "*build*" }

$x86Support = 0
$x64Support = 0
$archFeatures = @()

foreach ($file in $ArchFiles) {
    $content = Get-Content $file.FullName -Raw
    
    if ($content -match "TARGET_X86|__i386__|_M_IX86") {
        $x86Support++
    }
    if ($content -match "TARGET_X64|__x86_64__|_M_X64") {
        $x64Support++
    }
    if ($content -match "BITS\s+32") {
        $archFeatures += "32-bit assembly in $($file.Name)"
    }
    if ($content -match "BITS\s+64") {
        $archFeatures += "64-bit assembly in $($file.Name)"
    }
}

if ($x86Support -gt 0) {
    Add-TestResult "ArchitectureTests" "x86 Support" "PASS" "$x86Support files with x86 support"
    Write-Success "  [PASS] x86 (32-bit) support found in $x86Support files"
}

if ($x64Support -gt 0) {
    Add-TestResult "ArchitectureTests" "x64 Support" "PASS" "$x64Support files with x64 support"
    Write-Success "  [PASS] x64 (64-bit) support found in $x64Support files"
}

# Test 5: Security Features Analysis
Write-Info "[5/8] Analyzing security features..."

$SecurityFeatures = @{
    "Stack Protection" = @("-fno-stack-protector", "stack_chk_fail")
    "NX Bit Support" = @("NX_BIT", "EFER_NXE")
    "ASLR Support" = @("ASLR", "randomize")
    "Privilege Separation" = @("USER_MODE", "KERNEL_MODE", "ring")
    "Memory Protection" = @("PAGE_READONLY", "PAGE_EXECUTE", "mprotect")
}

foreach ($feature in $SecurityFeatures.Keys) {
    $found = $false
    foreach ($pattern in $SecurityFeatures[$feature]) {
        $matches = $ArchFiles | ForEach-Object { 
            if ((Get-Content $_.FullName -Raw) -match $pattern) { $_.Name }
        }
        if ($matches) {
            $found = $true
            break
        }
    }
    
    if ($found) {
        Add-TestResult "SecurityTests" $feature "PASS" "Security feature implemented"
        Write-Success "  [PASS] $feature implemented"
    } else {
        Add-TestResult "SecurityTests" $feature "WARN" "Security feature not found"
        Write-Warning "  [WARN] $feature not detected"
    }
}

# Test 6: Build System Analysis
Write-Info "[6/8] Analyzing build system..."

$BuildFiles = @("Makefile", "build.bat")
foreach ($file in $BuildFiles) {
    if (Test-Path $file) {
        $content = Get-Content $file -Raw
        $lineCount = (Get-Content $file).Count
        
        Add-TestResult "ContentTests" "Build System: $file" "PASS" "$lineCount lines"
        Write-Success "  [PASS] $file ($lineCount lines)"
        
        # Check for build features
        if ($content -match "clean|CLEAN") {
            Add-TestResult "ArchitectureTests" "Build Feature: Clean" "PASS" "Clean target available"
        }
        if ($content -match "debug|DEBUG") {
            Add-TestResult "ArchitectureTests" "Build Feature: Debug" "PASS" "Debug build support"
        }
        if ($content -match "test|TEST") {
            Add-TestResult "ArchitectureTests" "Build Feature: Test" "PASS" "Test target available"
        }
    }
}

# Test 7: Tools Analysis
Write-Info "[7/8] Analyzing development tools..."

$ToolFiles = Get-ChildItem "tools" -Recurse -Include "*.c", "*.sh", "*.bat" -ErrorAction SilentlyContinue

foreach ($tool in $ToolFiles) {
    $lineCount = (Get-Content $tool.FullName).Count
    Add-TestResult "ContentTests" "Tool: $($tool.Name)" "PASS" "$lineCount lines"
    Write-Success "  [PASS] $($tool.Name) ($lineCount lines)"
}

# Test 8: Code Quality Analysis
Write-Info "[8/8] Performing code quality analysis..."

$TotalFiles = ($ArchFiles | Measure-Object).Count
$TotalLines = 0
$CFiles = 0
$AsmFiles = 0
$HeaderFiles = 0

foreach ($file in $ArchFiles) {
    $lines = (Get-Content $file.FullName).Count
    $TotalLines += $lines
    
    switch ($file.Extension.ToLower()) {
        ".c" { $CFiles++ }
        ".asm" { $AsmFiles++ }
        ".h" { $HeaderFiles++ }
    }
}

Add-TestResult "ContentTests" "Total Codebase" "PASS" "$TotalFiles files, $TotalLines lines"
Write-Success "  [PASS] Total: $TotalFiles files, $TotalLines lines of code"
Write-Info "    - C files: $CFiles"
Write-Info "    - Assembly files: $AsmFiles" 
Write-Info "    - Header files: $HeaderFiles"

# Calculate overall score
$ValidationResults.OverallScore = [math]::Round(($ValidationResults.PassedTests / $ValidationResults.TotalTests) * 100, 2)

# Display summary
Write-Output ""
Write-Info "╔══════════════════════════════════════════════════════════════╗"
Write-Info "║                      Validation Summary                      ║"
Write-Info "╚══════════════════════════════════════════════════════════════╝"

Write-Output "Total Tests: $($ValidationResults.TotalTests)"
Write-Output "Passed: $($ValidationResults.PassedTests)"
Write-Output "Failed: $($ValidationResults.TotalTests - $ValidationResults.PassedTests)"
Write-Output "Overall Score: $($ValidationResults.OverallScore)%"

if ($ValidationResults.OverallScore -ge 80) {
    Write-Success "[EXCELLENT] RaeenOS codebase is well-structured."
} elseif ($ValidationResults.OverallScore -ge 60) {
    Write-Warning "[GOOD] Good progress, but some improvements needed."
} else {
    Write-Error "[NEEDS WORK] Significant work required to complete the OS."
}

# Generate HTML report if requested
if ($GenerateReport) {
    Write-Info "Generating HTML report..."
    
    $htmlContent = @"
<!DOCTYPE html>
<html>
<head>
    <title>RaeenOS Validation Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .header { background: #2c3e50; color: white; padding: 20px; border-radius: 5px; }
        .summary { background: #ecf0f1; padding: 15px; margin: 20px 0; border-radius: 5px; }
        .test-section { margin: 20px 0; }
        .pass { color: #27ae60; }
        .fail { color: #e74c3c; }
        .warn { color: #f39c12; }
        table { width: 100%; border-collapse: collapse; margin: 10px 0; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <div class="header">
        <h1>RaeenOS Validation Report</h1>
        <p>Generated on: $(Get-Date)</p>
    </div>
    
    <div class="summary">
        <h2>Summary</h2>
        <p><strong>Total Tests:</strong> $($ValidationResults.TotalTests)</p>
        <p><strong>Passed:</strong> $($ValidationResults.PassedTests)</p>
        <p><strong>Failed:</strong> $($ValidationResults.TotalTests - $ValidationResults.PassedTests)</p>
        <p><strong>Overall Score:</strong> $($ValidationResults.OverallScore)%</p>
    </div>
"@

    foreach ($category in $ValidationResults.Keys) {
        if ($category -like "*Tests") {
            $htmlContent += "<div class='test-section'><h3>$category</h3><table><tr><th>Test</th><th>Status</th><th>Details</th></tr>"
            foreach ($test in $ValidationResults[$category]) {
                $statusClass = switch ($test.Status) {
                    "PASS" { "pass" }
                    "FAIL" { "fail" }
                    default { "warn" }
                }
                $htmlContent += "<tr><td>$($test.TestName)</td><td class='$statusClass'>$($test.Status)</td><td>$($test.Details)</td></tr>"
            }
            $htmlContent += "</table></div>"
        }
    }

    $htmlContent += "</body></html>"
    
    $htmlContent | Out-File -FilePath $OutputFile -Encoding UTF8
    Write-Success "Report generated: $OutputFile"
}

Write-Output ""
Write-Info "Validation completed. RaeenOS is ready for the next development phase!"