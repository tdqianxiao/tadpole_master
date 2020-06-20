#include "src/iomanager.h"
#include "src/tcpserver.h"
#include "src/log.h"
#include "src/address.h"
#include "src/bytearray.h"
#include "src/http/http_session.h"

namespace tadpole{

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

class HttpServer:public TcpServer{
public:
	typedef std::shared_ptr<HttpServer> ptr;
	void newClient(Socket::ptr sock)override{
		HttpSession::ptr session(new HttpSession(sock));
		session->recvRequest();
		session->sendResponce();
	}	
};

}

int main (){
	tadpole::IOManager::ptr iom(new tadpole::IOManager(4));
	tadpole::HttpServer::ptr server(new tadpole::HttpServer);
	tadpole::Address::ptr ip = tadpole::IPAddress::Create("0.0.0.0",80);
	std::vector<tadpole::Address::ptr> vec = {ip};
	server->bind(vec);
}


