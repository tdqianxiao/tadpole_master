#include "util.h"
#include "fiber.h"
#include <sys/time.h>

using namespace std::chrono;

namespace tadpole{

static time_point<high_resolution_clock> g_begin_time = 
										high_resolution_clock::now();

uint32_t GetThreadId(){
	return syscall(__NR_gettid);
}

uint32_t GetFiberId(){
	return Fiber::GetFiberId();
}

uint64_t GetElapse(){
	return duration_cast<microseconds>(high_resolution_clock::now() 
										- g_begin_time).count() / 1000;
}

uint64_t GetTimeOfMS(){
	struct timeval tm= {0};
	gettimeofday(&tm,nullptr);
	return tm.tv_sec*1000 + tm.tv_usec/1000;
}

uint64_t GetTimeOfUS(){
	struct timeval tm= {0};
	gettimeofday(&tm,nullptr);
	return tm.tv_sec*1000*1000 + tm.tv_usec;
}

}
