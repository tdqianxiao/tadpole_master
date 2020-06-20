#ifndef __TADPOLE_HTTP_HTTP_SERVER_H__
#define __TADPOLE_HTTP_HTTP_SERVER_H__

#include "src/tcpserver.h"
#include "http_session.h"
#include "src/log.h"

namespace tadpole{

class HttpServer:public TcpServer{
public:
	typedef std::shared_ptr<HttpServer> ptr;
	HttpServer(IOManager * iom = IOManager::GetCurIOM()
			 , IOManager *acc = IOManager::GetCurIOM())
			 :TcpServer(iom,acc){		 
	}
	void newClient(Socket::ptr sock)override{
		HttpSession::ptr session(new HttpSession(sock));
		do{
		session->recvRequest();
		session->sendResponce();
		}while(session->isKeepalive());
	}	
};

}

#endif 
