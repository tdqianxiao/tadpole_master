cmake_minimum_required(VERSION 3.0)
project(tadpole)

include (utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(.)
include_directories(./src)

set(LIB_SRC
      src/log.cc
	  src/util.cc
    )

add_library(tadpole SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(tadpole)
find_library(PTHREAD pthread)

set(LIBS
        tadpole
        )

tadpole_add_executable(test_log "tests/test_log.cc" tadpole "${LIBS}")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
