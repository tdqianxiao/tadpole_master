cmake_minimum_required(VERSION 3.0)
project(tadpole)

include (utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(.)
include_directories(~/yaml-cpp-master/include/yaml-cpp)

set(LIB_SRC
	  src/hook.cc
	  src/config.cc
	  src/util.cc
	  src/fdmanager.cc
      src/log.cc
	  src/thread.cc
	  src/mutex.cc
	  src/fiber.cc
	  src/scheduler.cc
	  src/iomanager.cc
	  src/timer.cc
	  src/address.cc
	  src/socket.cc
	  src/bytearray.cc
	  src/tcpserver.cc
	  src/http/http11_parser.cc
	  src/http/httpclient_parser.cc
	  src/http/http_request.cc
	  src/http/http_responce.cc
	  src/http/http_session.cc
	  src/http/http_servlet.cc
	  examples/FightLandlord/Random.cc
	  examples/FightLandlord/ws_session.cc
	  examples/FightLandlord/Room.cc
	  examples/FightLandlord/protocal.cc
    )

add_library(tadpole SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(tadpole)
find_library(PTHREAD pthread)

set(LIBS
        tadpole
		yaml-cpp
		pthread
		dl
        )


tadpole_add_executable(test_log "tests/test_log.cc" tadpole "${LIBS}")
tadpole_add_executable(test_config "tests/test_config.cc" tadpole "${LIBS}")
tadpole_add_executable(test_yaml "tests/test_yaml.cc" tadpole "${LIBS}")
tadpole_add_executable(test_thread "tests/test_thread.cc" tadpole "${LIBS}")
tadpole_add_executable(test_fiber "tests/test_fiber.cc" tadpole "${LIBS}")
tadpole_add_executable(test_scheduler "tests/test_scheduler.cc" tadpole "${LIBS}")
tadpole_add_executable(test_iomanager "tests/test_iomanager.cc" tadpole "${LIBS}")
tadpole_add_executable(test_hook "tests/test_hook.cc" tadpole "${LIBS}")
tadpole_add_executable(test_addr "tests/test_addr.cc" tadpole "${LIBS}")
tadpole_add_executable(test_socket "tests/test_socket.cc" tadpole "${LIBS}")
tadpole_add_executable(test_bytearray "tests/test_bytearray.cc" tadpole "${LIBS}")
tadpole_add_executable(test_parser "tests/test_parser.cc" tadpole "${LIBS}")
tadpole_add_executable(test_tcpserver "tests/test_tcpserver.cc" tadpole "${LIBS}")
tadpole_add_executable(echo_server "examples/echoServer.cc" tadpole "${LIBS}")
tadpole_add_executable(test_httprequest "tests/test_httprequest.cc" tadpole "${LIBS}")
tadpole_add_executable(test_httpresponce "tests/test_httpresponce.cc" tadpole "${LIBS}")
tadpole_add_executable(test_httpsession "tests/test_httpsession.cc" tadpole "${LIBS}")
tadpole_add_executable(test_httpserver "tests/test_httpserver.cc" tadpole "${LIBS}")
tadpole_add_executable(test_random "tests/test_random.cc" tadpole "${LIBS}")
tadpole_add_executable(fl_server "examples/FLServer.cc" tadpole "${LIBS}")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
