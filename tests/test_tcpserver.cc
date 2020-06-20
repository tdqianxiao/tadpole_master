#include "src/iomanager.h"
#include "src/tcpserver.h"
#include "src/log.h"
#include "src/address.h"
#include <vector>

tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

int main (){
	tadpole::IOManager::ptr iom(new tadpole::IOManager(4));
	tadpole::TcpServer::ptr server(new tadpole::TcpServer);
	std::vector<tadpole::Address::ptr> vec ; 
	auto ip = tadpole::IPAddress::Create("127.0.0.1",8090);
	vec.push_back(ip);
	server->bind(vec);
	server->start();
}
