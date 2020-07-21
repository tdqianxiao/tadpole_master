#include "ws_session.h"
#include "src/bytearray.h"
#include "src/config.h"
#include "src/log.h"
#include "Room.h"

namespace tadpole{

WSSession::WSSession(Socket::ptr sock)
	:m_sock(sock){
	m_ba.reset(new ByteArray);
	m_rep.reset(new Responder(m_sock,m_ba));
}

WSSession::~WSSession(){
}

void WSSession::recvRequest(){
	//设置连接没有响应的时间
	m_sock->setRecvTimeout(5*60*1000);
	std::vector<iovec> vec;
	m_ba->getWriteBuffers(vec,1024);
	int ret = m_sock->recv(vec);
	
	TADPOLE_LOG_INFO(TADPOLE_FIND_LOGGER("root"))<< "iswait : "<< m_iswait<<" ret :"<< ret;
	if(m_iswait == 1){
		if(ret == 0 || ret == -1){
			m_closeSock = 1;
			RoomMgr::GetInstance()->outFromRoom(m_sock);
		}else{
			m_closeSock = 0 ; 
		}
		m_close = 2;
		return ; 
	}
	if(ret == -2){
		TADPOLE_LOG_INFO(TADPOLE_FIND_LOGGER("root"))<< "client is non responce !";
		RoomMgr::GetInstance()->outFromRoom(m_sock);
		m_close = 1;
		return ; 
	}else if(ret == -3){
		m_close = 1;
		m_closeSock = 0;
		return ;
	}else if(ret <= 0){
		RoomMgr::GetInstance()->outFromRoom(m_sock);
		m_close = 1;
		return ; 
	}
	
	m_ba->setUsedCount(ret);
	
	vec.clear();
	int16_t type = 0; 
	m_ba->readFInt16(type);
	TADPOLE_LOG_INFO(TADPOLE_FIND_LOGGER("root"))<<"type:"<< type;
	//返回值为2 表示已经开始游戏，保持连接，但这个函数不在继续调用
	m_close = m_rep->responce(type);
	if(m_close == 2){
		m_close = 0;
		m_iswait = 1;
	}
}

}
