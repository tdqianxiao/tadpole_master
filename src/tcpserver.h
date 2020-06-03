#ifndef __TADPOLE_TCP_SERVER_H__
#define __TADPOLE_TCP_SERVER_H__

#include <memory>
#include "socket.h"
#include "iomanager.h"
#include "bytearray.h"
#include "address.h"

namespace tadpole{

/**
 * @brief tcp服务器类
 */
class TcpServer:public std::enable_shared_from_this<TcpServer>{
public:
	/**
	 * @brief 类型定义
	 */
	typedef std::shared_ptr<TcpServer> ptr;

	/**
	 * @brief 构造函数
	 * @param[in] iom 准备在哪个iomanager上执行
	 */
	TcpServer(IOManager * iom = IOManager::GetCurIOM());

	/**
	 * @brief 析构函数，释放文件描述符
	 */
	virtual ~TcpServer();

	/**
	 * @brief 绑定端口，开始监听，运行
	 * @param[in] addrs 地址,可以绑定多个
	 */
	void bind(const std::vector<Address::ptr>& addrs);
	
	/**
	 * @brief 开始运行tcp服务器
	 */
	void start();

	/**
	 * @brief 开始接受新的连接
	 */
	void startAccept(Socket::ptr sfd);

	/**
	 * @brief 新的客户端到来要做的事情，供子类去重写
	 */
	virtual void newClient(Socket::ptr){}

private:
	//服务器是否已经开始
	bool m_isStart = false;
	//需绑定的文件描述符
	std::vector<Socket::ptr> m_socks; 
	//io线程池
	IOManager * m_iom = nullptr; 
};

}

#endif 
