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

# Utility rule file for symlink_libmysqlclient_r.so.18.

# Include the progress variables for this target.
include libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18.dir/progress.make

libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18: libmysql/libmysqlclient_r.so.18

libmysql/libmysqlclient_r.so.18: libmysql/libmysqlclient.so
	$(CMAKE_COMMAND) -E cmake_progress_report /home/steve/mysql-server/mysql-5.6/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating libmysqlclient_r.so.18"
	cd /home/steve/mysql-server/mysql-5.6/libmysql && /usr/bin/cmake -E remove -f /home/steve/mysql-server/mysql-5.6/libmysql/libmysqlclient_r.so.18
	cd /home/steve/mysql-server/mysql-5.6/libmysql && /usr/bin/cmake -E create_symlink libmysqlclient.so.18 libmysqlclient_r.so.18

symlink_libmysqlclient_r.so.18: libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18
symlink_libmysqlclient_r.so.18: libmysql/libmysqlclient_r.so.18
symlink_libmysqlclient_r.so.18: libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18.dir/build.make
.PHONY : symlink_libmysqlclient_r.so.18

# Rule to build all files generated by this target.
libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18.dir/build: symlink_libmysqlclient_r.so.18
.PHONY : libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18.dir/build

libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18.dir/clean:
	cd /home/steve/mysql-server/mysql-5.6/libmysql && $(CMAKE_COMMAND) -P CMakeFiles/symlink_libmysqlclient_r.so.18.dir/cmake_clean.cmake
.PHONY : libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18.dir/clean

libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18.dir/depend:
	cd /home/steve/mysql-server/mysql-5.6 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/steve/mysql-server/mysql-5.6 /home/steve/mysql-server/mysql-5.6/libmysql /home/steve/mysql-server/mysql-5.6 /home/steve/mysql-server/mysql-5.6/libmysql /home/steve/mysql-server/mysql-5.6/libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : libmysql/CMakeFiles/symlink_libmysqlclient_r.so.18.dir/depend

