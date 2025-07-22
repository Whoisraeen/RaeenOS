# UI CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# UI Framework sources
set(UI_FRAMEWORK_SOURCES)

# Check for existing UI framework files
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/framework)
    file(GLOB_RECURSE UI_FRAMEWORK_SOURCES "framework/*.c")
endif()

# Window manager sources
set(UI_WM_SOURCES)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/window_manager)
    file(GLOB_RECURSE UI_WM_SOURCES "window_manager/*.c")
endif()

# Graphics sources
set(UI_GRAPHICS_SOURCES)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/graphics)
    file(GLOB_RECURSE UI_GRAPHICS_SOURCES "graphics/*.c")
endif()

# Widget sources
set(UI_WIDGET_SOURCES)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/widgets)
    file(GLOB_RECURSE UI_WIDGET_SOURCES "widgets/*.c")
endif()

# Theme sources
set(UI_THEME_SOURCES)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/themes)
    file(GLOB_RECURSE UI_THEME_SOURCES "themes/*.c")
endif()

# All UI sources
set(UI_SOURCES
    ${UI_FRAMEWORK_SOURCES}
    ${UI_WM_SOURCES}
    ${UI_GRAPHICS_SOURCES}
    ${UI_WIDGET_SOURCES}
    ${UI_THEME_SOURCES}
)

# Filter out non-existent files
set(UI_SOURCES_FILTERED)
foreach(source ${UI_SOURCES})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${source})
        list(APPEND UI_SOURCES_FILTERED ${source})
    endif()
endforeach()

# Create UI library if sources exist
if(UI_SOURCES_FILTERED)
    add_library(ui_lib STATIC ${UI_SOURCES_FILTERED})
    
    # UI include directories
    target_include_directories(ui_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/framework/include
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/include
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/include
        ${CMAKE_SOURCE_DIR}/kernel/core/include
        ${CMAKE_SOURCE_DIR}/kernel/drivers/include
    )
    
    # UI compiler flags
    target_compile_options(ui_lib PRIVATE
        -ffreestanding
        -fno-builtin
        -fno-stack-protector
        -nostdlib
    )
    
    # Install UI library
    install(TARGETS ui_lib
        ARCHIVE DESTINATION lib
    )
else()
    # Create empty target if no UI sources exist
    add_custom_target(ui_lib
        COMMENT "No UI sources found - creating empty target"
    )
endif()

# UI applications
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/apps)
    file(GLOB UI_APP_SOURCES "apps/*.c")
    
    foreach(app_source ${UI_APP_SOURCES})
        get_filename_component(app_name ${app_source} NAME_WE)
        
        add_executable(${app_name} ${app_source})
        
        target_include_directories(${app_name} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/framework/include
            ${CMAKE_SOURCE_DIR}/kernel/core/include
        )
        
        target_compile_options(${app_name} PRIVATE
            -ffreestanding
            -fno-builtin
            -fno-stack-protector
            -nostdlib
        )
        
        if(TARGET ui_lib AND UI_SOURCES_FILTERED)
            target_link_libraries(${app_name} ui_lib)
        endif()
        
        install(TARGETS ${app_name}
            RUNTIME DESTINATION bin
        )
    endforeach()
endif()

# Create placeholder files if UI directory is empty
if(NOT UI_SOURCES_FILTERED AND NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/framework)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ui_placeholder.txt 
         "UI framework will be implemented here")
    
    add_custom_target(ui_placeholder
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ui_placeholder.txt
        COMMENT "UI framework placeholder"
    )
    
    # Alias for consistency
    add_custom_target(ui_lib DEPENDS ui_placeholder)
endif()