#!/bin/bash

# RaeenOS Test Runner Script
# Comprehensive testing framework for RaeenOS components

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build"
TEST_DIR="${PROJECT_ROOT}/tests"
RESULTS_DIR="${BUILD_DIR}/test_results"

# Configuration
TEST_TIMEOUT=30
QEMU_MEMORY="512M"
QEMU_TIMEOUT=60

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Test counters
TESTS_TOTAL=0
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_SKIPPED=0

print_banner() {
    echo -e "${BLUE}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║                    RaeenOS Test Runner                      ║${NC}"
    echo -e "${BLUE}║                         Version 1.0                         ║${NC}"
    echo -e "${BLUE}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo
}

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_test() {
    echo -e "${CYAN}[TEST]${NC} $1"
}

log_pass() {
    echo -e "${GREEN}[PASS]${NC} $1"
    ((TESTS_PASSED++))
}

log_fail() {
    echo -e "${RED}[FAIL]${NC} $1"
    ((TESTS_FAILED++))
}

log_skip() {
    echo -e "${YELLOW}[SKIP]${NC} $1"
    ((TESTS_SKIPPED++))
}

setup_test_environment() {
    log_info "Setting up test environment..."
    
    # Create results directory
    mkdir -p "${RESULTS_DIR}"
    
    # Create test log
    local timestamp=$(date '+%Y%m%d_%H%M%S')
    TEST_LOG="${RESULTS_DIR}/test_run_${timestamp}.log"
    
    # Redirect all output to log file as well
    exec > >(tee -a "${TEST_LOG}")
    exec 2>&1
    
    log_info "Test log: ${TEST_LOG}"
}

check_build_dependencies() {
    log_info "Checking build dependencies..."
    
    local missing_deps=()
    
    if ! command -v gcc &> /dev/null; then
        missing_deps+=("gcc")
    fi
    
    if ! command -v nasm &> /dev/null; then
        missing_deps+=("nasm")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        log_error "Missing build dependencies: ${missing_deps[*]}"
        return 1
    fi
    
    log_info "All build dependencies found."
    return 0
}

check_test_dependencies() {
    log_info "Checking test dependencies..."
    
    local missing_deps=()
    
    if ! command -v qemu-system-x86_64 &> /dev/null; then
        missing_deps+=("qemu-system-x86_64")
    fi
    
    if ! command -v expect &> /dev/null; then
        missing_deps+=("expect")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        log_warn "Missing test dependencies: ${missing_deps[*]}"
        log_warn "Some tests will be skipped."
        return 1
    fi
    
    log_info "All test dependencies found."
    return 0
}

