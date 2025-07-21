#!/bin/bash

# Build script for test program
# This compiles the test program to a flat binary format

set -e

# Configuration
CC="gcc"
CFLAGS="-nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -m64 -static"
LDFLAGS="-static -nostdlib -nostartfiles -nodefaultlibs"

# Source files
SOURCE="test_program.c"
OUTPUT="test_program.bin"
HEADER_SIZE=28  # Size of flat_binary_header_t

echo "Building test program..."

# Compile to object file
$CC $CFLAGS -c $SOURCE -o test_program.o

# Link to create executable
$CC $LDFLAGS test_program.o -o test_program.elf

# Extract the code section
objcopy -O binary --only-section=.text test_program.elf test_program.text
objcopy -O binary --only-section=.data test_program.elf test_program.data
objcopy -O binary --only-section=.rodata test_program.elf test_program.rodata

# Get sizes
TEXT_SIZE=$(stat -c%s test_program.text 2>/dev/null || stat -f%z test_program.text 2>/dev/null || echo 0)
DATA_SIZE=$(stat -c%s test_program.data 2>/dev/null || stat -f%z test_program.data 2>/dev/null || echo 0)
RODATA_SIZE=$(stat -c%s test_program.rodata 2>/dev/null || stat -f%z test_program.rodata 2>/dev/null || echo 0)

# Calculate total code size (text + rodata)
CODE_SIZE=$((TEXT_SIZE + RODATA_SIZE))

# Create the flat binary header
cat > header.bin << EOF
$(printf "\\x%02x" $((0x4E & 0xFF)))  # Magic: "RAEN" in little-endian
$(printf "\\x%02x" $((0x45 & 0xFF)))
$(printf "\\x%02x" $((0x41 & 0xFF)))
$(printf "\\x%02x" $((0x52 & 0xFF)))
$(printf "\\x%02x" $((HEADER_SIZE & 0xFF)))      # Entry point offset
$(printf "\\x%02x" $(((HEADER_SIZE >> 8) & 0xFF)))
$(printf "\\x%02x" $(((HEADER_SIZE >> 16) & 0xFF)))
$(printf "\\x%02x" $(((HEADER_SIZE >> 24) & 0xFF)))
$(printf "\\x%02x" $((CODE_SIZE & 0xFF)))        # Code size
$(printf "\\x%02x" $(((CODE_SIZE >> 8) & 0xFF)))
$(printf "\\x%02x" $(((CODE_SIZE >> 16) & 0xFF)))
$(printf "\\x%02x" $(((CODE_SIZE >> 24) & 0xFF)))
$(printf "\\x%02x" $((DATA_SIZE & 0xFF)))        # Data size
$(printf "\\x%02x" $(((DATA_SIZE >> 8) & 0xFF)))
$(printf "\\x%02x" $(((DATA_SIZE >> 16) & 0xFF)))
$(printf "\\x%02x" $(((DATA_SIZE >> 24) & 0xFF)))
$(printf "\\x%02x" $((0 & 0xFF)))                # BSS size (0 for now)
$(printf "\\x%02x" $(((0 >> 8) & 0xFF)))
$(printf "\\x%02x" $(((0 >> 16) & 0xFF)))
$(printf "\\x%02x" $(((0 >> 24) & 0xFF)))
$(printf "\\x%02x" $((4096 & 0xFF)))             # Stack size (4KB)
$(printf "\\x%02x" $(((4096 >> 8) & 0xFF)))
$(printf "\\x%02x" $(((4096 >> 16) & 0xFF)))
$(printf "\\x%02x" $(((4096 >> 24) & 0xFF)))
$(printf "\\x%02x" $((1 & 0xFF)))                # Flags (executable)
$(printf "\\x%02x" $(((1 >> 8) & 0xFF)))
$(printf "\\x%02x" $(((1 >> 16) & 0xFF)))
$(printf "\\x%02x" $(((1 >> 24) & 0xFF)))
EOF

# Combine header, code, and data
cat header.bin test_program.text test_program.rodata test_program.data > $OUTPUT

# Clean up temporary files
rm -f test_program.o test_program.elf test_program.text test_program.data test_program.rodata header.bin

echo "Test program built successfully: $OUTPUT"
echo "Code size: $CODE_SIZE bytes"
echo "Data size: $DATA_SIZE bytes"
echo "Total size: $(stat -c%s $OUTPUT 2>/dev/null || stat -f%z $OUTPUT 2>/dev/null || echo "unknown") bytes" 