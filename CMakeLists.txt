cmake_minimum_required(VERSION 3.0)
project(tadpole)

include (utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(.)
include_directories(./src)
include_directories(~/yaml-cpp-master/include/yaml-cpp)

set(LIB_SRC
      src/log.cc
	  src/util.cc
	  src/config.cc
	  src/thread.cc
	  src/mutex.cc
	  src/fiber.cc
	  src/scheduler.cc
    )

add_library(tadpole SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(tadpole)
find_library(PTHREAD pthread)

set(LIBS
        tadpole
		yaml-cpp
		pthread
        )


tadpole_add_executable(test_log "tests/test_log.cc" tadpole "${LIBS}")
tadpole_add_executable(test_config "tests/test_config.cc" tadpole "${LIBS}")
tadpole_add_executable(test_yaml "tests/test_yaml.cc" tadpole "${LIBS}")
tadpole_add_executable(test_thread "tests/test_thread.cc" tadpole "${LIBS}")
tadpole_add_executable(test_fiber "tests/test_fiber.cc" tadpole "${LIBS}")
tadpole_add_executable(test_scheduler "tests/test_scheduler.cc" tadpole "${LIBS}")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
