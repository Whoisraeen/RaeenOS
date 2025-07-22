# Tools CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Tools include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/kernel/core/include
    ${CMAKE_SOURCE_DIR}/kernel/hal/include
    ${CMAKE_SOURCE_DIR}/kernel/drivers/include
)

# Development tools sources
set(TOOLS_SOURCES)

# Check for existing tool files
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/debugger)
    file(GLOB_RECURSE DEBUGGER_SOURCES "debugger/*.c")
    list(APPEND TOOLS_SOURCES ${DEBUGGER_SOURCES})
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/profiler)
    file(GLOB_RECURSE PROFILER_SOURCES "profiler/*.c")
    list(APPEND TOOLS_SOURCES ${PROFILER_SOURCES})
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/emulator)
    file(GLOB_RECURSE EMULATOR_SOURCES "emulator/*.c")
    list(APPEND TOOLS_SOURCES ${EMULATOR_SOURCES})
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/mkfs)
    file(GLOB_RECURSE MKFS_SOURCES "mkfs/*.c")
    list(APPEND TOOLS_SOURCES ${MKFS_SOURCES})
endif()

# Build individual tools
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/debugger)
    file(GLOB DEBUGGER_SOURCES "debugger/*.c")
    if(DEBUGGER_SOURCES)
        add_executable(raeenos_debugger ${DEBUGGER_SOURCES})
        target_compile_options(raeenos_debugger PRIVATE -std=c11)
        install(TARGETS raeenos_debugger RUNTIME DESTINATION bin)
    endif()
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/profiler)
    file(GLOB PROFILER_SOURCES "profiler/*.c")
    if(PROFILER_SOURCES)
        add_executable(raeenos_profiler ${PROFILER_SOURCES})
        target_compile_options(raeenos_profiler PRIVATE -std=c11)
        install(TARGETS raeenos_profiler RUNTIME DESTINATION bin)
    endif()
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/emulator)
    file(GLOB EMULATOR_SOURCES "emulator/*.c")
    if(EMULATOR_SOURCES)
        add_executable(raeenos_emulator ${EMULATOR_SOURCES})
        target_compile_options(raeenos_emulator PRIVATE -std=c11)
        install(TARGETS raeenos_emulator RUNTIME DESTINATION bin)
    endif()
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/mkfs)
    file(GLOB MKFS_SOURCES "mkfs/*.c")
    if(MKFS_SOURCES)
        add_executable(raeenos_mkfs ${MKFS_SOURCES})
        target_compile_options(raeenos_mkfs PRIVATE -std=c11)
        install(TARGETS raeenos_mkfs RUNTIME DESTINATION bin)
    endif()
endif()

# ISO creation script
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/create_iso.sh)
    install(FILES create_iso.sh
        DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                   GROUP_READ GROUP_EXECUTE
                   WORLD_READ WORLD_EXECUTE
    )
endif()

# Build scripts
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/build_tools.sh)
    install(FILES build_tools.sh
        DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                   GROUP_READ GROUP_EXECUTE
                   WORLD_READ WORLD_EXECUTE
    )
endif()

# Create placeholder tools if directory is empty
if(NOT TOOLS_SOURCES)
    # Create a simple placeholder tool
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/raeenos_placeholder_tool.c
"#include <stdio.h>
int main() {
    printf(\"RaeenOS development tools will be implemented here\\n\");
    return 0;
}
")
    
    add_executable(raeenos_placeholder_tool 
        ${CMAKE_CURRENT_BINARY_DIR}/raeenos_placeholder_tool.c)
    
    install(TARGETS raeenos_placeholder_tool
        RUNTIME DESTINATION bin
    )
endif()

# Custom targets for tool development
add_custom_target(tools_all
    COMMENT "Building all RaeenOS development tools"
)

# Add dependencies if tools exist
if(TARGET raeenos_debugger)
    add_dependencies(tools_all raeenos_debugger)
endif()

if(TARGET raeenos_profiler)
    add_dependencies(tools_all raeenos_profiler)
endif()

if(TARGET raeenos_emulator)
    add_dependencies(tools_all raeenos_emulator)
endif()

if(TARGET raeenos_mkfs)
    add_dependencies(tools_all raeenos_mkfs)
endif()

if(TARGET raeenos_placeholder_tool)
    add_dependencies(tools_all raeenos_placeholder_tool)
endif()