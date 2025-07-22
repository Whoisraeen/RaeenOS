# Bootloader CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Find NASM executable
find_program(NASM_EXECUTABLE nasm)
if(NOT NASM_EXECUTABLE)
    message(WARNING "NASM not found. Bootloader assembly files will not be built.")
    set(NASM_FOUND FALSE)
else()
    message(STATUS "Found NASM: ${NASM_EXECUTABLE}")
    set(NASM_FOUND TRUE)
endif()

# Bootloader sources
set(BOOTLOADER_SOURCES
    boot.asm
    multiboot.c
)

# Stage 1 bootloader
if(NASM_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/stage1/stage1.asm)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stage1.bin
        COMMAND ${NASM_EXECUTABLE} -f bin ${CMAKE_CURRENT_SOURCE_DIR}/stage1/stage1.asm -o ${CMAKE_CURRENT_BINARY_DIR}/stage1.bin
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/stage1/stage1.asm
        COMMENT "Building Stage 1 bootloader"
    )
    set(STAGE1_TARGET ${CMAKE_CURRENT_BINARY_DIR}/stage1.bin)
else()
    set(STAGE1_TARGET)
endif()

# Stage 2 bootloader
if(NASM_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/stage2/stage2.asm)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stage2.bin
        COMMAND ${NASM_EXECUTABLE} -f bin ${CMAKE_CURRENT_SOURCE_DIR}/stage2/stage2.asm -o ${CMAKE_CURRENT_BINARY_DIR}/stage2.bin
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/stage2/stage2.asm
        COMMENT "Building Stage 2 bootloader"
    )
    set(STAGE2_TARGET ${CMAKE_CURRENT_BINARY_DIR}/stage2.bin)
else()
    set(STAGE2_TARGET)
endif()

# Multiboot object
if(NASM_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/boot.asm)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boot.o
        COMMAND ${NASM_EXECUTABLE} -f elf64 ${CMAKE_CURRENT_SOURCE_DIR}/boot.asm -o ${CMAKE_CURRENT_BINARY_DIR}/boot.o
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/boot.asm
        COMMENT "Building boot.o"
    )
    set(BOOT_TARGET ${CMAKE_CURRENT_BINARY_DIR}/boot.o)
else()
    set(BOOT_TARGET)
endif()

# Multiboot C code
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/multiboot.c)
    add_library(multiboot_obj OBJECT multiboot.c)
    target_include_directories(multiboot_obj PRIVATE 
        ${CMAKE_SOURCE_DIR}/kernel/include
        ${CMAKE_SOURCE_DIR}/kernel/memory/include
        ${CMAKE_SOURCE_DIR}/kernel/core/include
    )
    if(MSVC)
        target_compile_options(multiboot_obj PRIVATE /TC)
    else()
        target_compile_options(multiboot_obj PRIVATE -std=c11 -ffreestanding -nostdlib)
    endif()
    set(MULTIBOOT_TARGET $<TARGET_OBJECTS:multiboot_obj>)
else()
    set(MULTIBOOT_TARGET)
endif()

# Bootloader target
set(BOOTLOADER_TARGETS)
if(STAGE1_TARGET)
    list(APPEND BOOTLOADER_TARGETS ${STAGE1_TARGET})
endif()
if(STAGE2_TARGET)
    list(APPEND BOOTLOADER_TARGETS ${STAGE2_TARGET})
endif()
if(BOOT_TARGET)
    list(APPEND BOOTLOADER_TARGETS ${BOOT_TARGET})
endif()

if(BOOTLOADER_TARGETS OR MULTIBOOT_TARGET)
    add_custom_target(bootloader ALL
        DEPENDS ${BOOTLOADER_TARGETS}
        COMMENT "Building complete bootloader"
    )
    
    if(MULTIBOOT_TARGET)
        add_dependencies(bootloader multiboot_obj)
    endif()
    
    # Install bootloader files
    if(STAGE1_TARGET)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/stage1.bin DESTINATION boot)
    endif()
    if(STAGE2_TARGET)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/stage2.bin DESTINATION boot)
    endif()
    if(BOOT_TARGET)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/boot.o DESTINATION boot)
    endif()
else()
    # Create placeholder target
    add_custom_target(bootloader ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Bootloader components not available - NASM or source files missing"
        COMMENT "Bootloader placeholder"
    )
endif()