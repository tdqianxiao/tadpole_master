#include "src/socket.h"
#include "src/iomanager.h"
#include "src/log.h"

using namespace tadpole;

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

void test_socket(){

	Socket::ptr sock = Socket::CreateTcpSocket();

	std::vector<Address::ptr> vec;
	Address::Lookup(vec,"www.baidu.com","http");
	if(!vec.empty()){
		auto it = *(vec.begin());
		TADPOLE_LOG_INFO(g_logger)<< it->toString();
		if(!sock->connect(it)){
			TADPOLE_LOG_INFO(g_logger)<< "connect no success!";
			return ; 
		}
	}

	TADPOLE_LOG_INFO(g_logger)<< "connect no success!";
	char req[128] = "GET / HTTP/1.0\r\n\r\n";

	sock->send(req,128);

	char buf [4096] = {};

	sock->recv(buf,4096);

	TADPOLE_LOG_INFO(g_logger) << buf;
}

void test_server(){
	IPAddress::ptr ip = IPAddress::Create("127.0.0.1",8090);
	Socket::ptr sock = Socket::CreateTcpSocket();
	
	sock->bind(ip);
	sock->listen();
	IPAddress::ptr caddr(new IPv4Address);
	Socket::ptr cfd = sock->accept();

	int i = 80; 
	cfd->send(&i,4);
}

void test_echo(){
	IPAddress::ptr ip = IPAddress::Create("127.0.0.1",8091);
	Socket::ptr sock = Socket::CreateTcpSocket();
	sock->bind(ip);
	sock->listen();
	
	Socket::ptr cfd = sock->accept();
	char buf [1024] = {0};
	int size = 1 ; 
	while(true){
		size = cfd->recv(buf,1024);
		int i = size == -1 ? 0 : size ; 
		std::string str(buf,i);
		//std::cout<< size;
		std::cout<<str<<std::endl;
	}
}

void test_client(){
	IPAddress::ptr ip = IPAddress::Create("127.0.0.1",8091);

	Socket::ptr sock = Socket::CreateTcpSocket();
	sock->connect(ip);
	char buf[256];
	while(1){
		memset(buf,0,256);
		std::cin>> buf;
		if(memcmp(buf,"quit",5) == 0){
			sock->close();
			return ; 
		}
		sock->send(buf,strlen(buf));
	}

}

int main (){
	IOManager::ptr iom(new IOManager(4));
	//iom->schedule(test_socket);
	//iom->schedule(test_server);
	iom->schedule(test_client);
//	test_client();
}
