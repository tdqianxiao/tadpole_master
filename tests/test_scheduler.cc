#include "scheduler.h"
#include "log.h"
#include <unistd.h>

using namespace tadpole;

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

void fun(){
	TADPOLE_LOG_INFO(g_logger)<<"fun ~";
}

void test_scheduler(){
	Scheduler::ptr sche(new Scheduler(8,true,"thread pool"));
	sche->schedule(&fun);
	sche->start();
	sche->callerBegin();
	sche->stop();
}

int main (){
	test_scheduler();
}

