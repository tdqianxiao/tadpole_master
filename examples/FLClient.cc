#include "src/iomanager.h"
#include "src/socket.h"
#include "src/address.h"
#include "src/log.h"
#include "FightLandlord/protocal.h"
#include "src/bytearray.h"

using namespace tadpole;

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

void test_client(){
	Socket::ptr sock = Socket::CreateTcpSocket();
//	IPAddress::ptr ip = IPAddress::Create("14.215.177.38",80);
	IPAddress::ptr ip = IPAddress::Create("127.0.0.1",8090);
	
	sock->connect(ip);
	//TADPOLE_LOG_INFO(g_logger) << "connect successful : "<< ip->toString();
	

	ByteArray::ptr ba(new ByteArray);
	int ret = 0 ; 
	while(1){
		if(ba->getSize() == 0){
			ret = sock->recvTo(ba);
		}
		if(ret <= 0 ){
			TADPOLE_LOG_INFO(g_logger)<< "ret : "<< ret <<" recv error : "<< ret<< " errno: "
									  << errno << " strerror : "<< strerror(errno);
			return ; 
		}
		TADPOLE_LOG_INFO(g_logger)<< "size : "<< ret;
		uint32_t type = 0 ; 
		ba->readFUint32(type);
		if(type == 1001){
			RetAddSession::ptr ras = RetAddSession::parser(ba);
			TADPOLE_LOG_INFO(g_logger)<< "roomId: "<<ras->roomId;
		}else if(type == 501){
			std::string name = NotifyOtherUser::parser(ba);
			TADPOLE_LOG_INFO(g_logger)<< "adduser name : "<< name;
		}else if(type == 502){
			int pri = 0 ; 	
			std::shared_ptr<uint8_t> buffs = CardDistributor::parser(ba,pri);
			uint8_t *cards = buffs.get();
			for(int i = 0 ; i < 17; ++i){
				std::cout<< (int)cards[i]<< " ";
			}
			std::cout<<std::endl;
		}else if(type == 503){
			uint32_t a;
			int b ; 
			ba->readFUint32(a);
			ba->readFInt32(b);
		}
	}
	sock->close();
}

int main (){
	IOManager::ptr iom(new IOManager(4));
	iom->schedule(test_client);
	//test_client();
}
