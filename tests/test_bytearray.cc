#include "src/bytearray.h"
#include "src/macro.h"
#include "src/log.h"
#include "src/iomanager.h"
#include "src/socket.h"

using namespace tadpole;

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

void test_byterd(){
	ByteArray::ptr byte(new ByteArray(1));
#define XX(type,name) \
	for(int i = 0 ; i < 10000; ++i){\
		type a = (type)1000 ;\
		byte->write ## name(a);\
	} \
	TADPOLE_LOG_INFO(g_logger) <<#name<<"  --  "<<byte->getCapcity();\
	TADPOLE_LOG_INFO(g_logger) <<#name<<"  --  "<<byte->getSize();\
	for(int i = 0 ; i < 10000; ++i){\
		type b = 0 ; \
		byte->read ## name(b);\
	}
	
	XX(uint8_t,FUint8);
	XX(uint16_t,FUint16);
	XX(uint32_t,FUint32);
	XX(uint64_t,FUint64);
	
	XX(int8_t,FInt8);
	XX(int16_t,FInt16);
	XX(int32_t,FInt32);
	XX(int64_t,FInt64);

	XX(uint32_t,Uint32);
	XX(uint64_t,Uint64);
	XX(int32_t,Int32);
	XX(int64_t,Int64);
	
	std::string str("dsofafdhioahfdioahfdoiahdfioas hfdiosa f");
	byte->writeString(str);

	std::string bstr;
	byte->readString(bstr);
	TADPOLE_LOG_INFO(g_logger) << bstr;

#undef XX


	TADPOLE_LOG_INFO(g_logger) << byte->getCapcity();
	TADPOLE_LOG_INFO(g_logger) << byte->getSize();
}

void test_buff(){
	ByteArray::ptr byte(new ByteArray); 
	for(int i = 0 ; i < 10;++i){
		uint32_t a = 0xffff ;
		byte->writeFUint32(a);
	}
	std::vector<iovec> vec; 
	byte->getReadBuffers(vec,40);
	

	TADPOLE_LOG_INFO(g_logger)<<vec.size();
	TADPOLE_LOG_INFO(g_logger)<<vec[0].iov_len;
	for(auto &it : vec){
		int * buf = (int *)it.iov_base; 
		for(size_t i = 0 ; i < it.iov_len/4; ++i){
			TADPOLE_LOG_INFO(g_logger)<<buf[i];
		}
	}

	byte->freeReadBuffers();

	TADPOLE_LOG_INFO(g_logger) << byte->getCapcity();
	TADPOLE_LOG_INFO(g_logger) << byte->getSize();
}

void fun(){
	Socket::ptr sock = Socket::CreateTcpSocket();
	IPAddress::ptr ip= IPAddress::Create("127.0.0.1",8091);
	sock->bind(ip);
	sock->listen();
	ByteArray::ptr byte(new ByteArray);
	std::vector<iovec> vec;
	Socket::ptr cfd = sock->accept();
	
while(1){
	vec.clear();
	byte->getWriteBuffers(vec,1);
	int ret = cfd->recv(vec);
	if(ret > 0){
		byte->setUsedCount(ret);
		vec.clear();
		byte->getReadBuffers(vec,ret);
		for(auto &it : vec){
			TADPOLE_LOG_INFO(g_logger) << std::string(const_cast<const char *>((char * )it.iov_base),it.iov_len);
		}
		byte->freeReadBuffers();
	}
}
}

void test_server(){
	IOManager::ptr iom(new IOManager(4));
	iom->schedule(fun);
} 

int main (){
	//test_byterd();
//	test_buff();
	test_server();
}
