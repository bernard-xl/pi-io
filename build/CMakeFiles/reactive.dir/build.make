# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/parallels/new-pi

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/parallels/new-pi/build

# Include any dependencies generated for this target.
include CMakeFiles/reactive.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/reactive.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/reactive.dir/flags.make

CMakeFiles/reactive.dir/reactive.c.o: CMakeFiles/reactive.dir/flags.make
CMakeFiles/reactive.dir/reactive.c.o: ../reactive.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/parallels/new-pi/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/reactive.dir/reactive.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/reactive.dir/reactive.c.o   -c /home/parallels/new-pi/reactive.c

CMakeFiles/reactive.dir/reactive.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/reactive.dir/reactive.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/parallels/new-pi/reactive.c > CMakeFiles/reactive.dir/reactive.c.i

CMakeFiles/reactive.dir/reactive.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/reactive.dir/reactive.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/parallels/new-pi/reactive.c -o CMakeFiles/reactive.dir/reactive.c.s

CMakeFiles/reactive.dir/reactive.c.o.requires:
.PHONY : CMakeFiles/reactive.dir/reactive.c.o.requires

CMakeFiles/reactive.dir/reactive.c.o.provides: CMakeFiles/reactive.dir/reactive.c.o.requires
	$(MAKE) -f CMakeFiles/reactive.dir/build.make CMakeFiles/reactive.dir/reactive.c.o.provides.build
.PHONY : CMakeFiles/reactive.dir/reactive.c.o.provides

CMakeFiles/reactive.dir/reactive.c.o.provides.build: CMakeFiles/reactive.dir/reactive.c.o

# Object files for target reactive
reactive_OBJECTS = \
"CMakeFiles/reactive.dir/reactive.c.o"

# External object files for target reactive
reactive_EXTERNAL_OBJECTS =

reactive: CMakeFiles/reactive.dir/reactive.c.o
reactive: CMakeFiles/reactive.dir/build.make
reactive: io/libio.a
reactive: utils/libutils.a
reactive: /usr/lib/x86_64-linux-gnu/libbluetooth.so
reactive: CMakeFiles/reactive.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable reactive"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/reactive.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/reactive.dir/build: reactive
.PHONY : CMakeFiles/reactive.dir/build

CMakeFiles/reactive.dir/requires: CMakeFiles/reactive.dir/reactive.c.o.requires
.PHONY : CMakeFiles/reactive.dir/requires

CMakeFiles/reactive.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/reactive.dir/cmake_clean.cmake
.PHONY : CMakeFiles/reactive.dir/clean

CMakeFiles/reactive.dir/depend:
	cd /home/parallels/new-pi/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/parallels/new-pi /home/parallels/new-pi /home/parallels/new-pi/build /home/parallels/new-pi/build /home/parallels/new-pi/build/CMakeFiles/reactive.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/reactive.dir/depend

