#include <iostream>
#include <unistd.h>
#include "src/log.h"
#include "src/util.h"

static tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

void test2(){
	TADPOLE_LOG_INFO(g_logger)<< "hello logger";

	tadpole::FileLogAppender::ptr append(new tadpole::FileLogAppender("./log.txt"));
	g_logger->addAppender(append);
	
	sleep(2);
	TADPOLE_LOG_INFO(g_logger)<< "hello logger ~";
}

int main(){
	test2();
	return 0 ; 
}
