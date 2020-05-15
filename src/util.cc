#include "util.h"
using namespace std::chrono;

namespace tadpole{

static time_point<high_resolution_clock> g_begin_time = 
										high_resolution_clock::now();

uint32_t GetThreadId(){
	return syscall(__NR_gettid);
}

uint32_t GetFiberId(){
	return 0 ; 
}

uint64_t GetElapse(){
	return duration_cast<microseconds>(high_resolution_clock::now() 
										- g_begin_time).count() / 1000;
}
}
