# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/d/RaeenOS

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/d/RaeenOS/build

# Include any dependencies generated for this target.
include tools/CMakeFiles/raeenos_emulator.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tools/CMakeFiles/raeenos_emulator.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/CMakeFiles/raeenos_emulator.dir/progress.make

# Include the compile flags for this target's objects.
include tools/CMakeFiles/raeenos_emulator.dir/flags.make

tools/CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.o: tools/CMakeFiles/raeenos_emulator.dir/flags.make
tools/CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.o: /mnt/d/RaeenOS/tools/emulator/hardware_emulator.c
tools/CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.o: tools/CMakeFiles/raeenos_emulator.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/mnt/d/RaeenOS/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tools/CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.o"
	cd /mnt/d/RaeenOS/build/tools && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT tools/CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.o -MF CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.o.d -o CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.o -c /mnt/d/RaeenOS/tools/emulator/hardware_emulator.c

tools/CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.i"
	cd /mnt/d/RaeenOS/build/tools && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/d/RaeenOS/tools/emulator/hardware_emulator.c > CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.i

tools/CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.s"
	cd /mnt/d/RaeenOS/build/tools && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/d/RaeenOS/tools/emulator/hardware_emulator.c -o CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.s

# Object files for target raeenos_emulator
raeenos_emulator_OBJECTS = \
"CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.o"

# External object files for target raeenos_emulator
raeenos_emulator_EXTERNAL_OBJECTS =

tools/raeenos_emulator: tools/CMakeFiles/raeenos_emulator.dir/emulator/hardware_emulator.c.o
tools/raeenos_emulator: tools/CMakeFiles/raeenos_emulator.dir/build.make
tools/raeenos_emulator: tools/CMakeFiles/raeenos_emulator.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/mnt/d/RaeenOS/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable raeenos_emulator"
	cd /mnt/d/RaeenOS/build/tools && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/raeenos_emulator.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/CMakeFiles/raeenos_emulator.dir/build: tools/raeenos_emulator
.PHONY : tools/CMakeFiles/raeenos_emulator.dir/build

tools/CMakeFiles/raeenos_emulator.dir/clean:
	cd /mnt/d/RaeenOS/build/tools && $(CMAKE_COMMAND) -P CMakeFiles/raeenos_emulator.dir/cmake_clean.cmake
.PHONY : tools/CMakeFiles/raeenos_emulator.dir/clean

tools/CMakeFiles/raeenos_emulator.dir/depend:
	cd /mnt/d/RaeenOS/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/d/RaeenOS /mnt/d/RaeenOS/tools /mnt/d/RaeenOS/build /mnt/d/RaeenOS/build/tools /mnt/d/RaeenOS/build/tools/CMakeFiles/raeenos_emulator.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : tools/CMakeFiles/raeenos_emulator.dir/depend

