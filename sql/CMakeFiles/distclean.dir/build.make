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

# Utility rule file for distclean.

# Include the progress variables for this target.
include sql/CMakeFiles/distclean.dir/progress.make

sql/CMakeFiles/distclean:
	cd /home/steve/mysql-server/mysql-5.6/sql && /usr/bin/cmake -E echo WARNING: distclean target is not functional
	cd /home/steve/mysql-server/mysql-5.6/sql && /usr/bin/cmake -E echo Use "'bzr" "clean-tree'" with --unknown and/or --ignored parameter instead

distclean: sql/CMakeFiles/distclean
distclean: sql/CMakeFiles/distclean.dir/build.make
.PHONY : distclean

# Rule to build all files generated by this target.
sql/CMakeFiles/distclean.dir/build: distclean
.PHONY : sql/CMakeFiles/distclean.dir/build

sql/CMakeFiles/distclean.dir/clean:
	cd /home/steve/mysql-server/mysql-5.6/sql && $(CMAKE_COMMAND) -P CMakeFiles/distclean.dir/cmake_clean.cmake
.PHONY : sql/CMakeFiles/distclean.dir/clean

sql/CMakeFiles/distclean.dir/depend:
	cd /home/steve/mysql-server/mysql-5.6 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/steve/mysql-server/mysql-5.6 /home/steve/mysql-server/mysql-5.6/sql /home/steve/mysql-server/mysql-5.6 /home/steve/mysql-server/mysql-5.6/sql /home/steve/mysql-server/mysql-5.6/sql/CMakeFiles/distclean.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : sql/CMakeFiles/distclean.dir/depend

