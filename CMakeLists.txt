cmake_minimum_required(VERSION 3.16)
project(RaeenOS VERSION 0.1.0 LANGUAGES C CXX ASM)

# Project configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Target architecture
if(NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH "x64")
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler flags
if(MSVC)
    # MSVC flags
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX /EHsc")
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DDEBUG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zi /Od")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi /Od")
    else()
        add_definitions(-DNDEBUG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
    endif()
else()
    # GCC/Clang flags - reduced strictness for initial boot
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-error -nostdlib -fno-builtin -fno-stack-protector")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-error -nostdlib -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti")
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DDEBUG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    else()
        add_definitions(-DNDEBUG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    endif()
endif()

# Architecture-specific flags
if(TARGET_ARCH STREQUAL "x86")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/kernel/core/include
    ${CMAKE_SOURCE_DIR}/kernel/hal/include
    ${CMAKE_SOURCE_DIR}/kernel/drivers/include
    ${CMAKE_SOURCE_DIR}/kernel/security/include
    ${CMAKE_SOURCE_DIR}/ui/framework/include
)

# Subdirectories
add_subdirectory(bootloader)
add_subdirectory(kernel)
add_subdirectory(ui)
add_subdirectory(tools)

# --- ISO and Run Targets ---

# Find utilities required for creating the ISO
find_program(GRUB_MKRESCUE_EXECUTABLE grub-mkrescue)
if(NOT GRUB_MKRESCUE_EXECUTABLE)
    message(FATAL_ERROR "grub-mkrescue not found. Please install grub utilities (e.g., grub-pc-bin, grub2-tools).")
endif()

# Configure the grub config file from the template we created
configure_file(
    ${CMAKE_SOURCE_DIR}/tools/grub.cfg.in
    ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
    @ONLY
)

# Custom targets
add_custom_target(iso
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/grub
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/kernel/kernel.bin ${CMAKE_BINARY_DIR}/iso/boot/kernel.bin
    COMMAND ${GRUB_MKRESCUE_EXECUTABLE} -o ${CMAKE_BINARY_DIR}/raeenos.iso ${CMAKE_BINARY_DIR}/iso
    DEPENDS kernel
    COMMENT "Creating bootable ISO image"
    VERBATIM
)

add_custom_target(run
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/raeenos.iso -m 512M -serial stdio
    DEPENDS iso
    COMMENT "Running RaeenOS in QEMU"
)

add_custom_target(debug
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/raeenos.iso -m 512M -serial stdio -s -S
    DEPENDS iso
    COMMENT "Starting RaeenOS in QEMU with debugging enabled"
)