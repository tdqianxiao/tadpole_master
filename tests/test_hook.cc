#include "iomanager.h"
#include <unistd.h>
#include <time.h>
#include "hook.h"
#include "log.h"
#include "address.h"

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

void testConnect(){
	int fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd == -1){
		return ;
	}
	
	IPv4Address::ptr ipv4 = IPv4Address::Create("14.215.177.39",80);

	int ret = connect(fd,ipv4->getAddr(),ipv4->getAddrLen());

	if(ret == 0){
		TADPOLE_LOG_INFO(g_logger)<< "connect success";
	}

	char buf[64] = "GET / HTTP/1.0\r\n\r\n";
	send(fd,buf,64,0);

	char recvBuf[4096] = {0};
	recv(fd,recvBuf,4096,0);

	TADPOLE_LOG_INFO(g_logger)<< recvBuf;
}

int main(){
	IOManager::ptr iom(new IOManager(8));
	iom->schedule(testConnect);
}
