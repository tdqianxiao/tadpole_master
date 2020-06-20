#include "src/address.h"
#include "src/log.h"

using namespace tadpole;

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

void testLookup(){
	std::vector<Address::ptr> ips ; 
	Address::Lookup(ips,"www.baidu.com","http");
	for(auto &it : ips ){
		TADPOLE_LOG_INFO(g_logger)<<it->toString();
	}


	TADPOLE_LOG_INFO(g_logger)<<std::endl;

}

void testIfAddr(){
	std::multimap<std::string,Address::ptr> ifs;
	Address::LookupInterfaces(ifs);
	for(auto & it : ifs){
		TADPOLE_LOG_INFO(g_logger)<<it.first<<" - "<< it.second->toString();
	}

}

int main (){
	//IPAddress::ptr ip = IPAddress::Create("ffff::ffff",8090);
	//if(ip){
	//	TADPOLE_LOG_INFO(g_logger)<<ip->toString();
	//}

	//testLookup();
	testIfAddr();
	return 0 ; 
}
