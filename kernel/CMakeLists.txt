# Kernel CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Find NASM executable
find_program(NASM_EXECUTABLE nasm)
if(NOT NASM_EXECUTABLE)
    message(WARNING "NASM not found. Assembly files will not be built.")
    set(NASM_FOUND FALSE)
else()
    message(STATUS "Found NASM: ${NASM_EXECUTABLE}")
    set(NASM_FOUND TRUE)
endif()

# Kernel include directories
set(KERNEL_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/memory/include
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem/include
    ${CMAKE_CURRENT_SOURCE_DIR}/network/include
    ${CMAKE_CURRENT_SOURCE_DIR}/security/include
    ${CMAKE_CURRENT_SOURCE_DIR}/syscall/include
)

include_directories(${KERNEL_INCLUDE_DIRS})

# Assembly files
set(ASM_OBJECTS)

if(NASM_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64/kernel_entry.S)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel_entry.o
        COMMAND ${NASM_EXECUTABLE} -f elf64 ${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64/kernel_entry.S -o ${CMAKE_CURRENT_BINARY_DIR}/kernel_entry.o
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64/kernel_entry.S
        COMMENT "Building kernel_entry.o"
    )
    list(APPEND ASM_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/kernel_entry.o)
endif()

if(NASM_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/core/multiboot_header.asm)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/multiboot_header.o
        COMMAND ${NASM_EXECUTABLE} -f elf64 ${CMAKE_CURRENT_SOURCE_DIR}/core/multiboot_header.asm -o ${CMAKE_CURRENT_BINARY_DIR}/multiboot_header.o
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/core/multiboot_header.asm
        COMMENT "Building multiboot_header.o"
    )
    list(APPEND ASM_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/multiboot_header.o)
endif()

# Clean production kernel with real implementations
set(KERNEL_CORE_SOURCES
    core/kernel_main_clean_production.c
    core/string_simple.c
    core/kernel_infrastructure.c
    ai/ai_system.c
    security/advanced_security.c  
    security/enterprise_security.c
    gaming/gaming_system.c
    gui/raeen_gui_system.c
    gui/rendering/raeen_dx.c
    network/advanced_networking.c
)

# Simplified build - minimal sources for basic boot test
# HAL sources
set(KERNEL_HAL_SOURCES)

# Driver sources
set(KERNEL_DRIVER_SOURCES)

# Memory management sources
set(KERNEL_MEMORY_SOURCES)

# Filesystem sources
set(KERNEL_FS_SOURCES)

# Network sources
set(KERNEL_NETWORK_SOURCES)

# Security sources
set(KERNEL_SECURITY_SOURCES)

# Process management sources
set(KERNEL_PROCESS_SOURCES)

# System call sources
set(KERNEL_SYSCALL_SOURCES)

# Graphics and UI sources
set(KERNEL_GRAPHICS_SOURCES)

# All kernel sources
set(KERNEL_SOURCES
    ${KERNEL_CORE_SOURCES}
    ${KERNEL_HAL_SOURCES}
    ${KERNEL_DRIVER_SOURCES}
    ${KERNEL_MEMORY_SOURCES}
    ${KERNEL_FS_SOURCES}
    ${KERNEL_NETWORK_SOURCES}
    ${KERNEL_SECURITY_SOURCES}
    ${KERNEL_PROCESS_SOURCES}
    ${KERNEL_SYSCALL_SOURCES}
    ${KERNEL_GRAPHICS_SOURCES}
)

# Filter out non-existent files
set(KERNEL_SOURCES_FILTERED)
foreach(source ${KERNEL_SOURCES})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${source})
        list(APPEND KERNEL_SOURCES_FILTERED ${source})
    endif()
endforeach()

# Create object library for kernel sources
add_library(kernel_objects OBJECT ${KERNEL_SOURCES_FILTERED})

if(MSVC)
    target_compile_options(kernel_objects PRIVATE 
        /TC /W4
    )
else()
    target_compile_options(kernel_objects PRIVATE 
        -std=c11 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector
        -Wall -Wno-error -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function
        -m64
        -mno-red-zone
        -mno-mmx
        -mno-sse
        -mno-sse2
    )
endif()

target_include_directories(kernel_objects PRIVATE ${KERNEL_INCLUDE_DIRS})

# Link kernel binary
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
    COMMAND ${CMAKE_C_COMPILER} -T ${CMAKE_CURRENT_SOURCE_DIR}/kernel_fixed.ld 
            -o ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
            -ffreestanding -nostdlib -lgcc
            ${ASM_OBJECTS}
            $<TARGET_OBJECTS:kernel_objects>
    DEPENDS kernel_objects ${ASM_OBJECTS} ${CMAKE_CURRENT_SOURCE_DIR}/kernel_fixed.ld
    COMMENT "Linking kernel binary"
)

# Kernel target
add_custom_target(kernel ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
    COMMENT "Building kernel"
)

# Install kernel
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
    DESTINATION boot
    RENAME kernel.bin
)