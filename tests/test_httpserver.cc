#include "src/http/http_server.h"
#include "src/iomanager.h"
#include "src/http/http_servlet.h"

using namespace tadpole;

int main (){
	FunctionServlet::ptr fser(new FunctionServlet(
		[](HttpResponce::ptr res){
			res->setBody("res");
		}));
	ServletMgr::GetInstance()->addServlet("/res",fser);

	IOManager::ptr iom(new IOManager(4));
	IOManager::ptr acc(new IOManager(1));
	HttpServer::ptr https(new HttpServer(iom.get(),acc.get()));
	IPAddress::ptr ip = IPAddress::Create("0.0.0.0",80);
	std::vector<Address::ptr> ips;
	ips.push_back(ip);

	https->bind(ips);
}
