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
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -nostdlib -fno-builtin -fno-stack-protector")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -nostdlib -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else()
    add_definitions(-DNDEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
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

# Custom targets
add_custom_target(iso
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/grub
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bootloader> ${CMAKE_BINARY_DIR}/iso/boot/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel> ${CMAKE_BINARY_DIR}/iso/boot/
    COMMAND echo 'set timeout=0' > ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
    COMMAND echo 'set default=0' >> ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
    COMMAND echo 'menuentry "RaeenOS" {' >> ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
    COMMAND echo '  multiboot /boot/kernel.bin' >> ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
    COMMAND echo '}' >> ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
    COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/raeenos.iso ${CMAKE_BINARY_DIR}/iso
    DEPENDS bootloader kernel
    COMMENT "Creating bootable ISO image"
)

add_custom_target(run
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/raeenos.iso
    DEPENDS iso
    COMMENT "Running RaeenOS in QEMU"
)

add_custom_target(debug
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/raeenos.iso -s -S
    DEPENDS iso
    COMMENT "Starting RaeenOS in QEMU with debugging enabled"
)