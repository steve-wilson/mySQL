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
include tests/CMakeFiles/mysql_client_test.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/mysql_client_test.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/mysql_client_test.dir/flags.make

tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o: tests/CMakeFiles/mysql_client_test.dir/flags.make
tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o: tests/mysql_client_test.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/steve/mysql-server/mysql-5.6/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o"
	cd /home/steve/mysql-server/mysql-5.6/tests && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o   -c /home/steve/mysql-server/mysql-5.6/tests/mysql_client_test.c

tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mysql_client_test.dir/mysql_client_test.c.i"
	cd /home/steve/mysql-server/mysql-5.6/tests && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/steve/mysql-server/mysql-5.6/tests/mysql_client_test.c > CMakeFiles/mysql_client_test.dir/mysql_client_test.c.i

tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mysql_client_test.dir/mysql_client_test.c.s"
	cd /home/steve/mysql-server/mysql-5.6/tests && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/steve/mysql-server/mysql-5.6/tests/mysql_client_test.c -o CMakeFiles/mysql_client_test.dir/mysql_client_test.c.s

tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o.requires:
.PHONY : tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o.requires

tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o.provides: tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o.requires
	$(MAKE) -f tests/CMakeFiles/mysql_client_test.dir/build.make tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o.provides.build
.PHONY : tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o.provides

tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o.provides.build: tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o

# Object files for target mysql_client_test
mysql_client_test_OBJECTS = \
"CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o"

# External object files for target mysql_client_test
mysql_client_test_EXTERNAL_OBJECTS =

tests/mysql_client_test: tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o
tests/mysql_client_test: libmysql/libmysqlclient.a
tests/mysql_client_test: tests/CMakeFiles/mysql_client_test.dir/build.make
tests/mysql_client_test: tests/CMakeFiles/mysql_client_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable mysql_client_test"
	cd /home/steve/mysql-server/mysql-5.6/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mysql_client_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/mysql_client_test.dir/build: tests/mysql_client_test
.PHONY : tests/CMakeFiles/mysql_client_test.dir/build

tests/CMakeFiles/mysql_client_test.dir/requires: tests/CMakeFiles/mysql_client_test.dir/mysql_client_test.c.o.requires
.PHONY : tests/CMakeFiles/mysql_client_test.dir/requires

tests/CMakeFiles/mysql_client_test.dir/clean:
	cd /home/steve/mysql-server/mysql-5.6/tests && $(CMAKE_COMMAND) -P CMakeFiles/mysql_client_test.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/mysql_client_test.dir/clean

tests/CMakeFiles/mysql_client_test.dir/depend:
	cd /home/steve/mysql-server/mysql-5.6 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/steve/mysql-server/mysql-5.6 /home/steve/mysql-server/mysql-5.6/tests /home/steve/mysql-server/mysql-5.6 /home/steve/mysql-server/mysql-5.6/tests /home/steve/mysql-server/mysql-5.6/tests/CMakeFiles/mysql_client_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/mysql_client_test.dir/depend

