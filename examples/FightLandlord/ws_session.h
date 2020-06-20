#ifndef __TADPOLE_WS_SESSION_H__
#define __TADPOLE_WS_SESSION_H__

#include <memory>
#include "src/socket.h"
#include "src/bytearray.h"
#include "protocal.h"

namespace tadpole{

/**
 * @brief 会议，作为服务端
 */
class WSSession{
public:
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<WSSession> ptr;

	/**
	 * @brief 构造函数
	 */
	WSSession(Socket::ptr sock);

	/**
	 * @brief 析构函数
	 */
	~WSSession();

	/**
	 * @brief 接收请求
	 */
	void recvRequest();

	/**
	 * @brief 是否保持连接
	 */
	int isClose(){return m_close;}

private:
	//socket
	Socket::ptr m_sock; 
	//
	ByteArray::ptr m_ba;
	//Responder
	Responder::ptr m_rep; 
	//是否保持连接,0保持连接
	int m_close = 1;	
};

}


#endif 
