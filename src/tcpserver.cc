#include "tcpserver.h"
#include "log.h"

namespace tadpole{

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

TcpServer::TcpServer(IOManager * iom)
	:m_iom(iom){
}

TcpServer::~TcpServer(){
			std::cout<< "tag";
	for(auto & it: m_socks){
		it->close();
	}
}

void TcpServer::bind(const std::vector<Address::ptr>& addrs){
	for(auto & it : addrs){
		auto sock = Socket::CreateTcpSocket();
		m_socks.push_back(sock);
		while(1){
			bool is_success = false;
			is_success = sock->bind(it);
			if(!is_success){
				sleep(3);
			}else{
				TADPOLE_LOG_INFO(g_logger)<<"bind "<< it->toString()
									<< " successful !";
				break; 
			}
		}
		sock->listen();
	}
	start();
}

void TcpServer::start(){
	if(!m_isStart){
		for(auto &it : m_socks){
			m_iom->schedule(std::bind(&TcpServer::startAccept
							,shared_from_this(),it));
		}
		m_isStart = true; 
	}
}

void TcpServer::startAccept(Socket::ptr sfd){
	while(1){
		Socket::ptr cfd = sfd->accept();
		if(cfd){
			m_iom->schedule(std::bind(&TcpServer::newClient,
									shared_from_this(),cfd));	
		}
	}
}

}
