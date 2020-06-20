#include "src/iomanager.h"
#include "src/tcpserver.h"
#include "src/log.h"
#include "src/address.h"
#include "src/bytearray.h"
#include "src/http/http_request.h"
#include "src/http/http_responce.h"

namespace tadpole{

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

char html[]  = 	"<html>" 
				"<h2 align = \"center\">hello 浅笑</h2>"
			    "</html>";

class HttpServer:public TcpServer{
public:
	typedef std::shared_ptr<HttpServer> ptr;
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

		std::string req = byte->moveToString(ret);
		HttpRequestParser::ptr preq(new HttpRequestParser);
		preq->execute(req.c_str(),req.size());
		auto it = preq->getRequest();
		std::string str; 
		if(it->getMethod() == http_method::HTTP_GET){
			HttpResponce::ptr htpc(new HttpResponce);
			htpc->setField("Server","tadpole/1.0");
			htpc->setField("Content-type","text/html;charset=utf-8");
			std::stringstream ss; 
			ss<< strlen(html);
			htpc->setField("Content-Length",ss.str());
			htpc->setField("Date","Thu, 04 Jun 2020 19:40:28 GMT");
			str = htpc->toString();
		}

		sock->send(&str[0],str.size());
		sock->send(html,strlen(html));
		sock->close();
	}	
};

}

int main (){
	tadpole::IOManager::ptr iom(new tadpole::IOManager(4));
	tadpole::HttpServer::ptr server(new tadpole::HttpServer);
	tadpole::Address::ptr ip = tadpole::IPAddress::Create("127.0.0.1",80);
	std::vector<tadpole::Address::ptr> vec = {ip};
	server->bind(vec);
}


