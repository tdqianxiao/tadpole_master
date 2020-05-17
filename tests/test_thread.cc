#include "thread.h"
#include "log.h"
#include "mutex.h"
#include <unistd.h>

static tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");
void fun(){
	TADPOLE_LOG_INFO(g_logger)<< "fun ";
	TADPOLE_LOG_INFO(g_logger)<< tadpole::Thread::GetCurThread()->getThreadId();
	TADPOLE_LOG_INFO(g_logger)<< tadpole::Thread::GetCurThread()->getThreadName();
}

void test_thread(){
	std::vector<tadpole::Thread::ptr> threads; 
	threads.resize(4);
	for(int i = 0 ; i < 4; ++i){
		threads[i].reset(new tadpole::Thread(&fun,"thread"+ std::to_string(i)));
	}
	TADPOLE_LOG_INFO(g_logger)<< "main";
	sleep(1);
}

int count = 0 ; 
tadpole::SpinLock mutex ; 

void add(){
	for(int i = 0 ; i < 100000; ++i){
		TADPOLE_LOG_INFO(g_logger)<< "--------------";
	}
}

void test_mutex(){
	std::vector<tadpole::Thread::ptr> threads; 
	threads.resize(4);
	for(int i = 0 ; i < 4; ++i){
		threads[i].reset(new tadpole::Thread(&add,"thread"+ std::to_string(i)));
	}

	for(int i = 0 ; i < 4; ++i){
		threads[i]->join();
	}
	TADPOLE_LOG_INFO(TADPOLE_FIND_LOGGER("system")) << count;
}

int main (){
	//	test_thread();	

	test_mutex();
}