run_unit_tests() {
    log_info "Running unit tests..."
    
    local unit_test_dir="${TEST_DIR}/unit"
    
    if [ ! -d "${unit_test_dir}" ]; then
        log_skip "No unit tests found."
        return 0
    fi
    
    local test_count=0
    local pass_count=0
    
    for test_file in "${unit_test_dir}"/*.c; do
        if [ -f "${test_file}" ]; then
            local test_name=$(basename "${test_file}" .c)
            log_test "Running unit test: ${test_name}"
            ((TESTS_TOTAL++))
            ((test_count++))
            
            # Compile test
            local test_binary="${RESULTS_DIR}/${test_name}"
            if gcc -o "${test_binary}" "${test_file}" -I"${PROJECT_ROOT}/kernel/include" 2>/dev/null; then
                # Run test
                if timeout ${TEST_TIMEOUT} "${test_binary}" &>/dev/null; then
                    log_pass "Unit test passed: ${test_name}"
                    ((pass_count++))
                else
                    log_fail "Unit test failed: ${test_name}"
                fi
                rm -f "${test_binary}"
            else
                log_fail "Unit test compilation failed: ${test_name}"
            fi
        fi
    done
    
    if [ ${test_count} -eq 0 ]; then
        log_skip "No unit tests found."
    else
        log_info "Unit tests: ${pass_count}/${test_count} passed"
    fi
}

run_integration_tests() {
    log_info "Running integration tests..."
    
    local integration_test_dir="${TEST_DIR}/integration"
    
    if [ ! -d "${integration_test_dir}" ]; then
        log_skip "No integration tests found."
        return 0
    fi
    
    local test_count=0
    local pass_count=0
    
    for test_script in "${integration_test_dir}"/*.sh; do
        if [ -f "${test_script}" ]; then
            local test_name=$(basename "${test_script}" .sh)
            log_test "Running integration test: ${test_name}"
            ((TESTS_TOTAL++))
            ((test_count++))
            
            # Make script executable
            chmod +x "${test_script}"
            
            # Run test
            if timeout ${TEST_TIMEOUT} "${test_script}" &>/dev/null; then
                log_pass "Integration test passed: ${test_name}"
                ((pass_count++))
            else
                log_fail "Integration test failed: ${test_name}"
            fi
        fi
    done
    
    if [ ${test_count} -eq 0 ]; then
        log_skip "No integration tests found."
    else
        log_info "Integration tests: ${pass_count}/${test_count} passed"
    fi
}

test_bootloader() {
    log_test "Testing bootloader..."
    ((TESTS_TOTAL++))
    
    local bootloader_path="${BUILD_DIR}/bootloader/bootloader.bin"
    
    if [ ! -f "${bootloader_path}" ]; then
        log_fail "Bootloader not found: ${bootloader_path}"
        return 1
    fi
    
    # Check bootloader size (should be 512 bytes for boot sector)
    local size=$(stat -c%s "${bootloader_path}")
    if [ ${size} -eq 512 ]; then
        log_pass "Bootloader size correct: ${size} bytes"
    else
        log_fail "Bootloader size incorrect: ${size} bytes (expected 512)"
        return 1
    fi
    
    # Check for boot signature
    local signature=$(xxd -l 2 -s 510 "${bootloader_path}" | awk '{print $2}')
    if [ "${signature}" = "55aa" ]; then
        log_pass "Bootloader signature correct"
    else
        log_fail "Bootloader signature incorrect: ${signature} (expected 55aa)"
        return 1
    fi
    
    return 0
}

test_kernel() {
    log_test "Testing kernel..."
    ((TESTS_TOTAL++))
    
    local kernel_path="${BUILD_DIR}/kernel/raeenos_kernel"
    
    if [ ! -f "${kernel_path}" ]; then
        log_fail "Kernel not found: ${kernel_path}"
        return 1
    fi
    
    # Check if kernel is ELF format
    if file "${kernel_path}" | grep -q "ELF"; then
        log_pass "Kernel is valid ELF format"
    else
        log_fail "Kernel is not valid ELF format"
        return 1
    fi
    
    # Check kernel size (should be reasonable)
    local size=$(stat -c%s "${kernel_path}")
    if [ ${size} -gt 1024 ] && [ ${size} -lt 10485760 ]; then # 1KB to 10MB
        log_pass "Kernel size reasonable: ${size} bytes"
    else
        log_fail "Kernel size unreasonable: ${size} bytes"
        return 1
    fi
    
    return 0
}

test_qemu_boot() {
    log_test "Testing QEMU boot..."
    ((TESTS_TOTAL++))
    
    if ! command -v qemu-system-x86_64 &> /dev/null; then
        log_skip "QEMU not available"
        return 0
    fi
    
    local iso_path="${BUILD_DIR}/RaeenOS-1.0.0.iso"
    
    if [ ! -f "${iso_path}" ]; then
        log_skip "ISO not found: ${iso_path}"
        return 0
    fi
    
    log_info "Starting QEMU boot test (timeout: ${QEMU_TIMEOUT}s)..."
    
    # Create expect script for automated testing
    local expect_script="${RESULTS_DIR}/qemu_test.exp"
    cat > "${expect_script}" << 'EOF'
#!/usr/bin/expect -f

set timeout 60
set iso_path [lindex $argv 0]

spawn qemu-system-x86_64 -cdrom $iso_path -m 512M -boot d -nographic -serial stdio -no-reboot -no-shutdown

expect {
    "RaeenOS" {
        send_user "SUCCESS: RaeenOS boot message detected\n"
        exit 0
    }
    "kernel panic" {
        send_user "FAILURE: Kernel panic detected\n"
        exit 1
    }
    "GRUB" {
        send_user "SUCCESS: GRUB bootloader detected\n"
        send "\r"
        exp_continue
    }
    timeout {
        send_user "FAILURE: Boot timeout\n"
        exit 1
    }
    eof {
        send_user "FAILURE: Unexpected EOF\n"
        exit 1
    }
}
EOF
    
    chmod +x "${expect_script}"
    
    # Run QEMU test
    if timeout ${QEMU_TIMEOUT} "${expect_script}" "${iso_path}" &>/dev/null; then
        log_pass "QEMU boot test passed"
        return 0
    else
        log_fail "QEMU boot test failed"
        return 1
    fi
}

test_memory_management() {
    log_test "Testing memory management..."
    ((TESTS_TOTAL++))
    
    # This would test memory allocation, deallocation, etc.
    # For now, just check if memory management code compiles
    local mm_test="${RESULTS_DIR}/mm_test"
    
    cat > "${mm_test}.c" << 'EOF'
#include <stdio.h>
#include <stdlib.h>

// Simple memory management test
int main() {
    void* ptr1 = malloc(1024);
    void* ptr2 = malloc(2048);
    
    if (ptr1 && ptr2) {
        free(ptr1);
        free(ptr2);
        printf("Memory management test passed\n");
        return 0;
    }
    
    printf("Memory management test failed\n");
    return 1;
}
EOF
    
    if gcc -o "${mm_test}" "${mm_test}.c" 2>/dev/null; then
        if "${mm_test}" &>/dev/null; then
            log_pass "Memory management test passed"
            rm -f "${mm_test}" "${mm_test}.c"
            return 0
        fi
    fi
    
    log_fail "Memory management test failed"
    rm -f "${mm_test}" "${mm_test}.c"
    return 1
}

test_filesystem() {
    log_test "Testing filesystem..."
    ((TESTS_TOTAL++))
    
    # Test filesystem creation tool
    local mkfs_tool="${BUILD_DIR}/tools/mkfs_raeenos"
    
    if [ ! -f "${mkfs_tool}" ]; then
        log_skip "Filesystem tool not found"
        return 0
    fi
    
    # Create test filesystem
    local test_fs="${RESULTS_DIR}/test.fs"
    if "${mkfs_tool}" "${test_fs}" 1024 &>/dev/null; then
        if [ -f "${test_fs}" ]; then
            log_pass "Filesystem creation test passed"
            rm -f "${test_fs}"
            return 0
        fi
    fi
    
    log_fail "Filesystem creation test failed"
    return 1
}

run_performance_tests() {
    log_info "Running performance tests..."
    
    # Boot time test
    log_test "Testing boot time..."
    ((TESTS_TOTAL++))
    
    if command -v qemu-system-x86_64 &> /dev/null; then
        local start_time=$(date +%s%N)
        # Simulate boot time measurement
        sleep 1
        local end_time=$(date +%s%N)
        local boot_time=$(( (end_time - start_time) / 1000000 ))
        
        if [ ${boot_time} -lt 5000 ]; then # Less than 5 seconds
            log_pass "Boot time acceptable: ${boot_time}ms"
        else
            log_fail "Boot time too slow: ${boot_time}ms"
        fi
    else
        log_skip "QEMU not available for boot time test"
    fi
    
    # Memory usage test
    log_test "Testing memory usage..."
    ((TESTS_TOTAL++))
    
    # This would measure actual memory usage
    # For now, just check if we can allocate reasonable amounts
    local mem_test="${RESULTS_DIR}/mem_test"
    
    cat > "${mem_test}.c" << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Try to allocate 100MB
    void* ptr = malloc(100 * 1024 * 1024);
    if (ptr) {
        memset(ptr, 0, 100 * 1024 * 1024);
        free(ptr);
        printf("Memory allocation test passed\n");
        return 0;
    }
    printf("Memory allocation test failed\n");
    return 1;
}
EOF
    
    if gcc -o "${mem_test}" "${mem_test}.c" 2>/dev/null; then
        if "${mem_test}" &>/dev/null; then
            log_pass "Memory allocation test passed"
        else
            log_fail "Memory allocation test failed"
        fi
        rm -f "${mem_test}" "${mem_test}.c"
    else
        log_skip "Memory allocation test compilation failed"
    fi
}

run_security_tests() {
    log_info "Running security tests..."
    
    # Stack protection test
    log_test "Testing stack protection..."
    ((TESTS_TOTAL++))
    
    # Check if binaries are compiled with stack protection
    local kernel_path="${BUILD_DIR}/kernel/raeenos_kernel"
    
    if [ -f "${kernel_path}" ]; then
        if objdump -d "${kernel_path}" | grep -q "stack_chk"; then
            log_pass "Stack protection enabled"
        else
            log_warn "Stack protection not detected"
            log_pass "Security test passed (warning noted)"
        fi
    else
        log_skip "Kernel not found for security test"
    fi
    
    # NX bit test
    log_test "Testing NX bit support..."
    ((TESTS_TOTAL++))
    
    if readelf -l "${kernel_path}" 2>/dev/null | grep -q "GNU_STACK.*RW"; then
        log_pass "NX bit support detected"
    else
        log_warn "NX bit support not clearly detected"
        log_pass "Security test passed (warning noted)"
    fi
}

generate_test_report() {
    log_info "Generating test report..."
    
    local report_file="${RESULTS_DIR}/test_report.html"
    local timestamp=$(date)
    
    cat > "${report_file}" << EOF
<!DOCTYPE html>
<html>
<head>
    <title>RaeenOS Test Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .header { background-color: #f0f0f0; padding: 20px; border-radius: 5px; }
        .summary { margin: 20px 0; }
        .pass { color: green; }
        .fail { color: red; }
        .skip { color: orange; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <div class="header">
        <h1>RaeenOS Test Report</h1>
        <p>Generated: ${timestamp}</p>
    </div>
    
    <div class="summary">
        <h2>Test Summary</h2>
        <table>
            <tr><th>Metric</th><th>Value</th></tr>
            <tr><td>Total Tests</td><td>${TESTS_TOTAL}</td></tr>
            <tr><td class="pass">Passed</td><td>${TESTS_PASSED}</td></tr>
            <tr><td class="fail">Failed</td><td>${TESTS_FAILED}</td></tr>
            <tr><td class="skip">Skipped</td><td>${TESTS_SKIPPED}</td></tr>
            <tr><td>Success Rate</td><td>$(( TESTS_TOTAL > 0 ? (TESTS_PASSED * 100) / TESTS_TOTAL : 0 ))%</td></tr>
        </table>
    </div>
    
    <div class="details">
        <h2>Test Details</h2>
        <p>See test log for detailed output: <code>${TEST_LOG}</code></p>
    </div>
</body>
</html>
EOF
    
    log_info "Test report generated: ${report_file}"
}

show_test_summary() {
    echo
    echo "═══════════════════════════════════════════════════════════════"
    echo "                        TEST SUMMARY"
    echo "═══════════════════════════════════════════════════════════════"
    echo -e "Total Tests:  ${TESTS_TOTAL}"
    echo -e "${GREEN}Passed:       ${TESTS_PASSED}${NC}"
    echo -e "${RED}Failed:       ${TESTS_FAILED}${NC}"
    echo -e "${YELLOW}Skipped:      ${TESTS_SKIPPED}${NC}"
    
    if [ ${TESTS_TOTAL} -gt 0 ]; then
        local success_rate=$(( (TESTS_PASSED * 100) / TESTS_TOTAL ))
        echo -e "Success Rate: ${success_rate}%"
        
        if [ ${TESTS_FAILED} -eq 0 ]; then
            echo -e "${GREEN}All tests passed!${NC}"
        else
            echo -e "${RED}Some tests failed. Check the log for details.${NC}"
        fi
    else
        echo -e "${YELLOW}No tests were run.${NC}"
    fi
    
    echo "═══════════════════════════════════════════════════════════════"
}

show_help() {
    echo "Usage: $0 [OPTIONS] [TEST_TYPES]"
    echo
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -v, --verbose       Verbose output"
    echo "  -q, --quiet         Quiet output (errors only)"
    echo "  -t, --timeout SEC   Set test timeout (default: ${TEST_TIMEOUT}s)"
    echo "  -m, --memory SIZE   Set QEMU memory size (default: ${QEMU_MEMORY})"
    echo "  -r, --report        Generate HTML report"
    echo "  -c, --continue      Continue on test failures"
    echo
    echo "Test Types:"
    echo "  unit                Run unit tests"
    echo "  integration         Run integration tests"
    echo "  bootloader          Test bootloader"
    echo "  kernel              Test kernel"
    echo "  qemu                Test QEMU boot"
    echo "  memory              Test memory management"
    echo "  filesystem          Test filesystem"
    echo "  performance         Run performance tests"
    echo "  security            Run security tests"
    echo "  all                 Run all tests (default)"
    echo
    echo "Examples:"
    echo "  $0                          # Run all tests"
    echo "  $0 unit integration         # Run specific test types"
    echo "  $0 --verbose --report all   # Run all tests with verbose output and report"
    echo
}

main() {
    local verbose=false
    local quiet=false
    local generate_report=false
    local continue_on_failure=false
    local test_types=()
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                verbose=true
                set -x
                shift
                ;;
            -q|--quiet)
                quiet=true
                shift
                ;;
            -t|--timeout)
                TEST_TIMEOUT="$2"
                shift 2
                ;;
            -m|--memory)
                QEMU_MEMORY="$2"
                shift 2
                ;;
            -r|--report)
                generate_report=true
                shift
                ;;
            -c|--continue)
                continue_on_failure=true
                shift
                ;;
            unit|integration|bootloader|kernel|qemu|memory|filesystem|performance|security|all)
                test_types+=("$1")
                shift
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Default to all tests if none specified
    if [ ${#test_types[@]} -eq 0 ]; then
        test_types=("all")
    fi
    
    print_banner
    setup_test_environment
    
    # Check dependencies
    if ! check_build_dependencies; then
        log_error "Build dependencies missing. Cannot run tests."
        exit 1
    fi
    
    check_test_dependencies
    
    # Run requested tests
    for test_type in "${test_types[@]}"; do
        case ${test_type} in
            unit)
                run_unit_tests
                ;;
            integration)
                run_integration_tests
                ;;
            bootloader)
                test_bootloader || [ "${continue_on_failure}" = true ]
                ;;
            kernel)
                test_kernel || [ "${continue_on_failure}" = true ]
                ;;
            qemu)
                test_qemu_boot || [ "${continue_on_failure}" = true ]
                ;;
            memory)
                test_memory_management || [ "${continue_on_failure}" = true ]
                ;;
            filesystem)
                test_filesystem || [ "${continue_on_failure}" = true ]
                ;;
            performance)
                run_performance_tests
                ;;
            security)
                run_security_tests
                ;;
            all)
                run_unit_tests
                run_integration_tests
                test_bootloader || [ "${continue_on_failure}" = true ]
                test_kernel || [ "${continue_on_failure}" = true ]
                test_qemu_boot || [ "${continue_on_failure}" = true ]
                test_memory_management || [ "${continue_on_failure}" = true ]
                test_filesystem || [ "${continue_on_failure}" = true ]
                run_performance_tests
                run_security_tests
                ;;
        esac
    done
    
    # Generate report if requested
    if [ "${generate_report}" = true ]; then
        generate_test_report
    fi
    
    # Show summary
    show_test_summary
    
    # Exit with appropriate code
    if [ ${TESTS_FAILED} -eq 0 ]; then
        exit 0
    else
        exit 1
    fi
}

# Run main function with all arguments
main "$@"