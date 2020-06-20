#include "src/iomanager.h"
#include "src/tcpserver.h"
#include "src/log.h"
#include "src/address.h"
#include "src/bytearray.h"

namespace tadpole{

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

class EchoServer:public TcpServer{
public:
	typedef std::shared_ptr<EchoServer> ptr;
	void newClient(Socket::ptr sock)override{
		sock->setRecvTimeout(2000);	
		ByteArray::ptr byte(new ByteArray);
		std::vector<iovec> vec;
		byte->getWriteBuffers(vec,1024);
		int ret = sock->recv(vec);
		if(ret == -1){
			TADPOLE_LOG_ERROR(g_logger)<< "recv error errno : "
						<< errno <<" errstri : "<< strerror(errno);
			return;
		}else if(ret == 0){
			TADPOLE_LOG_INFO(g_logger) << "client logout !";
			return ; 
		}
		byte->setUsedCount(ret);

		TADPOLE_LOG_INFO(g_logger)<<byte->moveToString(ret);
		sock->close();
	}	
};

}

int main (){
	tadpole::IOManager::ptr iom(new tadpole::IOManager(4));
	tadpole::EchoServer::ptr server(new tadpole::EchoServer);
	tadpole::Address::ptr ip = tadpole::IPAddress::Create("0.0.0.0",80);
	std::vector<tadpole::Address::ptr> vec = {ip};
	server->bind(vec);
}


