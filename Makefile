# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/project/tadpole_master

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/project/tadpole_master

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/local/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/local/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /root/project/tadpole_master/CMakeFiles /root/project/tadpole_master/CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /root/project/tadpole_master/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named test_yaml

# Build rule for target.
test_yaml: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_yaml
.PHONY : test_yaml

# fast build rule for target.
test_yaml/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_yaml.dir/build.make CMakeFiles/test_yaml.dir/build
.PHONY : test_yaml/fast

#=============================================================================
# Target rules for targets named test_config

# Build rule for target.
test_config: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_config
.PHONY : test_config

# fast build rule for target.
test_config/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/build
.PHONY : test_config/fast

#=============================================================================
# Target rules for targets named test_log

# Build rule for target.
test_log: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_log
.PHONY : test_log

# fast build rule for target.
test_log/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_log.dir/build.make CMakeFiles/test_log.dir/build
.PHONY : test_log/fast

#=============================================================================
# Target rules for targets named tadpole

# Build rule for target.
tadpole: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 tadpole
.PHONY : tadpole

# fast build rule for target.
tadpole/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tadpole.dir/build.make CMakeFiles/tadpole.dir/build
.PHONY : tadpole/fast

src/config.o: src/config.cc.o

.PHONY : src/config.o

# target to build an object file
src/config.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tadpole.dir/build.make CMakeFiles/tadpole.dir/src/config.cc.o
.PHONY : src/config.cc.o

src/config.i: src/config.cc.i

.PHONY : src/config.i

# target to preprocess a source file
src/config.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tadpole.dir/build.make CMakeFiles/tadpole.dir/src/config.cc.i
.PHONY : src/config.cc.i

src/config.s: src/config.cc.s

.PHONY : src/config.s

# target to generate assembly for a file
src/config.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tadpole.dir/build.make CMakeFiles/tadpole.dir/src/config.cc.s
.PHONY : src/config.cc.s

src/log.o: src/log.cc.o

.PHONY : src/log.o

# target to build an object file
src/log.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tadpole.dir/build.make CMakeFiles/tadpole.dir/src/log.cc.o
.PHONY : src/log.cc.o

src/log.i: src/log.cc.i

.PHONY : src/log.i

# target to preprocess a source file
src/log.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tadpole.dir/build.make CMakeFiles/tadpole.dir/src/log.cc.i
.PHONY : src/log.cc.i

src/log.s: src/log.cc.s

.PHONY : src/log.s

# target to generate assembly for a file
src/log.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tadpole.dir/build.make CMakeFiles/tadpole.dir/src/log.cc.s
.PHONY : src/log.cc.s

src/util.o: src/util.cc.o

.PHONY : src/util.o

# target to build an object file
src/util.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tadpole.dir/build.make CMakeFiles/tadpole.dir/src/util.cc.o
.PHONY : src/util.cc.o

src/util.i: src/util.cc.i

.PHONY : src/util.i

# target to preprocess a source file
src/util.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tadpole.dir/build.make CMakeFiles/tadpole.dir/src/util.cc.i
.PHONY : src/util.cc.i

src/util.s: src/util.cc.s

.PHONY : src/util.s

# target to generate assembly for a file
src/util.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tadpole.dir/build.make CMakeFiles/tadpole.dir/src/util.cc.s
.PHONY : src/util.cc.s

tests/test_config.o: tests/test_config.cc.o

.PHONY : tests/test_config.o

# target to build an object file
tests/test_config.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/tests/test_config.cc.o
.PHONY : tests/test_config.cc.o

tests/test_config.i: tests/test_config.cc.i

.PHONY : tests/test_config.i

# target to preprocess a source file
tests/test_config.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/tests/test_config.cc.i
.PHONY : tests/test_config.cc.i

tests/test_config.s: tests/test_config.cc.s

.PHONY : tests/test_config.s

# target to generate assembly for a file
tests/test_config.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/tests/test_config.cc.s
.PHONY : tests/test_config.cc.s

tests/test_log.o: tests/test_log.cc.o

.PHONY : tests/test_log.o

# target to build an object file
tests/test_log.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_log.dir/build.make CMakeFiles/test_log.dir/tests/test_log.cc.o
.PHONY : tests/test_log.cc.o

tests/test_log.i: tests/test_log.cc.i

.PHONY : tests/test_log.i

# target to preprocess a source file
tests/test_log.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_log.dir/build.make CMakeFiles/test_log.dir/tests/test_log.cc.i
.PHONY : tests/test_log.cc.i

tests/test_log.s: tests/test_log.cc.s

.PHONY : tests/test_log.s

# target to generate assembly for a file
tests/test_log.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_log.dir/build.make CMakeFiles/test_log.dir/tests/test_log.cc.s
.PHONY : tests/test_log.cc.s

tests/test_yaml.o: tests/test_yaml.cc.o

.PHONY : tests/test_yaml.o

# target to build an object file
tests/test_yaml.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_yaml.dir/build.make CMakeFiles/test_yaml.dir/tests/test_yaml.cc.o
.PHONY : tests/test_yaml.cc.o

tests/test_yaml.i: tests/test_yaml.cc.i

.PHONY : tests/test_yaml.i

# target to preprocess a source file
tests/test_yaml.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_yaml.dir/build.make CMakeFiles/test_yaml.dir/tests/test_yaml.cc.i
.PHONY : tests/test_yaml.cc.i

tests/test_yaml.s: tests/test_yaml.cc.s

.PHONY : tests/test_yaml.s

# target to generate assembly for a file
tests/test_yaml.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_yaml.dir/build.make CMakeFiles/test_yaml.dir/tests/test_yaml.cc.s
.PHONY : tests/test_yaml.cc.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... test_yaml"
	@echo "... test_config"
	@echo "... edit_cache"
	@echo "... test_log"
	@echo "... tadpole"
	@echo "... src/config.o"
	@echo "... src/config.i"
	@echo "... src/config.s"
	@echo "... src/log.o"
	@echo "... src/log.i"
	@echo "... src/log.s"
	@echo "... src/util.o"
	@echo "... src/util.i"
	@echo "... src/util.s"
	@echo "... tests/test_config.o"
	@echo "... tests/test_config.i"
	@echo "... tests/test_config.s"
	@echo "... tests/test_log.o"
	@echo "... tests/test_log.i"
	@echo "... tests/test_log.s"
	@echo "... tests/test_yaml.o"
	@echo "... tests/test_yaml.i"
	@echo "... tests/test_yaml.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

