#!/bin/bash
# RaeenOS Production Build Test
# Tests compilation of core RaeenOS kernel components

echo "🚀 RaeenOS Production Build Test"
echo "================================="

# Configuration
BUILD_DIR="../build/test_production"
KERNEL_DIR="/mnt/d/RaeenOS/kernel"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Functions
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Setup
log_info "Setting up build environment..."
mkdir -p "$BUILD_DIR"
cd "$KERNEL_DIR"

# Check tools
log_info "Checking build tools..."
if ! command -v gcc &> /dev/null; then
    log_error "GCC not found!"
    exit 1
fi

if ! command -v nasm &> /dev/null; then
    log_error "NASM not found!"
    exit 1
fi

log_success "Build tools found"

# Build flags
CFLAGS="-std=c11 -Wall -Wno-unused-function -Wno-unused-variable -nostdlib -ffreestanding -fno-builtin -fno-stack-protector -m64 -DPRODUCTION_BUILD"
INCLUDES="-I. -Icore/include -Imemory/include -Ihal/include -Isecurity/include"

log_info "Build configuration:"
log_info "  CFLAGS: $CFLAGS"
log_info "  INCLUDES: $INCLUDES"

# Test individual component compilation
log_info "Testing component compilation..."

# Test 1: String functions
log_info "Testing string.c..."
if gcc $CFLAGS $INCLUDES -c core/string_simple.c -o "$BUILD_DIR/string.o" 2>/dev/null; then
    log_success "String functions compiled successfully"
else
    log_warning "String functions compilation failed (non-critical)"
fi

# Test 2: HAL stubs
log_info "Testing HAL stubs..."
if gcc $CFLAGS $INCLUDES -c hal/hal_stubs.c -o "$BUILD_DIR/hal_stubs.o" 2>/dev/null; then
    log_success "HAL stubs compiled successfully"
else
    log_warning "HAL stubs compilation failed"
fi

# Test 3: Security minimal
log_info "Testing security minimal..."
if gcc $CFLAGS $INCLUDES -c security/security_minimal.c -o "$BUILD_DIR/security_minimal.o" 2>/dev/null; then
    log_success "Security minimal compiled successfully"
else
    log_warning "Security minimal compilation failed"
fi

# Count successful compilations
successful=0
total=3

if [ -f "$BUILD_DIR/string.o" ]; then ((successful++)); fi
if [ -f "$BUILD_DIR/hal_stubs.o" ]; then ((successful++)); fi
if [ -f "$BUILD_DIR/security_minimal.o" ]; then ((successful++)); fi

# Results
echo ""
log_info "Build Test Results:"
log_info "=================="
log_info "Successful compilations: $successful/$total"

if [ $successful -eq $total ]; then
    log_success "🎉 All components compiled successfully!"
    log_success "RaeenOS production kernel components are ready for integration"
    echo ""
    log_info "Next steps:"
    log_info "1. Integration testing with full kernel build"
    log_info "2. Bootloader integration"
    log_info "3. ISO creation and testing"
elif [ $successful -gt 0 ]; then
    log_warning "⚠️  Partial success - $successful/$total components compiled"
    log_info "Some components need attention before full kernel build"
else
    log_error "❌ Build test failed - no components compiled successfully"
    log_error "Check build environment and dependencies"
fi

# Show file sizes
if [ $successful -gt 0 ]; then
    echo ""
    log_info "Compiled object file sizes:"
    ls -la "$BUILD_DIR"/*.o 2>/dev/null || log_info "No object files found"
fi

echo ""
log_info "RaeenOS build test completed"