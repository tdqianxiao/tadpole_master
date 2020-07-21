#include "protocal.h"
#include "src/log.h"
#include "src/iomanager.h"
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
std::pair<int,uint8_t> AddSession::parser(ByteArray::ptr ba,std::function<std::pair<int,uint8_t>(
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

int AddSessionRsp::send(Socket::ptr sock, int roomId,uint8_t pri){
	ByteArray::ptr ba(new ByteArray(16));
	ba->writeFInt16((uint16_t)MsgRsp::ADDSESSRSP);//协议
	ba->writeFUint16(5);//bodysize	

		
	char buf[8]; 
	memcpy(buf,(const char*)&roomId,4);
	memcpy(buf + 4,(const char*)&pri,1);
	int key = encrypt(buf,5);

	ba->writeInt32(key);
	ba->write(buf,5);

	return sock->sendFrom(ba,ba->getSize());
}

int DisplayPearName::send(Socket::ptr sock,
						std::unordered_map<uint8_t,std::string> names ){
	if(names.empty()){
		return 0; 
	}
	ByteArray::ptr ba(new ByteArray(128));
	ba->writeFInt16((uint16_t)MsgRsp::DISPEARNAME);//协议
	
	std::shared_ptr<char> buf(new char[128],[](char * ptr){delete [] ptr;});
	char * tmp = buf.get();
	
	uint8_t count = names.size() ; 
	memcpy(tmp,(const char *)&count,1);

	int pos = 1 ;//初始位置
	for(auto & it : names){
		memcpy(tmp + pos , (const char *)&it.first,1);
		++pos;
		int size = it.second.size();
		memcpy(tmp + pos , (const char *)&size,4);
		pos += 4; 
		memcpy(tmp + pos, it.second.c_str(),size);
		pos += size; 
	}
	
	ba->writeFUint16(pos);

	int key = encrypt(tmp,pos);
	ba->writeInt32(key);
	ba->write(tmp,pos);
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

int TickleAddUser::send(Socket::ptr sock ,const std::string & name,uint8_t pri){
	ByteArray::ptr ba(new ByteArray(64));
	ba->writeFInt16((int16_t)MsgRsp::ADDTICKLE);
	ba->writeFUint16(name.size()+1);

	char buf[20] = {0};
	memcpy(buf,name.c_str(),name.size());
	memcpy(buf+name.size(),(const char *)&pri,1);

	int key = encrypt(buf,name.size() + 1);
	ba->writeInt32(key);

	ba->write(buf,name.size()+1);
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

int LandlordCard::send(Socket::ptr sock ,const uint8_t * cards,uint8_t who){
	ByteArray::ptr ba(new ByteArray(32));
	ba->writeFInt16((int16_t)MsgRsp::LANDCARD);
	ba->writeFUint16(4);
	
	char buf[4] = {0} ; 
	buf[0] = who; 
	memcpy(buf+1,cards,3);

	int key = encrypt(buf,4);

	ba->writeInt32(key);
	ba->write(buf,4);

	return sock->sendFrom(ba,ba->getSize());
}

int Asking::send(Socket::ptr sock,uint8_t who){
	ByteArray::ptr ba(new ByteArray(4));
	ba->writeFInt16((uint16_t)MsgRsp::ASKING);
	//std::cout<< (int)who << std::endl;
	ba->writeFUint8(who);
	return sock->sendFrom(ba,ba->getSize());
}

int PutCard::parser(ByteArray::ptr ba ,std::function<void (
							 std::shared_ptr<char>, uint8_t,double,int)> cb ){
	uint16_t bodysize = 0 ; 
	ba->readFUint16(bodysize);
	if(bodysize == 0){
		cb(nullptr,0,0,0);
		return 0; 
	}
	assert(bodysize <= 32);

	int key = 0 ; 
	ba->readInt32(key);
	std::shared_ptr<char> buffs(new char[36],[](char * ptr){
		delete[] ptr; 
	});
	char * buf = buffs.get();
	ba->read(buf,bodysize);
	
	disencrypt((char*)buf,bodysize,key);//解密
	
	int count = bodysize -sizeof(double) - sizeof(int);
	double cardType = *(double*)(buf + count);
	int cardnum = *(int*)(buf+count+sizeof(double));

	cb(buffs,count,cardType,cardnum);
	return 0 ; 
}

int ShowCard::send(Socket::ptr sock ,std::shared_ptr<char> buf,
									uint8_t count,uint8_t who,double type,int num){
	char card[36] = {0};
	memcpy(card,buf.get(),count);
	memcpy(card + count,(char*)&who,1);
	memcpy(card + count + 1,(char*)&type,8);
	memcpy(card + count + 9,(char*)&num,4);
	
	count += 13; 

	ByteArray::ptr ba(new ByteArray(36));
	ba->writeFInt16((uint16_t)MsgRsp::SHOWCARD);
	ba->writeFUint16(count);

	int key = encrypt((char*)card,count);
	ba->writeInt32(key);
	
	ba->write(card,count);
	return sock->sendFrom(ba,ba->getSize());
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
		std::pair<int,uint8_t> ret = AddSession::parser(m_ba
		,std::bind(&PlayRoom::accessToRoom,RoomMgr::GetInstance(),m_sock,std::placeholders::_1));
		//加入会议返回会议号
		//std::cout<< (int)ret.second<<std::endl;
		AddSessionRsp::send(m_sock,ret.first,ret.second);

		return 2; 
	}
	return -1 ; 
}

}
