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

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/steve/mysql-server/mysql-5.6

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/steve/mysql-server/mysql-5.6

# Include any dependencies generated for this target.
include libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/depend.make

# Include the progress variables for this target.
include libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/progress.make

# Include the compile flags for this target's objects.
include libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/flags.make

libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o: libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/flags.make
libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o: tests/mysql_client_test.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/steve/mysql-server/mysql-5.6/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o"
	cd /home/steve/mysql-server/mysql-5.6/libmysqld/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o   -c /home/steve/mysql-server/mysql-5.6/tests/mysql_client_test.c

libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.i"
	cd /home/steve/mysql-server/mysql-5.6/libmysqld/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/steve/mysql-server/mysql-5.6/tests/mysql_client_test.c > CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.i

libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.s"
	cd /home/steve/mysql-server/mysql-5.6/libmysqld/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/steve/mysql-server/mysql-5.6/tests/mysql_client_test.c -o CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.s

libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o.requires:
.PHONY : libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o.requires

libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o.provides: libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o.requires
	$(MAKE) -f libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/build.make libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o.provides.build
.PHONY : libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o.provides

libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o.provides.build: libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o

# Object files for target mysql_client_test_embedded
mysql_client_test_embedded_OBJECTS = \
"CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o"

# External object files for target mysql_client_test_embedded
mysql_client_test_embedded_EXTERNAL_OBJECTS =

libmysqld/examples/mysql_client_test_embedded: libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o
libmysqld/examples/mysql_client_test_embedded: libmysqld/libmysqld.a
libmysqld/examples/mysql_client_test_embedded: libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/build.make
libmysqld/examples/mysql_client_test_embedded: libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable mysql_client_test_embedded"
	cd /home/steve/mysql-server/mysql-5.6/libmysqld/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mysql_client_test_embedded.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/build: libmysqld/examples/mysql_client_test_embedded
.PHONY : libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/build

libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/requires: libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/__/__/tests/mysql_client_test.c.o.requires
.PHONY : libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/requires

libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/clean:
	cd /home/steve/mysql-server/mysql-5.6/libmysqld/examples && $(CMAKE_COMMAND) -P CMakeFiles/mysql_client_test_embedded.dir/cmake_clean.cmake
.PHONY : libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/clean

libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/depend:
	cd /home/steve/mysql-server/mysql-5.6 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/steve/mysql-server/mysql-5.6 /home/steve/mysql-server/mysql-5.6/libmysqld/examples /home/steve/mysql-server/mysql-5.6 /home/steve/mysql-server/mysql-5.6/libmysqld/examples /home/steve/mysql-server/mysql-5.6/libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : libmysqld/examples/CMakeFiles/mysql_client_test_embedded.dir/depend

