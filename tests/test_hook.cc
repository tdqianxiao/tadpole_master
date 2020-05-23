#include "iomanager.h"
#include <unistd.h>
#include <time.h>
#include "hook.h"
#include "log.h"

using namespace tadpole;

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");


void testsleep(){
	TADPOLE_LOG_INFO(g_logger) << "test begin";
	sleep(1);
//	usleep(2000000);
//	struct timespec tm ; 
//	tm.tv_sec = 1;
//	tm.tv_nsec = 1*1000*1000*1000;
//	nanosleep((const timespec *)&tm,nullptr);
	TADPOLE_LOG_INFO(g_logger) << "test end";
	int x; 
	recv(4,&x,4,0);
}

int main(){
	set_hook_enable(false);
	IOManager::ptr iom(new IOManager(1));
	iom->schedule(testsleep);
	while(1);
}
