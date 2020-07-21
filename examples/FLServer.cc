#include "FightLandlord/ws_session.h"
#include "src/tcpserver.h"
#include "src/log.h"

using namespace tadpole;

class FLServer:public TcpServer{
public:
	void newClient(Socket::ptr sock)override{
		WSSession::ptr sess(new WSSession(sock));
		do{
			sess->recvRequest();
		}while(!sess->isClose());
		//关闭socket
		if(sess->closeSock()){
			sock->close();
		}
		//TADPOLE_LOG_DEBUG(TADPOLE_FIND_LOGGER("root"))<<sock->getSocket()<< " : out";
	}
};

int main(){
	signal(SIGPIPE,SIG_IGN);
	IOManager::ptr iom(new IOManager(4));
	IOManager::ptr acc(new IOManager(1));
	FLServer::ptr fl(new FLServer);
	IPAddress::ptr ip = IPAddress::Create("0.0.0.0",8090);
	std::vector<Address::ptr> ips;
	ips.push_back(ip);

	fl->bind(ips);
	return 0 ; 
}
