#include "iomanager.h"
#include "log.h"

#include <unistd.h>
#include <arpa/inet.h>

using namespace tadpole;

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

void fun(){
	TADPOLE_LOG_INFO(g_logger) << "fun ~";
}

void fun1(){
	TADPOLE_LOG_INFO(g_logger) << "fun1 ~";
}

void fun2(){
	TADPOLE_LOG_INFO(g_logger) << "fun22 ~";
}

void testEvent(){
	int fd = socket(AF_INET,SOCK_STREAM,0);

	sockaddr_in addr ; 
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8090);
	addr.sin_addr.s_addr = INADDR_ANY;
	bind(fd,(sockaddr*)&addr,sizeof(addr));

	listen(fd,36);

//	accept(fd,nullptr,nullptr);
	TADPOLE_LOG_INFO(g_logger) << "accept success";

	IOManager::ptr iom(new IOManager(8));

//	iom->addEvent(cfd,(IOManager::Event)EPOLLIN,&fun);
	
	std::shared_ptr<int> iptr(new int(15));

	Timer::ptr timer = iom->addCondTimer(5000,&fun,iptr,true);
	//Timer::ptr timer1 = iom->addTimer(500,&fun1,true);
	//Timer::ptr timer2 = iom->addTimer(500,&fun2,true);
	//auto ptr = IOManager::GetCurIOM();
	//ptr->addTimer(500,&fun,true);
	/*
	sleep(3);
	timer->cancel();
	sleep(2);
	timer1->cancel();
	sleep(5);
	*/
	timer->reflush();
	sleep(3);
//	iptr.reset();

	sleep(7);
}

void testTime(){
	int f = GetTimeOfMS();
	sleep(2);
	int x = GetTimeOfMS() - f ; 
	TADPOLE_LOG_INFO(g_logger) <<x;
}

void test_iomanager(){
	IOManager::ptr iom(new IOManager(8));
	iom->schedule(&fun);
}

int main (){
	//test_iomanager();	
	testEvent();
	//testTime();
	return 0 ; 
}
