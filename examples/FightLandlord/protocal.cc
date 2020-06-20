#include "protocal.h"
#include "src/log.h"
#include <assert.h>

namespace tadpole{

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

//简单对称加密
int encrypt(char * buf , size_t size) {
	int key = 0; 
	srand(time(NULL));
	key = rand() % 30000 + 1000;
	for (size_t i = 0; i < size; ++i) {
		buf[i] = buf[i] + (key | (key % ((i+1)<<4)));
	}
	return key; 
}
//解密
void disencrypt(char* buf, size_t size,int key) {
	for (size_t i = 0; i < size; ++i) {
		buf[i] = buf[i] - (key | (key % ((i+1) << 4)));
	}
}

//登录解析
//默认已经读取了type,
int Login::parser(ByteArray::ptr ba,
	std::function<void(const std::string & acc,
				       const std::string & pwd)> cb){
	uint16_t bodysize = 0 ; 
	ba->readFUint16(bodysize);//读body大小
	assert(bodysize < 64);
	//TADPOLE_LOG_INFO(g_logger) << "bodysize: "<<bodysize; 
	int key = 0 ; 
	ba->readInt32(key);//读密钥
	//TADPOLE_LOG_INFO(g_logger) << "key: "<<key; 

	char buf[bodysize]; 
	ba->read(buf,bodysize);//读body

	disencrypt(buf,bodysize,key);//解密

	uint8_t acclen = buf[0];
	//TADPOLE_LOG_INFO(g_logger) << "acclen: "<<(int)acclen; 
	std::string account(&buf[1],acclen);
	uint8_t pwdlen = buf[acclen+1];
	//TADPOLE_LOG_INFO(g_logger) << "pwdlen: "<<(int)pwdlen; 
	std::string password(&buf[acclen+2],pwdlen);

	cb(account,password);
	return 0; 
}

int LoginRsp::send(Socket::ptr sock , uint16_t status){
	ByteArray::ptr ba(new ByteArray(16));
	ba->writeFInt16((uint16_t)MsgRsp::LOGINRSP);//协议
	ba->writeFUint16(status);//返回状态	
	return sock->sendFrom(ba,ba->getSize());
}

//返回房间号
int AddSession::parser(ByteArray::ptr ba,std::function<int(
						const std::string &)> cb){
	uint16_t bodysize = 0 ; 
	ba->readFUint16(bodysize);//读body大小
	assert(bodysize < 32);
	int key = 0 ; 
	ba->readInt32(key);//读密钥

	char buf[bodysize]; 
	ba->read(buf,bodysize);//读body

	disencrypt(buf,bodysize,key);//解密

	uint8_t namelen = buf[0];
	std::string name(&buf[1],namelen);

	return cb(name);
}

int AddSessionRsp::send(Socket::ptr sock, int roomId){
	ByteArray::ptr ba(new ByteArray(16));
	ba->writeFInt16((uint16_t)MsgRsp::ADDSESSRSP);//协议
	ba->writeFInt32(roomId);//返回状态	
	return sock->sendFrom(ba,ba->getSize());
}

int CardDistributor::send(Socket::ptr sock 
						,const uint8_t* cards,uint8_t priority){
	ByteArray::ptr ba(new ByteArray(32));
	ba->writeFInt16((int16_t)MsgRsp::CARDDISC);
	ba->writeFUint16(18);
	
	char buf[20]; 
	buf[0] = priority; 
	memcpy(buf+1,cards,17);

	int key = encrypt(buf,18);

	ba->writeInt32(key);
	ba->write(buf,18);

	sock->sendFrom(ba,ba->getSize());
	return 0 ; 
}

int TickleAddUser::send(Socket::ptr sock ,const std::string & name){
	ByteArray::ptr ba(new ByteArray(64));
	ba->writeFInt16((int16_t)MsgRsp::ADDTICKLE);
	ba->writeFUint16(name.size());
	std::string str = name ; 
	int key = encrypt(&str[0],name.size());
	ba->writeInt32(key);

	ba->write(&str[0],name.size());
	sock->sendFrom(ba,ba->getSize());
	return 0 ; 
}

int CallLandlord::send(Socket::ptr sock ,uint8_t priority,uint8_t callOrRob){
	ByteArray::ptr ba(new ByteArray(16));
	ba->writeFInt16((uint16_t)MsgRsp::CALLLAND);
	//几号玩家抢
	ba->writeFUint8(priority);
	//叫地主还是抢地主1为叫，2为抢
	ba->writeFUint8(callOrRob);
	return sock->sendFrom(ba,4);
}

int IsCallLand::parser(ByteArray::ptr  ba , uint8_t & iscall){
	uint16_t bodysize = 0 ; 
	ba->readFUint16(bodysize);//读body大小
	assert(bodysize == 1);
	int key = 0 ; 
	ba->readInt32(key);//读密钥

	ba->read((char*)&iscall,bodysize);//读body

	disencrypt((char*)&iscall,bodysize,key);//解密

	return 0;
}

int TickleIsCall::send(Socket::ptr sock, uint8_t priority ,uint8_t callOrRob){
	ByteArray::ptr ba(new ByteArray(16));
	ba->writeFInt16((uint16_t)MsgRsp::TICKLEISCALL);
	//几号玩家抢
	ba->writeFUint8(priority);
	//叫地主还是抢地主1为叫，2为抢
	ba->writeFUint8(callOrRob);
	return sock->sendFrom(ba,4);
}

Responder::Responder(Socket::ptr sock , ByteArray::ptr ba)
	:m_sock(sock)
	,m_ba(ba){
}

//test 
void fun(const std::string & acc,const std::string & pwd){
	TADPOLE_LOG_INFO(g_logger)<< "账号: "<<acc;
	TADPOLE_LOG_INFO(g_logger)<< "密码: "<<pwd;
}

//ret 0 不关闭连接，1 关闭连接，2 不关闭连接但接管socket
int Responder::responce(int type){
	if(type == 1){
		Login::parser(m_ba,fun);
		LoginRsp::send(m_sock,200);
		return 0; 
	}else if(type == 2){
		//解析加入会议协议，
		//并将该用户加入到会议
		std::function<int(const std::string &)> cb = std::bind(&PlayRoom::accessToRoom,RoomMgr::GetInstance(),m_sock,std::placeholders::_1);
		int roomId = AddSession::parser(m_ba
		,std::bind(&PlayRoom::accessToRoom,RoomMgr::GetInstance(),m_sock,std::placeholders::_1));
		//加入会议返回会议号
		AddSessionRsp::send(m_sock,roomId);
		return 0; 
	}
	return -1 ; 
}

}
