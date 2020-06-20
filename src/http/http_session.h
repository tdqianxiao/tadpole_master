#ifndef __TADPOLE_HTTP_SESSION_H__
#define __TADPOLE_HTTP_SESSION_H__

#include <memory>
#include "src/socket.h"
#include "http_request.h"
#include "http_responce.h"

namespace tadpole{

/**
 * @brief http会议，作为服务端
 */
class HttpSession{
public:
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<HttpSession> ptr;

	/**
	 * @brief 构造函数
	 */
	HttpSession(Socket::ptr sock);

	/**
	 * @brief 析构函数
	 */
	~HttpSession();

	/**
	 * @brief 接收请求
	 */
	void recvRequest();

	/**
	 * @brief 应答
	 */
	void sendResponce();
	
	/**
	 * @brief 是否保持连接
	 */
	bool isKeepalive(){return !m_responce->isClose();}

private:
	//socket
	Socket::ptr m_sock; 
	//应答
	HttpResponce::ptr m_responce;
	//长连接超时
	bool m_isTimeout = false; 
};

}


#endif 
